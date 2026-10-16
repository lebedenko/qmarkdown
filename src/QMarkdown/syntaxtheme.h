#pragma once
#include <QObject>
#include <QColor>
#include <array>
#include <QtQml/qqmlregistration.h>

class SyntaxTheme : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_ADDED_IN_VERSION(1, 11)
    Q_PROPERTY(Preset preset READ preset WRITE setPreset NOTIFY changed FINAL)
    Q_PROPERTY(QColor keywordColor READ keywordColor WRITE setKeywordColor RESET resetKeywordColor NOTIFY changed FINAL)
    Q_PROPERTY(QColor typeColor READ typeColor WRITE setTypeColor RESET resetTypeColor NOTIFY changed FINAL)
    Q_PROPERTY(QColor functionColor READ functionColor WRITE setFunctionColor RESET resetFunctionColor NOTIFY changed FINAL)
    Q_PROPERTY(QColor variableColor READ variableColor WRITE setVariableColor RESET resetVariableColor NOTIFY changed FINAL)
    Q_PROPERTY(QColor builtInColor READ builtInColor WRITE setBuiltInColor RESET resetBuiltInColor NOTIFY changed FINAL)
    Q_PROPERTY(QColor stringColor READ stringColor WRITE setStringColor RESET resetStringColor NOTIFY changed FINAL)
    Q_PROPERTY(QColor numberColor READ numberColor WRITE setNumberColor RESET resetNumberColor NOTIFY changed FINAL)
    Q_PROPERTY(QColor commentColor READ commentColor WRITE setCommentColor RESET resetCommentColor NOTIFY changed FINAL)
    Q_PROPERTY(QColor preprocessorColor READ preprocessorColor WRITE setPreprocessorColor RESET resetPreprocessorColor NOTIFY changed FINAL)
    Q_PROPERTY(QColor operatorColor READ operatorColor WRITE setOperatorColor RESET resetOperatorColor NOTIFY changed FINAL)
    Q_PROPERTY(QColor constantColor READ constantColor WRITE setConstantColor RESET resetConstantColor NOTIFY changed FINAL)
    Q_PROPERTY(QColor errorColor READ errorColor WRITE setErrorColor RESET resetErrorColor NOTIFY changed FINAL)
public:
    enum Preset { Light, Dark };
    Q_ENUM(Preset)
    explicit SyntaxTheme(QObject *parent = nullptr) : QObject(parent) {}
    Preset preset() const { return m_preset; }
    void setPreset(Preset value) {
        if (m_preset == value) return;
        m_preset = value; emit changed();
    }
    QColor color(int role) const;
    QColor keywordColor() const { return color(0); }
    void setKeywordColor(const QColor &value) { m_colors[0] = value; m_set[0] = true; emit changed(); }
    void resetKeywordColor() { m_set[0] = false; emit changed(); }
    QColor typeColor() const { return color(1); }
    void setTypeColor(const QColor &value) { m_colors[1] = value; m_set[1] = true; emit changed(); }
    void resetTypeColor() { m_set[1] = false; emit changed(); }
    QColor functionColor() const { return color(2); }
    void setFunctionColor(const QColor &value) { m_colors[2] = value; m_set[2] = true; emit changed(); }
    void resetFunctionColor() { m_set[2] = false; emit changed(); }
    QColor variableColor() const { return color(3); }
    void setVariableColor(const QColor &value) { m_colors[3] = value; m_set[3] = true; emit changed(); }
    void resetVariableColor() { m_set[3] = false; emit changed(); }
    QColor builtInColor() const { return color(4); }
    void setBuiltInColor(const QColor &value) { m_colors[4] = value; m_set[4] = true; emit changed(); }
    void resetBuiltInColor() { m_set[4] = false; emit changed(); }
    QColor stringColor() const { return color(5); }
    void setStringColor(const QColor &value) { m_colors[5] = value; m_set[5] = true; emit changed(); }
    void resetStringColor() { m_set[5] = false; emit changed(); }
    QColor numberColor() const { return color(6); }
    void setNumberColor(const QColor &value) { m_colors[6] = value; m_set[6] = true; emit changed(); }
    void resetNumberColor() { m_set[6] = false; emit changed(); }
    QColor commentColor() const { return color(7); }
    void setCommentColor(const QColor &value) { m_colors[7] = value; m_set[7] = true; emit changed(); }
    void resetCommentColor() { m_set[7] = false; emit changed(); }
    QColor preprocessorColor() const { return color(8); }
    void setPreprocessorColor(const QColor &value) { m_colors[8] = value; m_set[8] = true; emit changed(); }
    void resetPreprocessorColor() { m_set[8] = false; emit changed(); }
    QColor operatorColor() const { return color(9); }
    void setOperatorColor(const QColor &value) { m_colors[9] = value; m_set[9] = true; emit changed(); }
    void resetOperatorColor() { m_set[9] = false; emit changed(); }
    QColor constantColor() const { return color(10); }
    void setConstantColor(const QColor &value) { m_colors[10] = value; m_set[10] = true; emit changed(); }
    void resetConstantColor() { m_set[10] = false; emit changed(); }
    QColor errorColor() const { return color(11); }
    void setErrorColor(const QColor &value) { m_colors[11] = value; m_set[11] = true; emit changed(); }
    void resetErrorColor() { m_set[11] = false; emit changed(); }
signals:
    void changed();
private:
    Preset m_preset = Light;
    std::array<QColor, 12> m_colors;
    std::array<bool, 12> m_set{};
};
