#pragma once
#include <stdexcept>
namespace tex {
inline thread_local int qmarkdownOperations = -1, qmarkdownAllocations = -1, qmarkdownDepth = 0;
inline void qmarkdownStep() {
    if (qmarkdownOperations >= 0 && --qmarkdownOperations <= 0) throw std::runtime_error("Math work limit");
}
struct QMarkdownDepth {
    QMarkdownDepth() { qmarkdownStep(); if (++qmarkdownDepth > 64) { --qmarkdownDepth; throw std::runtime_error("Math depth limit"); } }
    ~QMarkdownDepth() { --qmarkdownDepth; }
};
}
