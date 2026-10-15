#include "selectionregistration.h"

SelectionRegistration::~SelectionRegistration() { detach(); }
void SelectionRegistration::componentComplete() { m_complete = true; update(); }
void SelectionRegistration::detach()
{
    if (m_complete && m_coordinator) m_coordinator->unregisterSelectionItem(this);
}
void SelectionRegistration::update()
{
    if (m_complete && m_coordinator) {
        if (m_item) m_coordinator->registerSelectionItem(this);
        else m_coordinator->unregisterSelectionItem(this);
    }
}
void SelectionRegistration::setCoordinator(ViewState *value)
{
    if (m_coordinator == value) return;
    detach();
    disconnect(m_coordinatorDestroyed);
    m_coordinator = value;
    if (value) m_coordinatorDestroyed = connect(value, &QObject::destroyed, this, [this] {
        m_coordinator = nullptr;
        emit coordinatorChanged();
    });
    update();
    emit coordinatorChanged();
}
void SelectionRegistration::setItem(QQuickItem *value)
{
    if (m_item == value) return;
    detach();
    disconnect(m_itemDestroyed);
    m_item = value;
    if (value) m_itemDestroyed = connect(value, &QObject::destroyed, this, [this] {
        detach();
        m_item = nullptr;
        emit itemChanged();
    });
    update();
    emit itemChanged();
}
void SelectionRegistration::setKind(ViewState::SelectionItemKind value)
{
    if (m_kind == value) return;
    m_kind = value; update(); emit metadataChanged();
}
void SelectionRegistration::setOffset(int value)
{
    if (m_offset == value) return;
    m_offset = value; update(); emit metadataChanged();
}
void SelectionRegistration::setLength(int value)
{
    if (m_length == value) return;
    m_length = value; update(); emit metadataChanged();
}
void SelectionRegistration::setImageLink(const QString &value)
{
    if (m_imageLink == value) return;
    m_imageLink = value; update(); emit metadataChanged();
}
void SelectionRegistration::setImageLinked(bool value)
{
    if (m_imageLinked == value) return;
    m_imageLinked = value; update(); emit metadataChanged();
}

void SelectionRegistration::setImageLinkId(int value)
{
    if (m_imageLinkId == value) return;
    m_imageLinkId = value; update(); emit metadataChanged();
}
