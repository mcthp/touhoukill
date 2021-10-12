#ifndef _STRUCTS_H
#define _STRUCTS_H

#include "global.h"

class Skill;
class RoomObject;
class Card;
class Player;

#include <QVariant>

struct DamageStruct
{
    enum Nature
    {
        Normal, // normal slash, duel and most damage caused by skill
        Fire, // fire slash, fire attack and few damage skill (Yeyan, etc)
        Thunder, // lightning, thunder slash, and few damage skill (Leiji, etc)
        Ice
    };

    DamageStruct();
    DamageStruct(const Card *card, Player *from, Player *to, int damage = 1, Nature nature = Normal);
    DamageStruct(const QString &reason, Player *from, Player *to, int damage = 1, Nature nature = Normal);

    Player *from;
    Player *to;
    const Card *card;
    int damage;
    Nature nature;
    bool chain;
    bool transfer;
    bool by_user;
    QString reason;
    bool trigger_chain;
    QString trigger_info; //keep addtion info while record. since this damage event may be triggered lately by insertion of new damage event.

    QString getReason() const;
};

struct CardUseStruct
{
    enum CardUseReason
    {
        CARD_USE_REASON_UNKNOWN = 0x00,
        CARD_USE_REASON_PLAY = 0x01,
        CARD_USE_REASON_RESPONSE = 0x02,
        CARD_USE_REASON_RESPONSE_USE = 0x12
    } m_reason;

    CardUseStruct();
    CardUseStruct(const Card *card, Player *from, const QList<Player *> &to = QList<Player *>(), bool isOwnerUse = true);
    CardUseStruct(const Card *card, Player *from, Player *target, bool isOwnerUse = true);
    bool isValid(const QString &pattern) const;
    void parse(const QString &str, RoomObject *room);
    bool tryParse(const QVariant &usage, RoomObject *room);

    QString toString() const;

    const Card *card;
    Player *from;
    QList<Player *> to;
    bool m_isOwnerUse;
    bool m_addHistory;
    bool m_isHandcard;
    bool m_isLastHandcard;
    QList<int> m_showncards;
    QStringList nullified_list;
};

struct CardEffectStruct
{
    CardEffectStruct();

    const Card *card;

    Player *from;
    Player *to;

    bool multiple; // helper to judge whether the card has multiple targets
    // does not make sense if the card inherits SkillCard
    bool nullified;
    bool canceled; //for cancel process, like "yuyi"
    QList<int> effectValue;
};

struct SlashEffectStruct
{
    SlashEffectStruct();

    int jink_num;

    const Card *slash;
    const Card *jink;

    Player *from;
    Player *to;

    int drank;

    DamageStruct::Nature nature;
    bool multiple;
    bool nullified;
    bool canceled;
    QList<int> effectValue;
};

class CardMoveReason
{
public:
    enum MoveReasonCategory
    {
        S_REASON_UNKNOWN = 0x00,
        S_REASON_USE = 0x01,
        S_REASON_RESPONSE = 0x02,
        S_REASON_DISCARD = 0x03,
        S_REASON_RECAST = 0x04, // ironchain etc.
        S_REASON_PINDIAN = 0x05,
        S_REASON_DRAW = 0x06,
        S_REASON_GOTCARD = 0x07,
        S_REASON_SHOW = 0x08,
        S_REASON_TRANSFER = 0x09,
        S_REASON_PUT = 0x0A,

        //subcategory of use
        S_REASON_LETUSE = 0x11, // use a card when self is not current

        //subcategory of response
        S_REASON_RETRIAL = 0x12,

        //subcategory of discard
        S_REASON_RULEDISCARD = 0x13, //  discard at one's Player::Discard for gamerule
        S_REASON_THROW = 0x23, //  gamerule(dying or punish) as the cost of some skills
        S_REASON_DISMANTLE = 0x33, //  one throw card of another

        //subcategory of gotcard
        S_REASON_GIVE = 0x17, // from one hand to another hand
        S_REASON_EXTRACTION = 0x27, // from another's place to one's hand
        S_REASON_GOTBACK = 0x37, // from placetable to hand
        S_REASON_RECYCLE = 0x47, // from discardpile to hand
        S_REASON_ROB = 0x57, // got a definite card from other's hand
        S_REASON_PREVIEWGIVE = 0x67, // give cards after previewing, i.e. Yiji & Miji

        //subcategory of show
        S_REASON_TURNOVER = 0x18, // show n cards from drawpile
        S_REASON_JUDGE = 0x28, // show a card from drawpile for judge
        S_REASON_PREVIEW = 0x38, // Not done yet, plan for view some cards for self only(guanxing yiji miji)
        S_REASON_DEMONSTRATE = 0x48, // show a card which copy one to move to table

        //subcategory of transfer
        S_REASON_SWAP = 0x19, // exchange card for two players
        S_REASON_OVERRIDE = 0x29, // exchange cards from cards in game
        S_REASON_EXCHANGE_FROM_PILE = 0x39, // exchange cards from cards moved out of game (for qixing only)

        //subcategory of put
        S_REASON_NATURAL_ENTER = 0x1A, //  a card with no-owner move into discardpile e.g. delayed trick enters discardpile
        S_REASON_REMOVE_FROM_PILE = 0x2A, //  cards moved out of game go back into discardpile
        S_REASON_JUDGEDONE = 0x3A, //  judge card move into discardpile
        S_REASON_CHANGE_EQUIP = 0x4A, //  replace existed equip

        S_MASK_BASIC_REASON = 0x0F,
    };

    MoveReasonCategory m_reason;
    QString m_playerId; // the cause (not the source) of the movement, such as "lusu" when "dimeng", or "zhanghe" when "qiaobian"
    QString m_targetId; // To keep this structure lightweight, currently this is only used for UI purpose.
    // It will be set to empty if multiple targets are involved. NEVER use it for trigger condition
    // judgement!!! It will not accurately reflect the real reason.
    QString m_skillName; // skill that triggers movement of the cards, such as "longdang", "dimeng"
    QString m_eventName; // additional arg such as "lebusishu" on top of "S_REASON_JUDGE"
    QVariant m_extraData; // additional data and will not be parsed to clients
    QVariant m_provider; // additional data recording who provide this card for otherone to use or response, e.g. guanyu provide a slash for "jijiang"

    inline CardMoveReason()
    {
        m_reason = S_REASON_UNKNOWN;
    }
    inline CardMoveReason(MoveReasonCategory moveReason, const QString &playerId)
    {
        m_reason = moveReason;
        m_playerId = playerId;
    }

    inline CardMoveReason(MoveReasonCategory moveReason, const QString &playerId, const QString &skillName, const QString &eventName)
    {
        m_reason = moveReason;
        m_playerId = playerId;
        m_skillName = skillName;
        m_eventName = eventName;
    }

    inline CardMoveReason(MoveReasonCategory moveReason, const QString &playerId, const QString &targetId, const QString &skillName, const QString &eventName)
    {
        m_reason = moveReason;
        m_playerId = playerId;
        m_targetId = targetId;
        m_skillName = skillName;
        m_eventName = eventName;
    }

    bool tryParse(const QVariant &);
    QVariant toVariant() const;

    inline bool operator==(const CardMoveReason &other) const
    {
        return m_reason == other.m_reason && m_playerId == other.m_playerId && m_targetId == other.m_targetId && m_skillName == other.m_skillName
            && m_eventName == other.m_eventName;
    }
};

struct CardsMoveOneTimeStruct
{
    QList<int> card_ids;
    QList<QSanguosha::Place> from_places;
    QSanguosha::Place to_place;
    CardMoveReason reason;
    Player *from, *to;
    QStringList from_pile_names;
    QString to_pile_name;

    QList<QSanguosha::Place> origin_from_places;
    QSanguosha::Place origin_to_place;
    Player *origin_from, *origin_to;
    QStringList origin_from_pile_names;
    QString origin_to_pile_name; //for case of the movement transitted

    QList<bool> open; // helper to prevent sending card_id to unrelevant clients
    bool is_last_handcard;
    QList<int> broken_ids; //record broken equip IDs from EquipPlace
    QList<int> shown_ids; //record broken shown IDs from HandPlace

    inline void removeCardIds(const QList<int> &to_remove)
    {
        foreach (int id, to_remove) {
            int index = card_ids.indexOf(id);
            if (index != -1) {
                card_ids.removeAt(index);
                from_places.removeAt(index);
                from_pile_names.removeAt(index);
                open.removeAt(index);
            }
        }
    }
};

struct CardsMoveStruct
{
    CardsMoveStruct();
    CardsMoveStruct(const QList<int> &ids, Player *from, Player *to, QSanguosha::Place from_place, QSanguosha::Place to_place, const CardMoveReason &reason);
    CardsMoveStruct(const QList<int> &ids, Player *to, QSanguosha::Place to_place, const CardMoveReason &reason);
    CardsMoveStruct(int id, Player *from, Player *to, QSanguosha::Place from_place, QSanguosha::Place to_place, const CardMoveReason &reason);
    CardsMoveStruct(int id, Player *to, QSanguosha::Place to_place, const CardMoveReason &reason);
    bool operator==(const CardsMoveStruct &other) const;
    bool operator<(const CardsMoveStruct &other) const;

    QList<int> card_ids; // TODO: Replace with IDSet
    QSanguosha::Place from_place, to_place;
    QString from_player_name, to_player_name;
    QString from_pile_name, to_pile_name;
    Player *from, *to;
    CardMoveReason reason;
    bool open; // helper to prevent sending card_id to unrelevant clients
    bool is_last_handcard;

    QSanguosha::Place origin_from_place, origin_to_place;
    Player *origin_from, *origin_to;
    QString origin_from_pile_name, origin_to_pile_name; //for case of the movement transitted
    QList<int> broken_ids; //record broken equip IDs from EquipPlace
    QList<int> shown_ids; //record broken shown IDs from HandPlace

    bool tryParse(const QVariant &arg);
    QVariant toVariant() const;
    bool isRelevant(const Player *player) const;
};

struct DyingStruct
{
    DyingStruct();

    Player *who; // who is ask for help
    DamageStruct *damage; // if it is NULL that means the dying is caused by losing hp
    Player *nowAskingForPeaches; // who is asking for peaches
};

struct DeathStruct
{
    DeathStruct();

    Player *who; // who is dead
    DamageStruct *damage; // if it is NULL that means the dying is caused by losing hp

    Player *viewAsKiller;
    bool useViewAsKiller;
};

struct RecoverStruct
{
    RecoverStruct();

    int recover;
    Player *who;
    Player *to;
    const Card *card;
    QString reason;
};

struct PindianStruct
{
    PindianStruct();
    bool isSuccess() const;

    Player *from;
    Player *to;
    Player *askedPlayer;
    const Card *from_card;
    const Card *to_card;
    int from_number;
    int to_number;
    QString reason;
    bool success;
};

struct JudgeStruct
{
    JudgeStruct();
    bool isGood() const;
    inline bool isBad() const
    {
        return !isGood();
    }
    inline bool isEffected() const
    {
        return !negative == isGood();
    }

    void setCard(const Card *card);
    inline const Card *card() const
    {
        return m_card;
    }

    Player *who;
    QString pattern;
    bool good;
    QString reason;
    bool time_consuming;
    bool negative;
    bool play_animation;
    Player *retrial_by_response; // record whether the current judge card is provided by a response retrial
    Player *relative_player; // record relative player like skill owner of "huazhong", for processing the case like "huazhong -> dizhen -> huazhong"
    bool ignore_judge; //for tiandao

private:
    enum TrialResult
    {
        TRIAL_RESULT_UNKNOWN,
        TRIAL_RESULT_GOOD,
        TRIAL_RESULT_BAD
    } _m_result;
    const Card *m_card;
};

struct PhaseChangeStruct
{
    PhaseChangeStruct();

    QSanguosha::Phase from;
    QSanguosha::Phase to;
    Player *player;
};

struct PhaseSkippingStruct
{
    PhaseSkippingStruct();

    QSanguosha::Phase phase;
    Player *player;
    bool isCost;
};

struct PhaseStruct
{
    inline PhaseStruct()
        : phase(QSanguosha::PhaseNone)
        , skipped(0)
    {
    }

    QSanguosha::Phase phase;
    int skipped; // 0 - not skipped; 1 - skipped by effect; -1 - skipped by cost
};

struct CardResponseStruct
{
    inline explicit CardResponseStruct(const Card *card = nullptr, Player *who = nullptr, bool isuse = false, bool isRetrial = false, bool isProvision = false,
                                       Player *from = nullptr)
        : m_card(card)
        , m_who(who)
        , m_isUse(isuse)
        , m_isRetrial(isRetrial)
        , m_isProvision(isProvision)
        , m_isHandcard(false)
        , m_from(from)
        , m_isNullified(false)
        , m_isShowncard(false)
    {
    }

    const Card *m_card;
    Player *m_who;
    bool m_isUse;
    bool m_isRetrial;
    bool m_isProvision;
    bool m_isHandcard;
    Player *m_from;
    bool m_isNullified;
    bool m_isShowncard;
};

struct MarkChangeStruct
{
    MarkChangeStruct();

    int num;
    QString name;
    Player *player;
};

struct SkillAcquireDetachStruct
{
    SkillAcquireDetachStruct();

    const Skill *skill;
    Player *player;
    bool isAcquire;
};

struct CardAskedStruct
{
    CardAskedStruct();

    QString pattern;
    QString prompt;
    Player *player;
    QSanguosha::HandlingMethod method;
};

class Trigger;

class TriggerDetailPrivate;

class TriggerDetail
{
public:
    explicit TriggerDetail(RoomObject *room, const Trigger *trigger = nullptr, const QString &name = QString(), Player *owner = nullptr, Player *invoker = nullptr,
                           const QList<Player *> &targets = QList<Player *>(), bool isCompulsory = false, bool effectOnly = false);
    TriggerDetail(RoomObject *room, const Trigger *trigger, const QString &name, Player *owner, Player *invoker, Player *target, bool isCompulsory = false,
                  bool effectOnly = false);

    TriggerDetail(const TriggerDetail &other);
    TriggerDetail &operator=(const TriggerDetail &other);
    ~TriggerDetail();

    RoomObject *room() const;
    const Trigger *trigger() const;
    const QString &name() const;
    Player *owner() const;
    Player *invoker() const;
    QList<Player *> targets() const;
    bool isCompulsory() const;
    bool triggered() const;
    bool effectOnly() const;
    const QVariantMap &tag() const;

    void addTarget(Player *target);
    void setTriggered(bool t);
    QVariantMap &tag();

    bool operator<(const TriggerDetail &arg2) const; // the operator < for sorting the invoke order.
    // the operator ==. it only judge the skill name, the skill invoker, and the skill owner. it don't judge the skill target because it is chosen by the skill invoker
    bool sameTrigger(const TriggerDetail &arg2) const;
    // used to judge 2 skills has the same timing. only 2 structs with the same priority and the same invoker and the same "whether or not it is a skill of equip"
    bool sameTimingWith(const TriggerDetail &arg2) const;
    bool isValid() const; // validity check

    QVariant toVariant() const;
    QStringList toList() const;

private:
    TriggerDetailPrivate *d;
};

struct HpLostStruct
{
    HpLostStruct();

    Player *player;
    int num;
};

struct JinkEffectStruct
{
    JinkEffectStruct();

    SlashEffectStruct slashEffect;
    const Card *jink;
};

struct DrawNCardsStruct
{
    DrawNCardsStruct();

    Player *player;
    int n;
    bool isInitial;
};

struct SkillInvalidStruct
{
    SkillInvalidStruct();

    Player *player;
    const Skill *skill;
    bool invalid;
};

struct BrokenEquipChangedStruct
{
    BrokenEquipChangedStruct();

    Player *player;
    QList<int> ids;
    bool broken;
    bool moveFromEquip;
};

struct ShownCardChangedStruct
{
    ShownCardChangedStruct();

    Player *player;
    QList<int> ids;
    bool shown;
    bool moveFromHand;
};

struct ShowGeneralStruct
{
    ShowGeneralStruct();

    Player *player;
    bool isHead;
    bool isShow;
};

struct ChoiceMadeStruct
{
    inline ChoiceMadeStruct()
        : player(nullptr)
        , type(NoChoice)
    {
    }

    enum ChoiceType
    {
        NoChoice,

        SkillInvoke,
        SkillChoice,
        Nullification,
        CardChosen,
        CardResponded,
        CardUsed,
        AGChosen,
        CardShow,
        Peach,
        TriggerOrder,
        ReverseFor3v3,
        Activate,
        Suit,
        Kingdom,
        CardDiscard,
        CardExchange,
        ViewCards,
        PlayerChosen,
        Rende,
        Yiji,
        Pindian,

        NumOfChoices
    };

    Player *player;
    ChoiceType type;
    QStringList args;
    QVariant m_extraData;
};

struct ExtraTurnStruct
{
    ExtraTurnStruct();

    Player *player;
    QList<QSanguosha::Phase> set_phases;
    QString reason;
    Player *extraTarget; //record related target  --qinlue
};

Q_DECLARE_METATYPE(DamageStruct)
Q_DECLARE_METATYPE(CardEffectStruct)
Q_DECLARE_METATYPE(SlashEffectStruct)
Q_DECLARE_METATYPE(CardUseStruct)
Q_DECLARE_METATYPE(CardsMoveStruct)
Q_DECLARE_METATYPE(CardsMoveOneTimeStruct)
Q_DECLARE_METATYPE(DyingStruct)
Q_DECLARE_METATYPE(DeathStruct)
Q_DECLARE_METATYPE(RecoverStruct)
Q_DECLARE_METATYPE(PhaseChangeStruct)
Q_DECLARE_METATYPE(CardResponseStruct)
Q_DECLARE_METATYPE(MarkChangeStruct)
Q_DECLARE_METATYPE(ChoiceMadeStruct)
Q_DECLARE_METATYPE(SkillAcquireDetachStruct)
Q_DECLARE_METATYPE(CardAskedStruct)
Q_DECLARE_METATYPE(HpLostStruct)
Q_DECLARE_METATYPE(JinkEffectStruct)
Q_DECLARE_METATYPE(PhaseSkippingStruct)
Q_DECLARE_METATYPE(DrawNCardsStruct)
Q_DECLARE_METATYPE(QList<SkillInvalidStruct>)
Q_DECLARE_METATYPE(JudgeStruct *)
Q_DECLARE_METATYPE(PindianStruct *)
Q_DECLARE_METATYPE(ExtraTurnStruct)
Q_DECLARE_METATYPE(BrokenEquipChangedStruct)
Q_DECLARE_METATYPE(ShownCardChangedStruct)
Q_DECLARE_METATYPE(ShowGeneralStruct)

#endif
