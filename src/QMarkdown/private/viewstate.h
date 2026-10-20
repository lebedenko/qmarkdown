#pragma once
#include "../markdownstyle.h"
#include "syntaxhighlighter.h"
#include "mathitem.h"
#include "presentation.h"
#include "linkcontroller.h"
#include "resourcecontroller.h"
#include <QAbstractListModel>
#include <QPointer>
#include <QQuickItem>
#include <QElapsedTimer>
#include <QTimer>
#include <QTextLayout>
#include <QQmlComponent>

class SelectionRegistration;
class QQmlComponent;

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
        SelectionKindRole = Qt::UserRole + 17,
        CodeInfoRole = Qt::UserRole + 18,
        CodeFencedRole = Qt::UserRole + 19,
        CodeLanguageRole = Qt::UserRole + 20,
        ImageLinkIdRole = Qt::UserRole + 21,
        DisplayMathRole = Qt::UserRole + 22,
        FormulaSpansRole = Qt::UserRole + 24,
        OrderedListRole = Qt::UserRole + 23
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
    Q_PROPERTY(bool codeBlockWrapEnabled READ codeBlockWrapEnabled WRITE setCodeBlockWrapEnabled NOTIFY codeBlockSettingsChanged FINAL)
    Q_PROPERTY(bool codeBlockLineNumbersVisible READ codeBlockLineNumbersVisible WRITE setCodeBlockLineNumbersVisible NOTIFY codeBlockSettingsChanged FINAL)
    Q_PROPERTY(bool codeBlockHeightLimitEnabled READ codeBlockHeightLimitEnabled WRITE setCodeBlockHeightLimitEnabled NOTIFY codeBlockSettingsChanged FINAL)
    Q_PROPERTY(qreal codeBlockViewportHeight READ codeBlockViewportHeight WRITE setCodeBlockViewportHeight NOTIFY codeBlockSettingsChanged FINAL)
    Q_PROPERTY(qreal codeBlockHeightRatio READ codeBlockHeightRatio WRITE setCodeBlockHeightRatio NOTIFY codeBlockSettingsChanged FINAL)
    Q_PROPERTY(int codeBlockMinimumLines READ codeBlockMinimumLines WRITE setCodeBlockMinimumLines NOTIFY codeBlockSettingsChanged FINAL)
    Q_PROPERTY(int codeBlockMaximumLines READ codeBlockMaximumLines WRITE setCodeBlockMaximumLines NOTIFY codeBlockSettingsChanged FINAL)
    Q_PROPERTY(QQmlComponent * codeBlockScrollBar READ codeBlockScrollBar WRITE setCodeBlockScrollBar NOTIFY codeBlockSettingsChanged FINAL)
    Q_PROPERTY(bool inlineMathEnabled READ inlineMathEnabled WRITE setInlineMathEnabled NOTIFY inlineMathEnabledChanged FINAL)
    Q_PROPERTY(bool mathEnabled READ mathEnabled WRITE setMathEnabled NOTIFY mathEnabledChanged FINAL)
    Q_PROPERTY(bool mathAvailable READ mathAvailable CONSTANT FINAL)
    Q_PROPERTY(bool syntaxHighlightingEnabled READ syntaxHighlightingEnabled WRITE setSyntaxHighlightingEnabled NOTIFY syntaxHighlightingEnabledChanged FINAL)
    Q_PROPERTY(bool syntaxHighlightingAvailable READ syntaxHighlightingAvailable CONSTANT FINAL)
    Q_PROPERTY(SyntaxTheme *syntaxTheme READ syntaxTheme WRITE setSyntaxTheme NOTIFY syntaxThemeChanged FINAL)
    Q_PROPERTY(LinkController *linkController READ linkController CONSTANT FINAL)
    Q_PROPERTY(bool searchActive READ searchActive WRITE setSearchActive NOTIFY searchActiveChanged FINAL)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged FINAL)
    Q_PROPERTY(bool searchCaseSensitive READ searchCaseSensitive WRITE setSearchCaseSensitive NOTIFY searchCaseSensitiveChanged FINAL)
    Q_PROPERTY(int searchResultCount READ searchResultCount NOTIFY searchChanged FINAL)
    Q_PROPERTY(int currentSearchResult READ currentSearchResult NOTIFY searchChanged FINAL)
    Q_PROPERTY(uint searchRevision READ searchRevision NOTIFY searchChanged FINAL)
    Q_PROPERTY(QVariantList headings READ headings NOTIFY headingsChanged FINAL)
    Q_PROPERTY(QQuickItem *navigationView READ navigationView WRITE setNavigationView NOTIFY navigationViewChanged FINAL)
    Q_PROPERTY(QQuickItem *selectionViewport READ selectionViewport WRITE setSelectionViewport NOTIFY selectionViewportChanged FINAL)
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
    bool codeBlockWrapEnabled() const { return m_codeBlockWrapEnabled; }
    void setCodeBlockWrapEnabled(bool value) { if (m_codeBlockWrapEnabled == value) return; m_codeBlockWrapEnabled = value; emit codeBlockSettingsChanged(); }
    bool codeBlockLineNumbersVisible() const { return m_codeBlockLineNumbersVisible; }
    void setCodeBlockLineNumbersVisible(bool value) { if (m_codeBlockLineNumbersVisible == value) return; m_codeBlockLineNumbersVisible = value; emit codeBlockSettingsChanged(); }
    bool codeBlockHeightLimitEnabled() const { return m_codeBlockHeightLimitEnabled; }
    void setCodeBlockHeightLimitEnabled(bool value) { if (m_codeBlockHeightLimitEnabled == value) return; m_codeBlockHeightLimitEnabled = value; emit codeBlockSettingsChanged(); }
    qreal codeBlockViewportHeight() const { return m_codeBlockViewportHeight; }
    void setCodeBlockViewportHeight(qreal value) { if (m_codeBlockViewportHeight == value) return; m_codeBlockViewportHeight = value; emit codeBlockSettingsChanged(); }
    qreal codeBlockHeightRatio() const { return m_codeBlockHeightRatio; }
    void setCodeBlockHeightRatio(qreal value) { if (m_codeBlockHeightRatio == value) return; m_codeBlockHeightRatio = value; emit codeBlockSettingsChanged(); }
    int codeBlockMinimumLines() const { return m_codeBlockMinimumLines; }
    void setCodeBlockMinimumLines(int value) { if (m_codeBlockMinimumLines == value) return; m_codeBlockMinimumLines = value; emit codeBlockSettingsChanged(); }
    int codeBlockMaximumLines() const { return m_codeBlockMaximumLines; }
    void setCodeBlockMaximumLines(int value) { if (m_codeBlockMaximumLines == value) return; m_codeBlockMaximumLines = value; emit codeBlockSettingsChanged(); }
    QQmlComponent *codeBlockScrollBar() const { return m_codeBlockScrollBar; }
    void setCodeBlockScrollBar(QQmlComponent *value) {
        if (m_codeBlockScrollBar == value) return;
        disconnect(m_codeScrollBarDestroyed);
        m_codeBlockScrollBar = value;
        if (value) m_codeScrollBarDestroyed = connect(value, &QObject::destroyed, this, [this] { m_codeBlockScrollBar = nullptr; emit codeBlockSettingsChanged(); });
        emit codeBlockSettingsChanged();
    }
    bool inlineMathEnabled() const { return m_inlineMathEnabled; }
    void setInlineMathEnabled(bool value) { if (m_inlineMathEnabled == value) return; m_inlineMathEnabled = value; reparse(); emit inlineMathEnabledChanged(); }
    bool mathEnabled() const { return m_mathEnabled; }
    bool mathAvailable() const { return QMarkdownPrivate::mathAvailable(); }
    void setMathEnabled(bool value) { if (m_mathEnabled == value) return; m_mathEnabled = value; reparse(); emit mathEnabledChanged(); }
    bool syntaxHighlightingEnabled() const { return m_syntaxEnabled; }
    void setSyntaxHighlightingEnabled(bool value) {
        if (m_syntaxEnabled == value) return;
        m_syntaxEnabled = value; emit syntaxHighlightingEnabledChanged();
    }
    bool syntaxHighlightingAvailable() const { return QMarkdownPrivate::syntaxHighlightingAvailable(); }
    SyntaxTheme *syntaxTheme() { if (!m_defaultSyntaxTheme) m_defaultSyntaxTheme = new SyntaxTheme(this); return m_syntaxTheme ? m_syntaxTheme.data() : m_defaultSyntaxTheme; }
    void setSyntaxTheme(SyntaxTheme *value) {
        if (m_syntaxTheme == value) return;
        disconnect(m_syntaxThemeDestroyed);
        m_syntaxTheme = value;
        if (value) m_syntaxThemeDestroyed = connect(value, &QObject::destroyed, this, [this] { m_syntaxTheme = nullptr; emit syntaxThemeChanged(); });
        emit syntaxThemeChanged();
    }
    LinkController *linkController() const { return m_linkController; }
    bool searchActive() const { return m_searchActive; }
    void setSearchActive(bool value);
    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString &value);
    bool searchCaseSensitive() const { return m_searchCaseSensitive; }
    void setSearchCaseSensitive(bool value);
    int searchResultCount() const { return m_matches.size(); }
    int currentSearchResult() const { return m_currentMatch; }
    uint searchRevision() const { return m_searchRevision; }
    Q_INVOKABLE QVariantList searchRangesFor(int offset, int length) const;
    QVariantList headings() const { return m_headings; }
    QQuickItem *navigationView() const { return m_navigationView; }
    void setNavigationView(QQuickItem *view);
    Q_INVOKABLE bool searchNext();
    Q_INVOKABLE bool searchPrevious();
    Q_INVOKABLE bool navigateToHeading(int index);
    Q_INVOKABLE bool navigateToFragment(const QString &fragment);
    Q_INVOKABLE void refreshNavigation();
    enum RenderKind { PlainText = 0, FormattedText = 1, CodeBlock = 2, ThematicBreak = 3, List = 4, Quote = 5, ListItem = 6, HtmlBlock = 7, Image = 8, Segments = 9, Table = 10, SelectedText = 11, SelectedCode = 12, SelectedHtml = 13 };
    Q_ENUM(RenderKind)
    enum SelectionItemKind { TextSelection = 0, ImageSelection = 1, TableCellSelection = 2, ActionSelection = 3, MathSelection = 4 };
    Q_ENUM(SelectionItemKind)
    explicit ViewState(QObject *parent = nullptr);
    QQuickItem *selectionViewport() const { return m_selectionViewport; }
    void setSelectionViewport(QQuickItem *item);
    Q_INVOKABLE void dragSelection(QQuickItem *view, QPointF point);
    Q_INVOKABLE void endSelectionDrag();
    Q_INVOKABLE void cancelSelectionGesture();
    Q_INVOKABLE void finishSelectionTap(QQuickItem *view, QPointF point);
    bool selectionEnabled() const { return m_selectionEnabled; }
    void setSelectionEnabled(bool value);
    int selectionStart() const { return qMin(m_anchor, m_active); }
    int selectionEnd() const { return qMax(m_anchor, m_active); }
    QString selectedText() const { return m_projection.mid(selectionStart(), selectionEnd() - selectionStart()); }
    Q_INVOKABLE void registerSourceViewport(QQuickItem *item);
    Q_INVOKABLE QVariantMap plainCodeGeometry(const QVariantMap &input);
    Q_INVOKABLE QObject *createHostComponent(QQmlComponent *component, QQuickItem *parent, const QVariantMap &properties) const;
    Q_INVOKABLE QVariantMap codeLanguageMetadata(const QString &language) const;
    Q_INVOKABLE bool validCodeScrollBar(QObject *object) const;
    Q_INVOKABLE bool validFencedRenderer(QObject *object) const;
    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void requestCopy();
    void registerSelectionItem(SelectionRegistration *registration);
    void unregisterSelectionItem(SelectionRegistration *registration);
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
    void codeBlockSettingsChanged();
    void mathEnabledChanged();
    void inlineMathEnabledChanged();
    void syntaxHighlightingEnabledChanged();
    void syntaxThemeChanged();
    void searchActiveChanged();
    void searchQueryChanged();
    void searchCaseSensitiveChanged();
    void searchChanged();
    void headingsChanged();
    void navigationViewChanged();
    void navigationRequested(QRectF target);
    void selectionViewportChanged();
    void selectionScrollRequested(qreal deltaY);
    void selectionLinkActivated(const QString &destination);
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
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    bool m_codeBlockWrapEnabled = true;
    bool m_codeBlockLineNumbersVisible = false;
    bool m_codeBlockHeightLimitEnabled = false;
    qreal m_codeBlockViewportHeight = 0;
    qreal m_codeBlockHeightRatio = 0.5;
    int m_codeBlockMinimumLines = 8;
    int m_codeBlockMaximumLines = 30;
    QPointer<QQmlComponent> m_codeBlockScrollBar = nullptr;
    QVariantMap m_plainCodeInput;
    std::unique_ptr<QTextLayout> m_plainCodeLayout;
    QMetaObject::Connection m_codeScrollBarDestroyed;
    bool m_revealSource = false;
    bool m_mathEnabled = false;
    bool m_inlineMathEnabled = false;
    bool m_syntaxEnabled = false;
    SyntaxTheme *m_defaultSyntaxTheme = nullptr;
    QPointer<SyntaxTheme> m_syntaxTheme;
    QMetaObject::Connection m_syntaxThemeDestroyed;
    friend class LinkController;
    LinkController *m_linkController;
    void rebuildSearch();
    void rebuildHeadings();
    bool advanceSearch(int direction);
    void emitNavigation();
    struct ReaderRange { int start, length; };
    QVector<ReaderRange> m_matches, m_headingRanges;
    QVariantList m_headings;
    QString m_searchQuery;
    bool m_sourceChanged = false;
    bool m_searchActive = false, m_searchCaseSensitive = false;
    uint m_searchRevision = 0; // Zero is reserved for the pristine search state.
    int m_currentMatch = -1;
    int m_targetStart = -1, m_targetLength = 0;
    bool m_targetSearch = false, m_navigationQueued = false;
    QRectF m_lastTarget;
    QPointer<QQuickItem> m_navigationView;
    struct SelectionUnit { int start, end; bool lines; };
    QVector<SelectionUnit> m_selectionUnits;
    QPair<int, int> selectionUnitAt(QQuickItem *view, QPointF point, int granularity) const;
    QPointer<QQuickItem> m_selectionViewport, m_dragView;
    QMetaObject::Connection m_viewportDestroyed, m_windowActive, m_windowChanged;
    QTimer m_scrollTimer, m_linkTimer;
    QElapsedTimer m_scrollElapsed, m_clickElapsed;
    QPointF m_pointerScene, m_lastClickScene;
    QVariant m_pressedLink;
    int m_clickCount = 0, m_granularity = 1, m_unitStart = 0, m_unitEnd = 0;
    bool m_dragging = false, m_shiftClick = false, m_gestureActive = false;
    void scrollSelection();
    void observeSelectionWindow(QQuickItem *view);
    struct SelectionItem {
        SelectionRegistration *registration;
        QPointer<QQuickItem> item;
        int kind, offset, length;
        QString imageLink;
        bool imageLinked;
    };
    const SelectionItem *selectionMetadata(QQuickItem *item) const;
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
