#include "aux-skills.h"
#include "CardFace.h"
#include "client.h"
#include "clientplayer.h"
#include "engine.h"

DiscardSkill::DiscardSkill()
    : ViewAsSkill("discard")
    , num(0)
    , include_equip(false)
    , is_discard(true)
{
    // card->setParent(this);
}

DiscardSkill::~DiscardSkill()
{
}

void DiscardSkill::setNum(int num)
{
    this->num = num;
}

void DiscardSkill::setMinNum(int minnum)
{
    this->minnum = minnum;
}

void DiscardSkill::setIncludeEquip(bool include_equip)
{
    this->include_equip = include_equip;
}

void DiscardSkill::setIsDiscard(bool is_discard)
{
    this->is_discard = is_discard;
}

bool DiscardSkill::viewFilter(const QList<const Card *> &selected, const Card *card, const Player *Self) const
{
    if (selected.length() >= num)
        return false;

    if (!include_equip && Self->hasEquip(card))
        return false;

    if (is_discard && Self->isCardLimited(card, Card::MethodDiscard))
        return false;

    return true;
}

const Card *DiscardSkill::viewAs(const QList<const Card *> &cards, const Player *Self) const
{
    if (cards.length() >= minnum) {
        auto logic = Self->getRoomObject();
        auto card = logic->cloneDummyCard();
        card->setHandleMethod(Card::MethodNone);
        card->clearSubcards();
        foreach (const Card *c, cards)
            card->addSubcard(c);
        return card;
    } else
        return nullptr;
}

// -------------------------------------------

ResponseSkill::ResponseSkill()
    : OneCardViewAsSkill("response-skill")
{
    request = Card::MethodResponse;
}

void ResponseSkill::setPattern(const QString &pattern)
{
    this->pattern = Sanguosha->getPattern(pattern);
}

void ResponseSkill::setRequest(const Card::HandlingMethod request)
{
    this->request = request;
}

bool ResponseSkill::matchPattern(const Player *player, const Card *card) const
{
    if (request != Card::MethodNone && player->isCardLimited(card, request))
        return false;

    return pattern && pattern->match(player, card);
}

bool ResponseSkill::viewFilter(const Card *card, const Player *Self) const
{
    return matchPattern(Self, card);
}

const Card *ResponseSkill::viewAs(const Card *originalCard, const Player * /*Self*/) const
{
    return originalCard;
}

// -------------------------------------------

ShowOrPindianSkill::ShowOrPindianSkill()
{
    setObjectName("showorpindian-skill");
}

bool ShowOrPindianSkill::matchPattern(const Player *player, const Card *card) const
{
    return pattern && pattern->match(player, card);
}

// -------------------------------------------

class YijiCard : public SkillCard
{
    Q_OBJECT
public:
    YijiCard()
    {
        setTargetFixed(false);
        // mute = true;
        setThrowWhenUsing(false);
        // will_throw = false;
        // FIXME: How to pass the handling method to the card?
        // handling_method = Card::MethodNone;
    }

    void setPlayerNames(const QStringList &names)
    {
        set = QSet<QString>(names.begin(), names.end());
    }

    int targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *, const Card *card) const override
    {
        return targets.isEmpty() && set.contains(to_select->objectName()) ? 1 : 0;
    }

    void use(Room *room, const CardUseStruct &use) const override
    {
        ServerPlayer *source = use.from;
        ServerPlayer *target = use.to.first();

        room->broadcastSkillInvoke("rende");
        CardMoveReason reason(CardMoveReason::S_REASON_GIVE, source->objectName(), target->objectName(), "nosrende", QString());
        room->obtainCard(target, use.card, reason, false);

        int old_value = source->getMark("nosrende");
        int new_value = old_value + use.card->subcards().size();
        room->setPlayerMark(source, "nosrende", new_value);

        if (old_value < 2 && new_value >= 2) {
            RecoverStruct recover;
            recover.card = use.card;
            recover.who = source;
            room->recover(source, recover);
        }
    }

private:
    QSet<QString> set;
};

YijiViewAsSkill::YijiViewAsSkill()
    : ViewAsSkill("yiji")
{
    // card->setParent(this);
}

YijiViewAsSkill::~YijiViewAsSkill()
{
}

void YijiViewAsSkill::setCards(const QString &card_str)
{
    QStringList cards = card_str.split("+");
    ids = StringList2IntList(cards);
}

void YijiViewAsSkill::setMaxNum(int max_num)
{
    this->max_num = max_num;
}

void YijiViewAsSkill::setPlayerNames(const QStringList &names)
{
}

bool YijiViewAsSkill::viewFilter(const QList<const Card *> &selected, const Card *card, const Player * /*Self*/) const
{
    return ids.contains(card->id()) && selected.length() < max_num;
}

const Card *YijiViewAsSkill::viewAs(const QList<const Card *> &cards, const Player *player) const
{
    if (cards.isEmpty() || cards.length() > max_num)
        return nullptr;

    auto card = player->getRoomObject()->cloneSkillCard("YijiCard");
    card->setHandleMethod(Card::MethodNone);

    card->clearSubcards();
    foreach (const Card *c, cards)
        card->addSubcard(c);
    return card;
}

// ------------------------------------------------

class ChoosePlayerCard : public SkillCard
{
    Q_OBJECT
public:
    ChoosePlayerCard()
    {
        // target_fixed = false;
        setTargetFixed(false);
    }

    void setPlayerNames(const QStringList &names)
    {
        set = QSet<QString>(names.begin(), names.end());
    }

    int targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *, const Card *) const override
    {
        return targets.isEmpty() && set.contains(to_select->objectName()) ? 1 : 0;
    }

private:
    QSet<QString> set;
};

ChoosePlayerSkill::ChoosePlayerSkill()
    : ZeroCardViewAsSkill("choose_player")
{
}

ChoosePlayerSkill::~ChoosePlayerSkill()
{
    
}

void ChoosePlayerSkill::setPlayerNames(const QStringList &names)
{
}

const Card *ChoosePlayerSkill::viewAs(const Player *player) const
{
    return player->getRoomObject()->cloneSkillCard("ChoosePlayerCard");
}

#include "aux-skills.moc"
