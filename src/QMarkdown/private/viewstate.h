#pragma once
#include "../markdownstyle.h"
#include "presentation.h"
#include "resourcecontroller.h"
#include <QAbstractListModel>
#include <QPointer>
#include <QQuickItem>

class BlockModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        BlockTextRole = Qt::UserRole + 0,
        HeadingLevelRole = Qt::UserRole + 1,
        FormattedRole = Qt::UserRole + 2,
        FormatRangesRole = Qt::UserRole + 3,
        RenderKindRole = Qt::UserRole + 4,
        ChildBlocksRole = Qt::UserRole + 5,
        TightListRole = Qt::UserRole + 6,
        MarkersRole = Qt::UserRole + 7,
        LinkSpansRole = Qt::UserRole + 8,
        ImageDataRole = Qt::UserRole + 9,
        ImageLinkRole = Qt::UserRole + 10,
        ImageLinkedRole = Qt::UserRole + 11,
        TaskStateRole = Qt::UserRole + 12,
        TableAlignmentRole = Qt::UserRole + 13,
        CodeSpansRole = Qt::UserRole + 14,
        SelectionOffsetRole = Qt::UserRole + 15,
        SelectionLengthRole = Qt::UserRole + 16,
        SelectionKindRole = Qt::UserRole + 17
    };
    using QAbstractListModel::QAbstractListModel;
    int rowCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void replace(QVector<QMarkdownPrivate::PresentationBlock> blocks);
private:
    QVector<QMarkdownPrivate::PresentationBlock> m_blocks;
    QVector<BlockModel *> m_children;
};

class ViewState : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS
    Q_PROPERTY(bool selectionEnabled READ selectionEnabled WRITE setSelectionEnabled NOTIFY selectionEnabledChanged FINAL)
    Q_PROPERTY(QString selectedText READ selectedText NOTIFY selectionChanged FINAL)
    Q_PROPERTY(int selectionStart READ selectionStart NOTIFY selectionChanged FINAL)
    Q_PROPERTY(int selectionEnd READ selectionEnd NOTIFY selectionChanged FINAL)
    Q_PROPERTY(bool tablesEnabled READ tablesEnabled WRITE setTablesEnabled NOTIFY tablesEnabledChanged FINAL)
    Q_PROPERTY(bool tagFilterEnabled READ tagFilterEnabled WRITE setTagFilterEnabled NOTIFY tagFilterEnabledChanged FINAL)
    Q_PROPERTY(bool gfmEnabled READ gfmEnabled WRITE setGfmEnabled NOTIFY gfmEnabledChanged FINAL)
    Q_PROPERTY(QString markdown READ markdown WRITE setMarkdown NOTIFY markdownChanged FINAL)
    Q_PROPERTY(bool extendedAutolinksEnabled READ extendedAutolinksEnabled WRITE setExtendedAutolinksEnabled NOTIFY extendedAutolinksEnabledChanged FINAL)
    Q_PROPERTY(bool strikethroughEnabled READ strikethroughEnabled WRITE setStrikethroughEnabled NOTIFY strikethroughEnabledChanged FINAL)
    Q_PROPERTY(bool taskListsEnabled READ taskListsEnabled WRITE setTaskListsEnabled NOTIFY taskListsEnabledChanged FINAL)
    Q_PROPERTY(MarkdownStyle *style READ style WRITE setStyle RESET resetStyle NOTIFY styleChanged FINAL)
    Q_PROPERTY(QUrl baseUrl READ baseUrl WRITE setBaseUrl NOTIFY baseUrlChanged FINAL)
    Q_PROPERTY(MarkdownResourcePolicy *resourcePolicy READ resourcePolicy WRITE setResourcePolicy RESET resetResourcePolicy NOTIFY resourcePolicyChanged FINAL)
    Q_PROPERTY(QAbstractItemModel *blocks READ blocks CONSTANT FINAL)
public:
    enum RenderKind { PlainText = 0, FormattedText = 1, CodeBlock = 2, ThematicBreak = 3, List = 4, Quote = 5, ListItem = 6, HtmlBlock = 7, Image = 8, Segments = 9, Table = 10, SelectedText = 11, SelectedCode = 12, SelectedHtml = 13 };
    Q_ENUM(RenderKind)
    enum SelectionItemKind { TextSelection = 0, ImageSelection = 1, TableCellSelection = 2, ActionSelection = 3 };
    Q_ENUM(SelectionItemKind)
    explicit ViewState(QObject *parent = nullptr);
    bool selectionEnabled() const { return m_selectionEnabled; }
    void setSelectionEnabled(bool value);
    int selectionStart() const { return qMin(m_anchor, m_active); }
    int selectionEnd() const { return qMax(m_anchor, m_active); }
    QString selectedText() const { return m_projection.mid(selectionStart(), selectionEnd() - selectionStart()); }
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void requestCopy();
    Q_INVOKABLE void registerSelectionItem(QQuickItem *item, int kind);
    Q_INVOKABLE void unregisterSelectionItem(QQuickItem *item);
    Q_INVOKABLE bool beginSelection(QQuickItem *view, QPointF point, bool extend);
    Q_INVOKABLE void extendSelection(QQuickItem *view, QPointF point);
    Q_INVOKABLE QVariant selectionLink(QQuickItem *view, QPointF point) const;

    Q_INVOKABLE QFont boldFont(QFont font) const { font.setWeight(qMax(font.weight(), QFont::Bold)); return font; }
    QUrl baseUrl() const { return m_baseUrl; }
    void setBaseUrl(const QUrl &value);
    MarkdownResourcePolicy *resourcePolicy() const { return m_policy; }
    void setResourcePolicy(MarkdownResourcePolicy *value);
    void resetResourcePolicy();
    bool tablesEnabled() const { return m_tablesEnabled; }
    void setTablesEnabled(bool value);
    bool tagFilterEnabled() const { return m_tagFilterEnabled; }
    void setTagFilterEnabled(bool value);
    bool gfmEnabled() const { return m_gfmEnabled; }
    void setGfmEnabled(bool value);
    QString markdown() const { return m_markdown; }
    void setMarkdown(const QString &value);
    bool extendedAutolinksEnabled() const { return m_extendedAutolinksEnabled; }
    void setExtendedAutolinksEnabled(bool value);
    bool strikethroughEnabled() const { return m_strikethroughEnabled; }
    void setStrikethroughEnabled(bool value);
    bool taskListsEnabled() const { return m_taskListsEnabled; }
    void setTaskListsEnabled(bool value);
    MarkdownStyle *style() const { return m_style; }
    void setStyle(MarkdownStyle *value);
    void resetStyle();
    QAbstractItemModel *blocks() { return &m_blocks; }
signals:
    void presentationReset();
    void selectionEnabledChanged();
    void selectionChanged();
    void copyRequested(const QString &text);
    void baseUrlChanged();
    void resourcePolicyChanged();
    void tablesEnabledChanged();
    void tagFilterEnabledChanged();
    void gfmEnabledChanged();
    void markdownChanged();
    void taskListsEnabledChanged();
    void strikethroughEnabledChanged();
    void extendedAutolinksEnabledChanged();
    void styleChanged();
private:
    struct SelectionItem { QPointer<QQuickItem> item; int kind; };
    QVector<SelectionItem> m_selectionItems;
    QQuickItem *selectionItemAt(QQuickItem *view, QPointF point) const;
    int selectionPosition(QQuickItem *view, QPointF point) const;
    bool selectionExcluded(QQuickItem *view, QPointF point) const;
    void setSelection(int anchor, int active);
    QString m_projection;
    bool m_selectionEnabled = false, m_hasAnchor = false;
    int m_anchor = 0, m_active = 0;
    QMarkdownPrivate::ParseOptions m_parsedOptions;
    void reloadResources();
    void reparse();
    QUrl m_baseUrl;
    QVector<QMarkdownPrivate::Block> m_sourceBlocks;
    QVector<QMarkdownPrivate::PresentationBlock> m_presentationBlocks;
    MarkdownResourcePolicy m_defaultPolicy;
    QPointer<MarkdownResourcePolicy> m_policy;
    QMetaObject::Connection m_policyDestroyed, m_policyChanged;
    ResourceController m_resources;
    QString m_markdown;
    bool m_tablesEnabled = false;
    bool m_tagFilterEnabled = false;
    bool m_gfmEnabled = false;
    bool m_taskListsEnabled = false;
    bool m_strikethroughEnabled = false;
    bool m_extendedAutolinksEnabled = false;
    BlockModel m_blocks;
    MarkdownStyle m_default;
    QPointer<MarkdownStyle> m_style;
    QMetaObject::Connection m_destroyed;
};
