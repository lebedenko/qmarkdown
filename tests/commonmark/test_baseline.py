"""Authored expectations and faults protecting the independent conformance runner."""
import argparse
import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

from baseline import (DATA, HTML_OPTIONS, PARSE_OPTIONS, analyze, compare_ledger,
                      exception, json_text, load_fixtures, run_probe, strict_pass,
                      validate_fixtures, validate_model, validate_report, validate_response,
                      validate_authored, load_authored, HTML_BLOCK_IDS, REMAINING_IDS)
from oracle import Uncheckable, canonical_actual, expected, mask_image_ranges, utf16, normalize_inline

PROBE = None


def paragraph(text, ranges=None, links=None, images=None):
    return {"kind": "Paragraph", "text": text, "ranges": ranges or [], "links": links or [], "images": images or []}


def presentation(blocks):
    result = []
    for block in blocks:
        value = {k: v for k, v in block.items() if k != "inlines"}
        if "children" in value:
            value["children"] = presentation(value["children"])
        result.append(value)
    return result


def text_node(text):
    return {"kind": "Text", "literal": text}


def complete_plain(blocks):
    """Synthetic validation cases have plain text semantics, not parser output."""
    result = copy.deepcopy(blocks)
    for block in result:
        if block["kind"] in ("Paragraph", "Heading"):
            block["inlines"] = [text_node(block["text"])] if block["text"] else []
        if "children" in block:
            block["children"] = complete_plain(block["children"])
    return result


def link(start, length, destination):
    return {"start": start, "length": length, "destination": destination}


def image(start, length, destination, title="", enclosing="", linked=False):
    return {"start": start, "length": length, "destination": destination, "title": title,
            "enclosingLink": enclosing, "linked": linked}


def span(start, length, flags):
    return {"start": start, "length": length, "flags": flags}


def fixture(markdown="hello\n", html="<p>hello</p>\n"):
    return {"example": 1, "markdown": markdown, "html": html, "section": "Authored",
            "start_line": 1, "end_line": 2}


class OracleTest(unittest.TestCase):
    def project(self, html):
        return presentation(expected(html)[0])

    def test_nested_formatting_has_utf16_offsets(self):
        self.assertEqual(self.project("<p>😀 <strong>é <em>b</em> <code>c</code></strong></p>\n"),
            [paragraph("😀 é b c", [span(3, 2, 2), span(5, 1, 3), span(6, 1, 2), span(7, 1, 6)])])
        self.assertEqual(utf16("😀é"), 3)

    def test_soft_and_hard_breaks_have_different_text(self):
        blocks, limits, losses = expected("<p>soft\nnext<br />\nhard <em>x<br />\ny</em></p>\n")
        self.assertEqual(presentation(blocks), [paragraph("soft next\nhard x\ny", [span(15, 3, 1)])])
        self.assertEqual(losses, [])
        self.assertIn("softbreak-or-decoded-lf-source-unavailable", limits)

    def test_entity_newlines_are_not_rewritten_as_softbreaks(self):
        self.assertEqual(self.project("<p>a&#10;&#10;b &amp; &lt; &#x1f600;</p>\n"), [paragraph("a\n\nb & < 😀")])
        with self.assertRaisesRegex(Uncheckable, "literal-newline-or-softbreak"):
            expected("<p>a\n\nb</p>\n")

    def test_code_whitespace_and_entities_remain_exact(self):
        self.assertEqual(self.project('<pre><code class="language-c">&lt;x&gt;\n\t😀  \n</code></pre>\n'),
            [{"kind": "CodeBlock", "text": "<x>\n\t😀  \n", "language": "c"}])
        self.assertEqual(self.project("<p><code>a\nb</code></p>\n"), [paragraph("a\nb", [span(0, 3, 4)])])

    def test_recursive_tight_list_and_quote(self):
        self.assertEqual(self.project("<ol start=\"3\">\n<li>one\n<ul>\n<li><em>x</em></li>\n</ul>\n</li>\n<li>\n<blockquote>\n<p>q</p>\n</blockquote>\n</li>\n</ol>\n"),
            [{"kind": "List", "ordered": True, "start": 3, "tight": True, "children": [
                {"kind": "ListItem", "children": [paragraph("one"),
                    {"kind": "List", "ordered": False, "tight": True, "children": [
                        {"kind": "ListItem", "children": [paragraph("x", [span(0, 1, 1)])]}]}]},
                {"kind": "ListItem", "children": [{"kind": "Quote", "children": [paragraph("q")]}]}]}])

    def test_loose_list_empty_item_and_heading_separator(self):
        self.assertEqual(self.project("<ul>\n<li>\n<p>one</p>\n</li>\n<li></li>\n</ul>\n"),
            [{"kind": "List", "ordered": False, "tight": False, "children": [
                {"kind": "ListItem", "children": [paragraph("one")]}, {"kind": "ListItem", "children": []}]}])
        self.assertEqual(self.project("<ul>\n<li>\n<h2>title</h2>\nafter</li>\n</ul>\n")[0]["children"][0]["children"],
            [{**paragraph("title"), "kind": "Heading", "level": 2}, paragraph("after")])

    def test_image_metadata_and_enclosing_empty_destination(self):
        blocks, limits, losses = expected('<p><a href=""><img src="a&amp;b" alt="😀é" title="x &quot;y&quot;" /><img src="b" alt="" /></a></p>\n')
        self.assertEqual(presentation(blocks), [paragraph("😀é", links=[link(0, 3, "")],
            images=[image(0, 3, "a&b", 'x "y"', linked=True), image(3, 0, "b", linked=True)])])
        self.assertIn("image-description-formatting-unavailable", limits)

    def test_masking_images_preserves_adjacent_and_surrounding_ranges(self):
        value = paragraph("abcde", [span(0, 5, 2)], images=[image(1, 2, "a"), image(3, 0, "b")])
        self.assertEqual(mask_image_ranges(value)["ranges"], [span(0, 1, 2), span(3, 2, 2)])

    def test_link_title_empty_link_and_repeated_emphasis_preserve_presentation(self):
        blocks, limits, losses = expected('<p><a href="x" title="t"></a><em><em>x</em></em></p>\n')
        self.assertEqual(presentation(blocks), [paragraph("x", [span(0, 1, 1)])])
        self.assertEqual(losses, [])

    def test_balanced_unknown_inline_html_is_literal(self):
        blocks, limits, losses = expected('<p>before <B title="&amp;">x</B > <!-- y --> after</p>\n')
        self.assertEqual(presentation(blocks), [paragraph('before <B title="&amp;">x</B > <!-- y --> after')])
        self.assertEqual(losses, [])

    def test_raw_or_malformed_html_is_never_silently_dropped(self):
        for html in ('<div>\nx\n</div>\n', '<div incomplete\n', '<p>x<a></p>\n', '<p>x</p>\n<unfinished',
                     '<p class="raw">x</p>\n', '<p><a href="x" href="y">z</a></p>\n', '<![CDATA[x]]>'):
            with self.subTest(html=html), self.assertRaises(Uncheckable):
                expected(html)

    def test_short_comments_are_reported_consistently_across_python_versions(self):
        for html in ('<p>foo <!--> foo --&gt;</p>\n', '<p>foo <!---> foo --&gt;</p>\n'):
            with self.subTest(html=html), self.assertRaisesRegex(Uncheckable, 'short-html-comment-tokenization-unavailable'):
                expected(html)

    def test_urls_compare_encoded_form_without_changing_probe_destinations(self):
        actual = [{**paragraph("x", links=[link(0, 1, "é a%20b")]), "inlines": [
            {"kind": "Link", "destination": "é a%20b", "title": "", "children": [text_node("x")]}]}]
        self.assertEqual(presentation(canonical_actual(actual)), self.project('<p><a href="%C3%A9%20a%20b">x</a></p>\n'))
        self.assertEqual(actual[0]["links"][0]["destination"], "é a%20b")


class SemanticTest(unittest.TestCase):
    def nodes(self, markdown):
        response = run_probe(PROBE, [fixture(markdown)])
        return normalize_inline(response["examples"][0]["model"][0]["inlines"])

    def test_breaks_entity_newlines_and_code_remain_distinct(self):
        self.assertEqual(self.nodes("😀&#10;é\nnext  \nhard `x`"), [
            text_node("😀\né"), {"kind": "SoftBreak"}, text_node("next"),
            {"kind": "HardBreak"}, text_node("hard "), {"kind": "Code", "literal": "x"}])
        html = "<p>😀&#10;é\nnext<br />\nhard <code>x</code></p>\n"
        self.assertEqual(expected(html)[0][0]["inlines"], self.nodes("😀&#10;é\nnext  \nhard `x`"))

    def test_reference_titles_empty_links_and_nested_emphasis(self):
        want = [{"kind": "Link", "destination": "a*b?x=1&y=2", "title": "t &", "children": [
            {"kind": "Emphasis", "children": [{"kind": "Emphasis", "children": [text_node("é")]}]}]},
            {"kind": "Link", "destination": "empty", "title": "caption", "children": []}]
        self.assertEqual(self.nodes('[*_é_*][id][](empty "caption")\n\n[id]: a\\*b?x=1&amp;y=2 "t &amp;"'), want)

    def test_literal_html_keeps_identity_and_exact_spelling(self):
        want = [text_node("a "), {"kind": "Html", "literal": "<B title='&amp;'>"},
                text_node("x"), {"kind": "Html", "literal": "</B>"},
                {"kind": "Html", "literal": "<!-- y -->"}]
        self.assertEqual(self.nodes("a <B title='&amp;'>x</B><!-- y -->"), want)
        self.assertEqual(expected("<p>a <B title='&amp;'>x</B><!-- y --></p>\n")[0][0]["inlines"], want)

    def test_image_description_tree_is_retained_but_html_check_is_bounded(self):
        markdown = '![**é** [inner](ignored "t") ![*猫*](nested "n")](img "pic")'
        want = [{"kind": "Image", "destination": "img", "title": "pic", "children": [
            {"kind": "Strong", "children": [text_node("é")]}, text_node(" "),
            {"kind": "Link", "destination": "ignored", "title": "t", "children": [text_node("inner")]},
            text_node(" "), {"kind": "Image", "destination": "nested", "title": "n", "children": [
                {"kind": "Emphasis", "children": [text_node("猫")]}]}]}]
        self.assertEqual(self.nodes(markdown), want)
        response = run_probe(PROBE, [fixture(markdown, '<p><img src="img" alt="é inner 猫" title="pic" /></p>\n')])
        actual = response["examples"][0]["model"][0]
        self.assertEqual(actual["links"], [])
        self.assertEqual(len(actual["images"]), 1)
        expected_blocks, limits, _ = expected('<p><img src="img" alt="é inner 猫" title="pic" /></p>\n')
        self.assertEqual(canonical_actual([actual]), expected_blocks)
        self.assertIn("image-description-formatting-unavailable", limits)

    def test_normalization_only_merges_adjacent_text(self):
        self.assertEqual(normalize_inline([text_node("")]), [text_node("")])
        nodes = [text_node("a"), text_node("b"), {"kind": "SoftBreak"}, text_node("c"),
                 {"kind": "Emphasis", "children": [text_node("d"), text_node("e")]},
                 {"kind": "Emphasis", "children": [text_node("f")]}]
        self.assertEqual(normalize_inline(nodes), [text_node("ab"), {"kind": "SoftBreak"}, text_node("c"),
            {"kind": "Emphasis", "children": [text_node("de")]},
            {"kind": "Emphasis", "children": [text_node("f")]}])

    def test_semantic_faults_mismatch_with_presentation_unchanged(self):
        manifest, _ = load_fixtures()
        cases = [
            ("a\nb", "<p>a\nb</p>\n", "break"),
            ('[x](url "title")', '<p><a href="url" title="title">x</a></p>\n', "title"),
            ('[](url)', '<p><a href="url"></a></p>\n', "empty"),
            ('*_*x*_*', '<p><em><em><em>x</em></em></em></p>\n', "nesting"),
            ('a <b>x</b>', '<p>a <b>x</b></p>\n', "html")]
        for markdown, html, fault in cases:
            fixtures = [fixture(markdown, html)]
            response = run_probe(PROBE, fixtures)
            good = analyze(manifest, fixtures, response)
            self.assertEqual(good["totals"]["model"]["mismatch"], 0)
            original_presentation = presentation(response["examples"][0]["model"])
            nodes = response["examples"][0]["model"][0]["inlines"]
            if fault == "break": nodes[1]["kind"] = "HardBreak"
            if fault == "title": nodes[0]["title"] = ""
            if fault == "empty": nodes.clear()
            if fault == "nesting": nodes[:] = nodes[0]["children"]
            if fault == "html":
                for node in nodes:
                    if node["kind"] == "Html": node["kind"] = "Text"
            with self.subTest(fault=fault):
                self.assertEqual(presentation(response["examples"][0]["model"]), original_presentation)
                bad = analyze(manifest, fixtures, response)
                self.assertEqual(bad["totals"]["model"]["mismatch"], 1)
                ledger = {"schema": 1, "fixtureSha256": manifest["sha256"], "review": "reviewed", "examples": {}}
                if exception(good["examples"][0]):
                    ledger["examples"]["1"] = {**exception(good["examples"][0]), "review": "reviewed"}
                self.assertEqual(len(compare_ledger(bad, ledger)), 1)

    def test_inline_validation_rejects_missing_unknown_and_wrong_fields(self):
        for node in ({"kind": "Other"}, {"kind": "SoftBreak", "literal": " "},
                     {"kind": "Link", "destination": "x", "children": []},
                     {"kind": "Emphasis", "children": "x"}, {"kind": "Text", "literal": 1}):
            block = {**paragraph("x"), "inlines": [node]}
            with self.subTest(node=node), self.assertRaises(ValueError):
                validate_model([block])

    def test_stale_report_and_authored_formats_are_rejected(self):
        manifest, fixtures = load_fixtures()
        response = run_probe(PROBE, fixtures[:1])
        report = analyze(manifest, fixtures[:1], response)
        report["schema"] = 2
        with self.assertRaisesRegex(ValueError, "provenance"):
            validate_report(report, manifest, fixtures[:1])
        for filename, ids, annotations in (("html-block-expectations.json", HTML_BLOCK_IDS, False),
                                          ("remaining-expectations.json", REMAINING_IDS, True)):
            authored = json.loads((DATA / filename).read_text())
            authored["schema"] = 1
            with self.assertRaisesRegex(ValueError, "provenance"):
                validate_authored(authored, manifest, ids, annotations)


class ProductionModelTest(unittest.TestCase):
    def check(self, markdown, want):
        response = run_probe(PROBE, [fixture(markdown)])
        self.assertEqual(presentation(response["examples"][0]["model"]), want)

    def test_unicode_images_links_and_decoded_titles(self):
        self.check('😀 **é** [日本](a?x=1&amp;y=2 "title") ![*猫*](img "pic") ![](empty)\n',
            [paragraph("😀 é 日本 猫 ", [span(3, 1, 2), span(8, 1, 1)], [link(5, 2, "a?x=1&y=2")],
                       [image(8, 1, "img", "pic"), image(10, 0, "empty")])])

    def test_adjacent_images_retain_zero_length_and_enclosing_link(self):
        self.check('[![**😀**](a)![](b)](outer)', [paragraph("😀", [span(0, 2, 2)], [link(0, 2, "outer")],
            [image(0, 2, "a", enclosing="outer", linked=True), image(2, 0, "b", enclosing="outer", linked=True)])])
        self.check('[![](i)]()', [paragraph("", images=[image(0, 0, "i", linked=True)])])

    def test_empty_link_is_omitted_but_its_adjacent_text_survives(self):
        self.check('[](/x) next', [paragraph(" next")])

    def test_ordered_delimiter_and_loose_list_are_source_assertions(self):
        self.check('3) one\n4) two\n', [{"kind": "List", "ordered": True, "start": 3, "delimiter": ")", "tight": True,
            "children": [{"kind": "ListItem", "children": [paragraph("one")]}, {"kind": "ListItem", "children": [paragraph("two")]}]}])
        self.check('- one\n\n- two\n', [{"kind": "List", "ordered": False, "start": 0, "delimiter": ".", "tight": False,
            "children": [{"kind": "ListItem", "children": [paragraph("one")]}, {"kind": "ListItem", "children": [paragraph("two")]}]}])

    def test_full_fence_info_and_literal_html(self):
        self.check('``` lang rest &amp; stuff\n😀\t x\n```\n',
            [{"kind": "CodeBlock", "text": "😀\t x\n", "infoString": "lang rest & stuff"}])
        self.check('<div>\n*x*\n</div>\n', [{"kind": "HtmlBlock", "text": "<div>\n*x*\n</div>\n"}])
        self.check('before <b>x</b> after', [paragraph('before <b>x</b> after')])

    def test_original_unicode_literal_spaces_and_escapes_in_destinations(self):
        self.check('[α](<a b>) [β](a%20b) [γ](é)', [paragraph('α β γ', links=[
            link(0, 1, 'a b'), link(2, 1, 'a%20b'), link(4, 1, 'é')])])

    def test_entity_lf_and_source_line_endings_follow_production_parse(self):
        self.check('foo&#10;&#10;bar\n', [paragraph('foo\n\nbar')])
        self.check('a\r\nb\rc\0', [paragraph('a b c�')])

    def test_probe_rejects_duplicate_or_unknown_request_fields(self):
        for request in ({"examples": [{"example": 1, "markdown": "x"}] * 2},
                        {"examples": [], "unknown": True},
                        {"examples": [{"example": 1, "markdown": "x", "unknown": True}]},
                        {"examples": [{"example": 1.5, "markdown": "x"}]}):
            result = subprocess.run([str(Path(PROBE).resolve())], input=json_text(request), capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 2)
            self.assertFalse(result.stdout)


class AuthoredHtmlTest(unittest.TestCase):
    def setUp(self):
        self.manifest, all_fixtures = load_fixtures()
        self.fixtures = [e for e in all_fixtures if e["example"] in HTML_BLOCK_IDS]
        self.authored = load_authored(self.manifest)
        self.response = run_probe(PROBE, self.fixtures)

    def test_complete_raw_models_and_comparison_method(self):
        report = analyze(self.manifest, self.fixtures, self.response)
        self.assertEqual(report["schema"], 3)
        self.assertEqual(report["totals"]["model"], {"projection-pass": 43, "mismatch": 0, "uncheckable": 0})
        for entry in report["examples"]:
            self.assertEqual(entry["model"]["comparison"], "source-authored")
            self.assertEqual(entry["nativeModel"], self.authored[entry["example"]])

    def test_missing_ids_checksum_review_and_metadata_are_rejected(self):
        original = json.loads((DATA / "html-block-expectations.json").read_text())
        for fault in ("missing", "duplicate", "checksum", "review", "metadata"):
            value = copy.deepcopy(original)
            if fault == "missing": value["examples"].pop()
            if fault == "duplicate": value["examples"][-1] = value["examples"][0]
            if fault == "checksum": value["fixtureSha256"] = "bad"
            if fault == "review": value["examples"][0]["review"] = ""
            if fault == "metadata": value["examples"][0]["model"][1]["ranges"][0]["flags"] = 8
            with self.subTest(fault=fault), self.assertRaises(ValueError):
                validate_authored(value, self.manifest)
        missing = dict(self.authored)
        missing.pop(148)
        with self.assertRaisesRegex(ValueError, "Missing authored"):
            analyze(self.manifest, self.fixtures, self.response, missing)

    def test_literal_boundaries_nesting_and_valid_metadata_faults_mismatch(self):
        for id_, fault in ((149, "literal"), (190, "boundary"), (174, "nesting"), (175, "metadata")):
            response = copy.deepcopy(self.response)
            entry = next(e for e in response["examples"] if e["example"] == id_)
            if fault == "literal": entry["model"][0]["text"] += "x"
            if fault == "boundary":
                entry["model"][0]["text"] += entry["model"].pop(1)["text"]
            if fault == "nesting": entry["model"] = entry["model"][0]["children"] + entry["model"][1:]
            if fault == "metadata": entry["model"][0]["tight"] = False
            report = analyze(self.manifest, self.fixtures, response)
            self.assertEqual(report["totals"]["model"]["mismatch"], 1)
            ledger = {"schema": 1, "fixtureSha256": self.manifest["sha256"], "review": "clean", "examples": {}}
            self.assertEqual(len(compare_ledger(report, ledger)), 1)

    def test_stale_ledger_and_wrong_comparison_method_fail(self):
        report = analyze(self.manifest, self.fixtures, self.response)
        ledger = {"schema": 1, "fixtureSha256": self.manifest["sha256"], "review": "stale", "examples": {
            "148": {"parser": "pass", "model": "uncheckable", "limits": ["html-oracle-unsupported"], "losses": [],
                    "reason": "old", "modelSha256": "0" * 64, "review": "old HTML oracle"}}}
        self.assertEqual(len(compare_ledger(report, ledger)), 1)
        report["examples"][0]["model"]["comparison"] = "html-projection"
        with self.assertRaisesRegex(ValueError, "comparison method"):
            validate_report(report, self.manifest, self.fixtures)


class RemainingAuthoredTest(unittest.TestCase):
    def setUp(self):
        self.manifest, fixtures = load_fixtures()
        self.fixtures = [e for e in fixtures if e['example'] in REMAINING_IDS]
        self.response = run_probe(PROBE, self.fixtures)
        self.original = json.loads((DATA / 'remaining-expectations.json').read_text())

    def test_all_raw_models_and_semantic_annotations(self):
        report = analyze(self.manifest, self.fixtures, self.response)
        self.assertEqual(report['totals']['model'], {'projection-pass': 26, 'mismatch': 0, 'uncheckable': 0})
        self.assertEqual(report['totals']['examplesWithLimits'], 0)
        self.assertEqual(report['totals']['examplesWithLosses'], 0)
        for entry, authored in zip(report['examples'], self.original['examples']):
            with self.subTest(example=entry['example']):
                self.assertEqual(entry['nativeModel'], authored['model'])
                self.assertEqual(entry['model']['comparison'], 'source-authored')
                for category in ('limits', 'losses'):
                    self.assertEqual(entry['model'][category], authored[category])
        self.assertTrue(strict_pass(report))
        self.assertEqual(json_text(report), json_text(analyze(self.manifest, self.fixtures, self.response)))

    def test_fixture_provenance_exact_ids_reviews_and_metadata(self):
        for fault in ('missing', 'duplicate', 'unexpected', 'checksum', 'schema', 'review',
                      'entry-review', 'metadata', 'missing-losses', 'loss-type', 'duplicate-loss',
                      'unsorted-losses', 'invalid-limits'):
            value = copy.deepcopy(self.original)
            first = value['examples'][0]
            if fault == 'missing': value['examples'].pop()
            if fault == 'duplicate': value['examples'][-1] = first
            if fault == 'unexpected': first['example'] = 22
            if fault == 'checksum': value['fixtureSha256'] = 'bad'
            if fault == 'schema': value['schema'] = True
            if fault == 'review': value['review'] = ' '
            if fault == 'entry-review': first['review'] = ''
            if fault == 'metadata': first['model'][0]['unknown'] = True
            if fault == 'missing-losses': del first['losses']
            if fault == 'loss-type': first['losses'] = [False]
            if fault == 'duplicate-loss': first['losses'] = ['x', 'x']
            if fault == 'unsorted-losses': first['losses'] = ['z', 'a']
            if fault == 'invalid-limits': first['limits'] = [' ']
            with self.subTest(fault=fault), self.assertRaises(ValueError):
                validate_authored(value, self.manifest, REMAINING_IDS, annotations=True)

    def test_text_newlines_boundaries_nesting_and_metadata_faults(self):
        for id_, fault in ((630, 'entity'), (39, 'newlines'), (642, 'tag-newline'),
                           (201, 'boundary'), (308, 'nesting'), (309, 'metadata')):
            response = copy.deepcopy(self.response)
            entry = next(e for e in response['examples'] if e['example'] == id_)
            blocks = entry['model']
            if fault == 'entity': blocks[0]['text'] = blocks[0]['text'].replace('&ouml;', 'ö')
            if fault == 'newlines': blocks[0]['text'] = 'foo  bar'
            if fault == 'tag-newline': blocks[0]['text'] = blocks[0]['text'].replace('\n', ' ')
            if fault == 'boundary': blocks[0]['text'] += blocks.pop(1)['text']
            if fault == 'nesting': blocks[0]['children'][0]['children'] = [{'kind': 'Quote', 'children': blocks[0]['children'][0]['children']}]
            if fault == 'metadata': blocks[0]['tight'] = True
            report = analyze(self.manifest, self.fixtures, response)
            with self.subTest(fault=fault):
                self.assertEqual(report['totals']['model']['mismatch'], 1)
                ledger = json.loads((DATA / 'ledger.json').read_text())
                ledger['examples'] = {k: v for k, v in ledger['examples'].items() if int(k) in REMAINING_IDS}
                self.assertEqual([e['example'] for e in compare_ledger(report, ledger)], [id_])

    def test_report_cannot_hide_or_change_annotations_or_comparison(self):
        original = analyze(self.manifest, self.fixtures, self.response)
        for category, value in (('losses', ['other']), ('limits', ['other']),
                                ('comparison', 'html-projection')):
            report = copy.deepcopy(original)
            entry = next(e for e in report['examples'] if e['example'] == 494)
            entry['model'][category] = value
            with self.subTest(category=category, value=value), self.assertRaises(ValueError):
                validate_report(report, self.manifest, self.fixtures)
        report = copy.deepcopy(original)
        del report['examples'][0]['model']['losses']
        with self.assertRaises(ValueError):
            validate_report(report, self.manifest, self.fixtures)

    def test_stale_uncheckable_ledger_fails(self):
        report = analyze(self.manifest, self.fixtures, self.response)
        ledger = json.loads((DATA / 'ledger.json').read_text())
        ledger['examples'] = {k: v for k, v in ledger['examples'].items() if int(k) in REMAINING_IDS}
        self.assertFalse(compare_ledger(report, ledger))
        ledger['examples']['21'] = {'parser': 'pass', 'model': 'uncheckable',
            'limits': ['html-oracle-unsupported'], 'losses': [], 'reason': 'old',
            'modelSha256': '0' * 64, 'review': 'stale exception'}
        self.assertEqual([e['example'] for e in compare_ledger(report, ledger)], [21])


class ValidationTest(unittest.TestCase):
    def setUp(self):
        self.manifest, _ = load_fixtures()
        self.fixtures = [fixture()]
        self.response = {"schema": 2, "qt": "test", "cmark": "0.31.2", "parseOptions": PARSE_OPTIONS,
            "htmlOptions": HTML_OPTIONS, "examples": [{"example": 1, "html": '<p>hello</p>\n', "model": complete_plain([paragraph('hello')])}]}
        self.report = analyze(self.manifest, self.fixtures, self.response)
        self.ledger = {"schema": 1, "fixtureSha256": self.manifest["sha256"], "review": "Authored clean case", "examples": {}}

    def test_complete_official_input_is_pinned_and_ledger_covers_only_known_ids(self):
        manifest, fixtures = load_fixtures()
        self.assertEqual(len(fixtures), 652)
        self.assertEqual(manifest['sha256'], 'd431b29d97b6f73e69d547109cf5081578fac931e72afe95639ebe766c1b2a20')

    def test_fixture_duplicates_missing_ids_unknown_fields_and_types_fail(self):
        for fixtures, count in (([fixture(), fixture()], 2), ([], 1), ([{**fixture(), 'example': 2}], 1),
                                ([{**fixture(), 'example': True}], 1), ([{**fixture(), 'unknown': 1}], 1),
                                ([{**fixture(), 'start_line': 0}], 1)):
            with self.subTest(fixtures=fixtures), self.assertRaises(ValueError):
                validate_fixtures(fixtures, count)

    def test_corrupted_fixture_bytes_fail_before_probe(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)
            (path / 'fixture.json').write_bytes((DATA / 'fixture.json').read_bytes())
            (path / 'spec.json').write_bytes((DATA / 'spec.json').read_bytes() + b' ')
            with self.assertRaisesRegex(ValueError, 'checksum'):
                load_fixtures(path)

    def test_response_unknown_fields_ids_versions_options_and_missing_examples_fail(self):
        cases = []
        for key, value in (('unknown', 1), ('schema', True), ('schema', 1), ('cmark', 'other'), ('parseOptions', 'other'), ('examples', [])):
            cases.append({**self.response, key: value})
        wrong_id = copy.deepcopy(self.response)
        wrong_id['examples'][0]['example'] = 2
        cases.append(wrong_id)
        for response in cases:
            with self.subTest(response=response), self.assertRaises(ValueError):
                validate_response(response, self.fixtures)

    def test_invalid_model_ranges_unknown_fields_and_list_placement_fail(self):
        cases = ([paragraph('😀', [span(1, 1, 2)])], [paragraph('x', [span(0, 2, 1)])],
                 [paragraph('x', [span(0, 1, 8)])], [paragraph('x', links=[link(0, 0, 'x')])],
                 [{**paragraph('x'), 'unknown': 1}], [{'kind': 'ListItem', 'children': []}],
                 [paragraph('abc', images=[image(1, 2, 'a'), image(2, 0, 'b')])])
        for blocks in cases:
            with self.subTest(blocks=blocks), self.assertRaises(ValueError):
                validate_model(complete_plain(blocks))

    def test_report_missing_examples_unknown_fields_totals_and_diff_tampering_fail(self):
        for key, value in (('examples', []), ('unknown', 1), ('totals', {}), ('sections', {})):
            report = {**self.report, key: value}
            with self.subTest(key=key), self.assertRaises(ValueError):
                validate_report(report, self.manifest, self.fixtures)
        for field in ('parser', 'model'):
            report = copy.deepcopy(self.report)
            report['examples'][0][field] = []
            with self.subTest(field=field), self.assertRaises(ValueError):
                validate_report(report, self.manifest, self.fixtures)
        response = copy.deepcopy(self.response)
        response['examples'][0]['html'] = '<p>wrong</p>\n'
        report = analyze(self.manifest, self.fixtures, response)
        report['examples'][0]['parser']['diff'] = 'hidden'
        with self.assertRaisesRegex(ValueError, 'mismatch evidence'):
            validate_report(report, self.manifest, self.fixtures)

    def test_baseline_rejects_new_parser_model_and_oracle_failures(self):
        self.assertFalse(compare_ledger(self.report, self.ledger))
        for target, value in (('html', '<p>wrong</p>\n'), ('model', complete_plain([paragraph('wrong')]))):
            response = copy.deepcopy(self.response)
            response['examples'][0][target] = value
            report = analyze(self.manifest, self.fixtures, response)
            self.assertEqual(len(compare_ledger(report, self.ledger)), 1)
        fixtures = [fixture(html='<div>raw</div>\n')]
        report = analyze(self.manifest, fixtures, self.response)
        self.assertEqual(report['totals']['model']['uncheckable'], 1)
        self.assertEqual(len(compare_ledger(report, self.ledger)), 1)

    def test_ledger_requires_review_provenance_and_known_ids(self):
        for key, value in (('review', ''), ('fixtureSha256', 'bad'), ('unknown', 1), ('examples', {'2': {}})):
            with self.subTest(key=key), self.assertRaises(ValueError):
                compare_ledger(self.report, {**self.ledger, key: value})

    def test_unchanged_status_with_worse_evidence_cannot_pass_and_improvement_needs_review(self):
        response = copy.deepcopy(self.response)
        response['examples'][0]['model'] = complete_plain([paragraph('wrong')])
        report = analyze(self.manifest, self.fixtures, response)
        self.ledger['examples']['1'] = {**exception(report['examples'][0]), 'review': 'Authored known mismatch'}
        self.assertFalse(compare_ledger(report, self.ledger))
        response['examples'][0]['model'] = complete_plain([paragraph('worse')])
        worse = analyze(self.manifest, self.fixtures, response)
        self.assertEqual(len(compare_ledger(worse, self.ledger)), 1)
        self.assertEqual(len(compare_ledger(self.report, self.ledger)), 1)

    def test_strict_requires_complete_semantics_even_with_unchanged_baseline(self):
        self.assertTrue(strict_pass(self.report))
        for html, markdown in (('<p>a\nb</p>\n', 'a\nb\n'), ('<p><a href="x" title="t">x</a></p>\n', '[x](x "t")'),
                               ('<div>x</div>\n', '<div>x</div>\n')):
            fixtures = [fixture(markdown, html)]
            response = run_probe(PROBE, fixtures)
            report = analyze(self.manifest, fixtures, response)
            self.assertFalse(strict_pass(report))

    def test_opaque_html_and_image_formatting_changes_need_baseline_review(self):
        for fixtures in ([fixture('<div>x</div>\n', '<div>x</div>\n')],
                         [fixture('![*x*](i)', '<p><img src="i" alt="x" /></p>\n')]):
            response = run_probe(PROBE, fixtures)
            report = analyze(self.manifest, fixtures, response)
            ledger = {**self.ledger, 'examples': {'1': {**exception(report['examples'][0]), 'review': 'Authored opaque-field case'}}}
            self.assertFalse(compare_ledger(report, ledger))
            changed = copy.deepcopy(response)
            if changed['examples'][0]['model'][0]['kind'] == 'HtmlBlock':
                changed['examples'][0]['model'][0]['text'] = '<div>worse</div>\n'
            else:
                changed['examples'][0]['model'][0]['ranges'] = []
            changed_report = analyze(self.manifest, fixtures, changed)
            self.assertEqual(changed_report['examples'][0]['model']['status'], report['examples'][0]['model']['status'])
            self.assertEqual(len(compare_ledger(changed_report, ledger)), 1)

    def test_reports_are_repeatable(self):
        self.assertEqual(json_text(analyze(self.manifest, self.fixtures, self.response)), json_text(self.report))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--probe', type=Path, default=Path('build-shared/tests/qmarkdown-commonmark-probe'))
    args, remaining = parser.parse_known_args()
    PROBE = args.probe
    unittest.main(argv=[sys.argv[0], *remaining])
