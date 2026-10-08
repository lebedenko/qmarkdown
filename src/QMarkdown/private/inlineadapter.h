#pragma once
#include "inline.h"
struct cmark_node;
namespace QMarkdownPrivate {
// Borrowed public cmark inline nodes; the caller retains ownership.
Q_DECL_HIDDEN InlineContent adaptInlineNodes(cmark_node *node);
}
