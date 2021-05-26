#ifndef _GAME_RULE_H
#define _GAME_RULE_H

#include "skill.h"

class GameRule : public Rule
{
public:
    GameRule();
    bool trigger(TriggerEvent triggerEvent, Room *room, TriggerDetail invoke, QVariant &data) const override;

private:
    void onPhaseProceed(ServerPlayer *player) const;
    void rewardAndPunish(ServerPlayer *killer, ServerPlayer *victim) const;
    void changeGeneral1v1(ServerPlayer *player) const;
    void changeGeneralXMode(ServerPlayer *player) const;
    QString getWinner(ServerPlayer *victim) const;
};

#endif
