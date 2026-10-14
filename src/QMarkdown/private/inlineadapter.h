#pragma once
#include "inline.h"
// Keep ownership-template instantiations private as well as parser functions.
struct Q_DECL_HIDDEN cmark_node;
namespace QMarkdownPrivate {
// Borrowed public cmark inline nodes; the caller retains ownership.
Q_DECL_HIDDEN InlineContent adaptInlineNodes(cmark_node *node, bool tagFilter = false);
}
