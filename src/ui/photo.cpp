#include "photo.h"
#include "SkinBank.h"
#include "carditem.h"
#include "client.h"
#include "clientplayer.h"
#include "engine.h"
#include "hegemonyrolecombobox.h"
#include "pixmapanimation.h"
#include "rolecombobox.h"
#include "roomscene.h"
#include "settings.h"
#include "standard.h"

#include <QDrag>
#include <QFile>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTimer>

using namespace QSanProtocol;

// skins that remain to be extracted:
// equips
// mark
// emotions
// hp
// seatNumber
// death logo
// kingdom mask and kingdom icon (decouple from player)
// make layers (drawing order) configurable

Photo::Photo()
    : PlayerCardContainer()
{
    _m_mainFrame = nullptr;
    m_player = nullptr;
    _m_focusFrame = nullptr;
    _m_onlineStatusItem = nullptr;
    _m_layout = &G_PHOTO_LAYOUT;
    _m_frameType = S_FRAME_NO_FRAME;
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    setTransform(QTransform::fromTranslate(-G_PHOTO_LAYOUT.m_normalWidth / 2, -G_PHOTO_LAYOUT.m_normalHeight / 2), true);
    _m_skillNameItem = new QGraphicsPixmapItem(_m_groupMain);

    emotion_item = new Sprite(_m_groupMain);

    _createControls();
}

void Photo::refresh()
{
    PlayerCardContainer::refresh();
    if (!m_player)
        return;
    QString state_str = m_player->getState();
    if (!state_str.isEmpty() && state_str != "online") {
        QRect rect = G_PHOTO_LAYOUT.m_onlineStatusArea;
        QImage image(rect.size(), QImage::Format_ARGB32);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.fillRect(QRect(0, 0, rect.width(), rect.height()), G_PHOTO_LAYOUT.m_onlineStatusBgColor);
        G_PHOTO_LAYOUT.m_onlineStatusFont.paintText(&painter, QRect(QPoint(0, 0), rect.size()), Qt::AlignCenter, Sanguosha->translate(state_str));
        QPixmap pixmap = QPixmap::fromImage(image);
        _paintPixmap(_m_onlineStatusItem, rect, pixmap, _m_groupMain);
        _layBetween(_m_onlineStatusItem, _m_mainFrame, _m_chainIcon);
        if (!_m_onlineStatusItem->isVisible())
            _m_onlineStatusItem->show();
    } else if (_m_onlineStatusItem != nullptr && state_str == "online")
        _m_onlineStatusItem->hide();
}

QRectF Photo::boundingRect() const
{
    return QRect(0, 0, G_PHOTO_LAYOUT.m_normalWidth, G_PHOTO_LAYOUT.m_normalHeight);
}

void Photo::repaintAll()
{
    resetTransform();
    setTransform(QTransform::fromTranslate(-G_PHOTO_LAYOUT.m_normalWidth / 2, -G_PHOTO_LAYOUT.m_normalHeight / 2), true);
    _paintPixmap(_m_mainFrame, G_PHOTO_LAYOUT.m_mainFrameArea, QSanRoomSkin::S_SKIN_KEY_MAINFRAME);
    setFrame(_m_frameType);
    hideSkillName(); // @todo: currently we don't adjust skillName's position for simplicity,
    // consider repainting it instead of hiding it in the future.
    PlayerCardContainer::repaintAll();
    refresh();
}

void Photo::_adjustComponentZValues(bool killed)
{
    PlayerCardContainer::_adjustComponentZValues(killed);
    _layBetween(_m_mainFrame, _m_faceTurnedIcon, _m_equipRegions[3]);
    if (isHegemonyGameMode(ServerInfo.GameMode)) {
        _layBetween(emotion_item, _m_chainIcon, _m_hegemonyroleComboBox);
        _layBetween(_m_skillNameItem, _m_chainIcon, _m_hegemonyroleComboBox);
    } else {
        _layBetween(emotion_item, _m_chainIcon, _m_roleComboBox);
        _layBetween(_m_skillNameItem, _m_chainIcon, _m_roleComboBox);
    }

    _m_progressBarItem->setZValue(_m_groupMain->zValue() + 1);
}

void Photo::setEmotion(const QString &emotion, bool permanent)
{
    if (emotion == ".") {
        hideEmotion();
        return;
    }

    QString path = QString("image/system/emotion/%1.png").arg(emotion);
    if (QFile::exists(path)) {
        QPixmap pixmap = QPixmap(path);
        emotion_item->setPixmap(pixmap);
        emotion_item->setPos((G_PHOTO_LAYOUT.m_normalWidth - pixmap.width()) / 2, (G_PHOTO_LAYOUT.m_normalHeight - pixmap.height()) / 2);

        if (isHegemonyGameMode(ServerInfo.GameMode))
            _layBetween(emotion_item, _m_chainIcon, _m_hegemonyroleComboBox);
        else
            _layBetween(emotion_item, _m_chainIcon, _m_roleComboBox);

        QPropertyAnimation *appear = new QPropertyAnimation(emotion_item, "opacity");
        appear->setStartValue(0.0);
        if (permanent) {
            appear->setEndValue(1.0);
            appear->setDuration(500);
        } else {
            appear->setKeyValueAt(0.25, 1.0);
            appear->setKeyValueAt(0.75, 1.0);
            appear->setEndValue(0.0);
            appear->setDuration(2000);
        }
        appear->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        PixmapAnimation::GetPixmapAnimation(this, emotion);
    }
}

void Photo::tremble()
{
    QPropertyAnimation *vibrate = new QPropertyAnimation(this, "x");
    static qreal offset = 20;

    vibrate->setKeyValueAt(0.5, x() - offset);
    vibrate->setEndValue(x());

    vibrate->setEasingCurve(QEasingCurve::OutInBounce);

    vibrate->start(QAbstractAnimation::DeleteWhenStopped);
}

void Photo::hideEmotion()
{
    QPropertyAnimation *disappear = new QPropertyAnimation(emotion_item, "opacity");
    disappear->setStartValue(1.0);
    disappear->setEndValue(0.0);
    disappear->setDuration(500);
    disappear->start(QAbstractAnimation::DeleteWhenStopped);
}

const ClientPlayer *Photo::getPlayer() const
{
    return m_player;
}

void Photo::speak(const QString &)
{
    //@@todo:complete it
}

QList<CardItem *> Photo::removeCardItems(const QList<int> &card_ids, Player::Place place)
{
    QList<CardItem *> result;
    if (place == Player::PlaceHand || place == Player::PlaceSpecial) {
        result = _createCards(card_ids);
        updateHandcardNum();
    } else if (place == Player::PlaceEquip) {
        result = removeEquips(card_ids);
    } else if (place == Player::PlaceDelayedTrick) {
        result = removeDelayedTricks(card_ids);
    }

    // if it is just one card from equip or judge area, we'd like to keep them
    // to start from the equip/trick icon.
    if (result.size() > 1 || (place != Player::PlaceEquip && place != Player::PlaceDelayedTrick))
        _disperseCards(result, G_PHOTO_LAYOUT.m_cardMoveRegion, Qt::AlignCenter, false, false);

    update();
    return result;
}

bool Photo::_addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo)
{
    _disperseCards(card_items, G_PHOTO_LAYOUT.m_cardMoveRegion, Qt::AlignCenter, true, false);
    double homeOpacity = 0.0;
    bool destroy = true;

    Player::Place place = moveInfo.to_place;

    foreach (CardItem *card_item, card_items)
        card_item->setHomeOpacity(homeOpacity);
    if (place == Player::PlaceEquip) {
        addEquips(card_items);
        destroy = false;
    } else if (place == Player::PlaceDelayedTrick) {
        addDelayedTricks(card_items);
        destroy = false;
    } else if (place == Player::PlaceHand) {
        updateHandcardNum();
    }
    return destroy;
}

void Photo::setFrame(FrameType type)
{
    _m_frameType = type;
    if (type == S_FRAME_NO_FRAME) {
        if (_m_focusFrame) {
            if (_m_saveMeIcon && _m_saveMeIcon->isVisible())
                setFrame(S_FRAME_SOS);
            else if (m_player->getPhase() != Player::NotActive)
                setFrame(S_FRAME_PLAYING);
            else
                _m_focusFrame->hide();
        }
    } else {
        _paintPixmap(_m_focusFrame, G_PHOTO_LAYOUT.m_focusFrameArea, _getPixmap(QSanRoomSkin::S_SKIN_KEY_FOCUS_FRAME, QString::number(type)), _m_groupMain);
        _layBetween(_m_focusFrame, _m_avatarArea, _m_mainFrame);
        _m_focusFrame->show();
    }
    update();
}

void Photo::updatePhase()
{
    PlayerCardContainer::updatePhase();
    if (m_player->getPhase() != Player::NotActive)
        setFrame(S_FRAME_PLAYING);
    else
        setFrame(S_FRAME_NO_FRAME);
}

void Photo::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
}

QGraphicsItem *Photo::getMouseClickReceiver()
{
    return this;
}

QPointF Photo::getHeroSkinContainerPosition() const
{
    QRectF tableRect = RoomSceneInstance->getTableRect();

    QRectF photoRect = sceneBoundingRect();
    int photoWidth = photoRect.width();
    int photoHeight = photoRect.height();

    /*QRectF heroSkinContainerRect = (m_primaryHeroSkinContainer != NULL)
        ? m_primaryHeroSkinContainer->boundingRect()
        : m_secondaryHeroSkinContainer->boundingRect();*/
    QRectF heroSkinContainerRect = m_primaryHeroSkinContainer->boundingRect();
    int heroSkinContainerWidth = heroSkinContainerRect.width();
    int heroSkinContainerHeight = heroSkinContainerRect.height();

    const int tablePadding = 5;

    if (photoRect.right() <= tableRect.left()) {
        QPointF result(photoRect.right() + 10, photoRect.top() - ((heroSkinContainerHeight - photoHeight) / 2));

        int yBottomDiff = (result.y() + heroSkinContainerHeight) - (tableRect.bottom() + tablePadding);
        if (yBottomDiff > 0) {
            result.setY(result.y() - yBottomDiff);
        } else if (result.y() < tableRect.top() - tablePadding) {
            result.setY(tableRect.top() - tablePadding);
        }

        return result;
    } else if (photoRect.bottom() <= tableRect.top()) {
        QPointF result(photoRect.left() - ((heroSkinContainerWidth - photoWidth) / 2), photoRect.bottom() + 10);

        int xRightDiff = (result.x() + heroSkinContainerWidth) - (tableRect.right() + tablePadding);
        if (xRightDiff > 0) {
            result.setX(result.x() - xRightDiff);
        } else if (result.x() < tableRect.left() - tablePadding) {
            result.setX(tableRect.left() - tablePadding);
        }

        return result;
    } else {
        QPointF result(photoRect.left() - heroSkinContainerWidth - 10, photoRect.top() - ((heroSkinContainerHeight - photoHeight) / 2));

        int yBottomDiff = (result.y() + heroSkinContainerHeight) - (tableRect.bottom() + tablePadding);
        if (yBottomDiff > 0) {
            result.setY(result.y() - yBottomDiff);
        } else if (result.y() < tableRect.top() - tablePadding) {
            result.setY(tableRect.top() - tablePadding);
        }

        return result;
    }
}

QPropertyAnimation *Photo::initializeBlurEffect(GraphicsPixmapHoverItem *icon)
{
    QGraphicsBlurEffect *effect = new QGraphicsBlurEffect;
    effect->setBlurHints(QGraphicsBlurEffect::AnimationHint);
    effect->setBlurRadius(0);
    icon->setGraphicsEffect(effect);

    QPropertyAnimation *animation = new QPropertyAnimation(effect, "blurRadius");
    animation->setEasingCurve(QEasingCurve::OutInBounce);
    animation->setDuration(2000);
    animation->setStartValue(0);
    animation->setEndValue(5);
    return animation;
}

void Photo::_initializeRemovedEffect()
{
    _blurEffect = new QParallelAnimationGroup(this);
    _blurEffect->addAnimation(initializeBlurEffect(_m_avatarIcon));
    _blurEffect->addAnimation(initializeBlurEffect(_m_smallAvatarIcon));
}

void Photo::_createBattleArrayAnimations()
{
    QStringList kingdoms = Sanguosha->getKingdoms();
    kingdoms.removeAll("god");
    foreach (const QString &kingdom, kingdoms) {
        _m_frameBorders[kingdom] = new PixmapAnimation();
        _m_frameBorders[kingdom]->setZValue(30000);
        _m_roleBorders[kingdom] = new PixmapAnimation();
        _m_roleBorders[kingdom]->setZValue(30000);
        _m_frameBorders[kingdom]->setParentItem(_getFocusFrameParent());
        _m_roleBorders[kingdom]->setParentItem(_getRoleComboBoxParent());
        _m_frameBorders[kingdom]->setSize(QSize(G_PHOTO_LAYOUT.m_normalWidth * 1.2, G_PHOTO_LAYOUT.m_normalHeight * 1.2));
        _m_frameBorders[kingdom]->setPath(QString("image/kingdom/battlearray/small/%1/").arg(kingdom));
        _m_roleBorders[kingdom]->setPath(QString("image/kingdom/battlearray/roles/%1/").arg(kingdom));
        _m_frameBorders[kingdom]->setPlayTime(2000);
        _m_roleBorders[kingdom]->setPlayTime(2000);
        if (!_m_frameBorders[kingdom]->valid()) {
            delete _m_frameBorders[kingdom];
            delete _m_roleBorders[kingdom];
            _m_frameBorders[kingdom] = NULL;
            _m_roleBorders[kingdom] = NULL;
            continue;
        }
        _m_frameBorders[kingdom]->setPos(-G_PHOTO_LAYOUT.m_normalWidth * 0.1, -G_PHOTO_LAYOUT.m_normalHeight * 0.1);
        double scale = G_ROOM_LAYOUT.scale;
        QPixmap pix;
        pix.load("image/system/roles/careerist.png");
        int w = pix.width() * scale;
        int h = pix.height() * scale;
        _m_roleBorders[kingdom]->setPos(G_PHOTO_LAYOUT.m_roleComboBoxPos
                                        - QPoint((_m_roleBorders[kingdom]->boundingRect().width() - w) / 2, (_m_roleBorders[kingdom]->boundingRect().height() - h / 2) / 2));
        _m_frameBorders[kingdom]->setHideonStop(true);
        _m_roleBorders[kingdom]->setHideonStop(true);
        _m_frameBorders[kingdom]->hide();
        _m_roleBorders[kingdom]->hide();
    }
}

void Photo::playBattleArrayAnimations()
{
    QString kingdom = getPlayer()->getKingdom();
    _m_frameBorders[kingdom]->show();
    _m_frameBorders[kingdom]->start(true, 30);
    _m_roleBorders[kingdom]->preStart();
}
