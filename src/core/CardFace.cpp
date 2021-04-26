#include "CardFace.h"
#include "RoomObject.h"
#include "engine.h"
#include "player.h"
#include "room.h"
#include "util.h"

#include <QObject>
#include <QString>

namespace RefactorProposal {

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
    {
    }

    bool target_fixed;
    bool throw_when_using;
    bool has_preact;
    bool can_damage;
    bool can_recover;
    bool has_effectvalue;
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
    return staticMetaObject.className();
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
    return staticMetaObject.inherits(&(Sanguosha->getCardFace(cardType)->staticMetaObject)); // depends on Qt 5.7
}

bool CardFace::matchType(const QString &pattern) const
{
    foreach (const QString &ptn, pattern.split("+")) {
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

bool CardFace::canRecover() const
{
    return d->can_recover;
}

bool CardFace::hasEffectValue() const
{
    return d->has_effectvalue;
}

bool CardFace::throwWhenUsing() const
{
    return d->throw_when_using;
}

bool CardFace::hasPreAction() const
{
    return d->has_preact;
}

bool CardFace::targetFixed(const Player *, const Card *) const
{
    return d->target_fixed;
}

bool CardFace::targetsFeasible(const QList<const Player *> &targets, const Player *Self, const Card *card) const
{
    if (targetFixed(Self, card))
        return true;
    else
        return !targets.isEmpty();
}

int CardFace::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self, const Card *) const
{
    return (targets.isEmpty() && to_select != Self) ? 1 : 0;
}

bool CardFace::isAvailable(const Player *player, const Card *card) const
{
    return !player->isCardLimited(fixme_cast< ::Card *>(card), fixme_cast< ::Card::HandlingMethod>(card->handleMethod()))
        || (card->canRecast() && !player->isCardLimited(fixme_cast< ::Card *>(card), fixme_cast< ::Card::HandlingMethod>(Card::MethodRecast)));
}

bool CardFace::ignoreCardValidity(const Player *) const
{
    return false;
}

const Card *CardFace::validate(const CardUseStruct &use) const
{
    return use.card;
}

const Card *CardFace::validateInResponse(ServerPlayer *, const Card *original_card) const
{
    return original_card;
}

void CardFace::doPreAction(Room *, const CardUseStruct &) const
{
}

void CardFace::onUse(Room *room, const CardUseStruct &use) const
{
    CardUseStruct card_use = use;
    ServerPlayer *player = card_use.from;

    room->sortByActionOrder(card_use.to);

    QList<ServerPlayer *> targets = card_use.to;
    if (room->getMode() == "06_3v3" && (isKindOf("AOE") || isKindOf("GlobalEffect")))
        room->reverseFor3v3(fixme_cast< ::Card *>(card_use.card), player, targets);
    card_use.to = targets;

    bool hidden = (type() == TypeSkill && !card_use.card->throwWhenUsing());
    LogMessage log;
    log.from = player;
    if (!targetFixed(card_use.from, card_use.card) || card_use.to.length() > 1 || !card_use.to.contains(card_use.from))
        log.to = card_use.to;
    log.type = "#UseCard";
    log.card_str = card_use.card->toString(hidden);
    room->sendLog(log);

    QSet<int> used_cards;
    QList<CardsMoveStruct> moves;
    if (card_use.card->isVirtualCard())
        used_cards.unite(card_use.card->subcards());
    else
        used_cards.insert(card_use.card->effectiveID());

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = room->getThread();
    Q_ASSERT(thread != nullptr);
    thread->trigger(PreCardUsed, room, data);
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
            room->moveCardTo(fixme_cast< ::Card *>(card_use.card), player, nullptr, Player::DiscardPile, reason, true);
        }
        player->showHiddenSkill(card_use.card->showSkillName());
    }

    thread->trigger(CardUsed, room, data);
    thread->trigger(CardFinished, room, data);
}

void CardFace::use(Room *room, const CardUseStruct &use) const
{
    ServerPlayer *source = use.from;

    QStringList nullified_list = room->getTag("CardUseNullifiedList").toStringList();
    bool all_nullified = nullified_list.contains("_ALL_TARGETS");
    int magic_drank = 0;
    if (isNDTrick() && source && source->getMark("magic_drank") > 0)
        magic_drank = source->getMark("magic_drank");

    foreach (ServerPlayer *target, use.to) {
        CardEffectStruct effect;
        effect.card = use.card;
        effect.from = source;
        effect.to = target;
        effect.multiple = (use.to.length() > 1);
        effect.nullified = (all_nullified || nullified_list.contains(target->objectName()));
        if (use.card->hasFlag("mopao"))
            effect.effectValue.first() = effect.effectValue.first() + 1;
        if (use.card->hasFlag("mopao2"))
            effect.effectValue.last() = effect.effectValue.last() + 1;
        if (source->getMark("kuangji_value") > 0) {
            effect.effectValue.first() = effect.effectValue.first() + source->getMark("kuangji_value");
            effect.effectValue.last() = effect.effectValue.last() + source->getMark("kuangji_value");
            room->setPlayerMark(source, "kuangji_value", 0);
        }

        effect.effectValue.first() = effect.effectValue.first() + magic_drank;
        QVariantList players;
        for (int i = use.to.indexOf(target); i < use.to.length(); i++) {
            if (!nullified_list.contains(use.to.at(i)->objectName()) && !all_nullified)
                players.append(QVariant::fromValue(use.to.at(i)));
        }

        room->setTag("targets" + use.card->toString(), QVariant::fromValue(players));

        room->cardEffect(*(fixme_cast< ::CardEffectStruct *>(&effect)));
    }
    room->removeTag("targets" + use.card->toString()); //for ai?
    if (magic_drank > 0)
        room->setPlayerMark(source, "magic_drank", 0);

    if (room->getCardPlace(use.card->effectiveID()) == Player::PlaceTable) {
        CardMoveReason reason(CardMoveReason::S_REASON_USE, source->objectName(), QString(), use.card->skillName(), QString());
        if (use.to.size() == 1)
            reason.m_targetId = use.to.first()->objectName();
        reason.m_extraData = QVariant::fromValue(use.card);
        ServerPlayer *provider = nullptr;
        foreach (const QString &flag, use.card->flags()) {
            if (flag.startsWith("CardProvider_")) {
                QStringList patterns = flag.split("_");
                provider = room->findPlayerByObjectName(patterns.at(1));
                break;
            }
        }

        ServerPlayer *from = source;
        if (provider != nullptr)
            from = provider;
        room->moveCardTo(fixme_cast< ::Card *>(use.card), from, nullptr, Player::DiscardPile, reason, true);
    }
}

void CardFace::onEffect(const CardEffectStruct &) const
{
}

bool CardFace::isCancelable(const CardEffectStruct &) const
{
    return false;
}

void CardFace::onNullified(ServerPlayer *, const Card *) const
{
}

BasicCard::BasicCard()
{
}

CardFace::CardType BasicCard::type() const
{
    return TypeBasic;
}

QString BasicCard::typeName() const
{
    return "basic";
}

QString BasicCard::subTypeName() const
{
    return "basic";
}

}