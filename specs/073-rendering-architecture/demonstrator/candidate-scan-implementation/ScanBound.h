// Private experiment helper shared by copied SkParagraph and synthetic tests.
#pragma once
#include <algorithm>
#include <cfenv>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#if defined(__x86_64__) && defined(__SSE2__)
#include <xmmintrin.h>
#endif
namespace scan_bound {
enum Reason { safe, representation, environment, width, padding, intermediate, reasonCount };
template<class T> Reason platform() {
#if defined(__FAST_MATH__) || (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0)
    return environment;
#endif
    if (!std::is_same<T,float>::value || !std::numeric_limits<T>::is_iec559 ||
        std::numeric_limits<T>::radix != 2 || sizeof(T) != 4) return representation;
#if defined(__x86_64__) && defined(__SSE2__) && FLT_EVAL_METHOD == 0
    if (std::fegetround() != FE_TONEAREST || (_mm_getcsr() & ((1u<<15)|(1u<<6)|(3u<<13)))) return environment;
    return safe;
#else
    return environment;
#endif
}
template<class T> bool finite(T x) { return std::isfinite(x); }
template<class T> bool skip(T suffix, T available) { return suffix > available; }
template<class Entries> void suffix(Entries& entries) {
    for (size_t i=entries.size(); i>0; --i)
        entries[i-1].lower = i==entries.size() ? entries[i-1].trimmed :
            std::min(entries[i-1].trimmed, entries[i].lower);
}
} // namespace scan_bound
