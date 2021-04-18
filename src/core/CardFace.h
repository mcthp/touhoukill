#ifndef TOUHOUKILL_CARDFACE_H_
#define TOUHOUKILL_CARDFACE_H_

// Fs: do not use "#pragma once" because every header file does not use it

#include <QMetaObject>
#include <QString>

class Player;
class ServerPlayer;
class Room;
class Card;
class CardFacePrivate;

struct CardUseStruct;
struct CardEffectStruct;

/**
 * @interface The functional model of a given card. 
 */
class CardFace
{
    Q_GADGET

public:
    CardFace();
    virtual ~CardFace();

    // text property
    // FIXME: replace `name` with objectName ?
    virtual QString name() const;
    virtual QString description() const;
    virtual QString commmonEffectName() const;
    virtual QString effectName() const;

    // type property
    enum CardType
    {
        TypeUnknown,
        TypeSkill,
        TypeBasic,
        TypeTrick,
        TypeEquip
    };

    virtual CardType type() const = 0;
    virtual QString typeName() const = 0;
    virtual QString subTypeName() const = 0;
    virtual bool isKindOf(const char *cardType) const;
    virtual bool matchType(const QString &pattern) const;

    // Can we have a better way to replace this function? Maybe using `match`
    // Fs: This is just a convenience function....
    virtual bool isNDTrick() const;

    // property identifier.
    // CardFace provides the default value of these property
    // But they could be dynamic and explained by Card itself.
    // For example, some skill may let Slash not be regarded as damage card?
    // Fs: There is a skill which has a skill named "Xianshi" for God Patchouli in TouhouKill. It needs an extremely hacked Card/CardFace which changes all the effect of a certain Card.
    // Return value of "canDamage" and "canRecover" is affected by "Xianshi" in this case.
    virtual bool canDamage() const;
    virtual bool canRecover() const;
    // Fs: canRecast should be property of Card.
    // Seems like it should be dealt in UI and GameRule instead of the logic in Card/CardFace itself.
    // Currently CardFace::onUse and CardFace::targetFixed/targetFeasible are hacked to support recast
    // Note: In HulaoPass mode, all weapon can be recast according to the game rule.
    virtual bool canRecast() const;
    virtual bool hasEffectValue() const;
    virtual bool willThrow() const;
    virtual bool hasPreAction() const;

    // This is method is removed from the face. It's clear that this is totally dynamic.
    // virtual Card::HandlingMethod defaultHandlingMethod() const;

    // Functions
    virtual bool targetFixed(const Player *Self, const Card *card) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self, const Card *card) const;
    // FIXME: the following two functions should be merged into one.
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self, const Card *card) const;
    // FIXME: return tuple/pair rather than a pure bool and combine two functions together?
    // Fs: This depends on implementation of all cards, although I believe that this function is initially only for skill 'yeyan'
    // In fact return value of the function with maxVotes has no use, only 'maxVotes' is used in current UI.
    // Fs (Edit 2021/4/18): Maybe a single targetFilter with a return value of Integer is enough?
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self, const Card *card, int &maxVotes) const;

    virtual bool isAvailable(const Player *player, const Card *card) const;

    virtual bool ignoreCardValidity(const Player *player) const;
    virtual const Card *validate(const CardUseStruct &cardUse) const;
    virtual const Card *validateInResponse(ServerPlayer *user, const Card *original_card) const;

    virtual void doPreAction(Room *room, const CardUseStruct &card_use) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, const CardUseStruct &use) const;

    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;
    virtual void onNullified(ServerPlayer *target) const;

private:
    CardFacePrivate *d;
};

#endif
