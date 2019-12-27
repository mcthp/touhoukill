#include "th07.h"
#include "client.h"
#include "clientplayer.h"
#include "engine.h"
#include "general.h"
#include "skill.h"
#include "standard.h"

class Sidie : public TriggerSkill
{
public:
    Sidie()
        : TriggerSkill("sidie")
    {
        events << EventPhaseStart;
    }

    bool canPreshow() const
    {
        return false;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        ServerPlayer *uuz = data.value<ServerPlayer *>();
        if (uuz->isAlive() && uuz->hasSkill(this) && uuz->getPhase() == Player::Start) {
            foreach (ServerPlayer *p, room->getOtherPlayers(uuz)) {
                if (uuz->getHandcardNum() > p->getHandcardNum() && uuz->canSlash(p, false))
                    return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, uuz, uuz);
            }
        }
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        ServerPlayer *uuz = invoke->invoker;
        QList<ServerPlayer *> targets;
        if (uuz->isAlive() && uuz->hasSkill(this)) {
            foreach (ServerPlayer *p, room->getOtherPlayers(uuz)) {
                if (uuz->getHandcardNum() > p->getHandcardNum() && uuz->canSlash(p, false))
                    targets << p;
            }
        }
        ServerPlayer *target = room->askForPlayerChosen(uuz, targets, objectName(), "@sidie", true, true);
        if (target)
            invoke->targets << target;
        return target != NULL;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        Slash *slash = new Slash(Card::NoSuit, 0);
        slash->deleteLater();
        slash->setSkillName(objectName());
        room->useCard(CardUseStruct(slash, invoke->invoker, invoke->targets.first()), false);
        return false;
    }
};

class Huaxu : public TriggerSkill
{
public:
    Huaxu()
        : TriggerSkill("huaxu")
    {
        events << EventPhaseStart << EventPhaseChanging;
    }

    void record(TriggerEvent triggerEvent, Room *room, QVariant &data) const
    {
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive && change.player->getMark("sidie") > 0) {
                room->setPlayerMark(change.player, "sidie", 0);
                room->handleAcquireDetachSkills(change.player, "-sidie", true);
            }
        }
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent triggerEvent, const Room *room, const QVariant &data) const
    {
        if (triggerEvent != EventPhaseStart)
            return QList<SkillInvokeDetail>();
        QList<SkillInvokeDetail> d;
        ServerPlayer *player = data.value<ServerPlayer *>();
        if (player->getPhase() == Player::RoundStart) {
            foreach (ServerPlayer *p, room->findPlayersBySkillName(objectName())) {
                if (p != player && p->canDiscard(p, "e"))
                    d << SkillInvokeDetail(this, p, p, NULL, false, player);
            }
        }
        return d;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        ServerPlayer *uuz = invoke->invoker;
        ServerPlayer *target = invoke->targets.first();
        QList<const Card *> cards;
        foreach (const Card *c, uuz->getCards("e")) {
            if (uuz->canDiscard(uuz, c->getId()))
                cards << c;
        }

        if (!cards.isEmpty()) {
            DummyCard dummy;
            dummy.addSubcards(cards);
            room->throwCard(&dummy, uuz, uuz);
            uuz->drawCards(cards.length());
            if (!target->hasSkill("sidie", true, false)) {
                room->setPlayerMark(target, "sidie", 1);
                room->handleAcquireDetachSkills(target, "sidie", true);
            }
        }
        return false;
    }
};

class Moran : public TriggerSkill
{
public:
    Moran()
        : TriggerSkill("moran$")
    {
        events << Damaged << FinishJudge;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
    {
        QList<SkillInvokeDetail> details;
        if (event == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.to->isAlive() && damage.to->getKingdom() == "yym") {
                foreach (ServerPlayer *p, room->getOtherPlayers(damage.to)) {
                    if (p->hasLordSkill(objectName())) {
                        for (int i = 0; i < damage.damage; ++i)
                            details << SkillInvokeDetail(this, p, damage.to);
                    }
                }
            }
        } else if (event == FinishJudge) {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            if (judge->reason == objectName() && judge->isGood() && !judge->ignore_judge) {
                ServerPlayer *uuz = judge->relative_player;
                if (uuz && uuz->isAlive())
                    details << SkillInvokeDetail(this, uuz, uuz, NULL, true);
            }
        }
        return details;
    }

    bool cost(TriggerEvent triggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        if (triggerEvent == Damaged) {
            bool doJudge = invoke->invoker->askForSkillInvoke(objectName(), QVariant::fromValue(invoke->owner));
            if (doJudge) {
                room->broadcastSkillInvoke(objectName());
                room->notifySkillInvoked(invoke->owner, objectName());
                LogMessage log;
                log.type = "#InvokeOthersSkill";
                log.from = invoke->invoker;
                log.to << invoke->owner;
                log.arg = objectName();
                room->sendLog(log);
                return true;
            }
        } else
            return true;
        return false;
    }

    bool effect(TriggerEvent triggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        if (triggerEvent == Damaged) {
            JudgeStruct judge;
            judge.pattern = ".|black";
            judge.who = invoke->invoker;
            judge.reason = objectName();
            judge.good = true;
            judge.relative_player = invoke->owner;
            room->judge(judge);

        } else {
            JudgeStruct *judge = data.value<JudgeStruct *>();
            invoke->owner->obtainCard(judge->card);
        }
        return false;
    }
};

class Shenyin : public TriggerSkill
{
public:
    Shenyin()
        : TriggerSkill("shenyin")
    {
        events << Damaged << Damage;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *, const QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (!damage.to || !damage.to->isAlive() || damage.to->isRemoved())
            return QList<SkillInvokeDetail>();
        if (e == Damage && damage.from && damage.from->hasSkill(this))
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.from, damage.from, NULL, false, damage.to);

        if (e == Damaged && damage.to->hasSkill(this))
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.to, damage.to, NULL, false, damage.to);

        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        QString prompt = "target:" + invoke->preferredTarget->objectName();
        invoke->invoker->tag["shenyin-target"] = QVariant::fromValue(invoke->preferredTarget);
        return invoke->invoker->askForSkillInvoke(this, prompt);
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        if (!invoke->targets.first()->isRemoved()) {
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, invoke->invoker->objectName(), invoke->targets.first()->objectName());
            room->touhouLogmessage("#Shenyin1", invoke->targets.first(), objectName(), QList<ServerPlayer *>());
            room->setPlayerCardLimitation(invoke->targets.first(), "use", ".", "lure_tiger", true);
            room->setPlayerProperty(invoke->targets.first(), "removed", true);
        }
        return false;
    }
};

namespace XijianFunc {

bool checkXijianMove(const Player *src, const Player *dist);

bool isXijianPairs(const Player *target1, const Player *target2)
{
    if (target1 == target2)
        return false;
    if (!target1->inMyAttackRange(target2) && !target2->inMyAttackRange(target1))
        return checkXijianMove(target1, target2);
    return false;
}

bool checkXijianMove(const Player *src, const Player *dist)
{
    if (src == dist)
        return false;
    if (!src->isKongcheng())
        return true;

    foreach (const Card *card, src->getJudgingArea()) {
        if (!dist->containsTrick(card->objectName()))
            return true;
    }

    foreach (const Card *e, src->getEquips()) {
        const EquipCard *equip = qobject_cast<const EquipCard *>(e->getRealCard());
        if (!dist->getEquip(equip->location()))
            return true;
    }
    return false;
}
}

XijianCard::XijianCard()
{
    m_skillName = "xijian";
}

bool XijianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *) const
{
    if (targets.length() == 0) {
        if (to_select->isAllNude())
            return false;
        foreach (const Player *p, to_select->getAliveSiblings()) {
            if (XijianFunc::isXijianPairs(to_select, p))
                return true;
        }
    } else if (targets.length() == 1)
        return XijianFunc::isXijianPairs(targets.first(), to_select);
    return false;
}

bool XijianCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const
{
    return targets.length() == 2;
}

void XijianCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    CardUseStruct use = card_use;
    QVariant data = QVariant::fromValue(use);
    RoomThread *thread = room->getThread();

    thread->trigger(PreCardUsed, room, data);

    ServerPlayer *from = card_use.from;
    ServerPlayer *to1 = card_use.to.at(0);
    ServerPlayer *to2 = card_use.to.at(1);
    LogMessage log;
    log.from = card_use.from;
    log.to << card_use.to;
    log.type = "#UseCard";
    log.card_str = toString();
    room->sendLog(log);

    QList<int> disable;
    QString flag = "";
    if (to1->getHandcardNum() > 0)
        flag = flag + "hs";
    if (to1->getJudgingArea().length() > 0) {
        flag = flag + "j";
        foreach (const Card *card, to1->getJudgingArea()) {
            if (to2->containsTrick(card->objectName()))
                disable << card->getEffectiveId();
        }
    }
    if (to1->getEquips().length() > 0) {
        flag = flag + "e";
        foreach (const Card *e, to1->getEquips()) {
            const EquipCard *equip = qobject_cast<const EquipCard *>(e->getRealCard());
            if (to2->getEquip(equip->location())) {
                disable << e->getEffectiveId();
            }
        }
    }

    int card_id = room->askForCardChosen(from, to1, flag, "xijian", false, Card::MethodNone, disable);
    Player::Place place = room->getCardPlace(card_id);
    const Card *card = Sanguosha->getCard(card_id);
    card_use.from->showHiddenSkill("xijian");
    if (place == Player::PlaceHand)
        to2->obtainCard(card, false);
    else {
        room->moveCardTo(card, to1, to2, place, CardMoveReason(CardMoveReason::S_REASON_TRANSFER, from->objectName(), "xijian", QString()));
        if (place == Player::PlaceDelayedTrick) {
            CardUseStruct use(card, NULL, to2);
            QVariant _data = QVariant::fromValue(use);
            room->getThread()->trigger(TargetConfirming, room, _data);
            CardUseStruct new_use = _data.value<CardUseStruct>();
            if (new_use.to.isEmpty())
                card->onNullified(to2);

            room->getThread()->trigger(TargetConfirmed, room, _data);
        }
    }

    thread->trigger(CardUsed, room, data);
    use = data.value<CardUseStruct>();
    thread->trigger(CardFinished, room, data);
}

class XijianVS : public ZeroCardViewAsSkill
{
public:
    XijianVS()
        : ZeroCardViewAsSkill("xijian")
    {
        response_pattern = "@@xijian";
    }

    virtual const Card *viewAs() const
    {
        return new XijianCard;
    }
};

class Xijian : public TriggerSkill
{
public:
    Xijian()
        : TriggerSkill("xijian")
    {
        events << EventPhaseStart;
        view_as_skill = new XijianVS;
    }

    bool canPreshow() const
    {
        return true;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        ServerPlayer *yukari = data.value<ServerPlayer *>();
        if (yukari && yukari->isAlive() && yukari->getPhase() == Player::Finish && yukari->hasSkill(this)) {
            foreach (ServerPlayer *t1, room->getAlivePlayers()) {
                foreach (ServerPlayer *t2, room->getOtherPlayers(t1))
                    if (XijianFunc::isXijianPairs(t1, t2))
                        return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, yukari, yukari);
            }
        }
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        return room->askForUseCard(invoke->invoker, "@@xijian", "@xijian");
    }
};

class Jingjie : public TriggerSkill
{
public:
    Jingjie()
        : TriggerSkill("jingjie")
    {
        frequency = Frequent;
        events << Damaged << AfterDrawNCards;
        related_pile = "jingjie";
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent triggerEvent, const Room *, const QVariant &data) const
    {
        QList<SkillInvokeDetail> d;
        ServerPlayer *yukari = NULL;
        int num = 1;
        if (triggerEvent == AfterDrawNCards) {
            DrawNCardsStruct dc = data.value<DrawNCardsStruct>();
            yukari = dc.player;
        } else if (triggerEvent == Damaged) {
            num = data.value<DamageStruct>().damage;
            yukari = data.value<DamageStruct>().to;
        }

        QStringList trigger_list;
        if (yukari && yukari->isAlive() && yukari->hasSkill(this)) {
            for (int i = 1; i <= num; i++)
                d << SkillInvokeDetail(this, yukari, yukari);
        }
        return d;
    }

    //default cost

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        invoke->invoker->drawCards(1);
        if (!invoke->invoker->isKongcheng()) {
            const Card *cards = room->askForExchange(invoke->invoker, objectName(), 1, 1, false, "jingjie_exchange");
            DELETE_OVER_SCOPE(const Card, cards)
            int id = cards->getSubcards().first();
            invoke->invoker->addToPile("jingjie", id);
        }
        return false;
    }
};

class BayunziDiscardJingjie : public OneCardViewAsSkill
{
public:
    BayunziDiscardJingjie(const QString &name)
        : OneCardViewAsSkill(name)
    {
        expand_pile = "jingjie";
        response_pattern = "@@" + name;
        filter_pattern = ".|.|.|jingjie";
    }

    const Card *viewAs(const Card *originalCard) const
    {
        return originalCard;
    }
};

class Sisheng : public TriggerSkill
{
public:
    Sisheng()
        : TriggerSkill("sisheng")
    {
        events << Dying;
        view_as_skill = new BayunziDiscardJingjie("sisheng");
    }

    bool canPreshow() const
    {
        return true;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        QList<SkillInvokeDetail> d;
        ServerPlayer *who = data.value<DyingStruct>().who;
        QList<ServerPlayer *> yukaris = room->findPlayersBySkillName(objectName());
        foreach (ServerPlayer *p, yukaris) {
            if (who && p->getPile("jingjie").length() > 0 && p->canDiscard(who, "hes") && who->getHp() < who->dyingThreshold() && who->isAlive())
                d << SkillInvokeDetail(this, p, p);
        }
        return d;
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        ServerPlayer *yukari = invoke->invoker;
        const Card *c = room->askForCard(yukari, "@@sisheng", "@sisheng-invoke:" + data.value<DyingStruct>().who->objectName(), data, Card::MethodNone, NULL, false, "sisheng");
        if (c != NULL) {
            CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, "", NULL, objectName(), "");
            room->throwCard(c, reason, NULL);
            return true;
        }
        return false;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        ServerPlayer *yukari = invoke->invoker;
        ServerPlayer *who = data.value<DyingStruct>().who;
        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, yukari->objectName(), who->objectName());
        room->touhouLogmessage("#ChoosePlayerWithSkill", yukari, objectName(), QList<ServerPlayer *>() << who);

        int id2;
        if (yukari == who) {
            const Card *card = room->askForExchange(yukari, objectName(), 1, 1, true, "@sisheng");
            DELETE_OVER_SCOPE(const Card, card)
            id2 = card->getSubcards().first();
        } else
            id2 = room->askForCardChosen(yukari, who, "hes", objectName(), false, Card::MethodDiscard);

        if (yukari->canDiscard(who, id2))
            room->throwCard(id2, who, yukari == who ? NULL : yukari);

        RecoverStruct recover;
        recover.recover = 1;
        recover.who = yukari;
        room->recover(who, recover);
        return false;
    }
};

class Jingdong : public TriggerSkill
{
public:
    Jingdong()
        : TriggerSkill("jingdong")
    {
        events << EventPhaseChanging;
        view_as_skill = new BayunziDiscardJingjie("jingdong");
    }

    bool canPreshow() const
    {
        return true;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        QList<SkillInvokeDetail> d;
        PhaseChangeStruct change = data.value<PhaseChangeStruct>();
        if (change.to == Player::Discard) {
            QList<ServerPlayer *> yukaris = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *yukari, yukaris) {
                if (!yukari->getPile("jingjie").isEmpty())
                    d << SkillInvokeDetail(this, yukari, yukari);
            }
        }
        return d;
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        ServerPlayer *yukari = invoke->invoker;
        ServerPlayer *player = data.value<PhaseChangeStruct>().player;
        QString prompt = "@jingdong-target:" + player->objectName();
        const Card *c = room->askForCard(yukari, "@@jingdong", prompt, QVariant::fromValue(player), Card::MethodNone, NULL, false, "jingdong");
        if (c != NULL) {
            room->notifySkillInvoked(invoke->invoker, objectName());
            room->touhouLogmessage("#InvokeSkill", invoke->invoker, objectName());
            CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, "", NULL, objectName(), "");
            room->throwCard(c, reason, NULL);

            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, yukari->objectName(), player->objectName());
            return true;
        }
        return false;
    }

    bool effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
    {
        ServerPlayer *player = data.value<PhaseChangeStruct>().player;
        player->skip(Player::Discard);
        return false;
    }
};

ShihuiCard::ShihuiCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

void ShihuiCard::use(Room *room, ServerPlayer *, QList<ServerPlayer *> &targets) const
{
    room->obtainCard(targets.first(), this);
}

class ShihuiVS : public ViewAsSkill
{
public:
    ShihuiVS()
        : ViewAsSkill("shihuiVS")
    {
        response_pattern = "@@shihuiVS";
        response_or_use = true;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *) const
    {
        int maxnum = qMax(Self->getEquips().length(), 1);
        if (selected.length() >= maxnum)
            return false;
        return true;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        int maxnum = qMax(Self->getEquips().length(), 1);
        if (cards.length() == maxnum) {
            ExNihilo *exnihilo = new ExNihilo(Card::SuitToBeDecided, -1);
            exnihilo->addSubcards(cards);
            exnihilo->setSkillName("_shihui");
            return exnihilo;
        } else
            return NULL;
    }
};

class Shihui : public TriggerSkill
{
public:
    Shihui()
        : TriggerSkill("shihui")
    {
        events << Damage << DamageDone << EventPhaseChanging << TrickCardCanceling;
    }

    void record(TriggerEvent e, Room *room, QVariant &data) const
    {
        //record times of using card
        if (e == DamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from && damage.from->getPhase() == Player::Play) {
                if (!damage.from->hasFlag("shihui_first")) {
                    room->setPlayerFlag(damage.from, "shihui_first");
                    damage.trigger_info = "shihui_first";
                    data = QVariant::fromValue(damage);
                }
            }
        } else if (e == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.from == Player::Play) {
                change.player->setFlags("-shihui_first");
            }
        }
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *room, const QVariant &data) const
    {
        if (e == Damage) {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.from || damage.from->isDead() || damage.from->getPhase() != Player::Play || damage.trigger_info != "shihui_first")
                return QList<SkillInvokeDetail>();

            //int maxnum = qMax(damage.from->getEquips().length(), 1);
            QList<SkillInvokeDetail> d;
            foreach (ServerPlayer *p, room->findPlayersBySkillName(objectName()))
                d << SkillInvokeDetail(this, p, p, NULL, false, damage.from);
            return d;
        } else if (e == TrickCardCanceling) {
            CardEffectStruct effect = data.value<CardEffectStruct>();
            if (effect.to && effect.card && effect.card->getSkillName() == objectName())
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, effect.to, effect.to, NULL, true);
        }
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent e, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        if (e == Damage) {
            ServerPlayer *target = data.value<DamageStruct>().from;
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, invoke->invoker->objectName(), target->objectName());
            int maxnum = qMax(target->getEquips().length(), 1);
            const Card *c = room->askForUseCard(target, "@@shihuiVS", "shihuiuse");
            if (c == NULL)
                room->askForDiscard(target, objectName(), maxnum, maxnum, false, true);
        } else {
            return true;
        }
        return false;
    }
};

class Huanzang : public TriggerSkill
{
public:
    Huanzang()
        : TriggerSkill("huanzang")
    {
        events << Dying;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        QList<SkillInvokeDetail> d;
        ServerPlayer *who = data.value<DyingStruct>().who;
        if (who->getHp() < who->dyingThreshold() && who->isAlive() && !who->isAllNude()) {
            foreach (ServerPlayer *p, room->findPlayersBySkillName(objectName()))
                d << SkillInvokeDetail(this, p, p, NULL, false, who);
        }
        return d;
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        if (invoke->invoker->askForSkillInvoke(this, data)) {
            ServerPlayer *who = data.value<DyingStruct>().who;
            QStringList places;
            if (!who->getEquips().isEmpty())
                places << "e";
            if (!who->isKongcheng())
                places << "hs";
            if (!who->getJudgingArea().isEmpty())
                places << "j";
            QString choice = room->askForChoice(invoke->invoker, objectName(), places.join("+"), data);
            invoke->tag["huanzang"] = choice;
            return true;
        }
        return false;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, invoke->invoker->objectName(), invoke->targets.first()->objectName());

        QString flag = invoke->tag.value("huanzang").toString();
        ServerPlayer *who = data.value<DyingStruct>().who;
        QList<const Card *> cards = who->getCards(flag);
        bool rec = false;
        foreach (const Card *c, cards) {
            if (c->getTypeId() == Card::TypeEquip) {
                rec = true;
                break;
            }
        }

        DummyCard *dummy = new DummyCard;
        dummy->addSubcards(cards);
        dummy->deleteLater();

        LogMessage log;
        log.type = "#Card_Recast";
        log.from = who;
        log.card_str = IntList2StringList(dummy->getSubcards()).join("+");
        room->sendLog(log);

        CardMoveReason reason(CardMoveReason::S_REASON_RECAST, who->objectName());
        reason.m_skillName = objectName();

        room->moveCardTo(dummy, who, NULL, Player::DiscardPile, reason);
        who->broadcastSkillInvoke("@recast");

        who->drawCards(cards.length());

        if (rec && who->getHp() < who->dyingThreshold()) {
            room->setPlayerFlag(who, "huanzang_buff");
        }
        return false;
    }
};

class HuanzangEffect : public TriggerSkill
{
public:
    HuanzangEffect()
        : TriggerSkill("#huanzang")
    {
        events << QuitDying << PreHpRecover << EventPhaseChanging;
    }

    void record(TriggerEvent triggerEvent, Room *room, QVariant &data) const
    {
        switch (triggerEvent) {
        case QuitDying: {
            ServerPlayer *who = data.value<DyingStruct>().who;
            room->setPlayerFlag(who, "-huanzang_buff");
            break;
        }
        case EventPhaseChanging: {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive) {
                foreach (ServerPlayer *p, room->getAllPlayers()) {
                    room->setPlayerFlag(p, "-huanzang_buff");
                }
            }
            break;
        }
        default:
            break;
        }
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent triggerEvent, const Room *, const QVariant &data) const
    {
        if (triggerEvent == PreHpRecover) {
            RecoverStruct recover = data.value<RecoverStruct>();
            if (recover.to->hasFlag("huanzang_buff"))
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, recover.to, recover.to, NULL, true);
        }
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
    {
        RecoverStruct recover = data.value<RecoverStruct>();
        recover.recover = recover.recover + 1;
        data = QVariant::fromValue(recover);
        return false;
    }
};

class ZhaoliaoVS : public OneCardViewAsSkill
{
public:
    ZhaoliaoVS()
        : OneCardViewAsSkill("zhaoliaoVS")
    {
        response_pattern = "@@zhaoliaoVS";
        response_or_use = true;
    }

    virtual bool viewFilter(const Card *c) const
    {
        return !c->isEquipped();
    }

    virtual const Card *viewAs(const Card *c) const
    {
        ExNihilo *exnihilo = new ExNihilo(Card::SuitToBeDecided, 0);
        exnihilo->addSubcard(c);
        exnihilo->setSkillName("_zhaoliao");
        return exnihilo;
    }
};

class Zhaoliao : public TriggerSkill
{
public:
    Zhaoliao()
        : TriggerSkill("zhaoliao")
    {
        events << Damaged;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        QList<SkillInvokeDetail> d;
        ServerPlayer *a = data.value<DamageStruct>().to;
        QList<ServerPlayer *> rans = room->findPlayersBySkillName(objectName());
        foreach (ServerPlayer *ran, rans) {
            if (a->isAlive() && a != ran && !ran->isNude())
                d << SkillInvokeDetail(this, ran, ran);
        }
        return d;
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        ServerPlayer *a = data.value<DamageStruct>().to;
        ServerPlayer *ran = invoke->invoker;
        ran->tag["zhaoliao_target"] = QVariant::fromValue(a);
        const Card *cards = room->askForExchange(ran, "zhaoliao", ran->getCards("hes").length(), 1, true, "@zhaoliao:" + a->objectName(), true);
        ran->tag.remove("zhaoliao_target");
        if (cards) {
            ran->showHiddenSkill("zhaoliao");
            room->notifySkillInvoked(ran, objectName());
            QList<ServerPlayer *> logto;
            logto << a;
            room->touhouLogmessage("#ChoosePlayerWithSkill", ran, "zhaoliao", logto);

            room->obtainCard(a, cards, false);
            delete cards;
            cards = NULL;
            return true;
        }
        return false;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        ServerPlayer *player = data.value<DamageStruct>().to;
        ServerPlayer *ran = invoke->invoker;
        QString choice = room->askForChoice(ran, objectName(), "zhaoliao1+zhaoliao2");
        if (choice == "zhaoliao1")
            ran->drawCards(1);
        else
            room->askForUseCard(player, "@@zhaoliaoVS", "zhaoliaouse");
        return false;
    }
};

class Jiaoxia : public TriggerSkill
{
public:
    Jiaoxia()
        : TriggerSkill("jiaoxia")
    {
        events << Dying;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        ServerPlayer *who = data.value<DyingStruct>().who;
        if (who->hasSkill(this) && who->getEquips().length() == 0 && who->getHp() < who->dyingThreshold())
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, who, who);
        return QList<SkillInvokeDetail>();
    }

    //default cost

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        ServerPlayer *player = invoke->invoker;
        JudgeStruct judge;
        judge.reason = objectName();
        judge.who = player;
        judge.good = false;
        judge.pattern = ".|heart";

        room->judge(judge);
        if (judge.isGood()) {
            RecoverStruct recover;
            recover.recover = 1;
            recover.who = player;
            room->recover(player, recover);
        }
        return false;
    }
};

class Shuangren : public TargetModSkill
{
public:
    Shuangren()
        : TargetModSkill("shuangren")
    {
        frequency = NotFrequent;
        pattern = "Slash";
    }

    virtual int getExtraTargetNum(const Player *player, const Card *card) const
    {
        if (card->isKindOf("Slash") && player->hasSkill(objectName()))
            return 1;
        else
            return 0;
    }
};

class Zhanwang : public TriggerSkill
{
public:
    Zhanwang()
        : TriggerSkill("zhanwang")
    {
        events << DamageCaused;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.chain || damage.transfer || !damage.by_user || !damage.from || damage.from->isDead() || damage.from == damage.to || !damage.from->hasSkill(this))
            return QList<SkillInvokeDetail>();
        if (damage.card && damage.card->isKindOf("Slash") && !damage.to->getEquips().isEmpty())
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.from, damage.from, NULL, false, damage.to);
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, invoke->invoker->objectName(), invoke->targets.first()->objectName());
        room->broadcastSkillInvoke(objectName());
        QString prompt = "@zhanwang-discard:" + invoke->invoker->objectName();
        const Card *card = room->askForCard(invoke->targets.first(), ".|.|.|equipped", prompt, data, Card::MethodDiscard);
        if (card == NULL) {
            DamageStruct damage = data.value<DamageStruct>();
            damage.damage = damage.damage + 1;
            data = QVariant::fromValue(damage);
        }
        return false;
    }
};

class XiezouVS : public ZeroCardViewAsSkill
{
public:
    XiezouVS()
        : ZeroCardViewAsSkill("xiezou")
    {
        response_pattern = "@@xiezou";
    }

    virtual const Card *viewAs() const
    {
        QString cardname = Self->property("xiezou_card").toString();
        Card *card = Sanguosha->cloneCard(cardname);

        card->setSkillName("xiezou");
        return card;
    }
};

class Xiezou : public TriggerSkill
{
public:
    Xiezou()
        : TriggerSkill("xiezou")
    {
        events << EventPhaseEnd << PreCardUsed << CardResponded << EventPhaseChanging;
        view_as_skill = new XiezouVS;
    }

    bool canPreshow() const
    {
        return true;
    }

    void record(TriggerEvent triggerEvent, Room *room, QVariant &data) const
    {
        if (triggerEvent == PreCardUsed || triggerEvent == CardResponded) {
            ServerPlayer *player = NULL;
            const Card *card = NULL;
            if (triggerEvent == PreCardUsed) {
                player = data.value<CardUseStruct>().from;
                card = data.value<CardUseStruct>().card;
            } else {
                CardResponseStruct response = data.value<CardResponseStruct>();
                player = response.m_from;
                if (response.m_isUse)
                    card = response.m_card;
            }
            if (player && player->getPhase() == Player::Play && card && !card->isKindOf("SkillCard") && card->getHandlingMethod() == Card::MethodUse)
                room->setPlayerProperty(player, "xiezou_card", card->objectName());
        } else if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.from == Player::Play) {
                room->setPlayerProperty(change.player, "xiezou_card", QVariant());
            }
        }
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent triggerEvent, const Room *, const QVariant &data) const
    {
        if (triggerEvent == EventPhaseEnd) {
            ServerPlayer *player = data.value<ServerPlayer *>();
            if (player->getPhase() == Player::Play && player->hasSkill(this) && player->isAlive()) {
                QString cardname = player->property("xiezou_card").toString();
                Card *card = Sanguosha->cloneCard(cardname);
                if (card == NULL)
                    return QList<SkillInvokeDetail>();
                DELETE_OVER_SCOPE(Card, card)
                if (card->isKindOf("Slash") //|| card->isKindOf("Peach")
                    || (card->isNDTrick() && !card->isKindOf("Nullification"))) {
                    if (!player->isCardLimited(card, Card::MethodUse))
                        return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player);
                }
            }
        }
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        QString cardname = invoke->invoker->property("xiezou_card").toString();
        Card *card = Sanguosha->cloneCard(cardname);
        QString prompt = "@xiezou:" + card->objectName();
        delete card;
        room->setPlayerFlag(invoke->invoker, "Global_InstanceUse_Failed");
        return room->askForUseCard(invoke->invoker, "@@xiezou", prompt);
    }
};

class Hesheng : public TriggerSkill
{
public:
    Hesheng()
        : TriggerSkill("hesheng")
    {
        events << EventPhaseStart << CardsMoveOneTime << EventPhaseChanging;
        frequency = Frequent;
    }

    void record(TriggerEvent e, Room *room, QVariant &data) const
    {
        if (e == CardsMoveOneTime) {
            ServerPlayer *current = room->getCurrent();
            if (!current || current->isDead())
                return;
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            if (move.to_place == Player::DiscardPile)
                room->setPlayerMark(current, objectName(), current->getMark(objectName()) + move.card_ids.length());
        } else if (e == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::NotActive)
                room->setPlayerMark(change.player, objectName(), 0);
        }
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *, const QVariant &data) const
    {
        if (e != EventPhaseStart)
            return QList<SkillInvokeDetail>();
        ServerPlayer *current = data.value<ServerPlayer *>();
        if (current->hasSkill(this) && current->isAlive() && current->getPhase() == Player::Finish && current->getHp() < current->getMark(objectName()) && current->isWounded())
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, current, current);
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        RecoverStruct recover;
        room->recover(invoke->invoker, recover);
        return false;
    }
};

class ZhanzhenVS : public OneCardViewAsSkill
{
public:
    ZhanzhenVS()
        : OneCardViewAsSkill("zhanzhen")
    {
        filter_pattern = "EquipCard";
        response_or_use = true;
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return Slash::IsAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *, const QString &pattern) const
    {
        return matchAvaliablePattern("jink", pattern) || matchAvaliablePattern("slash", pattern);
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        if (originalCard != NULL) {
            bool play = (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_PLAY);
            QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();
            if (play || matchAvaliablePattern("slash", pattern)) {
                Slash *slash = new Slash(originalCard->getSuit(), originalCard->getNumber());
                slash->addSubcard(originalCard);
                slash->setSkillName(objectName());
                return slash;
            } else {
                Jink *jink = new Jink(originalCard->getSuit(), originalCard->getNumber());
                jink->addSubcard(originalCard);
                jink->setSkillName(objectName());
                return jink;
            }
        } else
            return NULL;
    }
};

class Zhanzhen : public TriggerSkill
{
public:
    Zhanzhen()
        : TriggerSkill("zhanzhen")
    {
        events << CardsMoveOneTime;
        view_as_skill = new ZhanzhenVS;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        ServerPlayer *player = qobject_cast<ServerPlayer *>(move.from);
        if (player != NULL && player->isAlive() && player->hasSkill(this) && move.card_ids.length() == 1 && move.to_place == Player::DiscardPile
            && ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_USE
                || (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_RESPONSE)) {
            const Card *card = move.reason.m_extraData.value<const Card *>();
            if (card && card->getSkillName() == objectName() && room->getCardPlace(move.card_ids.first()) == Player::DiscardPile)
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player);
        }
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        ServerPlayer *player = invoke->invoker;
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        const Card *card = move.reason.m_extraData.value<const Card *>();
        const Card *realcard = Sanguosha->getEngineCard(move.card_ids.first());
        player->tag["zhanzhen"] = QVariant::fromValue(realcard);
        ServerPlayer *target
            = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "@zhanzhen:" + card->objectName() + ":" + realcard->objectName(), true, true);
        if (target) {
            player->tag["zhanzhen_select"] = QVariant::fromValue(target);
            return true;
        }
        return false;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        ServerPlayer *player = invoke->invoker;
        ServerPlayer *target = player->tag.value("zhanzhen_select").value<ServerPlayer *>();
        player->tag.remove("zhanzhen_select");
        if (target) {
            CardsMoveStruct mo;
            mo.card_ids = move.card_ids;
            mo.to = target;
            mo.to_place = Player::PlaceHand;
            room->moveCardsAtomic(mo, true);
        }
        return false;
    }
};

class Renou : public TriggerSkill
{
public:
    Renou()
        : TriggerSkill("renou")
    {
        events << EventPhaseStart;
    }

    bool canPreshow() const
    {
        return false;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        ServerPlayer *player = data.value<ServerPlayer *>();
        if (player->getPhase() == Player::Start && player->hasSkill(this))
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player);
        return QList<SkillInvokeDetail>();
    }

    //default cost

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        QList<int> list = room->getNCards(5);
        ServerPlayer *player = invoke->invoker;
        CardsMoveStruct move(list, NULL, Player::PlaceTable, CardMoveReason(CardMoveReason::S_REASON_TURNOVER, invoke->invoker->objectName(), objectName(), QString()));
        room->moveCardsAtomic(move, true);

        QList<int> able;
        QList<int> disabled;
        foreach (int id, list) {
            Card *tmp_card = Sanguosha->getCard(id);
            if (tmp_card->isKindOf("EquipCard")) {
                able << id;
            } else {
                foreach (const Card *c, player->getCards("e")) {
                    if (c->getSuit() == tmp_card->getSuit()) {
                        disabled << id;
                        break;
                    }
                }
                if (!disabled.contains(id))
                    able << id;
            }
        }
        room->fillAG(list, NULL, disabled);
        int obtainId = -1;
        if (able.length() > 0) {
            obtainId = room->askForAG(player, able, true, objectName());
            if (obtainId > -1)
                room->obtainCard(player, obtainId, true);
        }

        room->getThread()->delay(1000);
        room->clearAG();

        //throw other cards
        if (obtainId > -1)
            list.removeOne(obtainId);
        if (!list.isEmpty()) {
            CardMoveReason reason(CardMoveReason::S_REASON_NATURAL_ENTER, invoke->invoker->objectName(), objectName(), QString());
            DummyCard dummy(list);
            room->throwCard(&dummy, reason, NULL);
        }
        return false;
    }
};

class YaoshuVS : public ZeroCardViewAsSkill
{
public:
    YaoshuVS()
        : ZeroCardViewAsSkill("yaoshu")
    {
        response_pattern = "@@yaoshu";
    }

    virtual const Card *viewAs() const
    {
        QString cardname = Self->property("yaoshu_card").toString();
        Card *card = Sanguosha->cloneCard(cardname);

        card->setSkillName("yaoshu");
        return card;
    }
};

class Yaoshu : public TriggerSkill
{
public:
    Yaoshu()
        : TriggerSkill("yaoshu")
    {
        events << CardFinished;
        view_as_skill = new YaoshuVS;
    }

    void record(TriggerEvent, Room *room, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        ServerPlayer *player = use.from;
        if (use.card->getSkillName() == "yaoshu")
            room->setPlayerProperty(player, "yaoshu_card", QVariant());
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        ServerPlayer *player = use.from;
        if (player && player->isAlive() && player->hasSkill(this) && use.card && use.card->isNDTrick() && !use.card->isKindOf("Nullification")) {
            if (use.card->getSkillName() == "yaoshu")
                return QList<SkillInvokeDetail>();

            if (!use.m_isHandcard)
                return QList<SkillInvokeDetail>();
            //for turnbroken
            if (player->hasFlag("Global_ProcessBroken") || !player->hasSkill(this))
                return QList<SkillInvokeDetail>();

            const Card *c = Sanguosha->cloneCard(use.card->objectName());
            if (c == NULL)
                return QList<SkillInvokeDetail>();
            DELETE_OVER_SCOPE(const Card, c)
            if (!player->isCardLimited(c, Card::MethodUse))
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player);
        }
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        ServerPlayer *player = invoke->invoker;
        room->setPlayerProperty(player, "yaoshu_card", use.card->objectName());
        room->setPlayerFlag(player, "Global_InstanceUse_Failed");
        room->askForUseCard(player, "@@yaoshu", "@yaoshu:" + use.card->objectName());
        return false;
    }
};

QimenCard::QimenCard()
{
}

static int qimenMax(const Player *chen)
{
    int maxNum = chen->getEquips().length();
    foreach (const Player *p, chen->getAliveSiblings()) {
        if (p->getEquips().length() > maxNum)
            maxNum = p->getEquips().length();
    }
    return maxNum;
}

bool QimenCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    //for extra targets or multiple targets like Collateral
    int maxnum = qimenMax(Self);

    QString cardname = Self->property("qimen_card").toString();
    Card *new_card = Sanguosha->cloneCard(cardname);
    DELETE_OVER_SCOPE(Card, new_card)
    new_card->setSkillName("qimen");
    if (targets.isEmpty() && new_card && to_select->getEquips().length() >= maxnum && !Self->isProhibited(to_select, new_card, targets))
        return (new_card->isKindOf("GlobalEffect") || new_card->targetFilter(targets, to_select, Self) || (new_card->isKindOf("Peach") && to_select->isWounded()));

    return false;
}

bool QimenCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    QString cardname = Self->property("qimen_card").toString();
    Card *new_card = Sanguosha->cloneCard(cardname);
    DELETE_OVER_SCOPE(Card, new_card)
    new_card->setSkillName("qimen");

    if (targets.length() < 1)
        return false;
    return new_card && new_card->targetsFeasible(targets, Self);
}

const Card *QimenCard::validate(CardUseStruct &card_use) const
{
    card_use.from->showHiddenSkill("qimen");
    QString cardname = card_use.from->property("qimen_card").toString();
    Card *card = Sanguosha->cloneCard(cardname);
    card->setSkillName("qimen");
    return card;
}

class QimenVS : public ZeroCardViewAsSkill
{
public:
    QimenVS()
        : ZeroCardViewAsSkill("qimen")
    {
        response_pattern = "@@qimen";
    }

    virtual const Card *viewAs() const
    {
        return new QimenCard;
    }
};

class Qimen : public TriggerSkill
{
public:
    Qimen()
        : TriggerSkill("qimen")
    {
        events << CardFinished;
        view_as_skill = new QimenVS;
    }

    bool canPreshow() const
    {
        return true;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        ServerPlayer *player = use.from;
        if (player && player->isAlive() && player->hasSkill(this) && use.card && use.card->isBlack()) { //&& use.to.length() == 1

            if (use.card->isKindOf("Nullification") || use.card->isKindOf("Jink"))
                return QList<SkillInvokeDetail>();
            if (player->hasFlag("Global_ProcessBroken")) //for turnbroken
                return QList<SkillInvokeDetail>();

            if (use.card->isNDTrick() || use.card->getTypeId() == Card::TypeBasic) {
                int maxnum = qimenMax(player);
                Card *c = Sanguosha->cloneCard(use.card->objectName());
                DELETE_OVER_SCOPE(Card, c)
                foreach (ServerPlayer *p, room->getAllPlayers()) {
                    if (p->getEquips().length() >= maxnum && !player->isCardLimited(c, Card::MethodUse) && !player->isProhibited(p, c))
                        return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player);
                }
            }
        }
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        ServerPlayer *player = invoke->invoker;
        room->setPlayerProperty(player, "qimen_card", use.card->objectName());
        return room->askForUseCard(player, "@@qimen", "@qimen:" + use.card->objectName());
    }
};

class QimenDistance : public TargetModSkill
{
public:
    QimenDistance()
        : TargetModSkill("#qimen-dist")
    {
        pattern = "BasicCard,TrickCard+^DelayedTrick";
    }

    int getDistanceLimit(const Player *, const Card *card) const
    {
        if (card->getSkillName() == "qimen")
            return 1000;
        return 0;
    }
};

//Only for AI dummy use
class QimenProhibitAI : public ProhibitSkill
{
public:
    QimenProhibitAI()
        : ProhibitSkill("#qimen-prohibit")
    {
    }

    virtual bool isProhibited(const Player *, const Player *to, const Card *card, const QList<const Player *> &, bool) const
    {
        int maxNum = qimenMax(to);
        return card->getSkillName() == "AIqimen" && to->getEquips().length() < maxNum;
    }
};

class Dunjia : public TriggerSkill
{
public:
    Dunjia()
        : TriggerSkill("dunjia")
    {
        events << DamageInflicted;
        show_type = "static";
    }

    bool canPreshow() const
    {
        return false;
    }

    static QList<ServerPlayer *> dunjiaInvokers(ServerPlayer *chen)
    {
        QList<ServerPlayer *> invokers;
        Room *room = chen->getRoom();
        foreach (ServerPlayer *p, room->getAllPlayers()) {
            if (p->getEquips().length() >= chen->getEquips().length() && p->canDiscard(p, "e")) {
                if (p == chen)
                    invokers << p;
                else if (chen->hasSkill("dunjia", false, false))
                    invokers << p;
            }
        }
        return invokers;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.nature != DamageStruct::Normal || !damage.to->hasSkill(this))
            return QList<SkillInvokeDetail>();
        if (isHegemonyGameMode(room->getMode()) && !damage.to->hasShownSkill(this))
            return QList<SkillInvokeDetail>();

        QList<ServerPlayer *> invokers = dunjiaInvokers(damage.to);
        QList<SkillInvokeDetail> d;
        foreach (ServerPlayer *p, invokers)
            d << SkillInvokeDetail(this, damage.to, p, NULL, false, damage.to);

        return d;
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        QString prompt;
        if (damage.from)
            prompt = QString("@dunjia1:%1:%2:%3").arg(damage.from->objectName()).arg(invoke->owner->objectName()).arg(QString::number(damage.damage));
        else
            prompt = QString("@dunjia2:%1:%2").arg(invoke->owner->objectName()).arg(QString::number(damage.damage));
        invoke->invoker->tag[objectName()] = data;
        const Card *card = room->askForCard(invoke->invoker, ".|.|.|equipped", prompt, data, objectName());
        return card != NULL;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        room->notifySkillInvoked(invoke->owner, objectName());
        if (invoke->invoker != invoke->owner)
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, invoke->invoker->objectName(), invoke->owner->objectName());

        DamageStruct damage = data.value<DamageStruct>();

        QString logo_type = "#Dunjia";
        room->touhouLogmessage(logo_type, invoke->owner, objectName(), QList<ServerPlayer *>(), QString::number(damage.damage));

        return true;
    }
};

class Jiyi : public TriggerSkill
{
public:
    Jiyi()
        : TriggerSkill("jiyi")
    {
        events << TurnedOver;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        ServerPlayer *player = data.value<ServerPlayer *>();
        if (player->hasSkill(this))
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player);
        return QList<SkillInvokeDetail>();
    }

    //default cost

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        ServerPlayer *player = invoke->invoker;
        player->drawCards(2);
        if (player->isKongcheng())
            return false;
        QList<int> hc = player->handCards();
        room->askForRende(player, hc, objectName(), false, true, qMin(2, player->getHandcardNum()));

        return false;
    }
};

class Chunmian : public TriggerSkill
{
public:
    Chunmian()
        : TriggerSkill("chunmian")
    {
        frequency = Compulsory;
        events << EventPhaseStart;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        ServerPlayer *player = data.value<ServerPlayer *>();
        if (player->getPhase() == Player::Finish && player->hasSkill(this))
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player, NULL, true);
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        ServerPlayer *player = invoke->invoker;
        room->touhouLogmessage("#TriggerSkill", player, objectName());
        room->notifySkillInvoked(player, objectName());
        player->turnOver();
        return false;
    }
};

class Baochun : public TriggerSkill
{
public:
    Baochun()
        : TriggerSkill("baochun")
    {
        events << Damaged;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.to->hasSkill(this) && damage.to->isAlive())
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.to, damage.to);
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        ServerPlayer *player = invoke->invoker;
        ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), objectName(), "@" + objectName() + ":" + QString::number(player->getLostHp()), true, true);
        if (target) {
            invoke->targets << target;
            return true;
        }

        return false;
    }

    bool effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        ServerPlayer *target = invoke->targets.first();
        invoke->invoker->tag.remove("baochun_target");
        target->drawCards(invoke->invoker->getLostHp());
        return false;
    }
};

class Chunyi : public TriggerSkill
{
public:
    Chunyi()
        : TriggerSkill("chunyi")
    {
        frequency = Compulsory;
        events << EventPhaseStart;
    }
    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        ServerPlayer *player = data.value<ServerPlayer *>();
        if (player->getPhase() == Player::Start && player->hasSkill(this) && player->getMaxHp() < 6)
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player, NULL, true);
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        ServerPlayer *player = invoke->invoker;
        room->notifySkillInvoked(player, objectName());

        room->setPlayerProperty(player, "maxhp", player->getMaxHp() + 1);
        room->touhouLogmessage("#TriggerSkill", player, objectName());
        room->touhouLogmessage("#GainMaxHp", player, QString::number(1));
        room->touhouLogmessage("#GetHp", player, QString::number(player->getHp()), QList<ServerPlayer *>(), QString::number(player->getMaxHp()));
        return false;
    }
};

class Zhancao : public TriggerSkill
{
public:
    Zhancao()
        : TriggerSkill("zhancao")
    {
        events << TargetConfirmed;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        QList<SkillInvokeDetail> d;
        if (use.card->isKindOf("Slash")) {
            QList<ServerPlayer *> srcs = room->findPlayersBySkillName(objectName());
            foreach (ServerPlayer *p, srcs) {
                if (!p->canDiscard(p, "hes"))
                    continue;
                foreach (ServerPlayer *to, use.to) {
                    if (to->isAlive() && (p->inMyAttackRange(to) || p == to))
                        d << SkillInvokeDetail(this, p, p, NULL, false, to);
                }
            }
        }
        return d;
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        invoke->invoker->tag["zhancao_target"] = QVariant::fromValue(invoke->preferredTarget);
        QString prompt = "@zhancao-discard:" + use.from->objectName() + ":" + invoke->preferredTarget->objectName();
        return room->askForCard(invoke->invoker, ".Equip", prompt, data, objectName()) != NULL;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        use.nullified_list << invoke->targets.first()->objectName();
        data = QVariant::fromValue(use);
        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, invoke->invoker->objectName(), invoke->targets.first()->objectName());
        room->touhouLogmessage("#zhancaoTarget", invoke->invoker, objectName(), QList<ServerPlayer *>() << invoke->targets.first());

        return false;
    }
};

MocaoCard::MocaoCard()
{
}

bool MocaoCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (!targets.isEmpty() || to_select == Self)
        return false;
    return !to_select->getEquips().isEmpty();
}

void MocaoCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();

    int card_id = room->askForCardChosen(effect.from, effect.to, "e", "mocao");
    room->obtainCard(effect.from, card_id);
    if (effect.to->isWounded())
        effect.to->drawCards(qMin(5, effect.to->getLostHp()));
}

class Mocao : public ZeroCardViewAsSkill
{
public:
    Mocao()
        : ZeroCardViewAsSkill("mocao")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("MocaoCard");
    }

    virtual const Card *viewAs() const
    {
        return new MocaoCard;
    }
};

class Youqu : public TriggerSkill
{
public:
    Youqu()
        : TriggerSkill("youqu")
    {
        frequency = Compulsory;
        events << EventPhaseStart;
        related_pile = "siling";
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        ServerPlayer *player = data.value<ServerPlayer *>();
        if (!player->hasSkill(this))
            return QList<SkillInvokeDetail>();
        if ((player->getPhase() == Player::Start && player->getPile("siling").length() >= 2) || player->getPhase() == Player::Finish)
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player, NULL, true);
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        ServerPlayer *player = invoke->invoker;
        if (player->getPhase() == Player::Start) {
            QList<int> sl = player->getPile("siling");
            room->notifySkillInvoked(player, objectName());

            room->touhouLogmessage("#TriggerSkill", player, objectName());

            CardsMoveStruct move(sl, player, player, Player::PlaceSpecial, Player::PlaceHand, CardMoveReason(CardMoveReason::S_REASON_UNKNOWN, QString()));
            room->moveCardsAtomic(move, false);

            room->touhouLogmessage("#silinggain", player, objectName(), QList<ServerPlayer *>(), QString::number(sl.length()));
            room->damage(DamageStruct("siling", player, player));

        } else if (player->getPhase() == Player::Finish) {
            room->notifySkillInvoked(player, objectName());
            room->touhouLogmessage("#TriggerSkill", player, objectName());
            QString choice = room->askForChoice(player, objectName(), "siling1+siling2+siling3");
            QList<int> list;
            if (choice == "siling1")
                list = room->getNCards(1);
            else if (choice == "siling2")
                list = room->getNCards(2);
            else if (choice == "siling3")
                list = room->getNCards(3);
            CardMoveReason reason(CardMoveReason::S_REASON_UNKNOWN, "", NULL, "siling", "");
            player->addToPile("siling", list, false, reason);
        }
        return false;
    }
};

class WangwuVS : public OneCardViewAsSkill
{
public:
    WangwuVS()
        : OneCardViewAsSkill("wangwu")
    {
        response_pattern = "@@wangwu";
        expand_pile = "siling";
    }

    bool viewFilter(const Card *to_select) const
    {
        if (!Self->getPile("siling").contains(to_select->getId()))
            return false;

        QString property = Self->property("wangwu").toString();
        if (property == "black")
            return to_select->isBlack();
        else
            return to_select->isRed();
    }

    const Card *viewAs(const Card *originalCard) const
    {
        return originalCard;
    }
};

class Wangwu : public TriggerSkill
{
public:
    Wangwu()
        : TriggerSkill("wangwu")
    {
        events << TargetConfirmed;
        view_as_skill = new WangwuVS;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        QList<SkillInvokeDetail> d;
        if (use.card && (use.card->isKindOf("Slash") || use.card->isNDTrick()) && (use.card->isRed() || use.card->isBlack())) {
            if (use.from && use.from->isAlive()) {
                foreach (ServerPlayer *p, use.to) {
                    if (p != use.from && p->hasSkill(this) && !p->getPile("siling").isEmpty()) {
                        foreach (int id, p->getPile("siling")) {
                            if (Sanguosha->getCard(id)->sameColorWith(use.card)) {
                                d << SkillInvokeDetail(this, p, p);
                                break;
                            }
                        }
                    }
                }
            }
        }
        return d;
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        ServerPlayer *player = invoke->invoker;

        bool flag = false;
        QList<int> list = player->getPile("siling");
        foreach (int id, list) {
            if (Sanguosha->getCard(id)->sameColorWith(use.card)) {
                flag = true;
                break;
            }
        }
        if (!flag)
            return false;

        if (use.card->isRed()) {
            player->setProperty("wangwu", "red");
            room->notifyProperty(player, player, "wangwu");
        } else {
            player->setProperty("wangwu", "black");
            room->notifyProperty(player, player, "wangwu");
        }
        // for ai record siling lack
        player->tag["wangwu_use"] = data;
        QString prompt = "@wangwu-invoke:" + use.from->objectName() + ":" + use.card->objectName();
        const Card *c = room->askForCard(player, "@@wangwu", prompt, data, Card::MethodNone, NULL, false, "wangwu");

        if (c != NULL) {
            CardMoveReason reason(CardMoveReason::S_REASON_REMOVE_FROM_PILE, QString(), NULL, objectName(), QString());
            room->throwCard(c, reason, NULL);
            room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, player->objectName(), use.from->objectName());
            player->drawCards(1);
            return true;
        }
        return false;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        room->damage(DamageStruct("wangwu", invoke->invoker, use.from));
        return false;
    }
};

class Shizhao : public TriggerSkill
{
public:
    Shizhao()
        : TriggerSkill("shizhao")
    {
        events << EventPhaseStart;
        //frequency = Frequent;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        ServerPlayer *p = data.value<ServerPlayer *>();
        if (p->isAlive() && p->isWounded() && p->getPhase() == Player::Start) {
            QList<SkillInvokeDetail> d;
            foreach (ServerPlayer *ran, room->getAllPlayers()) {
                if (ran->hasSkill(this))
                    d << SkillInvokeDetail(this, ran, ran);
            }
            return d;
        }

        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        ServerPlayer *p = data.value<ServerPlayer *>();
        QList<int> card_ids = room->getNCards(p->getLostHp());
        room->returnToTopDrawPile(card_ids);

        room->fillAG(card_ids, invoke->invoker);
        int to_throw = room->askForAG(invoke->invoker, card_ids, true, objectName());
        room->clearAG(invoke->invoker);
        if (to_throw == -1)
            return false;

        CardMoveReason reason(CardMoveReason::S_REASON_PUT, invoke->invoker->objectName(), objectName(), QString());
        room->throwCard(Sanguosha->getCard(to_throw), reason, NULL);

        return false;
    }
};

// the skill's 2 "subskill"s are of the same timing, and have different effect, so we must split them
class Jixiong : public TriggerSkill
{
public:
    Jixiong()
        : TriggerSkill("jixiong")
    {
        events << EventPhaseChanging;
    }
};

class Jixiong1 : public TriggerSkill
{
public:
    Jixiong1()
        : TriggerSkill("#jixiong1")
    {
        events << CardsMoveOneTime << EventPhaseChanging;
    }

    void record(TriggerEvent e, Room *room, QVariant &) const
    {
        if (e == EventPhaseChanging) {
            foreach (ServerPlayer *p, room->getAlivePlayers()) {
                if (p->hasFlag("jixiong_used"))
                    room->setPlayerFlag(p, "-jixiong_used");
            }
        }
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *room, const QVariant &data) const
    {
        if (e == EventPhaseChanging)
            return QList<SkillInvokeDetail>();

        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.to_place != Player::DiscardPile)
            return QList<SkillInvokeDetail>();
        ServerPlayer *current = room->getCurrent();
        if (current == NULL || current->isDead() || !current->isInMainPhase() || current->getPhase() == Player::Discard || !current->isWounded())
            return QList<SkillInvokeDetail>();

        QList<SkillInvokeDetail> d;
        foreach (int id, move.card_ids) {
            const Card *card = Sanguosha->getCard(id);
            if (card != NULL && card->getSuit() == Card::Heart && card->getNumber() >= 11 && card->getNumber() <= 13) {
                foreach (ServerPlayer *p, room->getAllPlayers()) {
                    if (p->hasSkill("jixiong") && !p->hasFlag("jixiong_used"))
                        d << SkillInvokeDetail(this, p, p, NULL, false, current);
                }
            }
        }
        return d;
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        if (invoke->invoker->askForSkillInvoke("jixiong1", QVariant::fromValue(invoke->preferredTarget))) {
            room->setPlayerFlag(invoke->invoker, "jixiong_used");
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = invoke->invoker;
            log.arg = "jixiong";
            room->sendLog(log);
            room->notifySkillInvoked(invoke->invoker, "jixiong");
            return true;
        }

        return false;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        room->recover(invoke->targets.first(), RecoverStruct());
        return false;
    }
};

class Jixiong2 : public TriggerSkill
{
public:
    Jixiong2()
        : TriggerSkill("#jixiong2")
    {
        events << CardsMoveOneTime;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.to_place != Player::DiscardPile)
            return QList<SkillInvokeDetail>();

        ServerPlayer *dying = room->getCurrentDyingPlayer();
        if (dying != NULL && dying->isAlive())
            return QList<SkillInvokeDetail>();

        ServerPlayer *current = room->getCurrent();
        if (current == NULL || current->isDead() || !current->isCurrent() || !current->isInMainPhase() || current->getPhase() == Player::Discard)
            return QList<SkillInvokeDetail>();

        QList<SkillInvokeDetail> d;
        foreach (int id, move.card_ids) {
            const Card *card = Sanguosha->getCard(id);
            if (card != NULL && card->getSuit() == Card::Spade && card->getNumber() >= 11 && card->getNumber() <= 13) {
                foreach (ServerPlayer *p, room->getAllPlayers()) {
                    if (p->hasSkill("jixiong") && !p->hasFlag("jixiong_used"))
                        d << SkillInvokeDetail(this, p, p, NULL, false, current);
                }
            }
        }
        return d;
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        if (invoke->invoker->askForSkillInvoke("jixiong2", QVariant::fromValue(invoke->preferredTarget))) {
            room->setPlayerFlag(invoke->invoker, "jixiong_used");
            LogMessage log;
            log.type = "#InvokeSkill";
            log.from = invoke->invoker;
            log.arg = "jixiong";
            room->sendLog(log);
            room->notifySkillInvoked(invoke->invoker, "jixiong");
            return true;
        }

        return false;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        room->damage(DamageStruct("jixiong", NULL, invoke->targets.first()));
        return false;
    }
};

class Shoushu : public TriggerSkill
{
public:
    Shoushu()
        : TriggerSkill("shoushu")
    {
        events << CardsMoveOneTime;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        ServerPlayer *player = qobject_cast<ServerPlayer *>(move.from);
        if (player != NULL && player->isAlive() && player->hasSkill(this) && move.to_place == Player::DiscardPile
            && ((move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_USE
                || (move.reason.m_reason & CardMoveReason::S_MASK_BASIC_REASON) == CardMoveReason::S_REASON_RESPONSE)) {
            const Card *card = move.reason.m_extraData.value<const Card *>();
            if (card && card->isKindOf("BasicCard") && room->getCardPlace(move.card_ids.first()) == Player::DiscardPile)
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player);
        }
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        ServerPlayer *player = invoke->invoker;
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        const Card *card = move.reason.m_extraData.value<const Card *>();
        ServerPlayer *target = room->askForPlayerChosen(player, room->getOtherPlayers(player), objectName(), "@shoushu:" + card->objectName(), true, true);
        if (target) {
            invoke->targets << target;
            return true;
        }
        return false;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        ServerPlayer *target = invoke->targets.first();
        if (target) {
            CardsMoveStruct mo;
            mo.card_ids = move.card_ids;
            mo.to = target;
            mo.to_place = Player::PlaceHand;
            room->moveCardsAtomic(mo, true);

            if (target->canDiscard(target, "hs"))
                room->askForDiscard(target, objectName(), 1, 1, false, false, "shoushu_discard");
        }
        return false;
    }
};

YujianCard::YujianCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
}

bool YujianCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    return targets.length() < 2 && !to_select->isKongcheng() && to_select != Self;
}

void YujianCard::use(Room *, ServerPlayer *player, QList<ServerPlayer *> &targets) const
{
    player->pindian(targets.first(), "yujian");
}

class YujianVS : public ZeroCardViewAsSkill
{
public:
    YujianVS()
        : ZeroCardViewAsSkill("yujian")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("YujianCard") && !player->isKongcheng();
    }

    virtual const Card *viewAs() const
    {
        return new YujianCard;
    }
};

class Yujian : public TriggerSkill
{
public:
    Yujian()
        : TriggerSkill("yujian")
    {
        events << Pindian;
        view_as_skill = new YujianVS;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        PindianStruct *pindian = data.value<PindianStruct *>();
        if (pindian->reason == "yujian" && pindian->success && pindian->from->isAlive() && pindian->to->isAlive()) {
            Slash *slash = new Slash(Card::SuitToBeDecided, 0);
            slash->deleteLater();
            slash->addSubcard(pindian->from_card);
            slash->addSubcard(pindian->to_card);
            if (pindian->from->canSlash(pindian->to, slash, false))
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, NULL, pindian->from, NULL, true, pindian->to, false);
        }
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail>, QVariant &data) const
    {
        PindianStruct *pindian = data.value<PindianStruct *>();
        Slash *slash = new Slash(Card::SuitToBeDecided, 0);
        slash->addSubcard(pindian->from_card);
        slash->addSubcard(pindian->to_card);
        slash->setSkillName("yujian");
        room->setCardFlag(slash, "chosenExtraSlashTarget");
        room->useCard(CardUseStruct(slash, pindian->from, pindian->to));
        return false;
    }
};

HuayinCard::HuayinCard()
{
    will_throw = false;
    handling_method = Card::MethodNone;
    m_skillName = "huayin";
}

bool HuayinCard::targetFixed() const
{
    Peach *card = new Peach(Card::NoSuit, 0);
    DELETE_OVER_SCOPE(Peach, card)
    return card->targetFixed();
}

bool HuayinCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    Peach *card = new Peach(Card::NoSuit, 0);
    DELETE_OVER_SCOPE(Peach, card)
    return card->targetFilter(targets, to_select, Self) && !Self->isProhibited(to_select, card, targets);
}

bool HuayinCard::targetsFeasible(const QList<const Player *> &targets, const Player *Self) const
{
    Peach *card = new Peach(Card::NoSuit, 0);
    DELETE_OVER_SCOPE(Peach, card)
    return card->targetsFeasible(targets, Self);
}

void HuayinCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const
{
    room->showAllCards(source);
    Peach *card = new Peach(Card::SuitToBeDecided, -1);
    foreach (int id, source->handCards()) {
        // room->showCard(source, id);
        if (!Sanguosha->getCard(id)->isKindOf("BasicCard"))
            card->addSubcard(id);
    }
    QList<const Player *> playerTargets;
    foreach (auto *p, targets)
        playerTargets << p;

    if (!card->getSubcards().isEmpty() && !source->isCardLimited(card, Card::MethodUse, true)
        && (Sanguosha->getCurrentCardUseReason() != CardUseStruct::CARD_USE_REASON_PLAY || card->isAvailable(source))
        && (card->targetFixed() || card->targetsFeasible(playerTargets, source))) {
        card->setSkillName("huayin");
        CardUseStruct use;
        use.from = source;
        use.card = card;
        use.to = targets;
        room->useCard(use);
    } else {
        delete card;
        room->setPlayerFlag(source, "Global_huayinFailed");
    }
}

class HuayinVS : public ZeroCardViewAsSkill
{
public:
    HuayinVS()
        : ZeroCardViewAsSkill("huayin")
    {
    }

    static bool cardLimit(const Player *player)
    {
        Peach *peach = new Peach(Card::SuitToBeDecided, -1);
        peach->deleteLater();
        foreach (const Card *c, player->getHandcards()) {
            if (c->isKindOf("BasicCard"))
                peach->addSubcard(c);
        }
        return !peach->getSubcards().isEmpty() && player->isCardLimited(peach, Card::MethodUse, true);
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->isKongcheng() && !player->hasFlag("Global_huayinFailed");
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &pattern) const
    {
        return matchAvaliablePattern("peach", pattern) && !player->isKongcheng() && player->getMark("Global_PreventPeach") == 0
            && (Sanguosha->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE_USE) && !player->hasFlag("Global_huayinFailed");
    }

    virtual const Card *viewAs() const
    {
        return new HuayinCard;
    }
};

class Huayin : public TriggerSkill
{
public:
    Huayin()
        : TriggerSkill("huayin")
    {
        events << CardsMoveOneTime << CardFinished;
        view_as_skill = new HuayinVS;
    }

    void record(TriggerEvent e, Room *r, QVariant &data) const
    {
        if (e != CardsMoveOneTime)
            return;
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        if (move.from_places.contains(Player::PlaceHand)) {
            ServerPlayer *from = qobject_cast<ServerPlayer *>(move.from);
            if (from != NULL && from->hasFlag("Global_huayinFailed"))
                r->setPlayerFlag(from, "-Global_huayinFailed");
        }
        if (move.to_place == Player::PlaceHand) {
            ServerPlayer *to = qobject_cast<ServerPlayer *>(move.to);
            if (to != NULL && to->hasFlag("Global_huayinFailed"))
                r->setPlayerFlag(to, "-Global_huayinFailed");
        }
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *, const QVariant &data) const
    {
        if (e == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->getSkillName() == objectName() && use.card->isKindOf("Peach") && !use.from->isKongcheng()) {
                ExNihilo *exnihilo = new ExNihilo(Card::SuitToBeDecided, -1);
                exnihilo->deleteLater();
                if (!use.from->isCardLimited(exnihilo, Card::MethodUse, true))
                    return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, use.from, use.from, NULL, true);
            }
        }
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        room->getThread()->delay();
        room->getThread()->delay();
        ExNihilo *exnihilo = new ExNihilo(Card::SuitToBeDecided, -1);
        exnihilo->addSubcards(invoke->invoker->getCards("hs"));
        exnihilo->setSkillName("_huayin");
        CardUseStruct use;
        use.from = invoke->invoker;
        use.to << invoke->invoker;
        use.card = exnihilo;
        room->useCard(use);
        return false;
    }
};

class Huanling : public TriggerSkill
{
public:
    Huanling()
        : TriggerSkill("huanling")
    {
        events << CardUsed << CardResponded;
        frequency = Frequent;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent triggerEvent, const Room *room, const QVariant &data) const
    {
        bool invoke = false;
        if (triggerEvent == CardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isRed() && !use.card->isBlack() && !use.card->isKindOf("SkillCard"))
                invoke = true;
        } else if (triggerEvent == CardResponded) {
            CardResponseStruct response = data.value<CardResponseStruct>();
            if (!response.m_card->isRed() && !response.m_card->isBlack() && !response.m_card->isKindOf("SkillCard") && response.m_isUse)
                invoke = true;
        }
        if (invoke) {
            QList<SkillInvokeDetail> d;
            foreach (ServerPlayer *p, room->getAllPlayers()) {
                if (p->hasSkill(this) && p->getHandcardNum() < 3)
                    d << SkillInvokeDetail(this, p, p);
            }
            return d;
        }
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        invoke->invoker->drawCards(1);
        return false;
    }
};

TH07Package::TH07Package()
    : Package("th07")
{
    General *yuyuko = new General(this, "yuyuko$", "yym", 4, false);
    yuyuko->addSkill(new Sidie);
    yuyuko->addSkill(new Huaxu);
    yuyuko->addSkill(new Moran);

    General *yukari = new General(this, "yukari", "yym", 4, false);
    yukari->addSkill(new Shenyin);
    yukari->addSkill(new Xijian);

    General *ran = new General(this, "ran", "yym", 3, false);
    ran->addSkill(new Shihui);
    ran->addSkill(new Huanzang);
    ran->addSkill(new HuanzangEffect);
    related_skills.insertMulti("huanzang", "#huanzang");

    General *youmu = new General(this, "youmu", "yym", 4, false);
    youmu->addSkill(new Shuangren);
    youmu->addSkill(new Zhanwang);

    General *prismriver = new General(this, "prismriver", "yym", 3, false);
    prismriver->addSkill(new Xiezou);
    prismriver->addSkill(new Hesheng);

    General *alice = new General(this, "alice", "yym", 4, false);
    alice->addSkill(new Zhanzhen);
    alice->addSkill(new Renou);

    General *chen = new General(this, "chen", "yym", 3, false);
    chen->addSkill(new Qimen);
    chen->addSkill(new Dunjia);
    chen->addSkill(new QimenDistance);
    chen->addSkill(new QimenProhibitAI);
    related_skills.insertMulti("qimen", "#qimen-dist");
    related_skills.insertMulti("qimen", "#qimen-prohibit");

    General *letty = new General(this, "letty", "yym", 4);
    letty->addSkill(new Jiyi);
    letty->addSkill(new Chunmian);

    General *lilywhite = new General(this, "lilywhite", "yym", 3);
    lilywhite->addSkill(new Baochun);
    lilywhite->addSkill(new Chunyi);

    General *shanghai = new General(this, "shanghai", "yym", 3);
    shanghai->addSkill(new Zhancao);
    shanghai->addSkill(new Mocao);

    General *yukari_sp = new General(this, "yukari_sp", "yym", 3);
    yukari_sp->addSkill(new Jingjie);
    yukari_sp->addSkill(new Sisheng);
    yukari_sp->addSkill(new Jingdong);

    General *yuyuko_sp = new General(this, "yuyuko_sp", "yym", 3);
    yuyuko_sp->addSkill(new Youqu);
    yuyuko_sp->addSkill(new Wangwu);

    General *ran_sp = new General(this, "ran_sp", "yym", 4);
    ran_sp->addSkill(new Shizhao);
    ran_sp->addSkill(new Jixiong);
    ran_sp->addSkill(new Jixiong1);
    ran_sp->addSkill(new Jixiong2);
    related_skills.insertMulti("jixiong", "#jixiong1");
    related_skills.insertMulti("jixiong", "#jixiong2");

    General *youki = new General(this, "youki", "yym", 4, true);
    youki->addSkill(new Shoushu);
    youki->addSkill(new Yujian);

    General *leira = new General(this, "leira", "yym", 3);
    leira->addSkill(new Huayin);
    leira->addSkill(new Huanling);

    addMetaObject<XijianCard>();
    addMetaObject<ShihuiCard>();
    addMetaObject<QimenCard>();
    addMetaObject<MocaoCard>();
    addMetaObject<YujianCard>();
    addMetaObject<HuayinCard>();

    skills << new ZhaoliaoVS << new ShihuiVS;
}

ADD_PACKAGE(TH07)
