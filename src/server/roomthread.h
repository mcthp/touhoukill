#ifndef _ROOM_THREAD_H
#define _ROOM_THREAD_H

#include <QSemaphore>
#include <QThread>
#include <QVariant>

#include "structs.h"

class GameRule;

struct LogMessage
{
    LogMessage();
    QString toString() const;
    QVariant toJsonValue() const;

    QString type;
    ServerPlayer *from;
    QList<ServerPlayer *> to;
    QString card_str;
    QString arg;
    QString arg2;
};

class RoomThread : public QThread
{
    Q_OBJECT

public:
    explicit RoomThread(Room *room);
    void constructTriggerTable();
    bool trigger(TriggerEvent triggerEvent, Room *room);

    void getSkillAndSort(TriggerEvent triggerEvent, Room *room, QList<QSharedPointer<SkillInvokeDetail> > &detailsList, const QList<QSharedPointer<SkillInvokeDetail> > &triggered,
                         const QVariant &data);
    // player is deleted. a lot of things is able to put in data. make a struct for every triggerevent isn't absolutely unreasonable.
    bool trigger(TriggerEvent triggerEvent, Room *room, QVariant &data);

    // refactor proposal: Temporarily use fixme_cast
    void getTriggerAndSort(TriggerEvent e, QList<QSharedPointer<RefactorProposal::TriggerDetail> > &detailsList,
                           const QList<QSharedPointer<RefactorProposal::TriggerDetail> > &triggered, const QVariant &data);
    bool triggerRefactorProposal(TriggerEvent e, QVariant &data);

    void addPlayerSkills(ServerPlayer *player, bool invoke_game_start = false);

    void addTriggerSkill(const TriggerSkill *skill);
    void delay(long msecs = -1);
    ServerPlayer *find3v3Next(QList<ServerPlayer *> &first, QList<ServerPlayer *> &second);
    void run3v3(QList<ServerPlayer *> &first, QList<ServerPlayer *> &second, GameRule *game_rule, ServerPlayer *current);
    void actionHulaoPass(ServerPlayer *uuz, QList<ServerPlayer *> league, GameRule *game_rule);
    ServerPlayer *findHulaoPassNext(ServerPlayer *uuz, QList<ServerPlayer *> league);
    void actionNormal(GameRule *game_rule);

    inline GameRule *gameRule() const
    {
        return game_rule;
    }

    void setNextExtraTurn(ServerPlayer *p)
    {
        nextExtraTurn = p;
    }

    inline bool hasExtraTurn() const
    {
        return nextExtraTurn != nullptr;
    }

    inline ServerPlayer *getNextExtraTurn() const
    {
        return nextExtraTurn;
    }

    inline ServerPlayer *getExtraTurnReturn() const
    {
        return extraTurnReturn;
    }

    inline Room *getRoom() const
    {
        return room;
    }

protected:
    void run() override;

private:
    void _handleTurnBroken3v3(QList<ServerPlayer *> &first, QList<ServerPlayer *> &second, GameRule *game_rule);
    void _handleTurnBrokenHulaoPass(ServerPlayer *uuz, QList<ServerPlayer *> league, GameRule *game_rule);
    void _handleTurnBrokenNormal(GameRule *game_rule);

    Room *room;

    QList<const TriggerSkill *> skill_table[NumOfEvents];
    QSet<QString> skillSet;

    GameRule *game_rule;

    ServerPlayer *nextExtraTurn;
    ServerPlayer *extraTurnReturn;
};

#endif
