#include "GenericCardContainerUI.h"
#include "clientplayer.h"
#include "engine.h"
#include "graphicspixmaphoveritem.h"
#include "roomscene.h"
#include "standard.h"

#include <QGraphicsColorizeEffect>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QLabel>
#include <QMenu>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTextDocument>

using namespace QSanProtocol;

QList<CardItem *> GenericCardContainer::cloneCardItems(QList<int> card_ids)
{
    return _createCards(card_ids);
}

QList<CardItem *> GenericCardContainer::_createCards(QList<int> card_ids)
{
    QList<CardItem *> result;
    foreach (int card_id, card_ids) {
        CardItem *item = _createCard(card_id);
        result.append(item);
    }
    return result;
}

CardItem *GenericCardContainer::_createCard(int card_id)
{
    const Card *card = Sanguosha->getCard(card_id);
    CardItem *item = new CardItem(card);
    item->setOpacity(0.0);
    item->setParentItem(this);
    return item;
}

void GenericCardContainer::_destroyCard()
{
    //CardItem *card = qobject_cast<CardItem *>(sender());
    CardItem *card = (CardItem *)sender();
    if (card != nullptr) {
        card->setVisible(false);
        card->deleteLater();
    }
}

bool GenericCardContainer::_horizontalPosLessThan(const CardItem *card1, const CardItem *card2)
{
    return (card1->x() < card2->x());
}

void GenericCardContainer::_disperseCards(QList<CardItem *> &cards, QRectF fillRegion, Qt::Alignment align, bool useHomePos, bool keepOrder)
{
    int numCards = cards.size();
    if (numCards == 0)
        return;
    if (!keepOrder)
        std::sort(cards.begin(), cards.end(), GenericCardContainer::_horizontalPosLessThan);
    double maxWidth = fillRegion.width();
    int cardWidth = G_COMMON_LAYOUT.m_cardNormalWidth;
    double step = qMin((double)cardWidth, (maxWidth - cardWidth) / (numCards - 1));
    align &= Qt::AlignHorizontal_Mask;
    for (int i = 0; i < numCards; i++) {
        CardItem *card = cards[i];
        double newX = 0;
        if (align == Qt::AlignHCenter)
            newX = fillRegion.center().x() + step * (i - (numCards - 1) / 2.0);
        else if (align == Qt::AlignLeft)
            newX = fillRegion.left() + step * i + card->boundingRect().width() / 2.0;
        else if (align == Qt::AlignRight)
            newX = fillRegion.right() + step * (i - numCards) + card->boundingRect().width() / 2.0;
        else
            continue;
        QPointF newPos = QPointF(newX, fillRegion.center().y());
        if (useHomePos)
            card->setHomePos(newPos);
        else
            card->setPos(newPos);
        card->setZValue(_m_highestZ++);
    }
}

void GenericCardContainer::onAnimationFinished()
{
    /*QParallelAnimationGroup *animationGroup = qobject_cast<QParallelAnimationGroup *>(sender());
    if (animationGroup) {
        animationGroup->clear();
        animationGroup->deleteLater();
    }*/
}

void GenericCardContainer::_doUpdate()
{
    update();
}

void GenericCardContainer::_playMoveCardsAnimation(QList<CardItem *> &cards, bool destroyCards)
{
    QParallelAnimationGroup *animation = new QParallelAnimationGroup(this);
    foreach (CardItem *card_item, cards) {
        if (destroyCards)
            connect(card_item, SIGNAL(movement_animation_finished()), this, SLOT(_destroyCard()));
        animation->addAnimation(card_item->getGoBackAnimation(true));
    }

    connect(animation, SIGNAL(finished()), this, SLOT(_doUpdate()));
    //if (!destroyCards) {
    connect(animation, SIGNAL(finished()), this, SLOT(onAnimationFinished()));
    //}

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void GenericCardContainer::addCardItems(QList<CardItem *> &card_items, const CardsMoveStruct &moveInfo)
{
    foreach (CardItem *card_item, card_items) {
        card_item->setPos(mapFromScene(card_item->scenePos()));
        card_item->setParentItem(this);
    }
    bool destroy = _addCardItems(card_items, moveInfo);
    _playMoveCardsAnimation(card_items, destroy);
}

void PlayerCardContainer::_paintPixmap(QGraphicsPixmapItem *&item, const QRect &rect, const QString &key)
{
    _paintPixmap(item, rect, _getPixmap(key));
}

void PlayerCardContainer::_paintPixmap(QGraphicsPixmapItem *&item, const QRect &rect, const QString &key, QGraphicsItem *parent)
{
    _paintPixmap(item, rect, _getPixmap(key), parent);
}

void PlayerCardContainer::_paintPixmap(QGraphicsPixmapItem *&item, const QRect &rect, const QPixmap &pixmap)
{
    _paintPixmap(item, rect, pixmap, _m_groupMain);
}

QPixmap PlayerCardContainer::_getPixmap(const QString &key, const QString &sArg, bool cache)
{
    Q_ASSERT(key.contains("%1"));
    if (key.contains("%2")) {
        QString rKey = key.arg(getResourceKeyName()).arg(sArg);

        if (G_ROOM_SKIN.isImageKeyDefined(rKey))
            return G_ROOM_SKIN.getPixmap(rKey, QString(), cache); // first try "%1key%2 = ...", %1 = "photo", %2 = sArg

        rKey = key.arg(getResourceKeyName());
        return G_ROOM_SKIN.getPixmap(rKey, sArg, cache); // then try "%1key = ..."
    } else {
        return G_ROOM_SKIN.getPixmap(key, sArg, cache); // finally, try "key = ..."
    }
}

QPixmap PlayerCardContainer::_getPixmap(const QString &key, bool cache)
{
    if (key.contains("%1") && G_ROOM_SKIN.isImageKeyDefined(key.arg(getResourceKeyName())))
        return G_ROOM_SKIN.getPixmap(key.arg(getResourceKeyName()), QString(), cache);
    else
        return G_ROOM_SKIN.getPixmap(key, QString(), cache);
}

void PlayerCardContainer::_paintPixmap(QGraphicsPixmapItem *&item, const QRect &rect, const QPixmap &pixmap, QGraphicsItem *parent)
{
    if (item == nullptr) {
        item = new QGraphicsPixmapItem(parent);
        item->setTransformationMode(Qt::SmoothTransformation);
    }
    item->setPos(rect.x(), rect.y());
    if (pixmap.size() == rect.size())
        item->setPixmap(pixmap);
    else
        item->setPixmap(pixmap.scaled(rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    item->setParentItem(parent);
}

void PlayerCardContainer::_clearPixmap(QGraphicsPixmapItem *pixmap)
{
    QPixmap dummy;
    if (pixmap == nullptr)
        return;
    pixmap->setPixmap(dummy);
    pixmap->hide();
}

void PlayerCardContainer::hideProgressBar()
{
    _m_progressBar->hide();
}

void PlayerCardContainer::showProgressBar(Countdown countdown)
{
    _m_progressBar->setCountdown(countdown);
    _m_progressBar->show();
}

void PlayerCardContainer::updateAvatar()
{
    if (_m_avatarIcon == nullptr) {
        _m_avatarIcon = new GraphicsPixmapHoverItem(this, _getAvatarParent());
        _m_avatarIcon->setTransformationMode(Qt::SmoothTransformation);

        // _m_avatarIcon->setFlag(QGraphicsItem::ItemStacksBehindParent);
    }

    const General *general = nullptr;
    if (m_player) {
        general = m_player->getAvatarGeneral();
        _m_layout->m_screenNameFont.paintText(_m_screenNameItem, _m_layout->m_screenNameArea, Qt::AlignCenter, m_player->screenName());
        if (ServerInfo.Enable2ndGeneral && getPlayer() == Self)
            _m_layout->m_screenNameFont.paintText(_m_screenNameItem, _m_layout->m_screenNameAreaDouble, Qt::AlignCenter, m_player->screenName());
    } else {
        _m_layout->m_screenNameFont.paintText(_m_screenNameItem, _m_layout->m_screenNameArea, Qt::AlignCenter, QString());
    }

    if (general != nullptr) {
        _m_avatarArea->setToolTip(m_player->getSkillDescription(true, "head"));
        QString name = general->objectName();
        if (m_player != nullptr && m_player->getMark("duozhi") > 0)
            name = "yingyingguai";

        QPixmap avatarIcon = _getAvatarIcon(name);
        QGraphicsPixmapItem *avatarIconTmp = _m_avatarIcon;
        QRect avatarArea = _m_layout->m_avatarArea; //(ServerInfo.Enable2ndGeneral && this->getPlayer() == Self) ? _m_layout->m_avatarAreaDouble : _m_layout->m_avatarArea;
        _paintPixmap(avatarIconTmp, avatarArea, avatarIcon, _getAvatarParent());
        // this is just avatar general, perhaps game has not started yet.

        if (m_player->getGeneral() != nullptr) {
            QString kingdom = m_player->getKingdom();
            if (!isHegemonyGameMode(ServerInfo.GameMode)) { // && m_player->getGeneral2()  == NULL
                _paintPixmap(_m_kingdomIcon, _m_layout->m_kingdomIconArea, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_KINGDOM_ICON, kingdom), _getAvatarParent());
                if (ServerInfo.Enable2ndGeneral && getPlayer() == Self)
                    _paintPixmap(_m_kingdomIcon, _m_layout->m_kingdomIconAreaDouble, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_KINGDOM_ICON, kingdom), _getAvatarParent());

                if (!ServerInfo.Enable2ndGeneral)
                    _paintPixmap(_m_kingdomColorMaskIcon, _m_layout->m_kingdomMaskArea, G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_KINGDOM_COLOR_MASK, kingdom),
                                 _getAvatarParent());
            }
            //@todo
            //we want this mask to start at zero piont of logbox width,
            //and keep the height to equal with the diff between middleFrame and rightFrame
            //if (!isHegemonyGameMode(ServerInfo.GameMode)) {
            if (ServerInfo.Enable2ndGeneral)
                _paintPixmap(_m_dashboardKingdomColorMaskIcon, _m_layout->m_dashboardPrimaryKingdomMaskArea,
                             G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_DASHBOARD_KINGDOM_COLOR_MASK, kingdom), _getAvatarParent());
            else
                _paintPixmap(_m_dashboardKingdomColorMaskIcon, _m_layout->m_dashboardKingdomMaskArea,
                             G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_DASHBOARD_KINGDOM_COLOR_MASK, kingdom), _getAvatarParent());
            //}

            _paintPixmap(_m_handCardBg, _m_layout->m_handCardArea, _getPixmap(QSanRoomSkin::S_SKIN_KEY_HANDCARDNUM, kingdom), _getAvatarParent());
            QString name = Sanguosha->translate("&" + general->objectName());
            if (name.startsWith("&"))
                name = Sanguosha->translate(general->objectName());
            if (ServerInfo.Enable2ndGeneral && getPlayer() == Self)
                _m_layout->m_avatarNameFont.paintText(_m_avatarNameItem, _m_layout->m_headAvatarNameArea, Qt::AlignLeft | Qt::AlignJustify, name);
            else
                _m_layout->m_avatarNameFont.paintText(_m_avatarNameItem, _m_layout->m_avatarNameArea, Qt::AlignLeft | Qt::AlignJustify, name);
        } else {
            _paintPixmap(_m_handCardBg, _m_layout->m_handCardArea, _getPixmap(QSanRoomSkin::S_SKIN_KEY_HANDCARDNUM, QString(QSanRoomSkin::S_SKIN_KEY_DEFAULT_SECOND)),
                         _getAvatarParent());
        }
    } else {
        QGraphicsPixmapItem *avatarIconTmp = _m_avatarIcon;
        QRect avatarArea = _m_layout->m_avatarArea; //(ServerInfo.Enable2ndGeneral && this->getPlayer() == Self) ? _m_layout->m_avatarAreaDouble : _m_layout->m_avatarArea;
        _paintPixmap(avatarIconTmp, avatarArea, QSanRoomSkin::S_SKIN_KEY_BLANK_GENERAL, _getAvatarParent());
        _clearPixmap(_m_kingdomColorMaskIcon);
        _clearPixmap(_m_dashboardKingdomColorMaskIcon);
        _clearPixmap(_m_kingdomIcon);
        _paintPixmap(_m_handCardBg, _m_layout->m_handCardArea, _getPixmap(QSanRoomSkin::S_SKIN_KEY_HANDCARDNUM, QString(QSanRoomSkin::S_SKIN_KEY_DEFAULT_SECOND)),
                     _getAvatarParent());
        _m_avatarArea->setToolTip(QString());
    }
    _m_avatarIcon->show();
    _adjustComponentZValues();
}

QPixmap PlayerCardContainer::paintByMask(QPixmap &source)
{
    QPixmap tmp = G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_GENERAL_CIRCLE_MASK, QString::number(_m_layout->m_circleImageSize), true);
    if (tmp.height() <= 1 && tmp.width() <= 1)
        return source;
    QPainter p(&tmp);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.drawPixmap(0, 0, _m_layout->m_smallAvatarArea.width(), _m_layout->m_smallAvatarArea.height(), source);
    return tmp;
}

void PlayerCardContainer::updateSmallAvatar()
{
    updateAvatar();
    if (_m_smallAvatarIcon == nullptr) {
        _m_smallAvatarIcon = new GraphicsPixmapHoverItem(this, _getAvatarParent());
        _m_smallAvatarIcon->setTransformationMode(Qt::SmoothTransformation);
        //_m_smallAvatarIcon->setFlag(QGraphicsItem::ItemStacksBehindParent);
    }

    const General *general = nullptr;
    bool fake_general = false;
    if (m_player)
        general = m_player->getGeneral2();
    if (general == nullptr && ServerInfo.Enable2ndGeneral && this->getPlayer() == Self) {
        general = Sanguosha->getGeneral("reimu");
        fake_general = true;
    }

    if (general != nullptr) {
        QString g_name = general->objectName();
        if (m_player != nullptr && m_player->getMark("duozhi") > 0)
            g_name = "yingyingguai";
        QPixmap smallAvatarIcon = G_ROOM_SKIN.getGeneralPixmap(g_name, QSanRoomSkin::GeneralIconSize(_m_layout->m_smallAvatarSize));
        smallAvatarIcon = paintByMask(smallAvatarIcon);
        QGraphicsPixmapItem *smallAvatarIconTmp = _m_smallAvatarIcon;
        _paintPixmap(smallAvatarIconTmp, _m_layout->m_smallAvatarArea, smallAvatarIcon, _getAvatarParent());
        _paintPixmap(_m_circleItem, _m_layout->m_circleArea, QString(QSanRoomSkin::S_SKIN_KEY_GENERAL_CIRCLE_IMAGE).arg(_m_layout->m_circleImageSize), _getAvatarParent());
        if (!fake_general) {
            _m_smallAvatarArea->setToolTip(m_player->getSkillDescription(true, "deputy"));
            //if (!isHegemonyGameMode(ServerInfo.GameMode)) {
            QString kingdom = m_player->getKingdom(); //(m_player->getGeneral()) ? m_player->getGeneral()->getKingdom() : "wai";
            if (isHegemonyGameMode(ServerInfo.GameMode))
                kingdom = general->getKingdom();

            _paintPixmap(_m_dashboardSecondaryKingdomColorMaskIcon, _m_layout->m_dashboardSecondaryKingdomMaskArea,
                         G_ROOM_SKIN.getPixmap(QSanRoomSkin::S_SKIN_KEY_DASHBOARD_KINGDOM_COLOR_MASK, kingdom), _getAvatarParent());
            //}
        }

        QString name = Sanguosha->translate("&" + general->objectName());
        if (name.startsWith("&"))
            name = Sanguosha->translate(general->objectName());

        if (!fake_general)
            _m_layout->m_smallAvatarNameFont.paintText(_m_smallAvatarNameItem, _m_layout->m_smallAvatarNameArea, Qt::AlignLeft | Qt::AlignJustify, name);
        _m_smallAvatarIcon->show();
    } else {
        _clearPixmap(_m_smallAvatarIcon);
        _clearPixmap(_m_circleItem);
        _clearPixmap(_m_dashboardSecondaryKingdomColorMaskIcon);
        _m_layout->m_smallAvatarNameFont.paintText(_m_smallAvatarNameItem, _m_layout->m_smallAvatarNameArea, Qt::AlignLeft | Qt::AlignJustify, QString());
        _m_smallAvatarArea->setToolTip(QString());
    }
    _adjustComponentZValues();
}

void PlayerCardContainer::updatePhase()
{
    if (!m_player || !m_player->isAlive())
        _clearPixmap(_m_phaseIcon);
    else if (m_player->getPhase() != Player::NotActive) {
        if (m_player->getPhase() == Player::PhaseNone)
            return;
        int index = static_cast<int>(m_player->getPhase());
        QRect phaseArea = _m_layout->m_phaseArea.getTranslatedRect(_getPhaseParent()->boundingRect().toRect());
        _paintPixmap(_m_phaseIcon, phaseArea, _getPixmap(QSanRoomSkin::S_SKIN_KEY_PHASE, QString::number(index), true), _getPhaseParent());
        _m_phaseIcon->show();
    } else {
        if (_m_progressBar)
            _m_progressBar->hide();
        if (_m_phaseIcon)
            _m_phaseIcon->hide();
    }
}

void PlayerCardContainer::updateHp()
{
    Q_ASSERT(_m_hpBox && _m_saveMeIcon && m_player);
    if (!m_player->hasSkill("banling")) {
        _m_hpBox->setHp(m_player->getHp(), m_player->dyingThreshold());
        _m_hpBox->setMaxHp(m_player->getMaxHp());
        _m_hpBox->update();
        _m_sub_hpBox->setHp(0);
        _m_sub_hpBox->setMaxHp(0);
        _m_sub_hpBox->update();
    } else {
        _m_hpBox->setHp(m_player->getRenHp(), m_player->dyingThreshold());
        _m_hpBox->setMaxHp(m_player->getMaxHp());
        _m_hpBox->update();
        _m_sub_hpBox->setHp(m_player->getLingHp(), m_player->dyingThreshold());
        _m_sub_hpBox->setMaxHp(m_player->getMaxHp());
        _m_sub_hpBox->update();
    }

    //if (m_player->getHp() > 0 || m_player->getMaxHp() == 0)
    if (m_player->getHp() >= m_player->dyingThreshold() || m_player->getMaxHp() < m_player->dyingThreshold())
        _m_saveMeIcon->setVisible(false);
}

void PlayerCardContainer::updatePile(const QString &pile_name)
{
    //ClientPlayer *player = (ClientPlayer *)sender();
    ClientPlayer *player = qobject_cast<ClientPlayer *>(sender());
    if (!player)
        player = m_player;
    if (!player)
        return;

    QString treasure_name;
    if (player->getTreasure())
        treasure_name = player->getTreasure()->objectName();

    QList<int> pile;
    if (pile_name == "shown_card")
        pile = player->getShownHandcards();
    else if (pile_name == "huashencard") {
        int n = player->getHiddenGenerals().length();
        if (n == 0)
            return;
        for (int i = 0; i < n; i++) {
            pile.append(i + 1);
        }
    } else
        pile = player->getPile(pile_name);

    QString shownpilename = RoomSceneInstance->getCurrentShownPileName();
    if (!shownpilename.isEmpty() && shownpilename == pile_name)
        hidePile();

    if (pile.size() == 0) {
        if (_m_privatePiles.contains(pile_name)) {
            delete _m_privatePiles[pile_name];
            _m_privatePiles[pile_name] = NULL;
            _m_privatePiles.remove(pile_name);
        }
    } else {
        // retrieve menu and create a new pile if necessary
        QPushButton *button = nullptr;
        if (!_m_privatePiles.contains(pile_name)) {
            button = new QPushButton(_m_privatePileArea->widget());
            //button = new QPushButton;
            button->setObjectName(pile_name);
            if (treasure_name == pile_name)
                button->setProperty("treasure", "true");
            else if (pile_name == "shown_card")
                button->setProperty("shown_card", "true");
            else
                button->setProperty("private_pile", "true");
            QGraphicsProxyWidget *button_widget = new QGraphicsProxyWidget(_getPileParent());
            button_widget->setObjectName(pile_name);
            button_widget->setWidget(button);
            _m_privatePiles[pile_name] = button_widget;
        } else {
            //button = (QPushButton *)(_m_privatePiles[pile_name]->widget());
            button = qobject_cast<QPushButton *>((_m_privatePiles[pile_name]->widget()));
            if (button == nullptr)
                qWarning("PlayerCardContainer::updatePile: button == NULL");
        }

        QString text = Sanguosha->translate(pile_name);
        if (pile.length() > 0)
            text.append(QString("(%1)").arg(pile.length()));
        button->setText(text);

        disconnect(button, &QPushButton::pressed, this, &PlayerCardContainer::showPile);
        connect(button, &QPushButton::pressed, this, &PlayerCardContainer::showPile);

        if (pile_name != "huashencard") {
            disconnect(button, &QPushButton::released, this, &PlayerCardContainer::hidePile);
            connect(button, &QPushButton::released, this, &PlayerCardContainer::hidePile);
        }
    }
    //set treasure pile at first
    QPoint start = (ServerInfo.Enable2ndGeneral && getPlayer() == Self) ? _m_layout->m_privatePileStartPosDouble : _m_layout->m_privatePileStartPos;
    QPoint step = _m_layout->m_privatePileStep;
    QSize size = _m_layout->m_privatePileButtonSize;
    QList<QGraphicsProxyWidget *> widgets_t, widgets_p, widgets = _m_privatePiles.values();
    foreach (QGraphicsProxyWidget *widget, widgets) {
        if (widget->objectName() == treasure_name)
            widgets_t << widget;
        else
            widgets_p << widget;
    }
    widgets = widgets_t + widgets_p;
    for (int i = 0; i < widgets.length(); i++) {
        QGraphicsProxyWidget *widget = widgets[i];
        widget->setPos(start + i * step);
        widget->resize(size);
    }
}

void PlayerCardContainer::showPile()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button) {
        const ClientPlayer *player = getPlayer();
        if (!player)
            return;
        QList<int> card_ids = player->getPile(button->objectName());
        if (button->objectName() == "huashencard") {
            if (player == Self)
                RoomSceneInstance->showPile(card_ids, button->objectName(), player);
            else
                return;
        }
        if (card_ids.isEmpty() || card_ids.contains(-1))
            return;
        RoomSceneInstance->showPile(card_ids, button->objectName(), player);
    }
}

void PlayerCardContainer::hidePile()
{
    RoomSceneInstance->hidePile();
}

void PlayerCardContainer::updateDrankState()
{
    if (m_player->getMark("drank") > 0 || m_player->getMark("magic_drank") > 0)
        _m_avatarArea->setBrush(G_PHOTO_LAYOUT.m_drankMaskColor);
    else
        _m_avatarArea->setBrush(Qt::NoBrush);
}

void PlayerCardContainer::updateHandcardNum()
{
    int num = 0;
    if (m_player && m_player->getGeneral())
        num = m_player->getHandcardNum();
    Q_ASSERT(num >= 0);
    _m_layout->m_handCardFont.paintText(_m_handCardNumText, _m_layout->m_handCardArea, Qt::AlignCenter, QString::number(num));
    _m_handCardNumText->setVisible(true);
}

void PlayerCardContainer::updateMarks()
{
    if (!_m_markItem)
        return;
    QRect parentRect = _getMarkParent()->boundingRect().toRect();
    QSize markSize = _m_markItem->boundingRect().size().toSize();
    QRect newRect = _m_layout->m_markTextArea.getTranslatedRect(parentRect, markSize);
    if (_m_layout == &G_PHOTO_LAYOUT) {
        //_m_markItem->setPos(newRect.topLeft());
        if (getFloatingArea().left() < newRect.left() - 20)
            _m_markItem->setPos(newRect.left() - 20, newRect.top());
        else
            _m_markItem->setPos(newRect.topLeft());
    } else {
        if (ServerInfo.Enable2ndGeneral)
            _m_markItem->setPos(newRect.left() - 150, newRect.top() + newRect.height() / 2);
        else
            _m_markItem->setPos(newRect.left(), newRect.top() + newRect.height() / 2);
    }
}

void PlayerCardContainer::updateBrokenEquips()
{
    _updateEquips();
}

void PlayerCardContainer::_updateEquips()
{
    for (int i = 0; i < 5; i++) {
        CardItem *equip = _m_equipCards[i];
        if (equip == nullptr)
            continue;

        const EquipCard *equip_card = qobject_cast<const EquipCard *>(equip->getCard()->getRealCard());
        QPixmap pixmap = _getEquipPixmap(equip_card);
        _m_equipLabel[i]->setPixmap(pixmap);
        /*if (m_player->isBrokenEquip(equip_card->getEffectiveId())) {
            QPalette palette;
            palette.setColor(QPalette::Background, G_PHOTO_LAYOUT.m_drankMaskColor);
            _m_equipLabel[i]->setPalette(palette);
        }*/
        _m_equipRegions[i]->setPos(_m_layout->m_equipAreas[i].topLeft());
    }
}

void PlayerCardContainer::refresh()
{
    if (!m_player || !m_player->getGeneral() || !m_player->isAlive()) {
        _m_faceTurnedIcon->setVisible(false);
        if (_m_faceTurnedIcon2)
            _m_faceTurnedIcon2->setVisible(false);
        _m_chainIcon->setVisible(false);
        _m_actionIcon->setVisible(false);
        _m_saveMeIcon->setVisible(false);
        _m_roleShownIcon->setVisible(false);
        leftDisableShowLock->setVisible(false);
        rightDisableShowLock->setVisible(false);
    } else if (m_player) {
        if (_m_faceTurnedIcon)
            _m_faceTurnedIcon->setVisible(!m_player->faceUp());
        if (_m_faceTurnedIcon2)
            _m_faceTurnedIcon2->setVisible(!m_player->faceUp());
        if (_m_chainIcon)
            _m_chainIcon->setVisible(m_player->isChained());
        if (_m_actionIcon)
            _m_actionIcon->setVisible(m_player->hasFlag("actioned"));
        if (_m_deathIcon && !(ServerInfo.GameMode == "04_1v3" && m_player->getGeneralName() != "yuyuko_1v32"))
            _m_deathIcon->setVisible(m_player->isDead());
        if (leftDisableShowLock)
            leftDisableShowLock->setVisible(!m_player->hasShownGeneral() && !m_player->disableShow(true).isEmpty());
        if (rightDisableShowLock)
            rightDisableShowLock->setVisible(m_player->getGeneral2() && !m_player->hasShownGeneral2() && !m_player->disableShow(false).isEmpty());
    }
    updateHandcardNum();
    _adjustComponentZValues();
}

void PlayerCardContainer::repaintAll()
{
    //if (ServerInfo.Enable2ndGeneral && this->getPlayer() == Self)
    //    _m_avatarArea->setRect(_m_layout->m_avatarAreaDouble);
    //else
    _m_avatarArea->setRect(_m_layout->m_avatarArea);
    _m_smallAvatarArea->setRect(_m_layout->m_smallAvatarArea);

    //stopHeroSkinChangingAnimation();

    updateAvatar();
    updateSmallAvatar();
    updatePhase();
    updateMarks();
    _updateProgressBar();
    _updateDeathIcon();
    _updateEquips();
    updateDelayedTricks();
    //_updatePrivatePilesGeometry();

    if (_m_huashenAnimation != nullptr) {
        startHuaShen(_m_huashenGeneralName, _m_huashenSkillName, _m_huashenGeneral2Name, _m_huashenSkill2Name);
    }

    const char *face_turned_mask = isHegemonyGameMode(ServerInfo.GameMode) ? QSanRoomSkin::S_SKIN_KEY_FACETURNEDMASK_HEGEMONY : QSanRoomSkin::S_SKIN_KEY_FACETURNEDMASK;
    if (this->getPlayer() == Self) {
        _paintPixmap(_m_faceTurnedIcon, _m_layout->m_avatarArea, face_turned_mask, _getAvatarParent());
        if (ServerInfo.Enable2ndGeneral)
            _paintPixmap(_m_faceTurnedIcon2, _m_layout->m_smallAvatarArea, face_turned_mask, _getAvatarParent());
    } else
        _paintPixmap(_m_faceTurnedIcon, _m_layout->m_avatarArea, face_turned_mask, _getAvatarParent());

    _paintPixmap(_m_chainIcon, _m_layout->m_chainedIconRegion, QSanRoomSkin::S_SKIN_KEY_CHAIN, _getAvatarParent());
    _paintPixmap(_m_saveMeIcon, _m_layout->m_saveMeIconRegion, QSanRoomSkin::S_SKIN_KEY_SAVE_ME_ICON, _getAvatarParent());
    _paintPixmap(_m_actionIcon, _m_layout->m_actionedIconRegion, QSanRoomSkin::S_SKIN_KEY_ACTIONED_ICON, _getAvatarParent());
    _paintPixmap(_m_roleShownIcon, _m_layout->m_roleShownArea, QSanRoomSkin::S_SKIN_KEY_ROLE_SHOWN, _getAvatarParent());
    if (nullptr != m_changePrimaryHeroSKinBtn) {
        m_changePrimaryHeroSKinBtn->setPos(_m_layout->m_changePrimaryHeroSkinBtnPos);
    }
    if (nullptr != m_changeSecondaryHeroSkinBtn) {
        m_changeSecondaryHeroSkinBtn->setPos(_m_layout->m_changeSecondaryHeroSkinBtnPos);
    }

    if (_m_seatItem != nullptr) {
        _paintPixmap(_m_seatItem, _m_layout->m_seatIconRegion, _getPixmap(QSanRoomSkin::S_SKIN_KEY_SEAT_NUMBER, QString::number(m_player->getInitialSeat())), _getAvatarParent());
        if (ServerInfo.Enable2ndGeneral && getPlayer() == Self)
            _paintPixmap(_m_seatItem, _m_layout->m_seatIconRegionDouble, _getPixmap(QSanRoomSkin::S_SKIN_KEY_SEAT_NUMBER, QString::number(m_player->getInitialSeat())),
                         _getAvatarParent());
    }

    if (!isHegemonyGameMode(ServerInfo.GameMode) && _m_roleComboBox != nullptr)
        _m_roleComboBox->setPos(_m_layout->m_roleComboBoxPos);

    if (isHegemonyGameMode(ServerInfo.GameMode) && _m_hegemonyroleComboBox != nullptr)
        _m_hegemonyroleComboBox->setPos(_m_layout->m_roleComboBoxPos);

    _m_hpBox->setIconSize(_m_layout->m_magatamaSize);
    _m_hpBox->setOrientation(_m_layout->m_magatamasHorizontal ? Qt::Horizontal : Qt::Vertical);
    _m_hpBox->setBackgroundVisible(_m_layout->m_magatamasBgVisible);
    _m_hpBox->setAnchorEnable(true);
    QPoint magatamas_anchor = (ServerInfo.Enable2ndGeneral && getPlayer() == Self) ? _m_layout->m_magatamasAnchorDouble : _m_layout->m_magatamasAnchor;
    _m_hpBox->setAnchor(magatamas_anchor, _m_layout->m_magatamasAlign);
    _m_hpBox->setImageArea(_m_layout->m_magatamaImageArea);
    _m_hpBox->update();

    _m_sub_hpBox->setIconSize(_m_layout->m_magatamaSize);
    _m_sub_hpBox->setOrientation(_m_layout->m_magatamasHorizontal ? Qt::Horizontal : Qt::Vertical);
    _m_sub_hpBox->setBackgroundVisible(_m_layout->m_magatamasBgVisible);
    _m_sub_hpBox->setAnchorEnable(true);
    _m_sub_hpBox->setAnchor(_m_layout->m_sub_magatamasAnchor, _m_layout->m_sub_magatamasAlign);
    _m_sub_hpBox->setImageArea(_m_layout->m_sub_magatamaImageArea);
    _m_sub_hpBox->update();
    //}

    QPixmap lock = _getPixmap(QSanRoomSkin::S_SKIN_KEY_DISABLE_SHOW_LOCK);
    _paintPixmap(leftDisableShowLock, _m_layout->leftDisableShowLockArea, lock, _getAvatarParent());
    _paintPixmap(rightDisableShowLock, _m_layout->rightDisableShowLockArea, lock, _getAvatarParent());

    _adjustComponentZValues();
    _initializeRemovedEffect();
    refresh();
}

void PlayerCardContainer::_createRoleComboBox()
{
    if (isHegemonyGameMode(ServerInfo.GameMode))
        _m_hegemonyroleComboBox = new HegemonyRoleComboBox(_getRoleComboBoxParent());
    else
        _m_roleComboBox = new RoleComboBox(_getRoleComboBoxParent());
}

void PlayerCardContainer::setPlayer(ClientPlayer *player)
{
    m_player = player;
    if (player) {
        //notice that:  child class "Dashboard" has void with the same name "updateAvatar".
        //connect(player, &ClientPlayer::general_changed, this, &PlayerCardContainer::updateAvatar);
        connect(player, SIGNAL(general_changed()), this, SLOT(updateAvatar()));
        connect(player, &ClientPlayer::general2_changed, this, &PlayerCardContainer::updateSmallAvatar);
        //connect(player, &ClientPlayer::kingdom_changed, this, &PlayerCardContainer::updateAvatar);
        connect(player, SIGNAL(kingdom_changed(QString)), this, SLOT(updateAvatar()));
        connect(player, &ClientPlayer::state_changed, this, &PlayerCardContainer::refresh);
        connect(player, &ClientPlayer::phase_changed, this, &PlayerCardContainer::updatePhase);
        connect(player, &ClientPlayer::drank_changed, this, &PlayerCardContainer::updateDrankState);
        connect(player, &ClientPlayer::duozhi_changed, this, &PlayerCardContainer::updateAvatar);
        connect(player, &ClientPlayer::action_taken, this, &PlayerCardContainer::refresh);
        connect(player, &ClientPlayer::pile_changed, this, &PlayerCardContainer::updatePile);
        if (isHegemonyGameMode(ServerInfo.GameMode))
            connect(player, &ClientPlayer::kingdom_changed, _m_hegemonyroleComboBox, &HegemonyRoleComboBox::fix);
        else
            connect(player, &ClientPlayer::role_changed, _m_roleComboBox, &RoleComboBox::fix);
        connect(player, &ClientPlayer::hp_changed, this, &PlayerCardContainer::updateHp);
        connect(player, &ClientPlayer::removedChanged, this, &PlayerCardContainer::onRemovedChanged);
        connect(player, &ClientPlayer::disable_show_changed, this, &PlayerCardContainer::refresh);

        QTextDocument *textDoc = m_player->getMarkDoc();
        Q_ASSERT(_m_markItem);
        _m_markItem->setDocument(textDoc);
        connect(textDoc, SIGNAL(contentsChanged()), this, SLOT(updateMarks()));
        connect(player, &ClientPlayer::brokenEquips_changed, this, &PlayerCardContainer::updateBrokenEquips);
    }
    updateAvatar();
    refresh();
}

QList<CardItem *> PlayerCardContainer::removeDelayedTricks(const QList<int> &cardIds)
{
    QList<CardItem *> result;
    foreach (int card_id, cardIds) {
        CardItem *item = CardItem::FindItem(_m_judgeCards, card_id);
        Q_ASSERT(item != nullptr);
        int index = _m_judgeCards.indexOf(item);
        QRect start = _m_layout->m_delayedTrickFirstRegion;
        QPoint step = _m_layout->m_delayedTrickStep;
        start.translate(step * index);
        item->setOpacity(0.0);
        item->setPos(start.center());
        _m_judgeCards.removeAt(index);
        //delete _m_judgeIcons.takeAt(index);
        QGraphicsPixmapItem *icon = _m_judgeIcons.takeAt(index);
        icon->setOpacity(0.0);
        delete icon;
        result.append(item);
    }
    updateDelayedTricks();
    return result;
}

void PlayerCardContainer::updateDelayedTricks()
{
    for (int i = 0; i < _m_judgeIcons.size(); i++) {
        QGraphicsPixmapItem *item = _m_judgeIcons[i];
        QRect start = _m_layout->m_delayedTrickFirstRegion;
        QPoint step = _m_layout->m_delayedTrickStep;
        start.translate(step * i);
        item->setPos(start.topLeft());
    }
}

void PlayerCardContainer::addDelayedTricks(QList<CardItem *> &tricks)
{
    foreach (CardItem *trick, tricks) {
        QGraphicsPixmapItem *item = new QGraphicsPixmapItem(_getDelayedTrickParent());
        QRect start = _m_layout->m_delayedTrickFirstRegion;
        QPoint step = _m_layout->m_delayedTrickStep;
        start.translate(step * _m_judgeCards.size());
        _paintPixmap(item, start, G_ROOM_SKIN.getCardJudgeIconPixmap(trick->getCard()->objectName()));
        trick->setHomeOpacity(0.0);
        trick->setHomePos(start.center());
        const Card *card = Sanguosha->getEngineCard(trick->getCard()->getEffectiveId());
        QString toolTip = QString("<font color=#FFFF33><b>%1 [</b><img src='image/system/log/%2.png' height = 12/><b>%3]</b></font>")
                              .arg(Sanguosha->translate(card->objectName()))
                              .arg(card->getSuitString())
                              .arg(card->getNumberString());
        item->setToolTip(toolTip);
        _m_judgeCards.append(trick);
        _m_judgeIcons.append(item);
    }
}

QPixmap PlayerCardContainer::_getEquipPixmap(const EquipCard *equip)
{
    const Card *realCard = Sanguosha->getEngineCard(equip->getEffectiveId());
    QPixmap equipIcon(_m_layout->m_equipAreas[0].size());
    equipIcon.fill(Qt::transparent);
    QPainter painter(&equipIcon);
    // icon / background
    if (!m_player->isBrokenEquip(equip->getEffectiveId()))
        painter.drawPixmap(_m_layout->m_equipImageArea, _getPixmap(QSanRoomSkin::S_SKIN_KEY_EQUIP_ICON, equip->objectName()));
    else
        painter.drawPixmap(_m_layout->m_equipImageArea, _getPixmap(QSanRoomSkin::S_SKIN_KEY_EQUIP_BROKEN_ICON, equip->objectName()));
    // equip name
    _m_layout->m_equipFont.paintText(&painter, _m_layout->m_equipTextArea, Qt::AlignLeft | Qt::AlignCenter, Sanguosha->translate(equip->objectName()));
    // equip suit
    painter.drawPixmap(_m_layout->m_equipSuitArea, G_ROOM_SKIN.getCardSuitPixmap(realCard->getSuit()));
    //test UI
    //if (m_player->isBrokenEquip(equip->getEffectiveId()))
    //    painter.drawPixmap(_m_layout->m_equipTianyiArea, G_ROOM_SKIN.getCardTianyiPixmap());
    /*if (m_player->isBrokenEquip(equip->getEffectiveId())) {
        painter.setRenderHints(QPainter::HighQualityAntialiasing);
        QPen pen(Qt::red);
        pen.setWidth(3);
        painter.setPen(pen);
        painter.drawLine(25, 6, 40, 20);
        painter.drawLine(25, 20, 40, 6);
    }*/

    // equip point
    if (realCard->isRed()) {
        _m_layout->m_equipPointFontRed.paintText(&painter, _m_layout->m_equipPointArea, Qt::AlignLeft | Qt::AlignVCenter, realCard->getNumberString());
    } else {
        _m_layout->m_equipPointFontBlack.paintText(&painter, _m_layout->m_equipPointArea, Qt::AlignLeft | Qt::AlignVCenter, realCard->getNumberString());
    }
    // distance
    int index = (int)(equip->location());
    QString distance;
    if (index == 0) {
        const Weapon *weapon = qobject_cast<const Weapon *>(equip);
        Q_ASSERT(weapon);
        if (weapon)
            distance = Sanguosha->translate(QString("CAPITAL(%1)").arg(QString::number(weapon->getRange())));
    } else if (index == 2) {
        const DefensiveHorse *horse = qobject_cast<const DefensiveHorse *>(equip);
        Q_ASSERT(horse);
        if (horse)
            distance = QString("+%1").arg(QString::number(horse->getCorrect()));
    } else if (index == 3) {
        const OffensiveHorse *horse = qobject_cast<const OffensiveHorse *>(equip);
        Q_ASSERT(horse);
        if (horse)
            distance = QString::number(horse->getCorrect());
    }
    if (index != 1 && index != 4) {
        _m_layout->m_equipFont.paintText(&painter, _m_layout->m_equipDistanceArea, Qt::AlignLeft | Qt::AlignVCenter, distance);
    }
    return equipIcon;
}

void PlayerCardContainer::setFloatingArea(QRect rect)
{
    _m_floatingAreaRect = rect;
    QPixmap dummy(rect.size());
    dummy.fill(Qt::transparent);
    _m_floatingArea->setPixmap(dummy);
    _m_floatingArea->setPos(rect.topLeft());
    if (_getPhaseParent() == _m_floatingArea)
        updatePhase();
    if (_getMarkParent() == _m_floatingArea)
        updateMarks();
    if (_getProgressBarParent() == _m_floatingArea)
        _updateProgressBar();
}

void PlayerCardContainer::addEquips(QList<CardItem *> &equips)
{
    //for tianyi
    //_updateEquips();
    foreach (CardItem *equip, equips) {
        const EquipCard *equip_card = qobject_cast<const EquipCard *>(equip->getCard()->getRealCard());
        int index = (int)(equip_card->location());
        Q_ASSERT(_m_equipCards[index] == nullptr);
        _m_equipCards[index] = equip;
        connect(equip, SIGNAL(mark_changed()), this, SLOT(_onEquipSelectChanged()));
        equip->setHomeOpacity(0.0);
        equip->setHomePos(_m_layout->m_equipAreas[index].center());
        _m_equipRegions[index]->setToolTip(equip_card->getDescription());
        QPixmap pixmap = _getEquipPixmap(equip_card);

        _m_equipLabel[index]->setPixmap(pixmap);

        _mutexEquipAnim.lock();
        _m_equipRegions[index]->setPos(_m_layout->m_equipAreas[index].topLeft() + QPoint(_m_layout->m_equipAreas[index].width() / 2, 0));
        _m_equipRegions[index]->setOpacity(0);
        _m_equipRegions[index]->show();
        _m_equipAnim[index]->stop();
        _m_equipAnim[index]->clear();
        QPropertyAnimation *anim = new QPropertyAnimation(_m_equipRegions[index], "pos");
        anim->setEndValue(_m_layout->m_equipAreas[index].topLeft());
        anim->setDuration(200);
        _m_equipAnim[index]->addAnimation(anim);
        anim = new QPropertyAnimation(_m_equipRegions[index], "opacity");
        anim->setEndValue(255);
        anim->setDuration(200);
        _m_equipAnim[index]->addAnimation(anim);
        _m_equipAnim[index]->start();
        _mutexEquipAnim.unlock();

        const Skill *skill = Sanguosha->getSkill(equip_card->objectName());
        if (skill == nullptr)
            continue;
        emit add_equip_skill(skill, true);
    }
}

QList<CardItem *> PlayerCardContainer::removeEquips(const QList<int> &cardIds)
{
    //for tianyi
    //_updateEquips();
    QList<CardItem *> result;
    foreach (int card_id, cardIds) {
        const EquipCard *equip_card = qobject_cast<const EquipCard *>(Sanguosha->getEngineCard(card_id));
        int index = (int)(equip_card->location());
        Q_ASSERT(_m_equipCards[index] != nullptr);
        CardItem *equip = _m_equipCards[index];
        equip->setHomeOpacity(0.0);
        equip->setPos(_m_layout->m_equipAreas[index].center());
        result.append(equip);
        _m_equipCards[index] = nullptr;
        _mutexEquipAnim.lock();
        _m_equipAnim[index]->stop();
        _m_equipAnim[index]->clear();
        QPropertyAnimation *anim = new QPropertyAnimation(_m_equipRegions[index], "pos");
        anim->setEndValue(_m_layout->m_equipAreas[index].topLeft() + QPoint(_m_layout->m_equipAreas[index].width() / 2, 0));
        anim->setDuration(200);
        _m_equipAnim[index]->addAnimation(anim);
        anim = new QPropertyAnimation(_m_equipRegions[index], "opacity");
        anim->setEndValue(0);
        anim->setDuration(200);
        _m_equipAnim[index]->addAnimation(anim);
        _m_equipAnim[index]->start();
        _mutexEquipAnim.unlock();

        const Skill *skill = Sanguosha->getSkill(equip_card->objectName());
        if (skill != nullptr)
            emit remove_equip_skill(skill->objectName());
    }
    return result;
}

void PlayerCardContainer::startHuaShen(QString generalName, QString skillName, QString general2Name, QString skill2Name)
{
    if (m_player == nullptr)
        return;

    _m_huashenGeneralName = generalName;
    _m_huashenSkillName = skillName;
    _m_huashenGeneral2Name = general2Name;
    _m_huashenSkill2Name = skill2Name;

    int huashen_size = 0;
    if (!generalName.isEmpty()) {
        huashen_size = (getPlayer() && getPlayer()->getGeneral2()) ? _m_layout->m_primaryAvatarSize : _m_layout->m_avatarSize; // 1 or 6;
    } else
        huashen_size = _m_layout->m_smallAvatarSize; //4 or 1;

    QPixmap pixmap = G_ROOM_SKIN.getGeneralPixmap((!generalName.isEmpty()) ? generalName : general2Name,
                                                  (QSanRoomSkin::GeneralIconSize)huashen_size); //(QSanRoomSkin::GeneralIconSize)_m_layout->m_avatarSize

    QRect animRect = (!generalName.isEmpty()) ? _m_layout->m_avatarArea : _m_layout->m_smallAvatarArea;
    if (pixmap.size() != animRect.size())
        pixmap = pixmap.scaled(animRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    stopHuaShen();

    _m_huashenAnimation = G_ROOM_SKIN.createHuaShenAnimation(pixmap, animRect.topLeft(), _getAvatarParent(), (!generalName.isEmpty()) ? _m_huashenItem : _m_huashenItem2);
    _m_huashenAnimation->start();
    _paintPixmap(_m_extraSkillBg, _m_layout->m_extraSkillArea, QSanRoomSkin::S_SKIN_KEY_EXTRA_SKILL_BG, _getAvatarParent());
    if (!skillName.isEmpty() || !skill2Name.isEmpty())
        _m_extraSkillBg->show();

    QString skill_name = (!skillName.isEmpty()) ? skillName : skill2Name;
    _m_layout->m_extraSkillFont.paintText(_m_extraSkillText, _m_layout->m_extraSkillTextArea, Qt::AlignCenter, Sanguosha->translate(skill_name).left(2));
    if (!skill_name.isEmpty()) {
        _m_extraSkillText->show();
        _m_extraSkillBg->setToolTip(Sanguosha->getSkill(skill_name)->getDescription(true, isHegemonyGameMode(ServerInfo.GameMode)));
    }
    _adjustComponentZValues();
}

void PlayerCardContainer::stopHuaShen()
{
    if (_m_huashenAnimation != nullptr) {
        _m_huashenAnimation->stop();
        _m_huashenAnimation->deleteLater();
        delete _m_huashenItem;
        delete _m_huashenItem2;
        _m_huashenAnimation = nullptr;
        _m_huashenItem = nullptr;
        _m_huashenItem2 = nullptr;
        _m_huashenGeneralName.clear();
        _m_huashenGeneral2Name.clear();
        _m_huashenSkillName.clear();
        _m_huashenSkill2Name.clear();
        _clearPixmap(_m_extraSkillBg);
        _clearPixmap(_m_extraSkillText);
    }
}

void PlayerCardContainer::updateAvatarTooltip()
{
    if (m_player) {
        QString description = m_player->getSkillDescription(true, "head");
        _m_avatarArea->setToolTip(description);
        if (m_player->getGeneral2()) {
            description = m_player->getSkillDescription(true, "deputy");
            _m_smallAvatarArea->setToolTip(description);
        }
    }
}

PlayerCardContainer::PlayerCardContainer()
{
    _m_layout = nullptr;
    _m_avatarArea = _m_smallAvatarArea = nullptr;
    _m_avatarNameItem = _m_smallAvatarNameItem = nullptr;
    _m_avatarIcon = nullptr;
    _m_smallAvatarIcon = nullptr;
    _m_circleItem = nullptr;
    _m_screenNameItem = nullptr;
    _m_chainIcon = nullptr;
    _m_faceTurnedIcon = _m_faceTurnedIcon2 = nullptr;
    _m_handCardBg = _m_handCardNumText = nullptr;
    _m_kingdomColorMaskIcon = _m_deathIcon = nullptr;
    _m_dashboardKingdomColorMaskIcon = _m_dashboardSecondaryKingdomColorMaskIcon = _m_deathIcon = nullptr;
    _m_actionIcon = nullptr;
    _m_kingdomIcon = nullptr;
    _m_saveMeIcon = nullptr;
    _m_phaseIcon = nullptr;
    _m_markItem = nullptr;
    _m_roleComboBox = nullptr;
    _m_hegemonyroleComboBox = nullptr;
    _m_roleShownIcon = nullptr;
    m_player = nullptr;
    _m_selectedFrame = nullptr;
    _m_privatePileArea = new QGraphicsProxyWidget(this);
    QWidget *pileArea = new QWidget(nullptr, Qt::Tool); //It currently needn't to be visible.
    pileArea->setAttribute(Qt::WA_TranslucentBackground);
    pileArea->resize(1, 1);
    _m_privatePileArea->setWidget(pileArea);
    leftDisableShowLock = rightDisableShowLock = nullptr;

    for (int i = 0; i < 5; i++) {
        _m_equipCards[i] = nullptr;
        _m_equipRegions[i] = nullptr;
        _m_equipAnim[i] = nullptr;
        _m_equipLabel[i] = nullptr;
    }
    _m_huashenItem = nullptr;
    _m_huashenItem2 = nullptr;
    _m_huashenAnimation = nullptr;
    _m_extraSkillBg = nullptr;
    _m_extraSkillText = nullptr;

    _m_floatingArea = nullptr;
    _m_votesGot = 0;
    _m_maxVotes = 1;
    _m_votesItem = nullptr;
    _m_distanceItem = nullptr;
    _m_groupMain = new QGraphicsPixmapItem(this);
    _m_groupMain->setFlag(ItemHasNoContents);
    _m_groupMain->setPos(0, 0);
    _m_groupDeath = new QGraphicsPixmapItem(this);
    _m_groupDeath->setFlag(ItemHasNoContents);
    _m_groupDeath->setPos(0, 0);
    _allZAdjusted = false;
    m_changePrimaryHeroSKinBtn = nullptr;
    m_changeSecondaryHeroSkinBtn = nullptr;
    m_primaryHeroSkinContainer = nullptr;
    m_secondaryHeroSkinContainer = nullptr;
    _m_seatItem = nullptr;
}

void PlayerCardContainer::hideAvatars()
{
    if (_m_avatarIcon)
        _m_avatarIcon->hide();
    if (_m_smallAvatarIcon)
        _m_smallAvatarIcon->hide();
}

void PlayerCardContainer::_layUnder(QGraphicsItem *item)
{
    _lastZ--;
    // Q_ASSERT((unsigned long)item != 0xcdcdcdcd);
    if (item)
        item->setZValue(_lastZ--);
    else
        _allZAdjusted = false;
}

bool PlayerCardContainer::_startLaying()
{
    if (_allZAdjusted)
        return false;
    _allZAdjusted = true;
    _lastZ = -1;
    return true;
}

void PlayerCardContainer::_layBetween(QGraphicsItem *middle, QGraphicsItem *item1, QGraphicsItem *item2)
{
    if (middle && item1 && item2)
        middle->setZValue((item1->zValue() + item2->zValue()) / 2.0);
    else
        _allZAdjusted = false;
}

void PlayerCardContainer::_adjustComponentZValues(bool killed)
{
    // all components use negative zvalues to ensure that no other generated
    // cards can be under us.

    // layout
    if (!_startLaying())
        return;

    _layUnder(_m_floatingArea);
    _layUnder(_m_distanceItem);
    _layUnder(_m_votesItem);
    if (!killed) {
        foreach (QGraphicsItem *pile, _m_privatePiles.values())
            _layUnder(pile);
    }
    foreach (QGraphicsItem *judge, _m_judgeIcons)
        _layUnder(judge);
    _layUnder(_m_markItem);
    _layUnder(_m_progressBarItem);
    _layUnder(_m_roleShownIcon);
    if (isHegemonyGameMode(ServerInfo.GameMode))
        _layUnder(_m_hegemonyroleComboBox);
    else
        _layUnder(_m_roleComboBox);
    // _layUnder(_m_chainIcon);
    _layUnder(_m_hpBox);
    _layUnder(_m_sub_hpBox);

    _layUnder(_m_handCardNumText);
    _layUnder(_m_handCardBg);
    _layUnder(_m_actionIcon);
    _layUnder(_m_saveMeIcon);
    _layUnder(_m_phaseIcon);
    _layUnder(_m_smallAvatarNameItem);
    _layUnder(_m_avatarNameItem);
    _layUnder(_m_kingdomIcon);
    _layUnder(_m_kingdomColorMaskIcon);
    _layUnder(_m_dashboardKingdomColorMaskIcon);
    _layUnder(_m_dashboardSecondaryKingdomColorMaskIcon);
    _layUnder(leftDisableShowLock);
    _layUnder(rightDisableShowLock);
    _layUnder(_m_chainIcon);

    _layUnder(_m_screenNameItem);
    for (int i = 0; i < 5; i++)
        _layUnder(_m_equipRegions[i]);
    _layUnder(_m_selectedFrame);
    _layUnder(_m_extraSkillText);
    _layUnder(_m_extraSkillBg);
    _layUnder(_m_faceTurnedIcon2);
    _layUnder(_m_faceTurnedIcon);
    _layUnder(_m_smallAvatarArea);
    _layUnder(_m_avatarArea);
    _layUnder(_m_circleItem);

    if (!killed)
        _layUnder(_m_huashenItem2);
    _layUnder(_m_smallAvatarIcon);

    if (!killed)
        _layUnder(_m_huashenItem);
    _layUnder(_m_avatarIcon);
    //setZValue(_lastZ--)
}

void PlayerCardContainer::updateRole(const QString &role)
{
    //if (ServerInfo.GameMode == "hegemony")
    _m_roleComboBox->fix(role);
}

void PlayerCardContainer::updateKingdom(const QString &kingdom)
{
    _m_hegemonyroleComboBox->fix(kingdom);
}

void PlayerCardContainer::_updateProgressBar()
{
    QGraphicsItem *parent = _getProgressBarParent();
    if (parent == nullptr)
        return;
    _m_progressBar->setOrientation(_m_layout->m_isProgressBarHorizontal ? Qt::Horizontal : Qt::Vertical);
    QRectF newRect = _m_layout->m_progressBarArea.getTranslatedRect(parent->boundingRect().toRect());
    _m_progressBar->setFixedHeight(newRect.height());
    _m_progressBar->setFixedWidth(newRect.width());
    _m_progressBarItem->setParentItem(parent);
    _m_progressBarItem->setPos(newRect.left(), newRect.top());
}

void PlayerCardContainer::_createControls()
{
    _m_floatingArea = new QGraphicsPixmapItem(_m_groupMain);

    _m_screenNameItem = new QGraphicsPixmapItem(_getAvatarParent());
    if (ServerInfo.Enable2ndGeneral && this->getPlayer() == Self)
        _m_avatarArea = new QGraphicsRectItem(_m_layout->m_avatarAreaDouble, _getAvatarParent());
    else
        _m_avatarArea = new QGraphicsRectItem(_m_layout->m_avatarArea, _getAvatarParent());

    _m_avatarArea->setPen(Qt::NoPen);
    _m_avatarNameItem = new QGraphicsPixmapItem(_getAvatarParent());

    _m_smallAvatarArea = new QGraphicsRectItem(_m_layout->m_smallAvatarArea, _getAvatarParent());
    _m_smallAvatarArea->setPen(Qt::NoPen);
    _m_smallAvatarNameItem = new QGraphicsPixmapItem(_getAvatarParent());

    _m_extraSkillText = new QGraphicsPixmapItem(_getAvatarParent());
    _m_extraSkillText->hide();

    _m_handCardNumText = new QGraphicsPixmapItem(_getAvatarParent());

    _m_hpBox = new MagatamasBoxItem(_getAvatarParent());
    _m_sub_hpBox = new MagatamasBoxItem(_getAvatarParent());
    // Now set up progress bar
    _m_progressBar = new QSanCommandProgressBar;
    _m_progressBar->setAutoHide(true);
    _m_progressBar->hide();
    _m_progressBarItem = new QGraphicsProxyWidget(_getProgressBarParent());
    _m_progressBarItem->setWidget(_m_progressBar);
    _updateProgressBar();

    for (int i = 0; i < 5; i++) {
        _m_equipLabel[i] = new QLabel;
        _m_equipLabel[i]->setStyleSheet("QLabel { background-color: transparent; }");
        _m_equipLabel[i]->setPixmap(QPixmap(_m_layout->m_equipAreas[i].size()));
        _m_equipRegions[i] = new QGraphicsProxyWidget();
        _m_equipRegions[i]->setWidget(_m_equipLabel[i]);
        _m_equipRegions[i]->setPos(_m_layout->m_equipAreas[i].topLeft());
        _m_equipRegions[i]->setParentItem(_getEquipParent());
        _m_equipRegions[i]->hide();
        _m_equipAnim[i] = new QParallelAnimationGroup;
    }

    _m_markItem = new QGraphicsTextItem(_getMarkParent());
    _m_markItem->setDefaultTextColor(Qt::white);

    m_changePrimaryHeroSKinBtn = new QSanButton("player_container", "change-heroskin", _getAvatarParent());
    m_changePrimaryHeroSKinBtn->hide();
    connect(m_changePrimaryHeroSKinBtn, SIGNAL(clicked()), this, SLOT(showHeroSkinList()));
    connect(m_changePrimaryHeroSKinBtn, SIGNAL(clicked_mouse_outside()), this, SLOT(heroSkinBtnMouseOutsideClicked()));

    m_changeSecondaryHeroSkinBtn = new QSanButton("player_container", "change-heroskin", _getAvatarParent());
    m_changeSecondaryHeroSkinBtn->hide();
    connect(m_changeSecondaryHeroSkinBtn, SIGNAL(clicked()), this, SLOT(showHeroSkinList()));
    connect(m_changeSecondaryHeroSkinBtn, SIGNAL(clicked_mouse_outside()), this, SLOT(heroSkinBtnMouseOutsideClicked()));

    _createRoleComboBox();
    repaintAll();

    connect(_m_avatarIcon, SIGNAL(hover_enter()), this, SLOT(onAvatarHoverEnter()));
    connect(_m_avatarIcon, SIGNAL(hover_leave()), this, SLOT(onAvatarHoverLeave()));
    connect(_m_avatarIcon, SIGNAL(skin_changing_start()), this, SLOT(onSkinChangingStart()));
    connect(_m_avatarIcon, SIGNAL(skin_changing_finished()), this, SLOT(onSkinChangingFinished()));

    connect(_m_smallAvatarIcon, SIGNAL(hover_enter()), this, SLOT(onAvatarHoverEnter()));
    connect(_m_smallAvatarIcon, SIGNAL(hover_leave()), this, SLOT(onAvatarHoverLeave()));
    connect(_m_smallAvatarIcon, SIGNAL(skin_changing_start()), this, SLOT(onSkinChangingStart()));
    connect(_m_smallAvatarIcon, SIGNAL(skin_changing_finished()), this, SLOT(onSkinChangingFinished()));
}

void PlayerCardContainer::_updateDeathIcon()
{
    if (!m_player || !m_player->isDead())
        return;
    QRect deathArea = _m_layout->m_deathIconRegion.getTranslatedRect(_getDeathIconParent()->boundingRect().toRect());
    _paintPixmap(_m_deathIcon, deathArea, QPixmap(m_player->getDeathPixmapPath()), _getDeathIconParent());
    _m_deathIcon->setZValue(30000.0);
}

void PlayerCardContainer::killPlayer()
{
    if (isHegemonyGameMode(ServerInfo.GameMode)) {
        _m_hegemonyroleComboBox->fix(m_player->getRole() == "careerist" ? "careerist" : m_player->getRole()); //m_player->getKingdom()
        _m_hegemonyroleComboBox->setEnabled(false);
    } else {
        _m_roleComboBox->fix(m_player->getRole());
        _m_roleComboBox->setEnabled(false);
    }

    _updateDeathIcon();
    _m_saveMeIcon->hide();
    if (_m_votesItem)
        _m_votesItem->hide();
    if (_m_distanceItem)
        _m_distanceItem->hide();
    QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect();
    effect->setColor(_m_layout->m_deathEffectColor);
    effect->setStrength(1.0);
    _m_groupMain->setGraphicsEffect(effect);
    refresh();
    if (ServerInfo.GameMode == "04_1v3" && !m_player->isLord()) {
        _m_deathIcon->hide();
        _m_votesGot = 6;
        updateVotes(false, true);
    } else
        _m_deathIcon->show();
}

void PlayerCardContainer::revivePlayer()
{
    _m_votesGot = 0;
    _m_groupMain->setGraphicsEffect(nullptr);
    Q_ASSERT(_m_deathIcon);
    _m_deathIcon->hide();
    refresh();
}

void PlayerCardContainer::mousePressEvent(QGraphicsSceneMouseEvent *)
{
}

void PlayerCardContainer::updateVotes(bool need_select, bool display_1)
{
    if ((need_select && !isSelected()) || _m_votesGot < 1 || (!display_1 && _m_votesGot == 1))
        _clearPixmap(_m_votesItem);
    else {
        _paintPixmap(_m_votesItem, _m_layout->m_votesIconRegion, _getPixmap(QSanRoomSkin::S_SKIN_KEY_VOTES_NUMBER, QString::number(_m_votesGot)), _getAvatarParent());
        _m_votesItem->setZValue(1);
        _m_votesItem->show();
    }
}

void PlayerCardContainer::updateReformState()
{
    _m_votesGot--;
    updateVotes(false, true);
}

void PlayerCardContainer::showDistance()
{
    bool isNull = (_m_distanceItem == nullptr);
    _paintPixmap(_m_distanceItem, _m_layout->m_votesIconRegion, _getPixmap(QSanRoomSkin::S_SKIN_KEY_VOTES_NUMBER, QString::number(Self->distanceTo(m_player))), _getAvatarParent());
    _m_distanceItem->setZValue(1.1);
    if (!Self->inMyAttackRange(m_player)) {
        QGraphicsColorizeEffect *effect = new QGraphicsColorizeEffect();
        effect->setColor(_m_layout->m_deathEffectColor);
        effect->setStrength(1.0);
        _m_distanceItem->setGraphicsEffect(effect);
    } else {
        _m_distanceItem->setGraphicsEffect(nullptr);
    }
    if (_m_distanceItem->isVisible() && !isNull)
        _m_distanceItem->hide();
    else
        _m_distanceItem->show();
}

void PlayerCardContainer::onRemovedChanged()
{
    QAbstractAnimation::Direction direction = m_player->isRemoved() ? QAbstractAnimation::Forward : QAbstractAnimation::Backward;

    _getPlayerRemovedEffect()->setDirection(direction);
    _getPlayerRemovedEffect()->start();
}

void PlayerCardContainer::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem *item = getMouseClickReceiver();
    if (item != nullptr && item->isUnderMouse() && isEnabled() && (flags() & QGraphicsItem::ItemIsSelectable)) {
        if (event->button() == Qt::RightButton)
            setSelected(false);
        else if (event->button() == Qt::LeftButton) {
            _m_votesGot++;
            setSelected(_m_votesGot <= _m_maxVotes);
            if (_m_votesGot > 1)
                emit selected_changed();
        }
        updateVotes();
    }
}

void PlayerCardContainer::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    if (Config.EnableDoubleClick)
        RoomSceneInstance->doOkButton();
}

QVariant PlayerCardContainer::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged) {
        if (!value.toBool()) {
            _m_votesGot = 0;
            _clearPixmap(_m_selectedFrame);
            _m_selectedFrame->hide();
        } else {
            QRect focusFrameArea = (this->getPlayer() == Self && ServerInfo.Enable2ndGeneral) ? _m_layout->m_focusFrameAreaDouble : _m_layout->m_focusFrameArea;
            _paintPixmap(_m_selectedFrame, focusFrameArea, _getPixmap(QSanRoomSkin::S_SKIN_KEY_SELECTED_FRAME, true), _getFocusFrameParent());
            _m_selectedFrame->show();
        }
        updateVotes();
        emit selected_changed();
    } else if (change == ItemEnabledHasChanged) {
        _m_votesGot = 0;
        emit enable_changed();
    }

    return QGraphicsObject::itemChange(change, value);
}

void PlayerCardContainer::_onEquipSelectChanged()
{
}

void PlayerCardContainer::showHeroSkinList()
{
    foreach (HeroSkinContainer *heroSkinContainer, RoomSceneInstance->getHeroSkinContainers()) {
        if (heroSkinContainer->isVisible()) {
            heroSkinContainer->hide();
        }
    }
    if (nullptr != m_player) {
        if (sender() == m_changePrimaryHeroSKinBtn) {
            showHeroSkinListHelper(m_player->getGeneral(), _m_avatarIcon); //, m_primaryHeroSkinContainer
        } else {
            showHeroSkinListHelper(m_player->getGeneral2(), _m_smallAvatarIcon); //, m_secondaryHeroSkinContainer
        }
    }
}

void PlayerCardContainer::showHeroSkinListHelper(const General *general, GraphicsPixmapHoverItem *avatarIcon) //, HeroSkinContainer *&heroSkinContainer
{
    if (nullptr == general) {
        return;
    }

    QString generalName = general->objectName();
    //find  heroSkinContainer every time. exclude m_primaryHeroSkinContainer
    //if (NULL == heroSkinContainer) {
    HeroSkinContainer *heroSkinContainer = RoomSceneInstance->findHeroSkinContainer(generalName);
    //}
    if (nullptr == heroSkinContainer) {
        heroSkinContainer = new HeroSkinContainer(generalName, general->getKingdom());

        connect(heroSkinContainer, SIGNAL(local_skin_changed(const QString &)), avatarIcon, SLOT(startChangeHeroSkinAnimation(const QString &)));

        connect(heroSkinContainer, SIGNAL(skin_changed(const QString &, int)), RoomSceneInstance, SLOT(doSkinChange(const QString &, int)));

        RoomSceneInstance->addHeroSkinContainer(m_player, heroSkinContainer);
        RoomSceneInstance->addItem(heroSkinContainer);
        QRectF photoRect = _m_avatarIcon->sceneBoundingRect();
        if (Self == m_player)

            heroSkinContainer->setPos(photoRect.left() - 400, photoRect.top() - 60); //QPointF(100,100)
        else {
            heroSkinContainer->setPos(photoRect.left(), photoRect.top());
        }
        //heroSkinContainer->setPos(getHeroSkinContainerPosition());
        RoomSceneInstance->bringToFront(heroSkinContainer);
    }

    if (!heroSkinContainer->isVisible()) {
        heroSkinContainer->show();
    }
    heroSkinContainer->bringToTopMost();
}

void PlayerCardContainer::heroSkinBtnMouseOutsideClicked()
{
    if (nullptr != m_player) {
        QSanButton *heroSKinBtn = nullptr;
        if (sender() == m_changePrimaryHeroSKinBtn) {
            heroSKinBtn = m_changePrimaryHeroSKinBtn;
        } else {
            heroSKinBtn = m_changeSecondaryHeroSkinBtn;
        }
        if (heroSKinBtn != nullptr) {
            QGraphicsItem *parent = heroSKinBtn->parentItem();
            if (nullptr != parent && !parent->isUnderMouse()) {
                heroSKinBtn->hide();

                if (Self == m_player && nullptr != _m_screenNameItem && _m_screenNameItem->isVisible()) {
                    _m_screenNameItem->hide();
                }
            }
        }
    }
}

void PlayerCardContainer::onAvatarHoverEnter()
{
    if (nullptr != m_player) {
        QObject *senderObj = sender();

        //bool second_zuoci = (m_player->getGeneralName() != "zuoci")
        //    && (m_player->getGeneral2Name() == "zuoci");

        const General *general = nullptr;
        GraphicsPixmapHoverItem *avatarItem = nullptr;
        QSanButton *heroSKinBtn = nullptr;

        if (senderObj == _m_avatarIcon) { // || (senderObj == _m_huashenItem && !second_zuoci))
            general = m_player->getGeneral();
            avatarItem = _m_avatarIcon;
            heroSKinBtn = m_changePrimaryHeroSKinBtn;

            m_changeSecondaryHeroSkinBtn->hide();
        } else if (senderObj == _m_smallAvatarIcon) { //|| (senderObj == _m_huashenItem && second_zuoci)
            general = m_player->getGeneral2();
            avatarItem = _m_smallAvatarIcon;
            heroSKinBtn = m_changeSecondaryHeroSkinBtn;

            m_changePrimaryHeroSKinBtn->hide();
        }

        if (nullptr != general && HeroSkinContainer::hasSkin(general->objectName()) && avatarItem->isSkinChangingFinished()) {
            heroSKinBtn->show();
        }
        //if (heroSKinBtn == NULL)
        //    _m_screenNameItem->hide();
        //showProgressBar(Countdown countdown)
    }
}

void PlayerCardContainer::onAvatarHoverLeave()
{
    if (nullptr != m_player) {
        QObject *senderObj = sender();

        //bool second_zuoci = (m_player->getGeneralName() != "zuoci")
        //    && (m_player->getGeneral2Name() == "zuoci");

        QSanButton *heroSKinBtn = nullptr;

        if (senderObj == _m_avatarIcon) { //|| (senderObj == _m_huashenItem && !second_zuoci))
            heroSKinBtn = m_changePrimaryHeroSKinBtn;
        } else if (senderObj == _m_smallAvatarIcon) { //|| (senderObj == _m_huashenItem && second_zuoci)
            heroSKinBtn = m_changeSecondaryHeroSkinBtn;
        }

        if ((nullptr != heroSKinBtn) && (!heroSKinBtn->isMouseInside())) {
            heroSKinBtn->hide();
            doAvatarHoverLeave();
        }
    }
}

void PlayerCardContainer::onSkinChangingStart()
{
    QSanButton *heroSKinBtn = nullptr;
    QString generalName;

    if (sender() == _m_avatarIcon) {
        heroSKinBtn = m_changePrimaryHeroSKinBtn;
        generalName = m_player->getGeneralName();
    } else {
        heroSKinBtn = m_changeSecondaryHeroSkinBtn;
        generalName = m_player->getGeneral2Name();
    }

    if (heroSKinBtn != nullptr)
        heroSKinBtn->hide();

    if (generalName == "zuoci" && _m_huashenAnimation != nullptr) {
        stopHuaShen();
    }
}

void PlayerCardContainer::onSkinChangingFinished()
{
    GraphicsPixmapHoverItem *avatarItem = nullptr;
    QSanButton *heroSKinBtn = nullptr;
    QString generalName;

    if (sender() == _m_avatarIcon) {
        avatarItem = _m_avatarIcon;
        heroSKinBtn = m_changePrimaryHeroSKinBtn;
        generalName = m_player->getGeneralName();
    } else {
        avatarItem = _m_smallAvatarIcon;
        heroSKinBtn = m_changeSecondaryHeroSkinBtn;
        generalName = m_player->getGeneral2Name();
    }

    if (isItemUnderMouse(avatarItem)) {
        heroSKinBtn->show();
    }

    /*if (generalName == "zuoci" && m_player->isAlive() && !_m_huashenGeneralName.isEmpty() && !_m_huashenSkillName.isEmpty()) {
        QString huashen_place = Self->tag.value("Huashen_place", QString()).toString();
        startHuaShen(_m_huashenGeneralName, _m_huashenSkillName, (huashen_place != "deputy"));
    }*/
}

void PlayerCardContainer::stopHeroSkinChangingAnimation()
{
    if ((nullptr != _m_avatarIcon) && !_m_avatarIcon->isSkinChangingFinished()) {
        _m_avatarIcon->stopChangeHeroSkinAnimation();
    }
    if ((nullptr != _m_smallAvatarIcon) && !_m_smallAvatarIcon->isSkinChangingFinished()) {
        _m_smallAvatarIcon->stopChangeHeroSkinAnimation();
    }
}

QPixmap PlayerCardContainer::_getAvatarIcon(const QString &heroName)
{
    int avatarSize = (m_player->getGeneral2() || (ServerInfo.Enable2ndGeneral && this->getPlayer() == Self)) ? _m_layout->m_primaryAvatarSize : _m_layout->m_avatarSize;
    return G_ROOM_SKIN.getGeneralPixmap(heroName, (QSanRoomSkin::GeneralIconSize)avatarSize);
}
QPixmap PlayerCardContainer::getSmallAvatarIcon(const QString &generalName)
{
    QPixmap smallAvatarIcon = G_ROOM_SKIN.getGeneralPixmap(generalName, QSanRoomSkin::GeneralIconSize(_m_layout->m_smallAvatarSize));
    smallAvatarIcon = paintByMask(smallAvatarIcon);

    return smallAvatarIcon;
}

void PlayerCardContainer::showSkillName(const QString &skill_name, bool isSelf)
{
    // ClientPlayer *player = ClientInstance->getPlayer(player_name);

    //getSkillNameFont().paintText(_m_skillNameItem,
    //     getSkillNameArea(),
    //    Qt::AlignLeft,
    //    Sanguosha->translate(skill_name));
    if (!isSelf) {
        G_PHOTO_LAYOUT.m_skillNameFont.paintText(_m_skillNameItem,
                                                 G_PHOTO_LAYOUT.m_skillNameArea, //getSkillNameArea()
                                                 Qt::AlignLeft, Sanguosha->translate(skill_name));
    } else {
        if (ServerInfo.Enable2ndGeneral && Self->inDeputySkills(skill_name))
            G_DASHBOARD_LAYOUT.m_skillNameFont.paintText(_m_skillNameItem, G_DASHBOARD_LAYOUT.m_secondarySkillNameArea, Qt::AlignLeft, Sanguosha->translate(skill_name));
        else
            G_DASHBOARD_LAYOUT.m_skillNameFont.paintText(_m_skillNameItem, G_DASHBOARD_LAYOUT.m_skillNameArea, Qt::AlignLeft, Sanguosha->translate(skill_name));
    }
    _m_skillNameItem->show();
    QTimer::singleShot(1000, this, SLOT(hideSkillName()));
}

void PlayerCardContainer::hideSkillName()
{
    _m_skillNameItem->hide();
}

void PlayerCardContainer::setRoleShown(bool shown)
{
    _m_roleShownIcon->setVisible(shown);
    refresh();
}

QString PlayerCardContainer::getHuashenSkillName(bool head)
{
    if (head)
        return _m_huashenGeneralName;
    else
        return _m_huashenGeneral2Name;
}

void PlayerCardContainer::showSeat()
{
    _paintPixmap(_m_seatItem, _m_layout->m_seatIconRegion, _getPixmap(QSanRoomSkin::S_SKIN_KEY_SEAT_NUMBER, QString::number(m_player->getSeat())), _getAvatarParent());
    //save the seat number for later use
    //m_player->setProperty("UI_Seat", m_player->getSeat());
    _m_seatItem->setZValue(1.1);
}
