#include "hegemonyGeneral.h"
#include "client.h"
#include "engine.h"
#include "general.h"
#include "maneuvering.h"
#include "skill.h"
#include "standard.h"
#include "th10.h"
#include "hegemonyCard.h"
//#include "th08.h"


class GameRule_AskForGeneralShowHead : public TriggerSkill
{
public:
    GameRule_AskForGeneralShowHead()
        : TriggerSkill("GameRule_AskForGeneralShowHead")
    {
        events << EventPhaseStart;
        global = true;
    }

    bool effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        invoke->invoker->showGeneral(true, true);
        return false;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        ServerPlayer *player = data.value<ServerPlayer *>();
        if (player != NULL && player->getPhase() == Player::Start && !player->hasShownGeneral() && player->disableShow(true).isEmpty())
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player);
        return QList<SkillInvokeDetail>();
    }
};

class GameRule_AskForGeneralShowDeputy : public TriggerSkill
{
public:
    GameRule_AskForGeneralShowDeputy()
        : TriggerSkill("GameRule_AskForGeneralShowDeputy")
    {
        events << EventPhaseStart;
        global = true;
    }

    bool effect(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        invoke->invoker->showGeneral(false, true);
        return false;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        ServerPlayer *player = data.value<ServerPlayer *>();
        if (ServerInfo.Enable2ndGeneral && player != NULL && player->getPhase() == Player::Start && !player->hasShownGeneral2() && player->disableShow(false).isEmpty())
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player);
        return QList<SkillInvokeDetail>();
    }
};

class GameRule_AskForArraySummon : public TriggerSkill
{
public:
    GameRule_AskForArraySummon() : TriggerSkill("GameRule_AskForArraySummon")
    {
        events << EventPhaseStart;
        global = true;
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        invoke->invoker->gainMark("@nima");
        foreach(const Skill *skill, invoke->invoker->getVisibleSkillList()) {
            if (!skill->inherits("BattleArraySkill")) continue;
            const BattleArraySkill *baskill = qobject_cast<const BattleArraySkill *>(skill);
            if (!invoke->invoker->askForSkillInvoke(objectName())) return false;
            invoke->invoker->gainMark("@dandan_" + skill->objectName());
            invoke->invoker->showGeneral(invoke->invoker->inHeadSkills(skill->objectName()));
            baskill->summonFriends(invoke->invoker);
            break;
        }
        return false;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
    {
        ServerPlayer *player = data.value<ServerPlayer *>();
        if (player == NULL || player->getPhase() != Player::Start || room->getAlivePlayers().length() < 4)
            return QList<SkillInvokeDetail>();

        foreach(const Skill *skill, player->getVisibleSkillList()) {
            if (!skill->inherits("BattleArraySkill")) continue;
            if (qobject_cast<const BattleArraySkill *>(skill)->getViewAsSkill()->isEnabledAtPlay(player)) {
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, player, player);
            }
                
            
        }
        return QList<SkillInvokeDetail>();
    }
};


NiaoxiangSummon::NiaoxiangSummon()
    : ArraySummonCard("niaoxiang")
{
}

class Niaoxiang : public BattleArraySkill
{
public:
    Niaoxiang() : BattleArraySkill("niaoxiang", "Siege")
    {
        events << TargetSpecified;
        //array_type = "Seige";
    }

    virtual bool canPreshow() const
    {
        return false;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
    {

        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card == NULL || !use.card->isKindOf("Slash"))
            return QList<SkillInvokeDetail>();

        QList<ServerPlayer *> skill_owners = room->findPlayersBySkillName(objectName());
        QList<SkillInvokeDetail> d;
        foreach(ServerPlayer *skill_owner, skill_owners) {
            if ( skill_owner->hasShownSkill(this)) {//!BattleArraySkill::triggerable(event, room, data).isEmpty() &&
                QList<ServerPlayer *> targets;
                foreach(ServerPlayer *to, use.to) {
                    if (use.from->inSiegeRelation(skill_owner, to))
                        targets << to;  //->objectName();
                }

                if (!targets.isEmpty())
                    d << SkillInvokeDetail(this, skill_owner, use.from, targets, true);
                //skill_list.insert(skill_owner, QStringList(objectName() + "->" + targets.join("+")));
            }
        }
        return d;
    }

    //virtual bool cost(TriggerEvent, Room *room, ServerPlayer *skill_target, QVariant &, ServerPlayer *ask_who) const  
    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        if (invoke->owner != NULL && invoke->owner->hasShownSkill(this)) {
            foreach (ServerPlayer *skill_target, invoke->targets)
                room->doBattleArrayAnimate(invoke->owner, skill_target);
            //room->broadcastSkillInvoke(objectName(), invoke->owner);
            return true;
        }
        return false;
    }

    //virtual bool effect(TriggerEvent, Room *room, ServerPlayer *skill_target, QVariant &data, ServerPlayer *ask_who) const
    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        //room->sendCompulsoryTriggerLog(ask_who, objectName(), true);
        //CardUseStruct use = data.value<CardUseStruct>();
        foreach(ServerPlayer *skill_target, invoke->targets)
            room->loseHp(skill_target);
        /*
        int x = use.to.indexOf(skill_target);
        QVariantList jink_list = use.from->tag["Jink_" + use.card->toString()].toList();
        if (jink_list.at(x).toInt() == 1)
            jink_list[x] = 2;
        use.from->tag["Jink_" + use.card->toString()] = jink_list;
        */
        return false;
    }
};



TuizhiHegemonyCard::TuizhiHegemonyCard()
{
    m_skillName = "tuizhi_hegemony";
}

bool TuizhiHegemonyCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    //return targets.isEmpty();
    return to_select->hasShownOneGeneral() && targets.isEmpty();
}

void TuizhiHegemonyCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();
    ServerPlayer *target = effect.to;
    QStringList select;
    if (target->hasShownGeneral())
        select << "head";
    if (target->getGeneral2() && target->hasShownGeneral2())
        select << "deputy";
    if (select.isEmpty())
        return;

    QString choice = room->askForChoice(target, objectName(), select.join("+"));
    bool ishead = (choice == "head");
    target->hideGeneral(ishead); //(ishead, true);

    //QString flag = (choice == "head") ? "h" : "d";
    //room->setPlayerDisableShow(target, flag, "huoshui");
}


class TuizhiHegemony : public ZeroCardViewAsSkill
{
public:
    TuizhiHegemony()
        : ZeroCardViewAsSkill("tuizhi_hegemony")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return true;//!player->hasUsed("BanyueHegemonyCard");
    }

    virtual const Card *viewAs() const
    {
        return new TuizhiHegemonyCard;
    }
};




//********  SPRING   **********

class LizhiHegemony : public TriggerSkill
{
public:
    LizhiHegemony()
        : TriggerSkill("lizhi_hegemony")
    {
        events << CardFinished << DamageDone;
    }

    void record(TriggerEvent triggerEvent, Room *room, QVariant &data) const
    {
        if (triggerEvent == DamageDone) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from && damage.card && damage.card->isKindOf("Slash"))
                room->setCardFlag(damage.card, "lizhiDamage");
        }

    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent event, const Room *room, const QVariant &data) const
    {
        if (event == CardFinished) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (!use.card->isKindOf("Slash") || use.card->hasFlag("lizhiDamage") || !use.from || use.from->isDead() || !use.from->hasSkill(this))
                return QList<SkillInvokeDetail>();

            QList<int> ids;
            if (use.card->isVirtualCard())
                ids = use.card->getSubcards();
            else
                ids << use.card->getEffectiveId();

            if (ids.isEmpty())
                return QList<SkillInvokeDetail>();
            foreach(int id, ids) {
                if (room->getCardPlace(id) != Player::DiscardPile) //place???
                    return QList<SkillInvokeDetail>();
            }

            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, use.from, use.from);
        }

        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        QList<ServerPlayer *> targets;
        foreach(ServerPlayer *p, room->getAlivePlayers()) {
            if (invoke->invoker->isFriendWith(p, true))
                targets << p;
        }

        ServerPlayer *target = room->askForPlayerChosen(invoke->invoker, targets, objectName(), "@lizhi", true, true);
        if (target)
            invoke->targets << target;
        return target != NULL;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        invoke->targets.first()->obtainCard(use.card);
        return false;
    }
};




class JingxiaHegemony : public MasochismSkill
{
public:
    JingxiaHegemony()
        : MasochismSkill("jingxia_hegemony")
    {
    }

    QList<SkillInvokeDetail> triggerable(const Room *room, const DamageStruct &damage) const
    {
        bool invoke = false;
        do {
            if (damage.to->isAlive() && damage.to->hasSkill(this)) {
                if (damage.from != NULL && damage.to->canDiscard(damage.from, "hes")) {
                    invoke = true;
                    break;
                }
                bool flag = false;
                foreach(ServerPlayer *p, room->getAllPlayers()) {
                    if (damage.to->canDiscard(p, "ej")) {
                        flag = true;
                        break;
                    }
                }
                if (flag) {
                    invoke = true;
                    break;
                }
            }
        } while (false);

        if (invoke) {
            QList<SkillInvokeDetail> d;
            for (int i = 0; i < damage.damage; ++i) {
                d << SkillInvokeDetail(this, damage.to, damage.to);
            }
            return d;
        }

        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        QStringList select;
        if (damage.from != NULL && damage.to->canDiscard(damage.from, "hes"))
            select << "discard";

        QList<ServerPlayer *> fieldcard;
        foreach(ServerPlayer *p, room->getAllPlayers()) {
            if (damage.to->canDiscard(p, "ej")) {
                select << "discardfield";
                break;
            }
        }
        select.prepend("dismiss");

        ServerPlayer *player = damage.to;

        player->tag["jingxia"] = QVariant::fromValue(damage);
        QString choice = room->askForChoice(player, objectName(), select.join("+"), QVariant::fromValue(damage));
        player->tag.remove("jingxia");
        if (choice == "dismiss")
            return false;

        invoke->tag["jingxia"] = choice;
        return true;
    }

    void onDamaged(Room *room, QSharedPointer<SkillInvokeDetail> invoke, const DamageStruct &damage) const
    {
        QList<ServerPlayer *> fieldcard;
        foreach(ServerPlayer *p, room->getAllPlayers()) {
            if (damage.to->canDiscard(p, "ej"))
                fieldcard << p;
        }
        ServerPlayer *player = damage.to;

        QString choice = invoke->tag.value("jingxia").toString();

        room->touhouLogmessage("#InvokeSkill", player, objectName());
        room->notifySkillInvoked(player, objectName());
        if (choice == "discard") {
            for (int i = 0; i < 2; i++) {
                if (player->isDead() || !player->canDiscard(damage.from, "hes"))
                    return;
                if (damage.from == player)
                    room->askForDiscard(player, "jingxia", 1, 1, false, true);
                else {
                    int card_id = room->askForCardChosen(player, damage.from, "hes", "jingxia", false, Card::MethodDiscard);
                    room->throwCard(card_id, damage.from, player);
                }
            }
        }
        else if (choice == "discardfield") {
            ServerPlayer *player1 = room->askForPlayerChosen(player, fieldcard, "jingxia", "@jingxia-discardfield");
            int card1 = room->askForCardChosen(player, player1, "ej", objectName(), false, Card::MethodDiscard);
            room->throwCard(card1, player1, player);
        }
    }
};


QingtingHegemonyCard::QingtingHegemonyCard()
{
    target_fixed = true;
    m_skillName = "qingting_hegemony";
}

void QingtingHegemonyCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    foreach(ServerPlayer *p, room->getOtherPlayers(source)) {
        if (p->isKongcheng())
            continue;
        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, source->objectName(), p->objectName());
    }
    foreach(ServerPlayer *p, room->getOtherPlayers(source)) {
        if (p->isKongcheng())
            continue;
        const Card *card;
        if (source->getMark("shengge") > 0 || p->getHandcardNum() == 1)
            card = new DummyCard(QList<int>() << room->askForCardChosen(source, p, "hs", "qingting"));
        else {
            p->tag["qingting_give"] = QVariant::fromValue(source);
            card = room->askForExchange(p, "qingting_give", 1, 1, false, "qingtingGive:" + source->objectName());
            p->tag.remove("qingting_give");
        }
        DELETE_OVER_SCOPE(const Card, card)

            source->obtainCard(card, false);
        room->setPlayerMark(p, "@qingting", 1);
    }

    //get delay
    if (source->isOnline())
        room->getThread()->delay(2000);

    foreach(ServerPlayer *p, room->getOtherPlayers(source)) {
        if (p->getMark("@qingting") == 0)
            continue;
        room->setPlayerMark(p, "@qingting", 0);
        if (source->isKongcheng())
            continue;

        source->tag["qingting_return"] = QVariant::fromValue(p);
        const Card *card = room->askForExchange(source, "qingting", 1, 1, false, "qingtingReturn:" + p->objectName());
        DELETE_OVER_SCOPE(const Card, card)
            source->tag.remove("qingting_return");
        p->obtainCard(card, false);
    }
}


class QingtingHegemony : public ZeroCardViewAsSkill
{
public:
    QingtingHegemony()
        : ZeroCardViewAsSkill("qingting_hegemony")
    {
    }
    static bool checkQingting(const Player *player)
    {
        foreach(const Player *p, player->getAliveSiblings()) {
            if (!p->isKongcheng())
                return true;
        }
        return false;
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("QingtingHegemonyCard") && checkQingting(player);
    }

    virtual const Card *viewAs() const
    {
        return new QingtingHegemonyCard;
    }
};


class ShezhengHegemony : public AttackRangeSkill
{
public:
    ShezhengHegemony()
        : AttackRangeSkill("shezheng_hegemony")
    {
        relate_to_place = "deputy";
    }

    virtual int getExtra(const Player *player, bool) const
    {
        if (player->hasSkill(objectName()) && player->hasShownSkill(objectName()) && !player->getWeapon())
            return 1;
        return 0;
    }
};


class ShezhengViewHas : public ViewHasSkill

{

public:

    ShezhengViewHas() : ViewHasSkill("#shezheng_hegemony")

    {
        
    }

    virtual bool ViewHas(const Player *player, const QString &skill_name, const QString &flag) const
    {
        if (flag == "weapon" && skill_name == "DoubleSwordHegemony" && player->isAlive() && player->hasSkill("shezheng_hegemony") && !player->getWeapon())         
            return true;

        return false;

    }

};


class ChilingHegemony : public TriggerSkill
{
public:
    ChilingHegemony()
        : TriggerSkill("chiling_hegemony")
    {
        events << CardsMoveOneTime;
        relate_to_place = "head";
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        ServerPlayer *miko = qobject_cast<ServerPlayer *>(move.from);
        if (miko != NULL && miko->isAlive() && miko->hasSkill(objectName()) //&& move.from_places.contains(Player::PlaceHand)
            && (move.to_place == Player::PlaceHand && move.to && move.to != miko))
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, miko, miko);
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        ServerPlayer *target = qobject_cast<ServerPlayer *>(move.to);

        room->askForUseCard(target, IntList2StringList(move.card_ids).join("#"), "@chiling:" + invoke->invoker->objectName(), -1, Card::MethodUse, false);
        return false;
    }
};



class FenleiHegemony : public TriggerSkill
{
public:
    FenleiHegemony()
        : TriggerSkill("fenlei_hegemony")
    {
        events << QuitDying;
    }



    QList<SkillInvokeDetail> triggerable(TriggerEvent triggerEvent, const Room *, const QVariant &data) const
    {
        ServerPlayer *who = data.value<DyingStruct>().who;
        if (who->isAlive() && who->hasSkill(this))
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, who, who);
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        ServerPlayer *tokizo = invoke->invoker;
        ServerPlayer *target = room->askForPlayerChosen(tokizo, room->getOtherPlayers(tokizo), objectName(), "@fenlei", true, true);
        if (target)
            invoke->targets << target;
        return target != NULL;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail>invoke, QVariant &) const
    {

        room->damage(DamageStruct(objectName(), NULL, invoke->targets.first(), 1, DamageStruct::Thunder));
        return false;
    }
};


//********  SUMMER   **********


class SkltKexueHegemony : public TriggerSkill
{
public:
    SkltKexueHegemony()
        : TriggerSkill("skltkexue_hegemony")
    {
        events  << EventAcquireSkill << EventLoseSkill << Death << Debut << Revive << GeneralShown << Dying; //<< GameStart
    }

    void record(TriggerEvent e, Room *room, QVariant &) const
    {
        if (e == Dying)
            return;

        static QString attachName = "skltkexue_attach"; // need rewrite vs skill
        QList<ServerPlayer *> sklts;
        foreach(ServerPlayer *p, room->getAllPlayers()) {
            if (p->hasSkill(this, true) && p->hasShownSkill(this))
                sklts << p;
        }

        if (sklts.length() > 1) {
            foreach(ServerPlayer *p, room->getAllPlayers()) {
                if (!p->hasSkill(attachName, true))
                    room->attachSkillToPlayer(p, attachName);
            }
        }
        else if (sklts.length() == 1) {
            foreach(ServerPlayer *p, room->getAllPlayers()) {
                if (p->hasSkill(this, true) && p->hasSkill(attachName, true))
                    room->detachSkillFromPlayer(p, attachName, true);
                else if (!p->hasSkill(this, true) && !p->hasSkill(attachName, true))
                    room->attachSkillToPlayer(p, attachName);
            }
        }
        else { // the case that sklts is empty
            foreach(ServerPlayer *p, room->getAllPlayers()) {
                if (p->hasSkill(attachName, true))
                    room->detachSkillFromPlayer(p, attachName, true);
            }
        }
    }


    QList<SkillInvokeDetail> triggerable(TriggerEvent triggerEvent, const Room *, const QVariant &data) const
    {
        if (triggerEvent == Dying) {
            DyingStruct dying = data.value<DyingStruct>();
            if (dying.who && dying.who->isAlive() && dying.who->hasSkill(this) && !dying.who->hasShownSkill(this))
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, dying.who, dying.who);
            return QList<SkillInvokeDetail>();
        }
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        if (invoke->invoker->askForSkillInvoke(this, data))
            invoke->invoker->showHiddenSkill(objectName());
        //invoke->invoker->showGeneral(invoke->invoker->inHeadSkills(objectName()))
        return false;
    }
};


class BeishuiHegemonyVS : public ViewAsSkill
{
public:
    BeishuiHegemonyVS()
        : ViewAsSkill("beishui_hegemony")
    {
        response_or_use = true;
    }

    static QStringList responsePatterns()
    {
        QString pattern = Sanguosha->currentRoomState()->getCurrentCardUsePattern();

        Card::HandlingMethod method = Card::MethodUse;
        QList<const Card *> cards = Sanguosha->findChildren<const Card *>();
        const Skill *skill = Sanguosha->getSkill("beishui_hegemony");

        QStringList checkedPatterns;
        foreach(const Card *card, cards) {
            if ((card->isKindOf("BasicCard")) && !ServerInfo.Extensions.contains("!" + card->getPackage())) {
                QString name = card->objectName();
                if (!checkedPatterns.contains(name) && skill->matchAvaliablePattern(name, pattern) && !Self->isCardLimited(card, method))
                    checkedPatterns << name;
            }
        }

        return checkedPatterns;
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        if (player->getMark("beishui") > 0)
            return false;
        if (Slash::IsAvailable(player) || Analeptic::IsAvailable(player))
            return true;
        Card *card = Sanguosha->cloneCard("peach", Card::NoSuit, 0);
        DELETE_OVER_SCOPE(Card, card)
            Card *card1 = Sanguosha->cloneCard("super_peach", Card::NoSuit, 0);
        DELETE_OVER_SCOPE(Card, card1)
            return card->isAvailable(player) || card1->isAvailable(player);
    }

    virtual bool isEnabledAtResponse(const Player *player, const QString &) const
    {
        if (Sanguosha->currentRoomState()->getCurrentCardUseReason() == CardUseStruct::CARD_USE_REASON_RESPONSE)
            return false;
        if (player->getMark("beishui") > 0)
            return false;

        //check main phase
        if (player->isCurrent()) {
            if (!player->isInMainPhase())
                return false;
        }
        else {
            foreach(const Player *p, player->getSiblings()) {
                if (p->isCurrent()) {
                    if (!p->isInMainPhase())
                        return false;
                    break;
                }
            }
        }

        QStringList checkedPatterns = responsePatterns();
        if (checkedPatterns.contains("peach") && checkedPatterns.length() == 1 && player->getMark("Global_PreventPeach") > 0)
            return false;

        return !checkedPatterns.isEmpty();
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *) const
    {
        //int num = qMax(1, Self->getHp());
        int roles = 1;
        if (Self->getRole() != "careerist") {
            foreach(const Player *p, Self->getAliveSiblings()) {
                if (p->getRole() == Self->getRole())
                    roles++;
            }
        }
        int num = qMax(roles, Self->getHp());
        return selected.length() < num;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        //int num = qMax(1, Self->getHp());
        int roles = 1;
        if (Self->getRole() != "careerist") {
            foreach(const Player *p, Self->getAliveSiblings()) {
                if (p->getRole() == Self->getRole())
                    roles++;
            }
        }

        int num = qMax(roles, Self->getHp());
        if (cards.length() != num)
            return NULL;

        QString name = Self->tag.value("beishui_hegemony", QString()).toString();
        if (name != NULL) {
            Card *card = Sanguosha->cloneCard(name);
            card->setSkillName(objectName());
            card->addSubcards(cards);
            return card;
        }
        else
            return NULL;
    }
};


class BeishuiHegemony : public TriggerSkill
{
public:
    BeishuiHegemony()
        : TriggerSkill("beishui_hegemony")
    {
        events << EventPhaseChanging << PreCardUsed << CardResponded;
        view_as_skill = new BeishuiHegemonyVS;
    }

    virtual QDialog *getDialog() const
    {
        return QijiDialog::getInstance("beishui_hegemony", true, false);
    }

    void record(TriggerEvent e, Room *room, QVariant &data) const
    {
        if (e == EventPhaseChanging) {
            foreach(ServerPlayer *p, room->getAlivePlayers()) {
                if (p->getMark("beishui") > 0)
                    room->setPlayerMark(p, "beishui", 0);
            }
        }
        //record for ai, since AI prefer use a specific card,  but not the SkillCard QijiCard.
        if (e == PreCardUsed) {
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->getSkillName() == objectName())
                room->setPlayerMark(use.from, "beishui", 1);
        }
        if (e == CardResponded) {
            CardResponseStruct response = data.value<CardResponseStruct>();
            if (response.m_from && response.m_isUse && !response.m_isProvision && response.m_card && response.m_card->getSkillName() == objectName())
                room->setPlayerMark(response.m_from, "beishui", 1);
        }
    }
};



/*
class YonghengHegemony : public TriggerSkill
{
public:
    YonghengHegemony()
        : TriggerSkill("yongheng_hegemony")
    {
        events << EventPhaseChanging << CardsMoveOneTime;
        frequency = Compulsory;
    }

    static void adjustHandcardNum(ServerPlayer *player, QString reason)
    {
        int card_num = player->getMaxHp(); //qMax(player->getHp(), 1);
        Room *room = player->getRoom();
        int hc_num = player->getHandcardNum();
        if (card_num != hc_num) {
            room->touhouLogmessage("#TriggerSkill", player, reason);
            room->notifySkillInvoked(player, reason);
        }
        if (card_num > hc_num)
            player->drawCards(card_num - hc_num, reason);
        else if (card_num < hc_num)
            room->askForDiscard(player, reason, hc_num - card_num, hc_num - card_num);
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent triggerEvent, const Room *, const QVariant &data) const
    {
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::Discard && change.player->hasSkill(objectName()))
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, change.player, change.player, NULL, true);
        } else if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            QList<ServerPlayer *> kaguyas;
            ServerPlayer *kaguya1 = qobject_cast<ServerPlayer *>(move.from);
            ServerPlayer *kaguya2 = qobject_cast<ServerPlayer *>(move.to);

            if (kaguya1 && kaguya1->isAlive() && kaguya1->hasSkill(this) && move.from_places.contains(Player::PlaceHand) && kaguya1->getHandcardNum() != kaguya1->getMaxHp()
                && kaguya1->getPhase() == Player::NotActive)
                kaguyas << kaguya1;
            if (kaguya2 && kaguya2->isAlive() && kaguya2->hasSkill(this) && move.to_place == Player::PlaceHand && kaguya2->getHandcardNum() != kaguya2->getMaxHp()
                && kaguya2->getPhase() == Player::NotActive)
                kaguyas << kaguya2;
            if (kaguyas.length() > 1)
                std::sort(kaguyas.begin(), kaguyas.end(), ServerPlayer::CompareByActionOrder);
            if (!kaguyas.isEmpty()) {
                QList<SkillInvokeDetail> d;
                foreach (ServerPlayer *p, kaguyas)
                    d << SkillInvokeDetail(this, p, p, NULL, true);
                return d;
            }
        }
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent triggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        if (triggerEvent == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            if (change.to == Player::Discard) {
                room->touhouLogmessage("#TriggerSkill", invoke->invoker, objectName());
                room->notifySkillInvoked(invoke->invoker, objectName());
                invoke->invoker->skip(change.to);
                adjustHandcardNum(invoke->invoker, objectName());
            }
        } else
            adjustHandcardNum(invoke->invoker, objectName());
        return false;
    }
};*/

class XuyuHegemony : public TriggerSkill
{
public:
    XuyuHegemony()
        : TriggerSkill("xuyu_hegemony")
    {
        events << CardsMoveOneTime;
        frequency = Compulsory;
        relate_to_place = "head";
    }


    QList<SkillInvokeDetail> triggerable(TriggerEvent triggerEvent, const Room *, const QVariant &data) const
    {
        if (triggerEvent == CardsMoveOneTime) {
            CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
            ServerPlayer *kaguya = qobject_cast<ServerPlayer *>(move.from);

            if (kaguya && kaguya->isAlive() && kaguya->hasSkill(this) && !kaguya->hasFlag("xuyu_invoked") 
                &&  move.from_places.contains(Player::PlaceHand) && kaguya->isKongcheng())
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, kaguya, kaguya, NULL, true);
        }
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        room->setPlayerFlag(invoke->invoker, "xuyu_invoked");
        invoke->invoker->removeGeneral(!invoke->invoker->inHeadSkills(objectName()));
        QString skillname = invoke->invoker->inHeadSkills(objectName()) ? "yongheng" : "yongheng!";
        room->handleAcquireDetachSkills(invoke->invoker, skillname);
        //room->acquireSkill(invoke->invoker, skillname);
        return false;
    }
};


class YaoshiHegemony : public TriggerSkill
{
public:
    YaoshiHegemony()
    : TriggerSkill("yaoshi_hegemony")
    {
    events << DamageInflicted;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from && damage.card  &&  damage.from->isAlive() && damage.from->hasSkill(this)) //&& damage.card->isKindOf("Slash")
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.from, damage.from, NULL, false, damage.to);

        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        room->touhouLogmessage("#yaoshi_log", invoke->targets.first(), objectName(), QList<ServerPlayer *>(), QString::number(damage.damage));

        RecoverStruct recover;
        room->recover(invoke->targets.first(), recover);

        return true;
    }
};



class XushiHegemony : public TriggerSkill
{
public:
    XushiHegemony()
        : TriggerSkill("xushi_hegemony")
    {
        events << TargetConfirming;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *room, const QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->getTypeId() == Card::TypeSkill || use.to.length() < 2)
            return QList<SkillInvokeDetail>();

        QList<SkillInvokeDetail> d;
        foreach(ServerPlayer *p, room->findPlayersBySkillName(objectName()))
            d << SkillInvokeDetail(this, p, p);
        return d;
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        QString prompt = "@xushi_hegemony_targetchosen:" + use.card->objectName();
        invoke->invoker->tag["xushi_hegemony_use"] = data;
        ServerPlayer *target = room->askForPlayerChosen(invoke->invoker, use.to, objectName(), prompt, true, true);
        if (target)
            invoke->targets << target;
        return target != NULL;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();

        ServerPlayer *target = invoke->targets.first();
        use.to.removeAll(target);
        data = QVariant::fromValue(use);

        LogMessage log;
        log.type = "#XushiHegemonySkillAvoid";
        log.from = target;
        log.arg = objectName();
        log.arg2 = use.card->objectName();
        room->sendLog(log);
        return false;
    }
};


XingyunHegemonyCard::XingyunHegemonyCard()
{
    will_throw = false;
    target_fixed = true;
    handling_method = Card::MethodNone;
    m_skillName = "xingyun_hegemony";
}

void XingyunHegemonyCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &) const
{
    foreach(int id, subcards)
        room->showCard(source, id);
}

class XingyunHegemonyVS : public OneCardViewAsSkill
{
public:
    XingyunHegemonyVS()
        : OneCardViewAsSkill("xingyun_hegemony")
    {
        response_pattern = "@@xingyun_hegemony";
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *to_select) const
    {
        return  selected.isEmpty() && to_select->hasFlag("xingyun");
    }

    virtual const Card *viewAs(const Card *originalCard) const
    {
        if (originalCard) {
            XingyunHegemonyCard *card = new XingyunHegemonyCard;
            card->addSubcard(originalCard);
            return card;
        }
        else
            return NULL;
    }
};


class XingyungHegemony : public TriggerSkill
{
public:
    XingyungHegemony()
        : TriggerSkill("xingyun_hegemony")
    {
        events << CardsMoveOneTime;
        view_as_skill = new XingyunHegemonyVS;
    }

    bool canPreshow() const
    {
        return true;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        if (room->getTag("FirstRound").toBool())
            return QList<SkillInvokeDetail>();

        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        ServerPlayer *tewi = qobject_cast<ServerPlayer *>(move.to);
        if (tewi != NULL && tewi->hasSkill(this) && move.to_place == Player::PlaceHand) {
            foreach(int id, move.card_ids) {
                if (Sanguosha->getCard(id)->getSuit() == Card::Heart && room->getCardPlace(id) == Player::PlaceHand) {
                    ServerPlayer *owner = room->getCardOwner(id);
                    if (owner && owner == tewi)
                        return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, tewi, tewi);
                }
            }
        }
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        ServerPlayer *tewi = invoke->invoker;
        CardsMoveOneTimeStruct move = data.value<CardsMoveOneTimeStruct>();
        foreach(int id, move.card_ids) {
            if (Sanguosha->getCard(id)->getSuit() == Card::Heart && room->getCardPlace(id) == Player::PlaceHand) {
                ServerPlayer *owner = room->getCardOwner(id);
                if (owner && owner == tewi)
                    room->setCardFlag(id, "xingyun");
            }
        }
        invoke->invoker->tag["xingyun_move"] = data;
        const Card *c = room->askForUseCard(tewi, "@@xingyun_hegemony", "@xingyun_hegemony");
        foreach(int id, move.card_ids)
            room->setCardFlag(id, "-xingyun");

        return c != NULL;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        ServerPlayer *player = invoke->invoker;        
        QString choice = "letdraw";
        //AI:askForChoice or askForPlayerChosen use the "xingyun" AI
        if (player->isWounded())
            choice = room->askForChoice(player, "xingyun", "letdraw+recover", data);
        if (choice == "letdraw") {
            ServerPlayer *target = room->askForPlayerChosen(player, room->getAlivePlayers(), "xingyun", "@xingyun-select");
            target->drawCards(1);
        }
        else if (choice == "recover") {
            RecoverStruct recover;
            recover.who = player;
            room->recover(player, recover);
        }
        
        return false;
    }
};



class YueshiHegemony : public TriggerSkill
{
public:
    YueshiHegemony()
        : TriggerSkill("yueshi_hegemony")
    {
        events << PostCardEffected;
        relate_to_place = "head";
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        CardEffectStruct effect = data.value<CardEffectStruct>();
        if (effect.from && effect.to->hasSkill(this) && effect.to->isWounded() && effect.to->isAlive() && effect.card->isNDTrick()) {
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, effect.to, effect.to);
        }

        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        CardEffectStruct effect = data.value<CardEffectStruct>();
        QString prompt = "invoke:" + effect.card->objectName();
        return invoke->invoker->askForSkillInvoke(objectName(), prompt);
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        JudgeStruct judge;
        judge.who = invoke->invoker;
        judge.pattern = ".|red";
        judge.good = true;
        judge.reason = objectName();
        room->judge(judge);

        if (judge.isGood() && !judge.ignore_judge) {
            RecoverStruct recover;
            recover.recover = 1;
            room->recover(invoke->invoker, recover);
        }
        return false;
    }
};


//********  AUTUMN   **********


QiankunHegemony::QiankunHegemony(const QString &owner) : MaxCardsSkill("qiankun_" + owner)
{
}

int QiankunHegemony::getExtra(const Player *target) const
{
    if (target->hasSkill(objectName()) && target->hasShownSkill(objectName()))
        return 2;
    else
        return 0;
}

class ChuanchengHegemony : public TriggerSkill
{
public:
    ChuanchengHegemony()
        : TriggerSkill("chuancheng_hegemony")
    {
        events << Death;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        DeathStruct death = data.value<DeathStruct>();
        if (death.who->hasSkill(objectName())) {
            QList<ServerPlayer *> targets;
            foreach(ServerPlayer *p, room->getOtherPlayers(death.who)) {
                if (death.who->isFriendWith(p))
                    targets << p;
            }
            if (!targets.isEmpty())
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, death.who, death.who, targets);
        }
            
        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        ServerPlayer *target = room->askForPlayerChosen(invoke->invoker, invoke->targets, objectName(), "@chuancheng_hegemony", true, true);
        if (target) {
            invoke->targets.clear();
            invoke->targets << target;
            return true;
        }
        return false;
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        ServerPlayer *target = invoke->targets.first();
        room->handleAcquireDetachSkills(target, "qiankun_suwako");
        room->handleAcquireDetachSkills(target, "chuancheng");
        if (invoke->invoker->getCards("hejs").length() > 0) {
            DummyCard *allcard = new DummyCard;
            allcard->deleteLater();
            allcard->addSubcards(invoke->invoker->getCards("hejs"));
            room->obtainCard(target, allcard, CardMoveReason(CardMoveReason::S_REASON_RECYCLE, target->objectName()), false);
        }
        return false;
    }
};






class DuxinHegemony : public TriggerSkill
{
public:
    DuxinHegemony()
        : TriggerSkill("duxin_hegemony")
    {
        events << TargetConfirmed;
        frequency = Compulsory;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        if (use.card->isKindOf("SkillCard") || use.from == NULL || use.to.length() != 1 || use.from == use.to.first()
            || use.from->hasFlag("Global_ProcessBroken"))
            return QList<SkillInvokeDetail>();

        ServerPlayer *satori = use.to.first();
        if (satori->hasSkill(objectName()) && use.from->getGeneral2() && !use.from->hasShownGeneral2()) {
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, satori, satori, NULL, true, use.from);
        }
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        QStringList list = room->getTag(invoke->targets.first()->objectName()).toStringList();
        //list.removeAt(choice == "showhead" ? 1 : 0);
        list.removeAt(0);//remove head
        foreach(const QString &name, list) {
            LogMessage log;
            log.type = "$KnownBothViewGeneral";
            log.from = invoke->invoker;
            log.to << invoke->targets.first();
            log.arg = name;
            log.arg2 = invoke->targets.first()->getRole();
            room->doNotify(invoke->invoker, QSanProtocol::S_COMMAND_LOG_SKILL, log.toJsonValue());
        }
        JsonArray arg;
        arg << objectName();
        arg << JsonUtils::toJsonArray(list);
        room->doNotify(invoke->invoker, QSanProtocol::S_COMMAND_VIEW_GENERALS, arg);
        return false;
    }
};


class WunianHgemony : public TriggerSkill
{
public:
    WunianHgemony()
        : TriggerSkill("wunian_hegemony")
    {
        events << Predamage << TargetConfirming;
        frequency = Compulsory;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *, const QVariant &data) const
    {
        if (e == Predamage) {
            DamageStruct damage = data.value<DamageStruct>();
            if (damage.from && damage.from->hasSkill(objectName()))
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.from, damage.from, NULL, true);
        }
        else if (e == TargetConfirming) {
            QList<SkillInvokeDetail> d;
            CardUseStruct use = data.value<CardUseStruct>();
            if (use.card->getTypeId() == Card::TypeTrick) {
                foreach(ServerPlayer *p, use.to) {
                    if (p->hasSkill(this) && p->isWounded() && use.from && use.from != p)
                        d << SkillInvokeDetail(this, p, p, NULL, true);
                }
            }
            return d;
        }

        return QList<SkillInvokeDetail>();
    }

    bool cost(TriggerEvent e, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        //for AI
        if (e == TargetConfirming) {
            CardUseStruct use = data.value<CardUseStruct>();
            room->setTag("wunian_hegemony_use", data);
        }
            
        return (invoke->invoker->hasShownSkill(this) || invoke->invoker->askForSkillInvoke(this, data));       
    }
    
    bool effect(TriggerEvent e, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        if (e == Predamage) {
            DamageStruct damage = data.value<DamageStruct>();

            ServerPlayer *target = room->askForPlayerChosen(invoke->invoker, room->getOtherPlayers(invoke->invoker), objectName(), "@wunian_transfer:" + damage.to->objectName(), false, true);
            damage.from = target;
            damage.transfer = true;
            //damage.by_user = false;

            //room->touhouLogmessage("#TriggerSkill", invoke->invoker, "wunian");
            //room->notifySkillInvoked(invoke->invoker, objectName());
            data = QVariant::fromValue(damage);
        }
        else if (e == TargetConfirming) {
            CardUseStruct use = data.value<CardUseStruct>();
            use.to.removeAll(invoke->invoker);
            data = QVariant::fromValue(use);
            LogMessage log;
            log.type = "#SkillAvoid";
            log.from = invoke->invoker;
            log.arg = objectName();
            log.arg2 = use.card->objectName();
            room->notifySkillInvoked(invoke->invoker, objectName());
            room->sendLog(log);
        }
        return false;
    }
};



//********  WINTER   **********
/*
class ShihuiHegemonyVS : public ViewAsSkill
{
public:
    ShihuiHegemonyVS()
        : ViewAsSkill("shihui_hegemonyVS")
    {
        response_pattern = "@@shihui_hegemonyVS";
        response_or_use = true;
    }

    virtual bool viewFilter(const QList<const Card *> &selected, const Card *c) const
    {
        return selected.isEmpty() && c->getTypeId() == Card::TypeEquip;
    }

    virtual const Card *viewAs(const QList<const Card *> &cards) const
    {
        int maxnum = 1;
        if (cards.length() == maxnum) {
            ExNihilo *exnihilo = new ExNihilo(Card::SuitToBeDecided, -1);
            exnihilo->addSubcards(cards);
            exnihilo->setSkillName("_shihui");
            return exnihilo;
        }

        return NULL;
    }
};
*/

/*
class ShihuiHegemony : public TriggerSkill
{
public:
    ShihuiHegemony()
        : TriggerSkill("shihui_hegemony")
    {
        events << Damage <<Damaged << EventPhaseChanging;
    }

    void record(TriggerEvent e, Room *room, QVariant &data) const
    {
        //record times of using card
        if (e == EventPhaseChanging) {
            PhaseChangeStruct change = data.value<PhaseChangeStruct>();
            //if (change.from == Player::Play) {
                foreach(ServerPlayer *p, room->getAllPlayers(true))
                    room->setPlayerFlag(p, "-shihui_used");
            //}
        }
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *room, const QVariant &data) const
    {
        if (e == Damage) {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.from || damage.from->isDead() || damage.card == NULL || !damage.card->isKindOf("Slash"))
                return QList<SkillInvokeDetail>();

            QList<SkillInvokeDetail> d;
            foreach(ServerPlayer *p, room->findPlayersBySkillName(objectName())) {
                if (!p->hasFlag("shihui_used") &&  p->isFriendWith(damage.from, true))
                    d << SkillInvokeDetail(this, p, p, NULL, false, damage.from);
            }
                
            return d;
        }
        if (e == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.to || damage.to->isDead() || damage.card == NULL || !damage.card->isKindOf("Slash"))
                return QList<SkillInvokeDetail>();

            QList<SkillInvokeDetail> d;
            foreach(ServerPlayer *p, room->findPlayersBySkillName(objectName())) {
                if (!p->hasFlag("shihui_used") && p->isFriendWith(damage.to, true))
                    d << SkillInvokeDetail(this, p, p, NULL, false, damage.to);
            }

            return d;
        }
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent e, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        room->setPlayerFlag(invoke->invoker, "shihui_used");
        ServerPlayer *target = invoke->targets.first();
        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, invoke->invoker->objectName(), target->objectName());
        //int maxnum = qMax(target->getEquips().length(), 1);
        room->askForUseCard(target, "@@shihui_hegemonyVS", "shihuiuse_hegemony");
        
            
        return false;
    }
};
*/

class ShihuiHegemony : public TriggerSkill
{
public:
    ShihuiHegemony()
        : TriggerSkill("shihui_hegemony")
    {
        events << Damage << Damaged;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *room, const QVariant &data) const
    {
        if (e == Damage) {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.from || damage.from->isDead() || damage.card == NULL || !damage.card->isKindOf("Slash"))
                return QList<SkillInvokeDetail>();

            AwaitExhaustedHegemony *card = new AwaitExhaustedHegemony(Card::SuitToBeDecided, -1);
            card->setSkillName(objectName());
            card->deleteLater();
            if (damage.from->hasSkill(this) && !damage.from->isCardLimited(card, Card::HandlingMethod::MethodUse))
                return QList<SkillInvokeDetail> () << SkillInvokeDetail(this, damage.from, damage.from);
        }
        else if (e == Damaged) {
            DamageStruct damage = data.value<DamageStruct>();
            if (!damage.to || damage.to->isDead() || damage.card == NULL || !damage.card->isKindOf("Slash"))
                return QList<SkillInvokeDetail>();

            AwaitExhaustedHegemony *card = new AwaitExhaustedHegemony(Card::SuitToBeDecided, -1);
            card->setSkillName(objectName());
            card->deleteLater();

            if (damage.to->hasSkill(this) && !damage.to->isCardLimited(card, Card::HandlingMethod::MethodUse))
                return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.to, damage.to);
        }
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent e, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        AwaitExhaustedHegemony *card = new AwaitExhaustedHegemony(Card::SuitToBeDecided, -1);
        card->setSkillName(objectName());
        room->useCard(CardUseStruct(card, invoke->invoker, NULL), false);

        return false;
    }
};



class DunjiaHegemony : public TriggerSkill
{
public:
    DunjiaHegemony()
        : TriggerSkill("dunjia_hegemony")
    {
        events << Damage << Damaged;
    }


    QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *room, const QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        if (damage.from == NULL || damage.to->isDead() || damage.from->isDead())
            return QList<SkillInvokeDetail>();
        if (damage.from == damage.to || damage.card == NULL || !damage.card->isKindOf("Slash"))
            return QList<SkillInvokeDetail>();
        int num1 = damage.from->getEquips().length();
        int num2 = damage.to->getEquips().length();
        if (num2 == 0 && num1 == 0)
            return QList<SkillInvokeDetail>();
        int diff = qAbs(num1 - num2);

        if (e == Damage &&  damage.from  && damage.from->hasSkill(this) && diff <= damage.from->getLostHp())
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.from, damage.from, NULL, false, damage.to);
        else if (e == Damaged &&  damage.to  && damage.to->hasSkill(this) && diff <= damage.to->getLostHp())
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.to, damage.to, NULL, false, damage.from);
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        ServerPlayer *first = invoke->invoker;
        ServerPlayer *second = invoke->targets.first();
        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, first->objectName(), second->objectName());

        QList<int> equips1, equips2;
        foreach(const Card *equip, first->getEquips())
            equips1.append(equip->getId());
        foreach(const Card *equip, second->getEquips())
            equips2.append(equip->getId());

        QList<CardsMoveStruct> exchangeMove;
        CardsMoveStruct move1(equips1, second, Player::PlaceEquip,
            CardMoveReason(CardMoveReason::S_REASON_SWAP, first->objectName(), second->objectName(), "dunjia_hegemony", QString()));
        CardsMoveStruct move2(equips2, first, Player::PlaceEquip,
            CardMoveReason(CardMoveReason::S_REASON_SWAP, second->objectName(), first->objectName(), "dunjia_hegemony", QString()));
        exchangeMove.push_back(move2);
        exchangeMove.push_back(move1);
        room->moveCardsAtomic(exchangeMove, false);

        
        return false;
    }
};


class ZhancaoHegemony : public TriggerSkill
{
public:
    ZhancaoHegemony()
        : TriggerSkill("zhancao_hegemony")
    {
        events << TargetConfirmed;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *room, const QVariant &data) const
    {
        CardUseStruct use = data.value<CardUseStruct>();
        QList<SkillInvokeDetail> d;
        if (use.card->isKindOf("Slash")) {
            QList<ServerPlayer *> srcs = room->findPlayersBySkillName(objectName());
            foreach(ServerPlayer *p, srcs) {
                if (!p->canDiscard(p, "e"))
                    continue;
                foreach(ServerPlayer *to, use.to) {
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
        QString prompt = "@zhancao_hegemony-discard:" + use.from->objectName() + ":" + invoke->preferredTarget->objectName();
        return room->askForCard(invoke->invoker, ".|.|.|equipped", prompt, data, objectName()) != NULL;
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


MocaoHegemonyCard::MocaoHegemonyCard()
{
    m_skillName = "mocao_hegemony";
}

bool MocaoHegemonyCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    if (!targets.isEmpty() || to_select == Self)
        return false;
    return !to_select->getEquips().isEmpty();
}

void MocaoHegemonyCard::onEffect(const CardEffectStruct &effect) const
{
    Room *room = effect.to->getRoom();

    int card_id = room->askForCardChosen(effect.from, effect.to, "e", "mocao");
    room->obtainCard(effect.from, card_id);
    effect.to->drawCards(qMax(1, effect.to->getLostHp()));
}

class MocaoHegemony : public ZeroCardViewAsSkill
{
public:
    MocaoHegemony()
        : ZeroCardViewAsSkill("mocao_hegemony")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("MocaoHegemonyCard");
    }

    virtual const Card *viewAs() const
    {
        return new MocaoHegemonyCard;
    }
};


class DongjieHegemony : public TriggerSkill
{
public:
    DongjieHegemony()
        : TriggerSkill("dongjie_hegemony")
    {
        events << DamageCaused; //<< EventPhaseChanging
    }


    QList<SkillInvokeDetail> triggerable(TriggerEvent e, const Room *room, const QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        //if (damage.chain || damage.transfer || !damage.by_user)
        //	return QList<SkillInvokeDetail>();
        if (damage.from  && damage.from->hasSkill(this) && damage.card && damage.card->isKindOf("Slash")) //   !damage.from->hasFlag(objectName())
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, damage.from, damage.from, NULL, false, damage.to);

        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        DamageStruct damage = data.value<DamageStruct>();
        room->doAnimate(QSanProtocol::S_ANIMATE_INDICATE, invoke->invoker->objectName(), damage.to->objectName());

        //invoke->invoker->setFlags(objectName());


        if (!room->askForCard(damage.to, ".|.|.|hand", "@dongjie_discard:" + damage.from->objectName(), data, Card::MethodDiscard)) {
            damage.to->drawCards(1);
            damage.to->turnOver();
            return true;
        }
        return false;
    }
};

class BingpoHgemony : public TriggerSkill
{
public:
    BingpoHgemony()
        : TriggerSkill("bingpo_hegemony")
    {
        events << Dying;
        frequency = Compulsory;
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        DyingStruct dying = data.value<DyingStruct>();
        if (dying.who->hasSkill(this) && dying.who->getHp() < dying.who->dyingThreshold() 
            && (dying.damage == NULL || dying.damage->nature != DamageStruct::Fire))
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, dying.who, dying.who, NULL, true);
        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &data) const
    {
        room->touhouLogmessage("#bingpo_hegemony_log", invoke->invoker, objectName(), QList<ServerPlayer *>(), QString::number(1));
        room->notifySkillInvoked(invoke->invoker, objectName());
        RecoverStruct recover;
        room->recover(invoke->invoker, recover);
        return false;
    }
};



class JuxianHegemony : public TriggerSkill
{
public:
    JuxianHegemony()
        : TriggerSkill("juxian_hegemony")
    {
        events << Dying;
        frequency = Limited;
        limit_mark = "@juxian";
    }

    QList<SkillInvokeDetail> triggerable(TriggerEvent, const Room *, const QVariant &data) const
    {
        DyingStruct dying = data.value<DyingStruct>();

        if (dying.who->hasSkill(this) && dying.who->isAlive() && dying.who->getHp() < dying.who->dyingThreshold() && dying.who->getMark("@juxian") > 0)
            return QList<SkillInvokeDetail>() << SkillInvokeDetail(this, dying.who, dying.who);

        return QList<SkillInvokeDetail>();
    }

    bool effect(TriggerEvent, Room *room, QSharedPointer<SkillInvokeDetail> invoke, QVariant &) const
    {
        room->removePlayerMark(invoke->invoker, "@juxian");
        room->doLightbox("$juxianAnimate", 4000);

        QList<int> list = room->getNCards(3);
        CardsMoveStruct move(list, NULL, Player::PlaceTable, CardMoveReason(CardMoveReason::S_REASON_TURNOVER, invoke->invoker->objectName(), objectName(), QString()));
        room->moveCardsAtomic(move, true);

        int s = 0; int h = 0; int c = 0; int d = 0;
        foreach(int id, list) {
            Card *card = Sanguosha->getCard(id);
            if (card->getSuit() == Card::Spade)
                s = 1;
            else if (card->getSuit() == Card::Heart)
                h = 1;
            else if (card->getSuit() == Card::Club)
                c = 1;
            else if (card->getSuit() == Card::Diamond)
                d = 1;
        }

        DummyCard dummy(list);
        invoke->invoker->obtainCard(&dummy);

        RecoverStruct recover;
        recover.recover = (s + h + c + d);
        room->recover(invoke->invoker, recover);

        return false;
    }
};


BanyueHegemonyCard::BanyueHegemonyCard()
{
    m_skillName = "banyue_hegemony";
}

bool BanyueHegemonyCard::targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const
{
    Card *card = Sanguosha->cloneCard("befriend_attacking");
    DELETE_OVER_SCOPE(Card, card)
    if (targets.isEmpty()) {
        return (to_select == Self || to_select->hasShownOneGeneral()) 
            && (!to_select->isCardLimited(card, Card::HandlingMethod::MethodUse));
    }
    else if (targets.length() == 1) {
        const Player *user = targets.first();
        return  to_select->hasShownOneGeneral() && !user->isFriendWith(to_select, (user == Self)) 
            && !user->isProhibited(to_select,card, QList<const Player *>());
    }
    return false;
}

bool BanyueHegemonyCard::targetsFeasible(const QList<const Player *> &targets, const Player *) const
{
    return targets.length() == 2;
}

void BanyueHegemonyCard::onUse(Room *room, const CardUseStruct &card_use) const
{
    CardUseStruct use = card_use;
    QVariant data = QVariant::fromValue(use);
    RoomThread *thread = room->getThread();
    use.from->showHiddenSkill("banyue_hegemony");
    thread->trigger(PreCardUsed, room, data);
    use = data.value<CardUseStruct>();

    ServerPlayer *from = card_use.from;
    ServerPlayer *to1 = card_use.to.at(0);
    ServerPlayer *to2 = card_use.to.at(1);
    QList<ServerPlayer *> logto;
    logto << to1 << to2;
    room->touhouLogmessage("#ChoosePlayerWithSkill", from, "banyue_hegemony", logto, "");
    room->notifySkillInvoked(card_use.from, "banyue_hegemony");

    thread->trigger(CardUsed, room, data);
    use = data.value<CardUseStruct>();
    thread->trigger(CardFinished, room, data);
}


void BanyueHegemonyCard::use(Room *room, ServerPlayer *source, QList<ServerPlayer *> &targets) const // onEffect is better?
{
    
    room->loseHp(source);
    ServerPlayer *to1 = targets.first();
    ServerPlayer *to2 = targets.last();
    Card *card = Sanguosha->cloneCard("befriend_attacking");
    card->setSkillName("_banyue_hegemony");

    CardUseStruct use;
    use.from = to1;
    use.to << to2;
    use.card = card;
    room->useCard(use);
}

class BanyueHegemony : public ZeroCardViewAsSkill
{
public:
    BanyueHegemony()
        : ZeroCardViewAsSkill("banyue_hegemony")
    {
    }

    virtual bool isEnabledAtPlay(const Player *player) const
    {
        return !player->hasUsed("BanyueHegemonyCard");
    }

    virtual const Card *viewAs() const
    {
        return new BanyueHegemonyCard;
    }
};




HegemonyGeneralPackage::HegemonyGeneralPackage()
    : Package("hegemonyGeneral")
{
    



    General *reimu_hegemony = new General(this, "reimu_hegemony", "zhu", 4);
    reimu_hegemony->addSkill(new TuizhiHegemony);
    //reimu_hegemony->addSkill("qixiang");
    //reimu_hegemony->addSkill("fengmo");
    reimu_hegemony->addCompanion("marisa_hegemony");
    reimu_hegemony->addCompanion("yukari_hegemony");
    reimu_hegemony->addCompanion("aya_hegemony");

    General *marisa_hegemony = new General(this, "marisa_hegemony", "zhu", 4);
    marisa_hegemony->addSkill("mofa");
    marisa_hegemony->addCompanion("patchouli_hegemony");
    marisa_hegemony->addCompanion("alice_hegemony");
    marisa_hegemony->addCompanion("nitori_hegemony");
    
    
    

//Spring
    General *byakuren_hegemony = new General(this, "byakuren_hegemony", "wu", 4);
    byakuren_hegemony->addSkill("pudu");
    byakuren_hegemony->addSkill("jiushu");
    byakuren_hegemony->addCompanion("toramaru_hegemony");
    byakuren_hegemony->addCompanion("murasa_hegemony");
    byakuren_hegemony->addCompanion("ichirin_hegemony");

    General *nue_hegemony = new General(this, "nue_hegemony", "wu", 3);
    nue_hegemony->addSkill("weizhi");
    nue_hegemony->addSkill("weizhuang");
    nue_hegemony->addCompanion("mamizou_hegemony");

    General *toramaru_hegemony = new General(this, "toramaru_hegemony", "wu", 4);
    toramaru_hegemony->addSkill("jinghua");
    toramaru_hegemony->addSkill("weiguang");
    toramaru_hegemony->addCompanion("nazrin_hegemony");

    General *murasa_hegemony = new General(this, "murasa_hegemony", "wu", 4);
    murasa_hegemony->addSkill("shuinan");
    murasa_hegemony->addSkill("nihuo");

    General *ichirin_hegemony = new General(this, "ichirin_hegemony", "wu", 4);
    ichirin_hegemony->addSkill(new LizhiHegemony);
    ichirin_hegemony->addSkill("yunshang");

    General *nazrin_hegemony = new General(this, "nazrin_hegemony", "wu", 3);
    nazrin_hegemony->addSkill("xunbao");
    nazrin_hegemony->addSkill("lingbai");

    General *miko_hegemony = new General(this, "miko_hegemony", "wu", 4);
    miko_hegemony->addSkill(new QingtingHegemony);
    miko_hegemony->addSkill(new ChilingHegemony);
    miko_hegemony->addSkill(new ShezhengHegemony);
    miko_hegemony->addSkill(new ShezhengViewHas);
    miko_hegemony->addCompanion("futo_hegemony");
    miko_hegemony->addCompanion("toziko_hegemony");
    miko_hegemony->addCompanion("seiga_hegemony");
    miko_hegemony->setHeadMaxHpAdjustedValue(-1);
    related_skills.insertMulti("shezheng_hegemony", "#shezheng_hegemony");

    General *mamizou_hegemony = new General(this, "mamizou_hegemony", "wu", 4);
    mamizou_hegemony->addSkill("xihua");
    mamizou_hegemony->addSkill("#xihua_clear");

    General *futo_hegemony = new General(this, "futo_hegemony", "wu", 3);
    futo_hegemony->addSkill("shijie");
    futo_hegemony->addSkill("fengshui");
    futo_hegemony->addCompanion("toziko_hegemony");

    General *toziko_hegemony = new General(this, "toziko_hegemony", "wu", 4);
    toziko_hegemony->addSkill("leishi");
    toziko_hegemony->addSkill(new FenleiHegemony);

    General *seiga_hegemony = new General(this, "seiga_hegemony", "wu", 3);
    seiga_hegemony->addSkill("xiefa");
    seiga_hegemony->addSkill("chuanbi");
    seiga_hegemony->addCompanion("yoshika_hegemony");

    General *yoshika_hegemony = new General(this, "yoshika_hegemony", "wu", 4);
    yoshika_hegemony->addSkill("duzhua");
    yoshika_hegemony->addSkill("#duzhuaTargetMod");
    yoshika_hegemony->addSkill("taotie");

    General *kyouko_hegemony = new General(this, "kyouko_hegemony", "wu", 3);
    kyouko_hegemony->addSkill("songjing");
    kyouko_hegemony->addSkill("gongzhen");



    General *kogasa_hegemony = new General(this, "kogasa_hegemony", "wu", 3);
    kogasa_hegemony->addSkill("yiwang");
    kogasa_hegemony->addSkill(new JingxiaHegemony);

    General *kokoro_hegemony = new General(this, "kokoro_hegemony", "wu", 4);
    kokoro_hegemony->addSkill("nengwu");
    kokoro_hegemony->addSkill("#nengwu2");
    kokoro_hegemony->addCompanion("miko_hegemony");


//Summer 

    General *remilia_hegemony = new General(this, "remilia_hegemony", "shu", 3);
    remilia_hegemony->addSkill(new SkltKexueHegemony);
    remilia_hegemony->addSkill("mingyun");
    remilia_hegemony->addCompanion("flandre_hegemony");
    remilia_hegemony->addCompanion("sakuya_hegemony");
    remilia_hegemony->addCompanion("patchouli_hegemony");

    General *flandre_hegemony = new General(this, "flandre_hegemony", "shu", 3);
    flandre_hegemony->addSkill("pohuai");
    flandre_hegemony->addSkill("yuxue");
    flandre_hegemony->addSkill("#yuxue-slash-ndl");
    flandre_hegemony->addSkill("shengyan");
    flandre_hegemony->addCompanion("meirin_hegemony");


    General *sakuya_hegemony = new General(this, "sakuya_hegemony", "shu", 4);
    sakuya_hegemony->addSkill("suoding");
    sakuya_hegemony->addSkill("huisu");
    sakuya_hegemony->addCompanion("meirin_hegemony");

    General *patchouli_hegemony = new General(this, "patchouli_hegemony", "shu", 3);
    patchouli_hegemony->addSkill("bolan");
    patchouli_hegemony->addSkill("hezhou");
    patchouli_hegemony->addCompanion("koakuma_hegemony");

    General *meirin_hegemony = new General(this, "meirin_hegemony", "shu", 4);
    meirin_hegemony->addSkill("taiji");
    meirin_hegemony->addSkill(new BeishuiHegemony);

    General *koakuma_hegemony = new General(this, "koakuma_hegemony", "shu", 3);
    koakuma_hegemony->addSkill("moqi");
    koakuma_hegemony->addSkill("sishu");

    General *kaguya_hegemony = new General(this, "kaguya_hegemony", "shu", 4);
    kaguya_hegemony->addSkill(new XuyuHegemony);
    kaguya_hegemony->addSkill("shenbao");
    kaguya_hegemony->addSkill("#shenbao_distance");
    kaguya_hegemony->addSkill("#shenbao");
    kaguya_hegemony->addSkill("#shenbao_viewhas");
    kaguya_hegemony->addCompanion("eirin_hegemony");
    kaguya_hegemony->addCompanion("mokou_hegemony");

    General *eirin_hegemony = new General(this, "eirin_hegemony", "shu", 4);
    eirin_hegemony->addSkill("ruizhi");
    eirin_hegemony->addSkill(new YaoshiHegemony);
    //eirin_hegemony->addSkill("miyao");
    eirin_hegemony->addCompanion("reisen_hegemony");

    General *mokou_hegemony = new General(this, "mokou_hegemony", "shu", 4);
    mokou_hegemony->addSkill("kaifeng");
    mokou_hegemony->addSkill("fengxiang");
    mokou_hegemony->addCompanion("keine_hegemony");
    mokou_hegemony->addCompanion("keine_sp_hegemony");

    General *reisen_hegemony = new General(this, "reisen_hegemony", "shu", 4);
    reisen_hegemony->addSkill("kuangzao");
    reisen_hegemony->addSkill("huanshi");
    reisen_hegemony->addCompanion("tewi_hegemony");


    General *keine_hegemony = new General(this, "keine_hegemony", "shu", 3);
    keine_hegemony->addSkill(new XushiHegemony);
    keine_hegemony->addSkill("xinyue");
    keine_hegemony->addCompanion("keine_sp_hegemony");

    General *tewi_hegemony = new General(this, "tewi_hegemony", "shu", 3);
    tewi_hegemony->addSkill("buxian");
    tewi_hegemony->addSkill("#buxian");
    tewi_hegemony->addSkill(new XingyungHegemony);

    General *keine_sp_hegemony = new General(this, "keine_sp_hegemony", "shu", 3);
    keine_sp_hegemony->addSkill("chuangshi");
    keine_sp_hegemony->addSkill("wangyue");

    General *toyohime_hegemony = new General(this, "toyohime_hegemony", "shu", 4);
    toyohime_hegemony->addSkill("lianxi");
    toyohime_hegemony->addSkill(new YueshiHegemony);
    toyohime_hegemony->setHeadMaxHpAdjustedValue(-1);
    toyohime_hegemony->addCompanion("yorihime_hegemony");

    General *yorihime_hegemony = new General(this, "yorihime_hegemony", "shu", 4);
    yorihime_hegemony->addSkill("pingyi");
    yorihime_hegemony->addSkill("#pingyi_handle");



//Autumn
    General *kanako_hegemony = new General(this, "kanako_hegemony", "qun", 4);
    kanako_hegemony->addSkill("shende");
    kanako_hegemony->addSkill(new QiankunHegemony("kanako"));
    //kanako_hegemony->addSkill(new Niaoxiang);
    kanako_hegemony->addCompanion("suwako_hegemony");
    kanako_hegemony->addCompanion("sanae_hegemony");

    General *suwako_hegemony = new General(this, "suwako_hegemony", "qun", 3);
    suwako_hegemony->addSkill("bushu");
    suwako_hegemony->addSkill(new QiankunHegemony("suwako"));
    suwako_hegemony->addSkill(new ChuanchengHegemony);
    suwako_hegemony->addCompanion("sanae_hegemony");

    General *sanae_hegemony = new General(this, "sanae_hegemony", "qun", 3);
    sanae_hegemony->addSkill("dfgzmjiyi");
    sanae_hegemony->addSkill("qiji");

    General *aya_hegemony = new General(this, "aya_hegemony", "qun", 3);
    aya_hegemony->addSkill("fengshen");
    aya_hegemony->addSkill("fengsu");
    aya_hegemony->addSkill("#fengsu-effect");
    aya_hegemony->addCompanion("momizi_hegemony");

    General *nitori_hegemony = new General(this, "nitori_hegemony", "qun", 3);
    nitori_hegemony->addSkill("xinshang");
    nitori_hegemony->addSkill("#xinshang_effect");
    nitori_hegemony->addSkill("micai");

    General *hina_hegemony = new General(this, "hina_hegemony", "qun", 3);
    hina_hegemony->addSkill("jie");
    hina_hegemony->addSkill("liuxing");

    General *momizi_hegemony = new General(this, "momizi_hegemony", "qun", 4);
    //momizi_hegemony->addSkill("shouhu");
    //momizi_hegemony->addSkill("shaojie");
    momizi_hegemony->addSkill("buju");

    General *minoriko_hegemony = new General(this, "minoriko_hegemony", "qun", 4);
    minoriko_hegemony->addSkill("fengrang");
    minoriko_hegemony->addSkill("shouhuo");
    minoriko_hegemony->addCompanion("shizuha_hegemony");

    General *shizuha_hegemony = new General(this, "shizuha_hegemony", "qun", 4);
    shizuha_hegemony->addSkill("jiliao");
    shizuha_hegemony->addSkill("zhongyan");

    General *satori_hegemony = new General(this, "satori_hegemony", "qun", 3);
    satori_hegemony->addSkill("xiangqi");
    //satori_hegemony->addSkill("duxin");
    satori_hegemony->addSkill(new DuxinHegemony);
    satori_hegemony->addCompanion("koishi_hegemony");

    General *koishi_hegemony = new General(this, "koishi_hegemony", "qun", 3);
    koishi_hegemony->addSkill("maihuo");
    koishi_hegemony->addSkill(new WunianHgemony);


    General *utsuho_hegemony = new General(this, "utsuho_hegemony", "qun", 4);
    utsuho_hegemony->addSkill("yaoban");
    utsuho_hegemony->addSkill("here");
    utsuho_hegemony->addCompanion("rin_hegemony");

    General *rin_hegemony = new General(this, "rin_hegemony", "qun", 4);
    rin_hegemony->addSkill("yuanling");
    rin_hegemony->addSkill("songzang");

    General *yugi_hegemony = new General(this, "yugi_hegemony", "qun", 4);
    yugi_hegemony->addSkill("guaili");
    yugi_hegemony->addSkill("jiuhao");
    yugi_hegemony->addCompanion("parsee_hegemony");

    General *parsee_hegemony = new General(this, "parsee_hegemony", "qun", 3);
    parsee_hegemony->addSkill("jidu");
    parsee_hegemony->addSkill("gelong");

//Winter
    General *yuyuko_hegemony = new General(this, "yuyuko_hegemony", "wei", 4, false);
    yuyuko_hegemony->addSkill("sidie");
    yuyuko_hegemony->addSkill("huaxu");
    yuyuko_hegemony->addCompanion("yukari_hegemony");
    yuyuko_hegemony->addCompanion("youmu_hegemony");

    General *yukari_hegemony = new General(this, "yukari_hegemony", "wei", 4, false);
    yukari_hegemony->addSkill("shenyin");
    yukari_hegemony->addSkill("xijian");
    yukari_hegemony->addCompanion("ran_hegemony");

    General *ran_hegemony = new General(this, "ran_hegemony", "wei", 3, false);
    ran_hegemony->addSkill(new ShihuiHegemony);
    ran_hegemony->addSkill("huanzang");
    ran_hegemony->addSkill("#huanzang");
    ran_hegemony->addCompanion("chen_hegemony");


    General *youmu_hegemony = new General(this, "youmu_hegemony", "wei", 4, false);
    youmu_hegemony->addSkill("shuangren");
    youmu_hegemony->addSkill("zhanwang");

    General *lunasa_hegemony = new General(this, "lunasa_hegemony", "wei", 3, false);
    lunasa_hegemony->addCompanion("merlin_hegemony");
    lunasa_hegemony->addCompanion("lyrica_hegemony");
    General *merlin_hegemony = new General(this, "merlin_hegemony", "wei", 3, false);
    merlin_hegemony->addCompanion("lyrica_hegemony");
    General *lyrica_hegemony = new General(this, "lyrica_hegemony", "wei", 3, false);
    

        

    General *alice_hegemony = new General(this, "alice_hegemony", "wei", 4, false);
    alice_hegemony->addSkill("zhanzhen");
    alice_hegemony->addSkill("renou");
    alice_hegemony->addCompanion("shanghai_hegemony");

    General *chen_hegemony = new General(this, "chen_hegemony", "wei", 3, false);
    chen_hegemony->addSkill("qimen");
    chen_hegemony->addSkill(new DunjiaHegemony);
    chen_hegemony->addSkill("#qimen-dist");
    chen_hegemony->addSkill("#qimen-prohibit");

    General *letty_hegemony = new General(this, "letty_hegemony", "wei", 4);
    letty_hegemony->addSkill("jiyi");
    letty_hegemony->addSkill("chunmian");

    General *lilywhite_hegemony = new General(this, "lilywhite_hegemony", "wei", 3);
    lilywhite_hegemony->addSkill("baochun");
    lilywhite_hegemony->addSkill("chunyi");

    General *shanghai_hegemony = new General(this, "shanghai_hegemony", "wei", 3);
    shanghai_hegemony->addSkill(new ZhancaoHegemony);
    shanghai_hegemony->addSkill(new MocaoHegemony);

    General *youki_hegemony = new General(this, "youki_hegemony", "wei", 4, true);
    youki_hegemony->addSkill("shoushu");
    youki_hegemony->addSkill("yujian");
    youki_hegemony->addCompanion("youmu_hegemony");

    General *cirno_hegemony = new General(this, "cirno_hegemony", "wei", 3);
    cirno_hegemony->addSkill(new DongjieHegemony);
    cirno_hegemony->addSkill(new BingpoHgemony);
    cirno_hegemony->addCompanion("daiyousei_hegemony");

    General *daiyousei_hegemony = new General(this, "daiyousei_hegemony", "wei", 3);
    daiyousei_hegemony->addSkill(new JuxianHegemony);
    daiyousei_hegemony->addSkill(new BanyueHegemony);


    addMetaObject<TuizhiHegemonyCard>();
    addMetaObject<NiaoxiangSummon>();

    addMetaObject<QingtingHegemonyCard>();

    addMetaObject<XingyunHegemonyCard>();
    
    
    addMetaObject<MocaoHegemonyCard>();

    addMetaObject<BanyueHegemonyCard>();

    skills <<  new GameRule_AskForGeneralShowHead << new GameRule_AskForGeneralShowDeputy << new GameRule_AskForArraySummon ; //<< new ShihuiHegemonyVS
}

ADD_PACKAGE(HegemonyGeneral)
