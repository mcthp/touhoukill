#include "CardFace.h"
#include "RoomObject.h"
#include "engine.h"
#include "player.h"
#include "room.h"
#include "util.h"

#include <QObject>
#include <QString>

class CardFacePrivate
{
public:
    CardFacePrivate()
        : target_fixed(false)
        , throw_when_using(true)
        , has_preact(false)
        , can_damage(false)
        , can_recover(false)
        , has_effectvalue(true)
        , default_method(QSanguosha::MethodUse)
    {
    }

    bool target_fixed;
    bool throw_when_using;
    bool has_preact;
    bool can_damage;
    bool can_recover;
    bool has_effectvalue;

    QSanguosha::HandlingMethod default_method;
};

CardFace::CardFace()
    : d(new CardFacePrivate)
{
}

CardFace::~CardFace()
{
    delete d;
}

QString CardFace::name() const
{
    return QString::fromUtf8(metaObject()->className());
}

QString CardFace::description() const
{
    return QString();
}

QString CardFace::commonEffectName() const
{
    return QString();
}

QString CardFace::effectName() const
{
    return QString();
}

bool CardFace::isKindOf(const char *cardType) const
{
    return inherits(cardType);
}

bool CardFace::matchType(const QString &pattern) const
{
    foreach (const QString &ptn, pattern.split(QStringLiteral("+"))) {
        if (typeName() == ptn || subTypeName() == ptn)
            return true;
    }
    return false;
}

bool CardFace::isNDTrick() const
{
    return false;
}

bool CardFace::canDamage() const
{
    return d->can_damage;
}

void CardFace::setCanDamage(bool can)
{
    d->can_damage = can;
}

bool CardFace::canRecover() const
{
    return d->can_recover;
}

void CardFace::setCanRecover(bool can)
{
    d->can_recover = can;
}

bool CardFace::hasEffectValue() const
{
    return d->has_effectvalue;
}

void CardFace::setHasEffectValue(bool can)
{
    d->has_effectvalue = can;
}

bool CardFace::throwWhenUsing() const
{
    return d->throw_when_using;
}

void CardFace::setThrowWhenUsing(bool can)
{
    d->throw_when_using = can;
}

bool CardFace::hasPreAction() const
{
    return d->has_preact;
}

void CardFace::setHasPreAction(bool can)
{
    d->has_preact = can;
}

QSanguosha::HandlingMethod CardFace::defaultHandlingMethod() const
{
    return d->default_method;
}

void CardFace::setDefaultHandlingMethod(QSanguosha::HandlingMethod can)
{
    d->default_method = can;
}

bool CardFace::targetFixed(const Player * /*unused*/, const Card * /*unused*/) const
{
    return d->target_fixed;
}

void CardFace::setTargetFixed(bool can)
{
    d->target_fixed = can;
}

bool CardFace::targetsFeasible(const QList<const Player *> &targets, const Player *Self, const Card *card) const
{
    if (targetFixed(Self, card))
        return true;
    else
        return !targets.isEmpty();
}

int CardFace::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self, const Card * /*unused*/) const
{
    return (targets.isEmpty() && to_select != Self) ? 1 : 0;
}

bool CardFace::isAvailable(const Player *player, const Card *card) const
{
    return !player->isCardLimited(card, card->handleMethod());
}

bool CardFace::ignoreCardValidity(const Player * /*unused*/) const
{
    return false;
}

const Card *CardFace::validate(const CardUseStruct &use) const
{
    return use.card;
}

const Card *CardFace::validateInResponse(ServerPlayer * /*unused*/, const Card *original_card) const
{
    return original_card;
}

void CardFace::doPreAction(Room * /*unused*/, const CardUseStruct & /*unused*/) const
{
}

void CardFace::onUse(Room *room, const CardUseStruct &use) const
{
    CardUseStruct card_use = use;
    ServerPlayer *player = card_use.from;

    room->sortByActionOrder(card_use.to);

    QList<ServerPlayer *> targets = card_use.to;
    if (room->getMode() == QStringLiteral("06_3v3") && (isKindOf("AOE") || isKindOf("GlobalEffect")))
        room->reverseFor3v3(card_use.card, player, targets);
    card_use.to = targets;

    bool hidden = (type() == TypeSkill && !card_use.card->throwWhenUsing());
    LogMessage log;
    log.from = player;
    if (!targetFixed(card_use.from, card_use.card) || card_use.to.length() > 1 || !card_use.to.contains(card_use.from))
        log.to = card_use.to;
    log.type = QStringLiteral("#UseCard");
    log.card_str = card_use.card->toString(hidden);
    room->sendLog(log);

    IDSet used_cards;
    QList<CardsMoveStruct> moves;
    if (card_use.card->isVirtualCard())
        used_cards.unite(card_use.card->subcards());
    else
        used_cards.insert(card_use.card->effectiveID());

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();
    Q_ASSERT(thread != nullptr);
    thread->trigger(PreCardUsed, data);
    card_use = data.value<CardUseStruct>();

    if (type() != TypeSkill) {
        CardMoveReason reason(CardMoveReason::S_REASON_USE, player->objectName(), QString(), card_use.card->skillName(), QString());
        if (card_use.to.size() == 1)
            reason.m_targetId = card_use.to.first()->objectName();

        reason.m_extraData = QVariant::fromValue(card_use.card);

        foreach (int id, used_cards) {
            CardsMoveStruct move(id, nullptr, Player::PlaceTable, reason);
            moves.append(move);
        }
        room->moveCardsAtomic(moves, true);
        // show general
        player->showHiddenSkill(card_use.card->showSkillName());
    } else {
        if (card_use.card->throwWhenUsing()) {
            CardMoveReason reason(CardMoveReason::S_REASON_THROW, player->objectName(), QString(), card_use.card->skillName(), QString());
            room->moveCardTo(card_use.card, player, nullptr, Player::DiscardPile, reason, true);
        }
        player->showHiddenSkill(card_use.card->showSkillName());
    }

    thread->trigger(CardUsed, data);
    thread->trigger(CardFinished, data);
}

void CardFace::use(Room *room, const CardUseStruct &use) const
{
    ServerPlayer *source = use.from;

    QStringList nullified_list = room->getTag(QStringLiteral("CardUseNullifiedList")).toStringList();
    bool all_nullified = nullified_list.contains(QStringLiteral("_ALL_TARGETS"));
    int magic_drank = 0;
    if (isNDTrick() && (source != nullptr) && source->getMark(QStringLiteral("magic_drank")) > 0)
        magic_drank = source->getMark(QStringLiteral("magic_drank"));

    foreach (ServerPlayer *target, use.to) {
        CardEffectStruct effect;
        effect.card = use.card;
        effect.from = source;
        effect.to = target;
        effect.multiple = (use.to.length() > 1);
        effect.nullified = (all_nullified || nullified_list.contains(target->objectName()));
        if (use.card->hasFlag(QStringLiteral("mopao")))
            effect.effectValue.first() = effect.effectValue.first() + 1;
        if (use.card->hasFlag(QStringLiteral("mopao2")))
            effect.effectValue.last() = effect.effectValue.last() + 1;
        if (source != nullptr && source->getMark(QStringLiteral("kuangji_value")) > 0) {
            effect.effectValue.first() = effect.effectValue.first() + source->getMark(QStringLiteral("kuangji_value"));
            effect.effectValue.last() = effect.effectValue.last() + source->getMark(QStringLiteral("kuangji_value"));
            room->setPlayerMark(source, QStringLiteral("kuangji_value"), 0);
        }

        effect.effectValue.first() = effect.effectValue.first() + magic_drank;
        QVariantList players;
        for (int i = use.to.indexOf(target); i < use.to.length(); i++) {
            if (!nullified_list.contains(use.to.at(i)->objectName()) && !all_nullified)
                players.append(QVariant::fromValue(use.to.at(i)));
        }

        room->setTag(QStringLiteral("targets") + use.card->toString(), QVariant::fromValue(players));

        room->cardEffect(effect);
    }
    room->removeTag(QStringLiteral("targets") + use.card->toString()); //for ai?
    if (source != nullptr && magic_drank > 0)
        room->setPlayerMark(source, QStringLiteral("magic_drank"), 0);

    if (room->getCardPlace(use.card->effectiveID()) == Player::PlaceTable) {
        CardMoveReason reason(CardMoveReason::S_REASON_USE, source != nullptr ? source->objectName() : QString(), QString(), use.card->skillName(), QString());
        if (use.to.size() == 1)
            reason.m_targetId = use.to.first()->objectName();
        reason.m_extraData = QVariant::fromValue(use.card);
        ServerPlayer *provider = nullptr;
        foreach (const QString &flag, use.card->flags()) {
            if (flag.startsWith(QStringLiteral("CardProvider_"))) {
                QStringList patterns = flag.split(QStringLiteral("_"));
                provider = room->findPlayerByObjectName(patterns.at(1));
                break;
            }
        }

        ServerPlayer *from = source;
        if (provider != nullptr)
            from = provider;
        room->moveCardTo(use.card, from, nullptr, Player::DiscardPile, reason, true);
    }
}

void CardFace::onEffect(const CardEffectStruct & /*unused*/) const
{
}

bool CardFace::isCancelable(const CardEffectStruct & /*unused*/) const
{
    return false;
}

void CardFace::onNullified(ServerPlayer * /*unused*/, const Card * /*unused*/) const
{
}

CardFace::CardType BasicCard::type() const
{
    return TypeBasic;
}

QString BasicCard::typeName() const
{
    return QStringLiteral("basic");
}

CardFace::CardType EquipCard::type() const
{
    return TypeEquip;
}

QString EquipCard::typeName() const
{
    return QStringLiteral("equip");
}

QString Weapon::subTypeName() const
{
    return QStringLiteral("weapon");
}

EquipCard::Location Weapon::location() const
{
    return WeaponLocation;
}

QString Armor::subTypeName() const
{
    return QStringLiteral("armor");
}

EquipCard::Location Armor::location() const
{
    return ArmorLocation;
}

QString DefensiveHorse::subTypeName() const
{
    return QStringLiteral("defensive_horse");
}

EquipCard::Location DefensiveHorse::location() const
{
    return DefensiveHorseLocation;
}

QString OffensiveHorse::subTypeName() const
{
    return QStringLiteral("offensive_horse");
}

EquipCard::Location OffensiveHorse::location() const
{
    return OffensiveHorseLocation;
}

QString Treasure::subTypeName() const
{
    return QStringLiteral("treasure");
}

EquipCard::Location Treasure::location() const
{
    return TreasureLocation;
}

CardFace::CardType TrickCard::type() const
{
    return TypeTrick;
}

QString TrickCard::typeName() const
{
    return QStringLiteral("trick");
}

QString NonDelayedTrick::subTypeName() const
{
    return QStringLiteral("non_delayed_trick");
}

bool NonDelayedTrick::isNDTrick() const
{
    return true;
}

DelayedTrick::DelayedTrick()
    : d(nullptr)
{
}

QString DelayedTrick::subTypeName() const
{
    return QStringLiteral("delayed_trick");
}

JudgeStruct DelayedTrick::judge() const
{
    if (d == nullptr)
        return JudgeStruct();

    return *d;
}

CardFace::CardType SkillCard::type() const
{
    return TypeSkill;
}

QString SkillCard::typeName() const
{
    return QStringLiteral("skill");
}

QString SkillCard::subTypeName() const
{
    return QStringLiteral("skill");
}
