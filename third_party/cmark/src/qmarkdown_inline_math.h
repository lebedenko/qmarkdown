#ifndef QMARKDOWN_INLINE_MATH_H
#define QMARKDOWN_INLINE_MATH_H
#include "utf8.h"
#include "cmark_ctype.h"

/* Byte offsets into the normalized inline input. Failure caches avoid rescanning
 * a physical line with many unmatched openers. No exported symbols. */
typedef struct { bufsize_t dollar_failed, paren_failed; } qmarkdown_math_scan;
static bool qm_math_space(const unsigned char *s, bufsize_t len, bufsize_t pos) {
  int32_t cp = 0;
  if (pos < 0 || pos >= len) return true;
  cmark_utf8proc_iterate(s + pos, len - pos, &cp);
  return cmark_utf8proc_is_space(cp);
}
static bool qm_math_escaped(const unsigned char *s, bufsize_t pos) {
  bufsize_t n = 0;
  while (pos && s[--pos] == '\\') ++n;
  return n % 2;
}
/* Return the end offset; body boundaries exclude the matching delimiters. */
static bufsize_t qm_math_match(const unsigned char *s, bufsize_t len, bufsize_t pos,
                              qmarkdown_math_scan *scan, bufsize_t *body,
                              bufsize_t *body_end) {
  bool dollar = s[pos] == '$';
  if (qm_math_escaped(s, pos)) return 0;
  if (dollar) {
    if ((pos && s[pos - 1] == '$') || pos + 1 >= len || s[pos + 1] == '$' ||
        qm_math_space(s, len, pos + 1)) return 0;
  } else if (s[pos] != '\\' || pos + 1 >= len || s[pos + 1] != '(') return 0;
  bufsize_t *failed = dollar ? &scan->dollar_failed : &scan->paren_failed;
  if (pos <= *failed) return 0;
  *body = pos + (dollar ? 1 : 2);
  bufsize_t i = *body;
  for (; i < len && s[i] != '\n' && s[i] != '\r'; ++i) {
    if (dollar && s[i] == '$' && i > *body && !qm_math_escaped(s, i) &&
        s[i - 1] != '$' && (i + 1 == len || (s[i + 1] != '$' && !cmark_isdigit(s[i + 1])))) {
      bufsize_t prev = i - 1;
      while (prev > *body && (s[prev] & 0xc0) == 0x80) --prev;
      if (!qm_math_space(s, len, prev)) { *body_end = i; return i + 1; }
    } else if (!dollar && s[i] == '\\' && i + 1 < len && s[i + 1] == ')' &&
               i > *body && !qm_math_escaped(s, i)) {
      *body_end = i; return i + 2;
    }
  }
  *failed = i;
  return 0;
}
#endif
