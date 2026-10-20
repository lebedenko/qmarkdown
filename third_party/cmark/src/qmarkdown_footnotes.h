#ifndef QMARKDOWN_FOOTNOTES_H
#define QMARKDOWN_FOOTNOTES_H
/* Footnote labels use the reference-label grammar and shared normalizer. */
static bufsize_t qm_note_label(const unsigned char *s, bufsize_t len, bufsize_t pos) {
  if (pos + 3 >= len || s[pos] != '[' || s[pos + 1] != '^') return 0;
  for (bufsize_t i = pos + 2; i < len && i - pos <= MAX_LINK_LABEL_LENGTH + 2; ++i) {
    if (s[i] == '\n' || s[i] == '\r' || s[i] == '[') return 0;
    if (s[i] == '\\' && i + 1 < len) { ++i; continue; }
    if (s[i] == ']') return i > pos + 2 ? i + 1 : 0;
  }
  return 0;
}
#endif
