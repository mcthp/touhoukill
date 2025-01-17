#include "qsanbutton.h"
#include "SkinBank.h"
#include "client.h"
#include "clientplayer.h"
#include "engine.h"
#include "roomscene.h"

#include <QBitmap>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QPixmap>

QSanButton::QSanButton(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    _m_state = S_STATE_UP;
    _m_style = S_STYLE_PUSH;
    _m_mouseEntered = false;
    setSize(QSize(0, 0));
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    //multi_state = false;
    //m_isFirstState = true;
}

QSanButton::QSanButton(const QString &groupName, const QString &buttonName, QGraphicsItem *parent)
    : QGraphicsObject(parent)
//multi_state(multi_state)
//, m_isFirstState(true)
{
    _m_state = S_STATE_UP;
    _m_style = S_STYLE_PUSH;
    _m_groupName = groupName;
    _m_buttonName = buttonName;
    _m_mouseEntered = false;

    for (int i = 0; i < (int)S_NUM_BUTTON_STATES; i++)
        _m_bgPixmap[i] = G_ROOM_SKIN.getButtonPixmap(groupName, buttonName, (QSanButton::ButtonState)i);
    setSize(_m_bgPixmap[0].size());

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

void QSanButton::click()
{
    if (isEnabled()) {
        _onMouseClick(true);
    }
}

QRectF QSanButton::boundingRect() const
{
    return QRectF(0, 0, _m_size.width(), _m_size.height());
}

void QSanButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->drawPixmap(0, 0, _m_bgPixmap[(int)_m_state]);
    //painter->drawPixmap(0, 0, _m_bgPixmap[(int)_m_state + (m_isFirstState ? 0 : S_NUM_BUTTON_STATES)]);
}

void QSanButton::setSize(QSize newSize)
{
    _m_size = newSize;
    if (_m_size.width() == 0 || _m_size.height() == 0) {
        _m_mask = QRegion();
        return;
    }
    Q_ASSERT(!_m_bgPixmap[0].isNull());
    QPixmap pixmap = _m_bgPixmap[0];
    _m_mask = QRegion(pixmap.mask().scaled(newSize));
}

void QSanButton::setRect(QRect rect)
{
    setSize(rect.size());
    setPos(rect.topLeft());
}

void QSanButton::setStyle(ButtonStyle style)
{
    _m_style = style;
}

void QSanButton::setEnabled(bool enabled)
{
    //bool changed = (enabled != isEnabled());
    //if (!changed) return;
    if (enabled) {
        setState(S_STATE_UP);
        _m_mouseEntered = false;
    }
    QGraphicsObject::setEnabled(enabled);
    if (!enabled)
        setState(S_STATE_DISABLED);
    update();
    emit enable_changed();
}

void QSanButton::setState(QSanButton::ButtonState state, bool ignore_change)
{
    if (ignore_change) {
        _m_state = state;
        update();
    } else if (_m_state != state) {
        _m_state = state;
        update();
    }
}

bool QSanButton::insideButton(QPointF pos) const
{
    return _m_mask.contains(QPoint(pos.x(), pos.y()));
}

void QSanButton::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (_m_state == S_STATE_DISABLED || _m_state == S_STATE_CANPRESHOW)
        return;
    QPointF point = event->pos();
    if (_m_mouseEntered || !insideButton(point))
        return; // fake event;

    if (_m_state == S_STATE_HOVER) //when askforSkillInvoke
        return;
    Q_ASSERT(_m_state != S_STATE_HOVER);

    _m_mouseEntered = true;
    if (_m_state == S_STATE_UP)
        setState(S_STATE_HOVER);
}

void QSanButton::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    if (_m_state == S_STATE_DISABLED || _m_state == S_STATE_CANPRESHOW)
        return;
    if (!_m_mouseEntered)
        return;

    Q_ASSERT(_m_state != S_STATE_DISABLED);
    if (_m_state == S_STATE_HOVER && _m_state != S_STATE_CANPRESHOW)
        setState(S_STATE_UP);
    _m_mouseEntered = false;
}

void QSanButton::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    QPointF point = event->pos();
    if (insideButton(point)) {
        if (!_m_mouseEntered)
            hoverEnterEvent(event);
    } else {
        if (_m_mouseEntered)
            hoverLeaveEvent(event);
    }
}

void QSanButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF point = event->pos();
    if (!insideButton(point))
        return;

    //Q_ASSERT(_m_state != S_STATE_DISABLED);
    //if (_m_style == S_STYLE_TOGGLE)
    //    return;
    if ((_m_style == S_STYLE_TOGGLE) //&& !multi_state
        || _m_state == S_STATE_DISABLED || _m_state == S_STATE_CANPRESHOW)
        return;
    setState(S_STATE_DOWN);
}

void QSanButton::_onMouseClick(bool inside)
{
    if (isHegemonyGameMode(ServerInfo.GameMode)) {
        bool changeState = true;
        if (inherits("QSanSkillButton")) {
            const Skill *skill = qobject_cast<const QSanSkillButton *>(this)->getSkill();
            if (skill->canPreshow() && !Self->hasShownSkill(skill))
                changeState = false;
        }
        //if (multi_state && inside)
        //    m_isFirstState = !m_isFirstState;
        if (_m_style == S_STYLE_PUSH && changeState)
            setState(S_STATE_UP);
        else if (_m_style == S_STYLE_TOGGLE) {
            if (_m_state == S_STATE_HOVER)
                _m_state = S_STATE_UP; // temporarily set, do not use setState!
            if (_m_state == S_STATE_DOWN) {
                if (inside)
                    _m_state = S_STATE_HOVER;
                else
                    _m_state = S_STATE_UP;
            } else if (_m_state == S_STATE_UP && inside)
                _m_state = S_STATE_DOWN;
        }
        update();

        if (inside) {
            emit clicked();
        }
        //else {
        //    _m_mouseEntered = false;
        //    emit clicked_outside(); //for  heroSkinButtonMouseOutsideClicked (hegemony version)
        //}
    } else {
        if (_m_style == S_STYLE_PUSH)
            setState(S_STATE_UP);
        else if (_m_style == S_STYLE_TOGGLE) {
            if (_m_state == S_STATE_HOVER)
                _m_state = S_STATE_UP; // temporarily set, do not use setState!

            if (_m_state == S_STATE_DOWN && inside)
                setState(S_STATE_UP);
            else if (_m_state == S_STATE_UP && inside)
                setState(S_STATE_DOWN);
        }
        update();

        if (inside)
            emit clicked();
    }
}

void QSanButton::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_ASSERT(_m_state != S_STATE_DISABLED);
    QPointF point = event->pos();
    bool inside = insideButton(point);
    _onMouseClick(inside);
}

bool QSanButton::isDown()
{
    return (_m_state == S_STATE_DOWN);
}

QSanSkillButton::QSanSkillButton(QGraphicsItem *parent)
    : QSanButton(parent)
{
    _m_groupName = QSanRoomSkin::S_SKIN_KEY_BUTTON_SKILL;
    _m_emitActivateSignal = false;
    _m_emitDeactivateSignal = false;
    _m_canEnable = true;
    _m_canDisable = true;
    _m_skill = nullptr;
    _m_viewAsSkill = nullptr;
    connect(this, SIGNAL(clicked()), this, SLOT(onMouseClick()));
    _m_skill = nullptr;
}

void QSanSkillButton::_setSkillType(SkillType type)
{
    _m_skillType = type;
}

void QSanSkillButton::onMouseClick()
{
    if (_m_skill == nullptr)
        return;

    if (!Self->hasPreshowedSkill(_m_skill) && _m_state == QSanButton::S_STATE_CANPRESHOW) {
        setState(S_STATE_DISABLED);
        ClientInstance->preshow(_m_skill->objectName(), true);
    } else if (Self->hasPreshowedSkill(_m_skill) && _m_state == QSanButton::S_STATE_DISABLED && _m_skill->canPreshow() && !Self->hasShownSkill(_m_skill)) {
        setState(QSanButton::S_STATE_CANPRESHOW);
        ClientInstance->preshow(_m_skill->objectName(), false);
    } else {
        if ((_m_style == S_STYLE_TOGGLE && isDown() && _m_emitActivateSignal) || _m_style == S_STYLE_PUSH) {
            emit skill_activated();
            emit skill_activated(_m_skill);
        } else if (!isDown() && _m_emitDeactivateSignal) {
            emit skill_deactivated();
            emit skill_deactivated(_m_skill);
        }
    }
}

void QSanSkillButton::setSkill(const Skill *skill)
{
    Q_ASSERT(skill != nullptr);
    _m_skill = skill;
    // This is a nasty trick because the server side decides to choose a nasty design
    // such that sometimes the actual viewas skill is nested inside a trigger skill.
    // Since the trigger skill is not relevant, we flatten it before we create the button.
    _m_viewAsSkill = ViewAsSkill::parseViewAsSkill(_m_skill);
    if (skill == nullptr)
        skill = _m_skill;

    Skill::Frequency freq = skill->getFrequency();
    if (skill->inherits("BattleArraySkill")) {
        setStyle(QSanButton::S_STYLE_TOGGLE);
        setState(QSanButton::S_STATE_DISABLED);
        _setSkillType(QSanInvokeSkillButton::S_SKILL_ARRAY);
        _m_emitActivateSignal = true;
        _m_emitDeactivateSignal = true;
    } else if (freq == Skill::Frequent || (freq == Skill::NotFrequent && skill->inherits("TriggerSkill") && !skill->inherits("EquipSkill") && _m_viewAsSkill == nullptr)) {
        setStyle(QSanButton::S_STYLE_TOGGLE);
        setState(freq == Skill::Frequent ? QSanButton::S_STATE_DOWN : QSanButton::S_STATE_UP);
        _setSkillType(QSanInvokeSkillButton::S_SKILL_FREQUENT);
        _m_emitActivateSignal = false;
        _m_emitDeactivateSignal = false;
        _m_canEnable = true;
        _m_canDisable = false;
    } else if (freq == Skill::Limited || freq == Skill::NotFrequent) { //|| ((skill->inherits("WeaponSkill") || skill->inherits("ArmorSkill")) && _m_viewAsSkill != NULL)
        setState(QSanButton::S_STATE_DISABLED);
        if (skill->isAttachedLordSkill())
            _setSkillType(QSanInvokeSkillButton::S_SKILL_ATTACHEDLORD);
        else if (freq == Skill::Limited)
            _setSkillType(QSanInvokeSkillButton::S_SKILL_ONEOFF_SPELL);
        else
            _setSkillType(QSanInvokeSkillButton::S_SKILL_PROACTIVE);

        setStyle(QSanButton::S_STYLE_TOGGLE);

        _m_emitDeactivateSignal = true;
        _m_emitActivateSignal = true;
        _m_canEnable = true;
        _m_canDisable = true;
    } else if (freq == Skill::Wake) {
        setState(QSanButton::S_STATE_DISABLED);
        setStyle(QSanButton::S_STYLE_PUSH);
        _setSkillType(QSanInvokeSkillButton::S_SKILL_AWAKEN);
        _m_emitActivateSignal = false;
        _m_emitDeactivateSignal = false;
        _m_canEnable = true;
        _m_canDisable = true;
    } else if (freq == Skill::Compulsory || freq == Skill::Eternal || freq == Skill::NotCompulsory) { //  we have to set it in such way for WeiDi
        if (isHegemonyGameMode(ServerInfo.GameMode))
            setState(QSanButton::S_STATE_DISABLED);
        else
            setState(QSanButton::S_STATE_UP);
        setStyle(QSanButton::S_STYLE_PUSH);
        _setSkillType(QSanInvokeSkillButton::S_SKILL_COMPULSORY);
        _m_emitActivateSignal = false;
        _m_emitDeactivateSignal = false;
        _m_canEnable = true;
        _m_canDisable = true;
    } else
        Q_ASSERT(false);
    setToolTip(skill->getDescription(true, isHegemonyGameMode(ServerInfo.GameMode)));

    if (isHegemonyGameMode(ServerInfo.GameMode)) {
        if (!Self->hasShownSkill(skill) && skill->canPreshow())
            setState(QSanButton::S_STATE_CANPRESHOW);
    }

    Q_ASSERT((int)_m_skillType <= 5 && _m_state <= 4);
    _repaint();
}

void QSanSkillButton::setState(ButtonState state, bool ignore_change)
{
    //refine state here for certain conditions
    if (_m_skillType == S_SKILL_COMPULSORY && Self->hasShownSkill(_m_skill))
        state = S_STATE_DISABLED;

    QSanButton::setState(state, ignore_change);
}

//no use?
/*void QSanSkillButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF point = event->pos();
    if (!insideButton(point)) return;

    if (_m_skillType == S_SKILL_COMPULSORY || _m_skillType == S_SKILL_AWAKEN)
        return;
    else
        QSanButton::mousePressEvent(event);
}*/

void QSanSkillButton::setEnabled(bool enabled)
{
    if (isHegemonyGameMode(ServerInfo.GameMode)) {
        //bool head = objectName() == "left";
        if (!enabled && _m_skill->canPreshow() && (!Self->hasShownSkill(_m_skill) || Self->hasFlag("hiding"))) {
            setState(Self->hasPreshowedSkill(_m_skill) ? S_STATE_DISABLED : S_STATE_CANPRESHOW);
        } else {
            QSanButton::setEnabled(enabled);
        }
    } else {
        if (!_m_canEnable && enabled)
            return;
        if (!_m_canDisable && !enabled)
            return;
        QSanButton::setEnabled(enabled);
    }
}

void QSanInvokeSkillButton::_repaint()
{
    for (int i = 0; i < (int)S_NUM_BUTTON_STATES; i++) {
        _m_bgPixmap[i] = G_ROOM_SKIN.getSkillButtonPixmap((ButtonState)i, _m_skillType, _m_enumWidth);
        Q_ASSERT(!_m_bgPixmap[i].isNull());

        if (i == S_STATE_CANPRESHOW) {
            QPixmap temp(_m_bgPixmap[i]);
            temp.fill(Qt::transparent); //
            QPainter painter(&temp);
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.drawPixmap(0, 0, _m_bgPixmap[i]);
            painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            painter.fillRect(temp.rect(), QColor(0, 0, 0, 160));
            _m_bgPixmap[i] = temp;
        }

        const IQSanComponentSkin::QSanShadowTextFont &font = G_DASHBOARD_LAYOUT.getSkillTextFont((ButtonState)i, _m_skillType, _m_enumWidth);
        QPainter painter(&_m_bgPixmap[i]);
        QString skillName = Sanguosha->translate(_m_skill->objectName());
        if (_m_enumWidth != S_WIDTH_WIDE)
            skillName = skillName.left(2);
        // need adjust rect?
        font.paintText(&painter, (ButtonState)i == S_STATE_DOWN ? G_DASHBOARD_LAYOUT.m_skillTextAreaDown[_m_enumWidth] : G_DASHBOARD_LAYOUT.m_skillTextArea[_m_enumWidth],
                       Qt::AlignCenter, skillName);
    }
    setSize(_m_bgPixmap[0].size());
}

void QSanInvokeSkillButton::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->drawPixmap(0, 0, _m_bgPixmap[(int)_m_state]);
    if (_m_skillType == S_SKILL_ATTACHEDLORD) {
        int nline = _m_skill->objectName().indexOf("-");
        if (nline == -1)
            nline = _m_skill->objectName().indexOf("_");
        QString engskillname = _m_skill->objectName().split("_").first(); //left(nline);
        QString HegSkillname = engskillname + "_hegemony";
        QString generalName = "";

        foreach (const Player *p, Self->getSiblings()) {
            const General *general = p->getGeneral();
            if (general->hasSkill(engskillname) || general->hasSkill(HegSkillname)) {
                generalName = general->objectName();
                break;
            }
            if (ServerInfo.Enable2ndGeneral) {
                const General *general2 = p->getGeneral2();
                if (general2->hasSkill(engskillname) || general2->hasSkill(HegSkillname)) {
                    generalName = general2->objectName();
                    break;
                }
            }
        }
        if (generalName == "") {
            const General *general = Self->getGeneral();
            if (general && (general->hasSkill(engskillname) || general->hasSkill(HegSkillname)))
                generalName = general->objectName();
            if (ServerInfo.Enable2ndGeneral) {
                const General *general2 = Self->getGeneral2();
                if (general2->hasSkill(engskillname) || general2->hasSkill(HegSkillname)) {
                    generalName = general2->objectName();
                }
            }
        }
        if (generalName != "") {
            if (generalName.endsWith("_hegemony"))
                generalName = generalName.replace("_hegemony", "");

            QString path = G_ROOM_SKIN.getButtonPixmapPath(G_ROOM_SKIN.S_SKIN_KEY_BUTTON_SKILL, getSkillTypeString(_m_skillType), _m_state);
            int n = path.lastIndexOf("/");
            path = path.left(n + 1) + generalName + ".png";
            QPixmap pixmap = G_ROOM_SKIN.getPixmapFromFileName(path);
            if (!pixmap.isNull()) {
                int h = pixmap.height() - _m_bgPixmap[(int)_m_state].height();
                painter->drawPixmap(0, -h, pixmap.width(), pixmap.height(), pixmap);
            }
        }
    }

    if (Self->isSkillInvalid(_m_skill->objectName())) { //for SkillInvalid
        painter->setRenderHints(QPainter::HighQualityAntialiasing);
        QPen pen(Qt::red);
        pen.setWidth(3);
        painter->setPen(pen);
        painter->drawLine(25, 6, _m_size.width() - 6, 20);
        painter->drawLine(25, 20, _m_size.width() - 6, 6);
    }

    if (getState() == S_STATE_CANPRESHOW) { //for  Hegemony mode S_STATE_CANPRESHOW
        painter->setRenderHints(QPainter::HighQualityAntialiasing);
        QPen pen(Qt::yellow);
        pen.setWidth(3);
        painter->setPen(pen);

        painter->drawLine(0, 2, 0, _m_size.height() - 2);
        painter->drawLine(_m_size.width() - 6, 6, _m_size.width() - 6, _m_size.height() - 2);
        painter->drawLine(0, 2, _m_size.width() - 6, 2);
        painter->drawLine(0, _m_size.height() - 2, _m_size.width() - 6, _m_size.height() - 2);
    }
}

QSanSkillButton *QSanInvokeSkillDock::addSkillButtonByName(const QString &skillName)
{
    Q_ASSERT(getSkillButtonByName(skillName) == nullptr);
    QSanInvokeSkillButton *button = new QSanInvokeSkillButton(this);

    const Skill *skill = Sanguosha->getSkill(skillName);
    button->setSkill(skill);
    connect(button, SIGNAL(skill_activated(const Skill *)), this, SIGNAL(skill_activated(const Skill *)));
    connect(button, SIGNAL(skill_deactivated(const Skill *)), this, SIGNAL(skill_deactivated(const Skill *)));
    _m_buttons.append(button);
    update();
    return button;
}

int QSanInvokeSkillDock::width() const
{
    return _m_width;
}

int QSanInvokeSkillDock::height() const
{
    return _m_buttons.length() / 3 * G_DASHBOARD_LAYOUT.m_skillButtonsSize[0].height();
}

void QSanInvokeSkillDock::setWidth(int width)
{
    _m_width = width;
}

void QSanInvokeSkillDock::update()
{
    if (!_m_buttons.isEmpty()) {
        QList<QSanInvokeSkillButton *> regular_buttons, lordskill_buttons, all_buttons;

        foreach (QSanInvokeSkillButton *btn, _m_buttons) {
            if (!btn->getSkill()->shouldBeVisible(Self)) {
                btn->setVisible(false);
                continue;
            } else {
                btn->setVisible(true);
            }
            if (btn->getSkill()->isAttachedLordSkill())
                lordskill_buttons << btn;
            else
                regular_buttons << btn;
        }
        all_buttons = regular_buttons + lordskill_buttons;

        int numButtons = regular_buttons.length();
        int lordskillNum = lordskill_buttons.length();
        //Q_ASSERT(lordskillNum <= 6); // HuangTian, ZhiBa, DrJiuYuan and XianSi
        int rows = (numButtons == 0) ? 0 : (numButtons - 1) / 3 + 1;
        int rowH = G_DASHBOARD_LAYOUT.m_skillButtonsSize[0].height();
        //G_DASHBOARD_LAYOUT.m_normalHeight
        //  _m_width  G_DASHBOARD_LAYOUT.m_rightWidth
        int *btnNum = new int[rows + 2 + 1]; //new int[rows + 2 + 1]; // we allocate one more row in case we need it.
        int remainingBtns = numButtons;
        int *lordBtnNum = new int[2 + 1]; //just consider 2 rows lord skills

        //set button width
        for (int i = 0; i < rows; i++) {
            btnNum[i] = qMin(3, remainingBtns);
            remainingBtns -= 3;
        }
        if (lordskillNum > 3) {
            int half = lordskillNum / 2;
            lordBtnNum[0] = half; //btnNum[rows] = half;
            lordBtnNum[1] = lordskillNum - half; //btnNum[rows + 1] = lordskillNum - half;
        } else if (lordskillNum > 0) {
            lordBtnNum[0] = lordskillNum; //btnNum[rows] = lordskillNum;
        }

        // If the buttons in rows are 3, 1, then balance them to 2, 2
        if (rows >= 2) {
            if (btnNum[rows - 1] == 1 && btnNum[rows - 2] == 3) {
                btnNum[rows - 1] = 2;
                btnNum[rows - 2] = 2;
            }
        } else if (rows == 1 && btnNum[0] == 3) { //&& lordskillNum == 0
            btnNum[0] = 2;
            btnNum[1] = 1;
            rows = 2;
        }

        int m = 0;

        //for regular skill
        for (int i = 0; i < rows; i++) {
            int rowTop = (RoomSceneInstance->m_skillButtonSank) ? (-rowH - 2 * (rows - i - 1)) : ((-rows + i) * rowH);
            int btnWidth = (_m_width - 20) / btnNum[i];
            if (ServerInfo.Enable2ndGeneral)
                btnWidth = (this->objectName() == "left") ? (_m_width + 30) / btnNum[i] : (_m_width - 20) / btnNum[i];
            for (int j = 0; j < btnNum[i]; j++) {
                QSanInvokeSkillButton *button = regular_buttons[m++]; //all_buttons[m++];
                button->setButtonWidth((QSanInvokeSkillButton::SkillButtonWidth)(btnNum[i] - 1));
                button->setPos(btnWidth * j, rowTop);
                if (ServerInfo.Enable2ndGeneral)
                    button->setPos(btnWidth * j + 30, rowTop);
                //QStringList l;
                //l << this->objectName() << button->getSkill()->objectName() << QString::number(_m_width) << QString::number(btnWidth)
                //    << QString::number(btnWidth * j)  << QString::number(rowTop);
                //RoomSceneInstance->addlog(l);
            }
        }

        //for lord skill
        int w = 0;
        int x_ls = 0;
        if (lordskillNum > 0)
            x_ls++;
        if (lordskillNum > 3)
            x_ls++;
        for (int i = 0; i < x_ls; i++) {
            int rowTop = (RoomSceneInstance->m_skillButtonSank) ? (-rowH - 2 * (x_ls - i - 1)) : ((-x_ls + i) * rowH);
            int btnWidth = _m_width / lordBtnNum[i];
            if (lordBtnNum[i] == 1)
                btnWidth = _m_width / 2;
            if (ServerInfo.Enable2ndGeneral)
                btnWidth = btnWidth + 20;
            for (int j = 0; j < lordBtnNum[i]; j++) {
                QSanInvokeSkillButton *button = lordskill_buttons[w++];
                int btntype = lordBtnNum[i] - 1;
                if (btntype == 0)
                    btntype = 1;
                button->setButtonWidth((QSanInvokeSkillButton::SkillButtonWidth)(btntype));
                //button->setPos(0 - btnWidth * (j + 1) - 15, rowTop - G_DASHBOARD_LAYOUT.m_normalHeight);
                button->setPos(0 - btnWidth * (j + 1) - G_DASHBOARD_LAYOUT.m_rightWidth + 45, rowTop - G_DASHBOARD_LAYOUT.m_normalHeight);
                //
                // QStringList l;
                //l << this->objectName() << button->getSkill()->objectName() << QString::number(0 - btnWidth * (j + 1) - G_DASHBOARD_LAYOUT.m_rightWidth + 45)
                //    << QString::number(rowTop - G_DASHBOARD_LAYOUT.m_normalHeight);
                //RoomSceneInstance->addlog(l);
            }
        }

        delete[] btnNum;
        delete[] lordBtnNum;
    }
    QGraphicsObject::update();
}

QSanInvokeSkillButton *QSanInvokeSkillDock::getSkillButtonByName(const QString &skillName) const
{
    foreach (QSanInvokeSkillButton *button, _m_buttons) {
        if (button->getSkill()->objectName() == skillName)
            return button;
    }
    return nullptr;
}

bool QSanButton::isMouseInside() const
{
    QGraphicsScene *scenePtr = scene();
    if (nullptr == scenePtr) {
        return false;
    }

    QPoint cursorPos = QCursor::pos();
    foreach (QGraphicsView *view, scenePtr->views()) {
        QPointF pos = mapFromScene(view->mapToScene(view->mapFromGlobal(cursorPos)));
        if (_isMouseInside(pos)) {
            return true;
        }
    }

    return false;
}
