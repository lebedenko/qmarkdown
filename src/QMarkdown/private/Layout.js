.pragma library

function metric(value) {
    return Number.isFinite(value) ? Math.max(0, value) : 0
}

function headingStyle(style, level) {
    return level > 0 ? style.headings["h" + level] : null
}
