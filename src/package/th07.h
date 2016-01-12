#ifndef _th07_H
#define _th07_H

#include "package.h"
#include "card.h"

class MocaoCard : public SkillCard
{
    Q_OBJECT

public:
    Q_INVOKABLE MocaoCard();

    virtual bool targetFilter(const QList<const Player *> &targets, const Player *to_select, const Player *Self) const;
    virtual void onEffect(const CardEffectStruct &effect) const;
};


class TH07Package : public Package
{
    Q_OBJECT

public:
    TH07Package();
};

#endif

