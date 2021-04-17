#ifndef _CARD_H
#define _CARD_H

#include <QIcon>
#include <QMap>
#include <QObject>

class Room;
class RoomObject;
class Player;
class ServerPlayer;
class Client;
class ClientPlayer;
class CardItem;

struct CardEffectStruct;
struct CardUseStruct;

class Card : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString suit READ getSuitString CONSTANT)
    Q_PROPERTY(bool red READ isRed STORED false CONSTANT)
    Q_PROPERTY(bool black READ isBlack STORED false CONSTANT)
    Q_PROPERTY(int id READ getId CONSTANT)
    Q_PROPERTY(int number READ getNumber WRITE setNumber)
    Q_PROPERTY(QString number_string READ getNumberString CONSTANT)
    Q_PROPERTY(QString type READ getType CONSTANT)
    Q_PROPERTY(bool mute READ isMute CONSTANT)
    Q_PROPERTY(Color color READ getColor)
    Q_PROPERTY(bool can_recast READ canRecast WRITE setCanRecast)

    Q_ENUMS(Suit)
    Q_ENUMS(CardType)
    Q_ENUMS(HandlingMethod)

public:
    // enumeration type
    enum Suit
    {
        Spade,
        Club,
        Heart,
        Diamond,
        NoSuitBlack,
        NoSuitRed,
        NoSuit,
        SuitToBeDecided = -1
    };
    enum Color
    {
        Red,
        Black,
        Colorless
    };
    enum HandlingMethod
    {
        MethodNone,
        MethodUse,
        MethodResponse,
        MethodDiscard,
        MethodRecast,
        MethodPindian
    };

    static const Suit AllSuits[4];

    // card types
    enum CardType
    {
        TypeSkill,
        TypeBasic,
        TypeTrick,
        TypeEquip
    };

    // constructor
    Card(Suit suit, int number, bool target_fixed = false);

    // property getters/setters
    QString getSuitString() const;
    bool isRed() const;
    bool isBlack() const;
    int getId() const;
    virtual void setId(int id);
    int getEffectiveId() const;

    int getNumber() const;
    virtual void setNumber(int number);
    QString getNumberString() const;

    Suit getSuit() const;
    virtual void setSuit(Suit suit);

    bool sameColorWith(const Card *other) const;
    Color getColor() const;
    QString getFullName(bool include_suit = false) const;
    QString getLogName() const;
    QString getName() const;
    QString getSkillName(bool removePrefix = true) const;
    virtual void setSkillName(const QString &skill_name);
    QString getDescription(bool yellow = true) const;

    virtual bool isMute() const;
    virtual bool canDamage() const;
    virtual bool canRecover() const;
    virtual bool hasEffectValue() const;
    virtual bool willThrow() const;
    virtual bool canRecast() const;
    void setCanRecast(bool can);
    virtual bool hasPreAction() const;
    virtual Card::HandlingMethod getHandlingMethod() const;

    virtual void setFlags(const QString &flag) const;
    inline virtual void setFlags(const QStringList &fs)
    {
        flags = fs;
    }
    bool hasFlag(const QString &flag) const;
    virtual void clearFlags() const;

    virtual QString getPackage() const;
    inline virtual QString getClassName() const
    {
        return metaObject()->className();
    }
    virtual bool isVirtualCard() const;
    virtual bool isEquipped(const Player *Self) const;
    virtual QString getCommonEffectName() const;
    virtual bool matchTypeOrName(const QString &pattern) const;

    virtual void addSubcard(int card_id);
    virtual void addSubcard(const Card *card);
    virtual QList<int> getSubcards() const;
    virtual void clearSubcards();
    virtual QString subcardString() const;
    virtual void addSubcards(const QList<const Card *> &cards);
    virtual void addSubcards(const QList<int> &subcards_list);
    virtual int subcardsLength() const;

    virtual QString getType() const = 0;
    virtual QString getSubtype() const = 0;
    virtual CardType getTypeId() const = 0;
    virtual bool isNDTrick() const;

    // card target selection
    virtual bool targetFixed(const Player *Self) const;
    virtual bool targetsFeasible(const QList<const Player *> &targets, const Player *Self) const;
    // @todo: the following two functions should be merged into one.
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self, int &maxVotes) const;
    virtual bool isAvailable(const Player *player) const;
    virtual bool ignoreCardValidty(const Player *player) const;

    inline virtual const Card *getRealCard() const
    {
        return this;
    }
    virtual const Card *validate(CardUseStruct &cardUse) const;
    virtual const Card *validateInResponse(ServerPlayer *user) const;

    virtual void doPreAction(Room *room, const CardUseStruct &card_use) const;
    virtual void onUse(Room *room, const CardUseStruct &card_use) const;
    virtual void use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
    virtual bool isCancelable(const CardEffectStruct &effect) const;

    virtual QString showSkill() const;
    virtual void setShowSkill(const QString &skill_name);

    inline virtual bool isKindOf(const char *cardType) const
    {
        Q_ASSERT(cardType);
        return inherits(cardType);
    }
    inline virtual QStringList getFlags() const
    {
        return flags;
    }

    inline virtual bool isModified() const
    {
        return false;
    }
    inline virtual void onNullified(ServerPlayer * /* target */) const
    {
        return;
    }

    // static functions
    static bool CompareByNumber(const Card *a, const Card *b);
    static bool CompareBySuit(const Card *a, const Card *b);
    static bool CompareByType(const Card *a, const Card *b);
    static Card *Clone(const Card *card);
    static QString Suit2String(Suit suit);
    static const int S_UNKNOWN_CARD_ID;

    static const Card *Parse(const QString &str, RoomObject *room);
    virtual QString toString(bool hidden = false) const;

    virtual QString getEffectName() const;

protected:
    QList<int> subcards;
    bool target_fixed;
    bool mute;
    bool will_throw;
    bool has_preact;
    bool can_recast;
    Suit m_suit;
    int m_number;
    int m_id;
    bool can_damage;
    bool can_recover;
    bool has_effectvalue;
    QString m_skillName;
    Card::HandlingMethod handling_method;

    mutable QStringList flags;
    QString show_skill;
};

class SkillCard : public Card
{
    Q_OBJECT

public:
    SkillCard();
    void setUserString(const QString &user_string);
    QString getUserString() const;

    QString getSubtype() const override;
    QString getType() const override;
    CardType getTypeId() const override;
    QString toString(bool hidden = false) const override;

protected:
    QString user_string;
};

class ShowDistanceCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE ShowDistanceCard();

    const Card *validate(CardUseStruct &card_use) const override;
};

class ArraySummonCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit ArraySummonCard(const QString &name);

    const Card *validate(CardUseStruct &card_use) const override;
};

class DummyCard : public SkillCard
{
    Q_OBJECT

public:
    DummyCard();
    explicit DummyCard(const QList<int> &subcards);

    QString getSubtype() const override;
    QString getType() const override;
    QString toString(bool hidden = false) const override;
};

class CardFace;

namespace RefactorProposal {

// To keep compatibility, the new card class is defined here.
// After its interface is carefully examined, we will push it to replace existing class.

class Card final {
    Q_GADGET
public:
    // Maybe enum color?
    enum Suit 
    {
        Spade,
        Club,
        Heart,
        Diamond,
        NoSuitBlack,
        NoSuitRed,
        NoSuit,
        SuitToBeDecided = -1
    };

    enum Color {
        Red,
        Black,
        Colorless
    };

    enum HandlingMethod
    {
        MethodNone,
        MethodUse,
        MethodResponse,
        MethodDiscard,
        MethodRecast,
        MethodPindian
    };

    // constructor to create real card
    explicit Card(const CardFace *face, Suit suit = SuitToBeDecided, int number = -1, int id = -1);
    
    // Suit method
    Card::Suit suit() const;
    inline void setSuit(Suit suit) { m_suit = suit; }
    QString suitString() const;
    inline bool isRed() const { return suit() == Heart || suit() == Diamond || suit() == NoSuitRed; }
    inline bool isBlack() const { return suit() == Spade || suit() == Club || suit() == NoSuitBlack; }
    Color color() const;

    // Number method
    int number() const;
    inline void setNumber(int number) { m_number = number; }
    inline QString numberString() const {return QString::number(m_number); }

    // id
    inline int id() const { return m_id; }
    int effectiveID() const;

    // name
    QString name() const;
    QString fullName(bool include_suit = false) const;
    QString logName() const;

    // skill name
    inline QString skillName() const { return m_skill_name; }
    inline void setSkillName(const QString &skill_name) { m_skill_name = skill_name; }
    inline QString showSkillName() const { return m_show_skill_name; }
    inline void setShowSkillName(const QString &show_skill_name) { m_show_skill_name = show_skill_name; }

    // handling method
    inline Card::HandlingMethod handleMethod() const { return m_handling_method; }
    
    // property (override the CardFace)
    inline bool canDamage() const { return m_can_damage; };
    void setCanDamage(bool can) { m_can_damage = can; }
    inline bool canRecover() const {return m_can_recover; }
    void setCanRecover(bool can) { m_can_recover = can; }
    inline bool canRecast() const { return m_can_recast; }
    void setCanRecast(bool can) { m_can_recast = can; }
    inline bool hasEffectValue() const;
    void setHasEffectValue(bool has) { m_has_effect_value = has; }
    inline bool thrownAfterUse() const { return m_thrown_after_use; }
    void thrownAfterUse(bool thrown) { m_thrown_after_use = thrown; }

    // Face (functional model)
    // Face cannot be changed once given
    inline const CardFace *face() const { return m_face; }

    // Flags
    inline const QStringList &flags() const { return m_flags; }
    void addFlag(const QString &flag);
    void addFlags(const QStringList &flags);
    void removeFlag(const QString &flag);
    void removeFlag(const QStringList &flag);
    inline void clearFlags() { m_flags.clear(); }
    inline bool hasFlag(const QString &flag) const {return flags().contains(flag); }

    // Virtual Card
    bool isVirtualCard() const;
    const Card *realCard() const;

    // Subcard
    const QList<int> &subcards() const { return m_sub_cards; }
    void addSubcard(int card_id);
    void addSubcard(const Card *card);
    void addSubcards(const QList<int> &subcards);
    inline void clearSubcards() { m_sub_cards.clear(); }
    QString subcardString() const; // Used for converting card to string

    // Status
    // FIXME: move this method to player?
    bool isEquipped(const Player *self) const;

    // UI property
    inline bool isMute() const { return m_mute; }
    inline void setMute(bool mute) { m_mute = mute; }

    // toString
    QString toString() const;

    // helpers
    static Card *Clone(const Card *other);
    static QString SuitToString(Suit suit);
    static const Card *parse(const QString &str, RoomObject *room);

private:
    // basic information
    const CardFace *m_face; // functional model
    Suit m_suit;
    int m_number;
    int m_id; // real card has id.

    QList<int> m_sub_cards; // for real card this should be empty.
    
    // Skill name related
    QString m_skill_name;
    QString m_show_skill_name;

    // handling method
    Card::HandlingMethod m_handling_method;

    // property
    bool m_can_damage;
    bool m_can_recover;
    bool m_can_recast;
    bool m_has_effect_value;
    bool m_thrown_after_use;

    // flags
    // Flag should be unique right?
    // FIXME: replace QStringList with QStringSet?
    QStringList m_flags; 

    // UI
    bool m_mute;

};

};


#endif
