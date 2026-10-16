#include "syntaxtheme.h"

QColor SyntaxTheme::color(int role) const
{
    static const char *light[] = {"#7c3aed", "#0f766e", "#1d4ed8", "#9a3412", "#0f766e", "#15803d", "#b45309", "#64748b", "#7c3aed", nullptr, "#9a3412", "#b91c1c"};
    static const char *dark[] = {"#c4b5fd", "#5eead4", "#93c5fd", "#fdba74", "#5eead4", "#86efac", "#fcd34d", "#94a3b8", "#c4b5fd", nullptr, "#fdba74", "#fca5a5"};
    if (role < 0 || role >= int(m_colors.size())) return {};
    if (m_set[role]) return m_colors[role];
    const auto value = m_preset == Dark ? dark[role] : light[role];
    return value ? QColor(value) : QColor();
}
