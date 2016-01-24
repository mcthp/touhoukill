#include "skill.h"
#include "settings.h"
#include "engine.h"
#include "player.h"
#include "room.h"
#include "client.h"
#include "standard.h"
#include "scenario.h"

#include <QFile>

Skill::Skill(const QString &name, Frequency frequency)
    : frequency(frequency), limit_mark(QString()), attached_lord_skill(false)
{
    static QChar lord_symbol('$');

    if (name.endsWith(lord_symbol)) {
        QString copy = name;
        copy.remove(lord_symbol);
        setObjectName(copy);
        lord_skill = true;
    } else {
        setObjectName(name);
        lord_skill = false;
    }
}

bool Skill::isLordSkill() const
{
    return lord_skill;
}

bool Skill::isAttachedLordSkill() const
{
    return attached_lord_skill;
}

bool Skill::shouldBeVisible(const Player *Self) const
{
    return Self != NULL;
}

QString Skill::getDescription(bool yellow) const
{
    bool normal_game = ServerInfo.DuringGame && isNormalGameMode(ServerInfo.GameMode);
    QString name = QString("%1%2").arg(objectName()).arg(normal_game ? "_p" : "");
    QString des_src = Sanguosha->translate(":" + name);
    if (normal_game && des_src.startsWith(":"))
        des_src = Sanguosha->translate(":" + objectName());
    if (des_src.startsWith(":"))
        return QString();
    return QString("<font color=%1>%2</font>").arg(yellow ? "#FFFF33" : "#FF0080").arg(des_src);
}

QString Skill::getNotice(int index) const
{
    if (index == -1)
        return Sanguosha->translate("~" + objectName());

    return Sanguosha->translate(QString("~%1%2").arg(objectName()).arg(index));
}

bool Skill::isVisible() const
{
    return !objectName().startsWith("#");
}

int Skill::getEffectIndex(const ServerPlayer *, const Card *) const
{
    return -1;
}

void Skill::initMediaSource()
{
    sources.clear();
    for (int i = 1;; i++) {
        QString effect_file = QString("audio/skill/%1%2.ogg").arg(objectName()).arg(QString::number(i));
        if (QFile::exists(effect_file))
            sources << effect_file;
        else
            break;
    }

    if (sources.isEmpty()) {
        QString effect_file = QString("audio/skill/%1.ogg").arg(objectName());
        if (QFile::exists(effect_file))
            sources << effect_file;
    }
}

void Skill::playAudioEffect(int index) const
{
    if (!sources.isEmpty()) {
        if (index == -1)
            index = qrand() % sources.length();
        else
            index--;

        // check length
        QString filename;
        if (index >= 0 && index < sources.length())
            filename = sources.at(index);
        else if (index >= sources.length()) {
            while (index >= sources.length())
                index -= sources.length();
            filename = sources.at(index);
        } else
            filename = sources.first();

        Sanguosha->playAudioEffect(filename);
        if (ClientInstance)
            ClientInstance->setLines(filename);
    }
}

Skill::Frequency Skill::getFrequency() const
{
    return frequency;
}

QString Skill::getLimitMark() const
{
    return limit_mark;
}

QStringList Skill::getSources() const
{
    return sources;
}

QDialog *Skill::getDialog() const
{
    return NULL;
}

ViewAsSkill::ViewAsSkill(const QString &name)
    : Skill(name), response_pattern(QString()), response_or_use(false), expand_pile(QString())
{
}

bool ViewAsSkill::isAvailable(const Player *invoker,
    CardUseStruct::CardUseReason reason,
    const QString &pattern) const
{
    if (!invoker->hasSkill(objectName()) && !invoker->hasLordSkill(objectName())
        && !invoker->hasFlag(objectName())) // For Shuangxiong
        return false;
    switch (reason) {
        case CardUseStruct::CARD_USE_REASON_PLAY: return isEnabledAtPlay(invoker);
        case CardUseStruct::CARD_USE_REASON_RESPONSE:
        case CardUseStruct::CARD_USE_REASON_RESPONSE_USE: return isEnabledAtResponse(invoker, pattern);
        default:
            return false;
    }
}

bool ViewAsSkill::isEnabledAtPlay(const Player *) const
{
    return response_pattern.isEmpty();
}

bool ViewAsSkill::isEnabledAtResponse(const Player *, const QString &pattern) const
{
    if (!response_pattern.isEmpty())
        return pattern == response_pattern;
    return false;
}

bool ViewAsSkill::isEnabledAtNullification(const ServerPlayer *) const
{
    return false;
}

const ViewAsSkill *ViewAsSkill::parseViewAsSkill(const Skill *skill)
{
    if (skill == NULL) return NULL;
    if (skill->inherits("ViewAsSkill")) {
        const ViewAsSkill *view_as_skill = qobject_cast<const ViewAsSkill *>(skill);
        return view_as_skill;
    }
    if (skill->inherits("TriggerSkill")) {
        const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
        Q_ASSERT(trigger_skill != NULL);
        const ViewAsSkill *view_as_skill = trigger_skill->getViewAsSkill();
        if (view_as_skill != NULL) return view_as_skill;
    }
    return NULL;
}

ZeroCardViewAsSkill::ZeroCardViewAsSkill(const QString &name)
    : ViewAsSkill(name)
{
}

const Card *ZeroCardViewAsSkill::viewAs(const QList<const Card *> &cards) const
{
    if (cards.isEmpty())
        return viewAs();
    else
        return NULL;
}

bool ZeroCardViewAsSkill::viewFilter(const QList<const Card *> &, const Card *) const
{
    return false;
}

OneCardViewAsSkill::OneCardViewAsSkill(const QString &name)
    : ViewAsSkill(name), filter_pattern(QString())
{
}

bool OneCardViewAsSkill::viewFilter(const QList<const Card *> &selected, const Card *to_select) const
{
    return selected.isEmpty() && !to_select->hasFlag("using") && viewFilter(to_select);
}

bool OneCardViewAsSkill::viewFilter(const Card *to_select) const
{
    if (!inherits("FilterSkill") && !filter_pattern.isEmpty()) {
        QString pat = filter_pattern;
        if (pat.endsWith("!")) {
            if (Self->isJilei(to_select)) return false;
            pat.chop(1);
        } else if (response_or_use && pat.contains("hand")) {
            pat.replace("hand", "hand,wooden_ox,piao");
        }
        ExpPattern pattern(pat);
        return pattern.match(Self, to_select);
    }
    return false;
}

const Card *OneCardViewAsSkill::viewAs(const QList<const Card *> &cards) const
{
    if (cards.length() != 1)
        return NULL;
    else
        return viewAs(cards.first());
}

FilterSkill::FilterSkill(const QString &name)
    : OneCardViewAsSkill(name)
{
    frequency = Compulsory;
}

TriggerSkill::TriggerSkill(const QString &name)
    : Skill(name), view_as_skill(NULL), global(false)
{
}

const ViewAsSkill *TriggerSkill::getViewAsSkill() const
{
    return view_as_skill;
}

QList<TriggerEvent> TriggerSkill::getTriggerEvents() const
{
    return events;
}

int TriggerSkill::getPriority() const
{
    return 2;
}

void TriggerSkill::record(TriggerEvent, Room *, QVariant &) const
{

}

QList<SkillInvokeDetail> TriggerSkill::triggerable(TriggerEvent, const Room *, const QVariant &) const
{
    return QList<SkillInvokeDetail>();
}

bool TriggerSkill::cost(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
{
    if (invoke->isCompulsory)
        return true;
    else {
        if (invoke->invoker != NULL)
            return invoke->invoker->askForSkillInvoke(this, data);
    }

    return false;
}

bool TriggerSkill::effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail>, QVariant &) const
{
    return false;
}

ScenarioRule::ScenarioRule(Scenario *scenario)
    :TriggerSkill(scenario->objectName())
{
    setParent(scenario);
}

int ScenarioRule::getPriority() const
{
    return 1;
}

QList<SkillInvokeDetail> ScenarioRule::triggerable(TriggerEvent triggerEvent, const Room * room, const QVariant & data) const
{
    return QList<SkillInvokeDetail>() << SkillInvokeDetail(this);
}

MasochismSkill::MasochismSkill(const QString &name)
    : TriggerSkill(name)
{
    events << Damaged;
}

bool MasochismSkill::effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
{
    DamageStruct damage = data.value<DamageStruct>();
    onDamaged(damage);

    return false;
}

PhaseChangeSkill::PhaseChangeSkill(const QString &name)
    : TriggerSkill(name)
{
    events << EventPhaseStart;
}

bool PhaseChangeSkill::effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
{
    ServerPlayer *player = data.value<ServerPlayer *>();
    return onPhaseChange(player);
}

DrawCardsSkill::DrawCardsSkill(const QString &name, bool is_initial)
    : TriggerSkill(name), is_initial(is_initial)
{
    if (is_initial)
        events << DrawInitialCards;
    else
        events << DrawNCards;
}

bool DrawCardsSkill::effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
{
    DrawNCardsStruct s = data.value<DrawNCardsStruct>();
    s.n = getDrawNum(s);
    data = QVariant::fromValue(s);
    return false;
}

GameStartSkill::GameStartSkill(const QString &name)
    : TriggerSkill(name)
{
    events << GameStart;
}

bool GameStartSkill::effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail>, QVariant &) const
{
    onGameStart();
    return false;
}

int MaxCardsSkill::getExtra(const Player *) const
{
    return 0;
}

int MaxCardsSkill::getFixed(const Player *) const
{
    return -1;
}

ProhibitSkill::ProhibitSkill(const QString &name)
    : Skill(name, Skill::Compulsory)
{
}

DistanceSkill::DistanceSkill(const QString &name)
    : Skill(name, Skill::Compulsory)
{
}

MaxCardsSkill::MaxCardsSkill(const QString &name)
    : Skill(name, Skill::Compulsory)
{
}

TargetModSkill::TargetModSkill(const QString &name)
    : Skill(name, Skill::Compulsory)
{
    pattern = "Slash";
}

QString TargetModSkill::getPattern() const
{
    return pattern;
}

int TargetModSkill::getResidueNum(const Player *, const Card *) const
{
    return 0;
}

int TargetModSkill::getDistanceLimit(const Player *, const Card *) const
{
    return 0;
}

int TargetModSkill::getExtraTargetNum(const Player *, const Card *) const
{
    return 0;
}

AttackRangeSkill::AttackRangeSkill(const QString &name) : Skill(name, Skill::Compulsory)
{

}

int AttackRangeSkill::getExtra(const Player *, bool) const
{
    return 0;
}

int AttackRangeSkill::getFixed(const Player *, bool) const
{
    return -1;
}

SlashNoDistanceLimitSkill::SlashNoDistanceLimitSkill(const QString &skill_name)
    : TargetModSkill(QString("#%1-slash-ndl").arg(skill_name)), name(skill_name)
{
}

int SlashNoDistanceLimitSkill::getDistanceLimit(const Player *from, const Card *card) const
{
    if (from->hasSkill(name) && card->getSkillName() == name)
        return 1000;
    else
        return 0;
}

FakeMoveSkill::FakeMoveSkill(const QString &name)
    : TriggerSkill(QString("#%1-fake-move").arg(name)), name(name)
{
    events << BeforeCardsMove << CardsMoveOneTime;
}

int FakeMoveSkill::getPriority() const
{
    return 10;
}

bool FakeMoveSkill::effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail>, QVariant &) const
{
    return true;
}

QList<SkillInvokeDetail> FakeMoveSkill::triggerable(TriggerEvent, const Room *room, const QVariant &) const
{
    ServerPlayer *owner = NULL;
    foreach (ServerPlayer *p, room->getAllPlayers()) {
        if (p->hasSkill(this)) {
            owner = p;
            break;
        }
    }

    QString flag = QString("%1_InTempMoving").arg(name);
    foreach (ServerPlayer *p, room->getAllPlayers()) {
        if (p->hasFlag(flag))
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, owner, p);
    }

    return QList<SkillInvokeDetail>();
}

EquipSkill::EquipSkill(const QString &name)
    : TriggerSkill(name)
{

}

bool EquipSkill::equipAvailable(const ServerPlayer *p, EquipCard::Location location, const QString &equipName)
{
    if (p == NULL)
        return false;

    if (p->getMark("Equips_Nullified_to_Yourself") > 0)
        return false;

    switch (location) {
        case EquipCard::WeaponLocation:
            if (!p->hasWeapon(equipName))
                return false;
            break;
        case EquipCard::ArmorLocation:
            if (!p->hasArmorEffect(equipName))
                return false;
            break;
        case EquipCard::TreasureLocation:
            if (!p->hasTreasure(equipName))
                return false;
            break;
        default:
            break; // shenmegui?
    }

    return true;
}

bool EquipSkill::equipAvailable(const ServerPlayer *p, const EquipCard *card)
{
    if (card == NULL)
        return false;

    return equipAvailable(p, card->location(), card->objectName());
}

WeaponSkill::WeaponSkill(const QString &name)
    : EquipSkill(name)
{
}

ArmorSkill::ArmorSkill(const QString &name)
    : EquipSkill(name)
{
}

TreasureSkill::TreasureSkill(const QString &name)
    : EquipSkill(name)
{
}

