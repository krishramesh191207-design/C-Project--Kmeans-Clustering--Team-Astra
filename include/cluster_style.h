#pragma once

#include <cstddef>

// Names match the fixed cluster colours used in the SVG plot and CLI report.
inline const char* clusterColorName(size_t clusterIndex) {
    static const char* const names[] = {
        "blue", "orange", "green", "purple", "red",
        "teal", "pink", "indigo", "lime", "brown"
    };
    return names[clusterIndex % (sizeof(names) / sizeof(names[0]))];
}
