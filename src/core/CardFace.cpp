#include "CardFace.h"
#include "RoomObject.h"
#include "card.h"
#include "engine.h"
#include "lua-wrapper.h"
#include "player.h"
#include "util.h"

// TODO: kill this
#include "room.h"

#include "lua.hpp"

#include <QObject>
#include <QString>

#include <optional>

using namespace QSanguosha;

class CardFacePrivate
{
public:
    CardFacePrivate()
    {
    }

    QString name;
    QString subTypeName;

    QStringList kind;

    std::optional<bool> target_fixed;
    std::optional<bool> has_preact;
    std::optional<bool> can_damage;
    std::optional<bool> can_recover;
    std::optional<bool> has_effectvalue;

    std::optional<HandlingMethod> default_method;
};

// somewhat not-even-be-a-method method for Lua calls which need to be done on SWIG side
// SWIG don't provide a binary-compatible way to export its constant variables
namespace CardFaceLuaCall {
bool targetFixed(lua_State *l, const Player *player, const Card *card);
bool targetsFeasible(lua_State *l, const QList<const Player *> &targets, const Player *Self, const Card *card);
} // namespace CardFaceLuaCall

// -- type (which is specified by desc.type using SecondTypeMask / ThirdTypeMask)
CardFace::CardFace(const QString &name)
    : d(new CardFacePrivate)
{
    d->name = name;
}

CardFace::~CardFace()
{
    delete d;
}

// -- name -> string
QString CardFace::name() const
{
    return d->name;
}

// -- subTypeName -> string
QString CardFace::subTypeName() const
{
    if (d->subTypeName.isEmpty()) {
        LuaStatePointer l = LuaMultiThreadEnvironment::luaStateForCurrentThread();
        bool pushed = l->pushCardFace(d->name); // { CardFace(if successful) }
        if (!pushed)
            return QString();

        do {
            int type = lua_getfield(l, -1, "subTypeName"); // { CardFace.subTypeName, CardFace }
            do {
                if (type != LUA_TSTRING)
                    break;
                d->subTypeName = QString::fromUtf8(lua_tostring(l, -1));
            } while (false);
            lua_pop(l, 1); // { CardFace }
        } while (false);
        lua_pop(l, 1); // { }
    }

    return d->subTypeName;
}

// -- kind -> table (sequence) of strings
bool CardFace::isKindOf(const QString &cardType) const
{
    if (d->kind.isEmpty()) {
        LuaStatePointer l = LuaMultiThreadEnvironment::luaStateForCurrentThread();
        bool pushed = l->pushCardFace(d->name); // { CardFace(if successful) }
        if (!pushed)
            return cardType == QStringLiteral("CardFace");

        do {
            int type = lua_getfield(l, -1, "kind"); // { CardFace.kind, CardFace }
            do {
                if (type != LUA_TTABLE)
                    break;

                for (lua_pushnil(l); (bool)(lua_next(l, -2)); lua_pop(l, 1)) { // { v, k, CardFace.kind, CardFace }
                    type = lua_type(l, -1);
                    if (type != LUA_TSTRING)
                        continue;

                    d->kind << QString::fromUtf8(lua_tostring(l, -1));
                } // { CardFace.kind, CardFace }
            } while (false);
            lua_pop(l, 1); // { CardFace }
        } while (false);
        lua_pop(l, 1); // { }
    }

    if (d->kind.isEmpty())
        return cardType == QStringLiteral("CardFace");

    return d->kind.contains(cardType);
}

// --  - canDamage = function() -> boolean
bool CardFace::canDamage() const
{
    bool r = false;

    if (!d->can_damage.has_value()) {
        LuaStatePointer l = LuaMultiThreadEnvironment::luaStateForCurrentThread();
        bool pushed = l->pushCardFace(d->name); // { CardFace(if successful) }
        if (!pushed)
            return false;

        do {
            int type = lua_getfield(l, -1, "canDamage"); // { CardFace.canDamage, CardFace }
            do {
                if (type == LUA_TBOOLEAN) {
                    r = lua_toboolean(l, -1);
                    d->can_damage = r;
                } else if (type == LUA_TFUNCTION) {
                    // we should do the function call and return
                    // error should be catched here
                    int call = lua_pcall(l, 0, 1, 0); // { cardFace.canDamage() / error, CardFace }
                    if (call == LUA_OK)
                        r = lua_toboolean(l, -1);

                    // DO NOT STORE d->can_damage
                } else {
                    // neither boolean nor function, ignored
                }
            } while (false);
            lua_pop(l, 1); // { CardFace }
        } while (false);
        lua_pop(l, 1); // { }
    }

    return d->can_damage.value_or(r);
}

void CardFace::setCanDamage(bool can)
{
    d->can_damage = can;
}

// --  - canRecover = function() -> boolean
bool CardFace::canRecover() const
{
    bool r = false;

    if (!d->can_recover.has_value()) {
        LuaStatePointer l = LuaMultiThreadEnvironment::luaStateForCurrentThread();
        bool pushed = l->pushCardFace(d->name); // { CardFace(if successful) }
        if (!pushed)
            return false;

        do {
            int type = lua_getfield(l, -1, "canRecover"); // { CardFace.canRecover, CardFace }
            do {
                if (type == LUA_TBOOLEAN) {
                    r = lua_toboolean(l, -1);
                    d->can_recover = r;
                } else if (type == LUA_TFUNCTION) {
                    // we should do the function call and return
                    // error should be catched here
                    int call = lua_pcall(l, 0, 1, 0); // { cardFace.canRecover() / error, CardFace }
                    if (call == LUA_OK)
                        r = lua_toboolean(l, -1);

                    // DO NOT STORE d->can_recover
                } else {
                    // neither boolean nor function, ignored
                }
            } while (false);
            lua_pop(l, 1); // { CardFace }
        } while (false);
        lua_pop(l, 1); // { }
    }

    return d->can_recover.value_or(r);
}

void CardFace::setCanRecover(bool can)
{
    d->can_recover = can;
}

// --  - hasEffectValue = function() -> boolean
bool CardFace::hasEffectValue() const
{
    bool r = false;

    if (!d->has_effectvalue.has_value()) {
        LuaStatePointer l = LuaMultiThreadEnvironment::luaStateForCurrentThread();
        bool pushed = l->pushCardFace(d->name); // { CardFace(if successful) }
        if (!pushed)
            return false;

        do {
            int type = lua_getfield(l, -1, "hasEffectValue"); // { CardFace.hasEffectValue, CardFace }
            do {
                if (type == LUA_TBOOLEAN) {
                    r = lua_toboolean(l, -1);
                    d->has_effectvalue = r;
                } else if (type == LUA_TFUNCTION) {
                    // we should do the function call and return
                    // error should be catched here
                    int call = lua_pcall(l, 0, 1, 0); // { cardFace.hasEffectValue() / error, CardFace }
                    if (call == LUA_OK)
                        r = lua_toboolean(l, -1);

                    // DO NOT STORE d->has_effectvalue
                } else {
                    // neither boolean nor function, ignored
                }
            } while (false);
            lua_pop(l, 1); // { CardFace }
        } while (false);
        lua_pop(l, 1); // { }
    }

    return d->has_effectvalue.value_or(r);
}

void CardFace::setHasEffectValue(bool can)
{
    d->has_effectvalue = can;
}

// --  - hasPreAction = function() -> boolean
bool CardFace::hasPreAction() const
{
    bool r = false;

    if (!d->has_preact.has_value()) {
        LuaStatePointer l = LuaMultiThreadEnvironment::luaStateForCurrentThread();
        bool pushed = l->pushCardFace(d->name); // { CardFace(if successful) }
        if (!pushed)
            return false;

        do {
            int type = lua_getfield(l, -1, "hasPreAction"); // { CardFace.hasPreAction, CardFace }
            do {
                if (type == LUA_TBOOLEAN) {
                    r = lua_toboolean(l, -1);
                    d->has_preact = r;
                } else if (type == LUA_TFUNCTION) {
                    // we should do the function call and return
                    // error should be catched here
                    int call = lua_pcall(l, 0, 1, 0); // { cardFace.hasPreAction() / error, CardFace }
                    if (call == LUA_OK)
                        r = lua_toboolean(l, -1);

                    // DO NOT STORE d->has_preact
                } else {
                    // neither boolean nor function, ignored
                }
            } while (false);
            lua_pop(l, 1); // { CardFace }
        } while (false);
        lua_pop(l, 1); // { }
    }

    return d->has_preact.value_or(r);
}

void CardFace::setHasPreAction(bool can)
{
    d->has_preact = can;
}

// --  - defaultHandlingMethod = function() -> QSanguosha_HandlingMethod
HandlingMethod CardFace::defaultHandlingMethod() const
{
    int r = static_cast<int>(MethodNone);

    if (!d->default_method.has_value()) {
        LuaStatePointer l = LuaMultiThreadEnvironment::luaStateForCurrentThread();
        bool pushed = l->pushCardFace(d->name); // { CardFace(if successful) }
        if (!pushed)
            return MethodNone;

        do {
            int type = lua_getfield(l, -1, "defaultHandlingMethod"); // { CardFace.defaultHandlingMethod, CardFace }
            do {
                if (type == LUA_TNUMBER) {
                    r = lua_tointeger(l, -1);
                    d->default_method = static_cast<HandlingMethod>(r);
                } else if (type == LUA_TFUNCTION) {
                    // we should do the function call and return
                    // error should be catched here
                    int call = lua_pcall(l, 0, 1, 0); // { cardFace.defaultHandlingMethod() / error, CardFace }
                    if (call == LUA_OK)
                        r = lua_tointeger(l, -1);

                    // DO NOT STORE d->default_method
                } else {
                    // neither integer nor function, ignored
                }
            } while (false);
            lua_pop(l, 1); // { CardFace }
        } while (false);
        lua_pop(l, 1); // { }
    }

    return d->default_method.value_or(static_cast<HandlingMethod>(r));
}

void CardFace::setDefaultHandlingMethod(HandlingMethod can)
{
    d->default_method = can;
}

// --  - targetFixed = function(player, card) -> boolean
bool CardFace::targetFixed(const Player *player, const Card *card) const
{
    bool r = false;

    if (!d->target_fixed.has_value()) {
        LuaStatePointer l = LuaMultiThreadEnvironment::luaStateForCurrentThread();
        bool pushed = l->pushCardFace(d->name); // { CardFace(if successful) }
        if (!pushed)
            return false;

        do {
            int type = lua_getfield(l, -1, "targetFixed"); // { CardFace.targetFixed, CardFace }
            do {
                if (type == LUA_TBOOLEAN) {
                    r = lua_toboolean(l, -1);
                    d->target_fixed = r;
                } else if (type == LUA_TFUNCTION) {
                    // we should do the function call and return
                    // error should be catched here
                    bool call = CardFaceLuaCall::targetFixed(l, player, card); // { returnValue / error, CardFace }
                    if (call)
                        r = lua_toboolean(l, -1);
                    else {
                        // error
                        // since the stack top is the error object, we temporarily ignore it
                    }

                    // DO NOT STORE d->target_fixed
                } else {
                    // neither boolean nor function, ignored
                }
            } while (false);
            lua_pop(l, 1); // { CardFace }
        } while (false);
        lua_pop(l, 1); // { }
    }

    return d->target_fixed.value_or(r);
}

void CardFace::setTargetFixed(bool can)
{
    d->target_fixed = can;
}

#if 0
-- Card Face common
-- As a Card Face, the following are mandatory
-- (if different than default) properties, including
-- these may be a fixed value or Lua Function, depanding on its usage. Function prototype is provided in case a function should be used.
-- methods, including
--  - targetFilter - function(playerList, player, player, card) -> integer
--  - isAvailable - function(player, card) -> boolean
--  - validate - function(cardUse) -> card
--  - validateInResponse - function(player, card) -> card
--  - doPreAction - function(room, cardUse)
--  - onUse - function(room, cardUse)
--  - use - function(room, cardUse)
--  - onEffect(cardEffect)
--  - isCancelable(cardEffect) -> boolean
--  - onNullified(player, card)
-- All of them are optional but this card does nothing if none is provided.
#endif

// --  - targetsFeasible - function(playerList, player, card) -> boolean
bool CardFace::targetsFeasible(const QList<const Player *> &targets, const Player *Self, const Card *card) const
{
    std::optional<bool> r;

    LuaStatePointer l = LuaMultiThreadEnvironment::luaStateForCurrentThread();
    bool pushed = l->pushCardFace(d->name); // { CardFace(if successful) }
    if (pushed) {
        do {
            int type = lua_getfield(l, -1, "targetsFeasible"); // { CardFace.targetsFeasible, CardFace }
            do {
                if (type == LUA_TFUNCTION) {
                    // we should do the function call and return
                    // error should be catched here
                    // TODO: store real parameters after SWIG is ready
                    bool call = CardFaceLuaCall::targetsFeasible(l, targets, Self, card); // { returnValue / error, CardFace }
                    if (call)
                        r = lua_toboolean(l, -1);
                    else {
                        // error
                        // since the stack top is the error object, we temporarily ignore it
                    }
                } else {
                    // not function, ignored
                }
            } while (false);
            lua_pop(l, 1); // { CardFace }
        } while (false);
        lua_pop(l, 1); // { }
    }

    if (r.has_value())
        return r.value();

    // default
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

const Card *CardFace::validateInResponse(Player * /*unused*/, const Card *original_card) const
{
    return original_card;
}

void CardFace::doPreAction(RoomObject * /*unused*/, const CardUseStruct & /*unused*/) const
{
}

void CardFace::onUse(RoomObject *room, const CardUseStruct &use) const
{
    CardUseStruct card_use = use;
    Player *player = card_use.from;

    room->sortPlayersByActionOrder(card_use.to);

    QList<Player *> targets = card_use.to;
    // TODO
    // if (room->getMode() == QStringLiteral("06_3v3") && (isKindOf("AOE") || isKindOf("GlobalEffect")))
    //     room->reverseFor3v3(card_use.card, player, targets);
    card_use.to = targets;

    LogMessage log;
    log.from = player;
    if (!targetFixed(card_use.from, card_use.card) || card_use.to.length() > 1 || !card_use.to.contains(card_use.from))
        log.to = card_use.to;
    log.type = QStringLiteral("#UseCard");
    log.card_str = card_use.card->toString(false);
    RefactorProposal::fixme_cast<Room *>(room)->sendLog(log);

    IDSet used_cards;
    QList<CardsMoveStruct> moves;
    if (card_use.card->isVirtualCard())
        used_cards.unite(card_use.card->subcards());
    else
        used_cards.insert(card_use.card->effectiveID());

    QVariant data = QVariant::fromValue(card_use);
    RoomThread *thread = RefactorProposal::fixme_cast<Room *>(room)->getThread();
    Q_ASSERT(thread != nullptr);
    thread->trigger(PreCardUsed, data);
    card_use = data.value<CardUseStruct>();

    CardMoveReason reason(CardMoveReason::S_REASON_USE, player->objectName(), QString(), card_use.card->skillName(), QString());
    if (card_use.to.size() == 1)
        reason.m_targetId = card_use.to.first()->objectName();

    reason.m_extraData = QVariant::fromValue(card_use.card);

    foreach (int id, used_cards) {
        CardsMoveStruct move(id, nullptr, PlaceTable, reason);
        moves.append(move);
    }
    RefactorProposal::fixme_cast<Room *>(room)->moveCardsAtomic(moves, true);

    RefactorProposal::fixme_cast<ServerPlayer *>(player)->showHiddenSkill(card_use.card->showSkillName());

    thread->trigger(CardUsed, data);
    thread->trigger(CardFinished, data);
}

void CardFace::use(RoomObject *room, const CardUseStruct &use) const
{
    Player *source = use.from;

    QStringList nullified_list = use.nullified_list; // room->getTag(QStringLiteral("CardUseNullifiedList")).toStringList();
    bool all_nullified = nullified_list.contains(QStringLiteral("_ALL_TARGETS"));
    int magic_drank = 0;
    if (isNDTrick() && (source != nullptr) && source->mark(QStringLiteral("magic_drank")) > 0)
        magic_drank = source->mark(QStringLiteral("magic_drank"));

    foreach (Player *target, use.to) {
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
        if (source != nullptr && source->mark(QStringLiteral("kuangji_value")) > 0) {
            effect.effectValue.first() = effect.effectValue.first() + source->mark(QStringLiteral("kuangji_value"));
            effect.effectValue.last() = effect.effectValue.last() + source->mark(QStringLiteral("kuangji_value"));
            source->setMark(QStringLiteral("kuangji_value"), 0);
        }

        effect.effectValue.first() = effect.effectValue.first() + magic_drank;
        QVariantList players;
        for (int i = use.to.indexOf(target); i < use.to.length(); i++) {
            if (!nullified_list.contains(use.to.at(i)->objectName()) && !all_nullified)
                players.append(QVariant::fromValue(use.to.at(i)));
        }

        //room->setTag(QStringLiteral("targets") + use.card->toString(), QVariant::fromValue(players));

        // TODO: full card effect procedure //room->cardEffect(effect);
        effect.card->face()->onEffect(effect);
    }
    //room->removeTag(QStringLiteral("targets") + use.card->toString()); //for ai?
    if (source != nullptr && magic_drank > 0)
        source->setMark(QStringLiteral("magic_drank"), 0);

    if (RefactorProposal::fixme_cast<Room *>(room)->getCardPlace(use.card->effectiveID()) == PlaceTable) {
        CardMoveReason reason(CardMoveReason::S_REASON_USE, source != nullptr ? source->objectName() : QString(), QString(), use.card->skillName(), QString());
        if (use.to.size() == 1)
            reason.m_targetId = use.to.first()->objectName();
        reason.m_extraData = QVariant::fromValue(use.card);
        Player *provider = nullptr;
        foreach (const QString &flag, use.card->flags()) {
            if (flag.startsWith(QStringLiteral("CardProvider_"))) {
                QStringList patterns = flag.split(QStringLiteral("_"));
                provider = room->findPlayer(patterns.at(1));
                break;
            }
        }

        Player *from = source;
        if (provider != nullptr)
            from = provider;
        RefactorProposal::fixme_cast<Room *>(room)->moveCardTo(use.card, RefactorProposal::fixme_cast<ServerPlayer *>(from), nullptr, PlaceDiscardPile, reason, true);
    }
}

void CardFace::onEffect(const CardEffectStruct & /*unused*/) const
{
}

bool CardFace::isCancelable(const CardEffectStruct & /*unused*/) const
{
    return false;
}

void CardFace::onNullified(Player * /*unused*/, const Card * /*unused*/) const
{
}

BasicCard::BasicCard(const QString &name)
    : CardFace(name)
{
}

CardType BasicCard::type() const
{
    return TypeBasic;
}

QString BasicCard::typeName() const
{
    return QStringLiteral("basic");
}

EquipCard::EquipCard(const QString &name)
    : CardFace(name)
{
}

CardType EquipCard::type() const
{
    return TypeEquip;
}

QString EquipCard::typeName() const
{
    return QStringLiteral("equip");
}

void EquipCard::onInstall(Player *) const
{
#if 0
    // Shouldn't these logic be in GameLogic?

    Room *room = player->getRoom();

    const Skill *skill = Sanguosha->getSkill(this);
    if (skill) {
        if (skill->inherits("ViewAsSkill")) {
            room->attachSkillToPlayer(player, objectName());
        } else if (skill->inherits("TriggerSkill")) {
            const TriggerSkill *trigger_skill = qobject_cast<const TriggerSkill *>(skill);
            room->getThread()->addTriggerSkill(trigger_skill);

            if (trigger_skill->getViewAsSkill())
                room->attachSkillToPlayer(player, skill->objectName());
        }
    }
#endif
}

void EquipCard::onUninstall(Player *) const
{
#if 0
    Room *room = player->getRoom();
    const Skill *skill = Sanguosha->getSkill(this);

    if (skill && (skill->inherits("ViewAsSkill") || (skill->inherits("TriggerSkill") && qobject_cast<const TriggerSkill *>(skill)->getViewAsSkill())))
        room->detachSkillFromPlayer(player, objectName(), true);
#endif
}

class WeaponPrivate
{
public:
    int range;
    WeaponPrivate()
        : range(1)
    {
    }
};

Weapon::Weapon(const QString &name)
    : EquipCard(name)
    , d(new WeaponPrivate)
{
}

Weapon::~Weapon()
{
    delete d;
}

EquipLocation Weapon::location() const
{
    return WeaponLocation;
}

int Weapon::range() const
{
    return d->range;
}

void Weapon::setRange(int r)
{
    d->range = r;
}

Armor::Armor(const QString &name)
    : EquipCard(name)
{
}

EquipLocation Armor::location() const
{
    return ArmorLocation;
}

DefensiveHorse::DefensiveHorse(const QString &name)
    : EquipCard(name)
{
}

EquipLocation DefensiveHorse::location() const
{
    return DefensiveHorseLocation;
}

OffensiveHorse::OffensiveHorse(const QString &name)
    : EquipCard(name)
{
}

EquipLocation OffensiveHorse::location() const
{
    return OffensiveHorseLocation;
}

Treasure::Treasure(const QString &name)
    : EquipCard(name)
{
}

EquipLocation Treasure::location() const
{
    return TreasureLocation;
}

TrickCard::TrickCard(const QString &name)
    : CardFace(name)
{
}

CardType TrickCard::type() const
{
    return TypeTrick;
}

QString TrickCard::typeName() const
{
    return QStringLiteral("trick");
}

NonDelayedTrick::NonDelayedTrick(const QString &name)
    : TrickCard(name)
{
}

DelayedTrick::DelayedTrick(const QString &name)
    : TrickCard(name)
    , j(nullptr)
{
}

void DelayedTrick::takeEffect(Player *target) const
{
}

JudgeStruct DelayedTrick::judge() const
{
    if (j == nullptr)
        return JudgeStruct();

    return *j;
}

class SkillCardPrivate
{
public:
    bool throw_when_using;
    SkillCardPrivate()
        : throw_when_using(true)
    {
    }
};

SkillCard::SkillCard(const QString &name)
    : CardFace(name)
    , d(new SkillCardPrivate)

{
}

SkillCard::~SkillCard()
{
    delete d;
}

CardType SkillCard::type() const
{
    return TypeSkill;
}

QString SkillCard::typeName() const
{
    return QStringLiteral("skill");
}

void SkillCard::onUse(RoomObject *room, const CardUseStruct &_use) const
{
    CardUseStruct card_use = _use;
    Player *player = card_use.from;

    room->sortPlayersByActionOrder(card_use.to);

    QList<Player *> targets = card_use.to;
    card_use.to = targets;

    LogMessage log;
    log.from = player;
    if (!targetFixed(card_use.from, card_use.card) || card_use.to.length() > 1 || !card_use.to.contains(card_use.from))
        log.to = card_use.to;
    log.type = QStringLiteral("#UseCard");
    log.card_str = card_use.card->toString(!throwWhenUsing());
    RefactorProposal::fixme_cast<Room *>(room)->sendLog(log);

    if (throwWhenUsing()) {
        CardMoveReason reason(CardMoveReason::S_REASON_THROW, player->objectName(), QString(), card_use.card->skillName(), QString());
        RefactorProposal::fixme_cast<Room *>(room)->moveCardTo(card_use.card, RefactorProposal::fixme_cast<ServerPlayer *>(player), nullptr, PlaceDiscardPile, reason, true);
    }

    RefactorProposal::fixme_cast<ServerPlayer *>(player)->showHiddenSkill(card_use.card->showSkillName());

    use(room, card_use);
}

void SkillCard::use(RoomObject * /*room*/, const CardUseStruct &use) const
{
    Player *source = use.from;
    foreach (Player *target, use.to) {
        CardEffectStruct effect;
        effect.card = use.card;
        effect.from = source;
        effect.to = target;
        onEffect(effect);
    }
}

bool SkillCard::throwWhenUsing() const
{
    return d->throw_when_using;
}

void SkillCard::setThrowWhenUsing(bool can)
{
    d->throw_when_using = can;
}

// TODO: find a suitable place for them
class SurrenderCard : public SkillCard
{
public:
    SurrenderCard();

    void onUse(RoomObject *room, const CardUseStruct &use) const override;
};

class CheatCard : public SkillCard
{
public:
    CheatCard();

    void onUse(RoomObject *room, const CardUseStruct &use) const override;
};

SurrenderCard::SurrenderCard()
    : SkillCard(QStringLiteral("surrender"))
{
    setTargetFixed(true);
    setDefaultHandlingMethod(MethodNone);
}

void SurrenderCard::onUse(RoomObject *room, const CardUseStruct &use) const
{
    RefactorProposal::fixme_cast<Room *>(room)->makeSurrender(RefactorProposal::fixme_cast<ServerPlayer *>(use.from));
}

CheatCard::CheatCard()
    : SkillCard(QStringLiteral("cheat"))
{
    setTargetFixed(true);
    setDefaultHandlingMethod(MethodNone);
}

void CheatCard::onUse(RoomObject *room, const CardUseStruct &use) const
{
    QString cheatString = use.card->userString();
    JsonDocument doc = JsonDocument::fromJson(cheatString.toUtf8().constData());
    if (doc.isValid())
        RefactorProposal::fixme_cast<Room *>(room)->cheat(RefactorProposal::fixme_cast<ServerPlayer *>(use.from), doc.toVariant());
}
