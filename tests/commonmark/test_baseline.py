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
                      validate_fixtures, validate_model, validate_report, validate_response)
from oracle import Uncheckable, canonical_actual, expected, mask_image_ranges, utf16

PROBE = None


def paragraph(text, ranges=None, links=None, images=None):
    return {"kind": "Paragraph", "text": text, "ranges": ranges or [], "links": links or [], "images": images or []}


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
        return expected(html)[0]

    def test_nested_formatting_has_utf16_offsets(self):
        self.assertEqual(self.project("<p>😀 <strong>é <em>b</em> <code>c</code></strong></p>\n"),
            [paragraph("😀 é b c", [span(3, 2, 2), span(5, 1, 3), span(6, 1, 2), span(7, 1, 6)])])
        self.assertEqual(utf16("😀é"), 3)

    def test_soft_and_hard_breaks_have_different_text(self):
        blocks, limits, losses = expected("<p>soft\nnext<br />\nhard <em>x<br />\ny</em></p>\n")
        self.assertEqual(blocks, [paragraph("soft next\nhard x\ny", [span(15, 3, 1)])])
        self.assertIn("softbreak-kind-projected", losses)

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
        self.assertEqual(blocks, [paragraph("😀é", links=[link(0, 3, "")],
            images=[image(0, 3, "a&b", 'x "y"', linked=True), image(3, 0, "b", linked=True)])])
        self.assertIn("image-description-formatting-unavailable", limits)

    def test_masking_images_preserves_adjacent_and_surrounding_ranges(self):
        value = paragraph("abcde", [span(0, 5, 2)], images=[image(1, 2, "a"), image(3, 0, "b")])
        self.assertEqual(mask_image_ranges(value)["ranges"], [span(0, 1, 2), span(3, 2, 2)])

    def test_link_title_empty_link_and_repeated_emphasis_are_visible_losses(self):
        blocks, limits, losses = expected('<p><a href="x" title="t"></a><em><em>x</em></em></p>\n')
        self.assertEqual(blocks, [paragraph("x", [span(0, 1, 1)])])
        self.assertEqual(losses, ["empty-link-omitted", "link-title-omitted", "repeated-emphasis-depth-flattened"])

    def test_balanced_unknown_inline_html_is_literal(self):
        blocks, limits, losses = expected('<p>before <B title="&amp;">x</B > <!-- y --> after</p>\n')
        self.assertEqual(blocks, [paragraph('before <B title="&amp;">x</B > <!-- y --> after')])
        self.assertIn("inline-html-identity-projected", losses)

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
        actual = [paragraph("x", links=[link(0, 1, "é a%20b")])]
        self.assertEqual(canonical_actual(actual), self.project('<p><a href="%C3%A9%20a%20b">x</a></p>\n'))
        self.assertEqual(actual[0]["links"][0]["destination"], "é a%20b")


class ProductionModelTest(unittest.TestCase):
    def check(self, markdown, want):
        response = run_probe(PROBE, [fixture(markdown)])
        self.assertEqual(response["examples"][0]["model"], want)

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


class ValidationTest(unittest.TestCase):
    def setUp(self):
        self.manifest, _ = load_fixtures()
        self.fixtures = [fixture()]
        self.response = {"schema": 1, "qt": "test", "cmark": "0.31.2", "parseOptions": PARSE_OPTIONS,
            "htmlOptions": HTML_OPTIONS, "examples": [{"example": 1, "html": '<p>hello</p>\n', "model": [paragraph('hello')]}]}
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
        for key, value in (('unknown', 1), ('schema', True), ('cmark', 'other'), ('parseOptions', 'other'), ('examples', [])):
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
                validate_model(blocks)

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
        for target, value in (('html', '<p>wrong</p>\n'), ('model', [paragraph('wrong')])):
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
        response['examples'][0]['model'] = [paragraph('wrong')]
        report = analyze(self.manifest, self.fixtures, response)
        self.ledger['examples']['1'] = {**exception(report['examples'][0]), 'review': 'Authored known mismatch'}
        self.assertFalse(compare_ledger(report, self.ledger))
        response['examples'][0]['model'] = [paragraph('worse')]
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
