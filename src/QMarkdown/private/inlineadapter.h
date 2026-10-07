#pragma once
#include "inline.h"
struct cmark_node;

namespace QMarkdownPrivate {
// Adapt a borrowed public cmark document; the caller retains ownership.
Q_DECL_HIDDEN InlineContent adaptInlineDocument(const QString &source, cmark_node *document);
}
