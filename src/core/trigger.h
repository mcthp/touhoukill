#ifndef TOUHOUKILL_TRIGGER_H
#define TOUHOUKILL_TRIGGER_H

#include "global.h"

// #include "structs.h"

#include <QList>
#include <QString>
#include <QVariant>

class Card;
class Player;
class RoomObject;
class TriggerDetail;

class TriggerPrivate;

class Trigger // DO NOT INHERIT QObject since it is used in QObject-derived class
{
public:
    Trigger();
    virtual ~Trigger();

    QSanguosha::TriggerEvents triggerEvents() const;
    bool canTrigger(QSanguosha::TriggerEvent e) const;
    void addTriggerEvent(QSanguosha::TriggerEvent e);
    void addTriggerEvents(const QSanguosha::TriggerEvents &e);
    bool isGlobal() const;
    void setGlobal(bool global);

    virtual Q_ALWAYS_INLINE bool isEquipSkill() const
    {
        return false;
    }

    // Let the trigger type to set its priority!
    // Triggers whose priority is not in range {-5,5} are considered to be record with event process

    // Record with event process (Fake move, etc): 10
    // ----- Separator: 5 -----
    // Regular skill: 2
    // Equip skill: 2 (with isEquipSkill set to True)
    // Triggers which is meant to change rule: 1 (return true afterwards!)
    // Game Rule: 0
    // Scenario specific rule: -1
    // ----- Separator: -5 -----
    // Other priority is undefined for now
    // Note that a minus priority is processed after game rule.
    // It may not be what you think since game rule does a lot of things which may not be explained by its name
    // E.g., Minus priority of event TurnStart triggers after turn ends since game rule process the whole turn during its trigger with priority 0.
    virtual int priority() const = 0;

    // Should not trigger other events and affect other things in principle
    virtual void record(QSanguosha::TriggerEvent event, RoomObject *room, QVariant &data) const;

    // TODO: make RoomObject const:
    // Current implementation is: To create a TriggerDetail in this function. All data saved in TriggerDetail is non-const
    // This makes the RoomObject not able to be const even if it should be.
    // EXACTLY STRICTLY NOTHING should be even TOUCHED in this function
    virtual QList<TriggerDetail> triggerable(QSanguosha::TriggerEvent event, RoomObject *room, const QVariant &data) const = 0;

    // TODO: make TriggerDetail implicitly shared
    virtual bool trigger(QSanguosha::TriggerEvent event, RoomObject *room, const TriggerDetail &detail, QVariant &data) const;

private:
    Q_DISABLE_COPY_MOVE(Trigger)
    TriggerPrivate *const d;
};

class Rule : public Trigger
{
public:
    Rule();
    ~Rule() override = default;

    // fixed 0
    int priority() const final override;
    QList<TriggerDetail> triggerable(QSanguosha::TriggerEvent, RoomObject *room, const QVariant &) const final override;
};

class SkillTriggerPrivate;

class SkillTrigger : public Trigger
{
public:
    // TODO: overload (const Skill *)?
    SkillTrigger(const QString &name);
    ~SkillTrigger() override;

    const QString &skillName() const;

    // 2 by default, optional override
    int priority() const override;

    // force subclass override this function
    // virtual QList<TriggerDetail> triggerable(TriggerEvent event, RoomObject *room, const QVariant &data) const = 0;

    bool trigger(QSanguosha::TriggerEvent event, RoomObject *room, const TriggerDetail &detail, QVariant &data) const final override;

    // Limited modification to TriggerDetail, notably tag and target
    virtual bool cost(QSanguosha::TriggerEvent event, RoomObject *room, TriggerDetail &detail, QVariant &data) const;
    // No modification to TriggerDetail since the cost is done
    virtual bool effect(QSanguosha::TriggerEvent event, RoomObject *room, const TriggerDetail &detail, QVariant &data) const = 0;

private:
    SkillTriggerPrivate *const d;
};

class EquipSkillTrigger : public SkillTrigger
{
public:
    EquipSkillTrigger(const QString &name);
    ~EquipSkillTrigger() override = default;

    Q_ALWAYS_INLINE bool isEquipSkill() const final override
    {
        return true;
    }

    static bool equipAvailable(const Player *p, QSanguosha::EquipLocation location, const QString &equip_name, const Player *to = nullptr);
    static bool equipAvailable(const Player *p, const Card *equip, const Player *to = nullptr);

    // fixed 2
    int priority() const final override;
};

class GlobalRecord : public Trigger
{
public:
    GlobalRecord();
    ~GlobalRecord() override = default;

    // fixed 10
    int priority() const final override;

    // Since it may use only Record, override this function here
    // Optional override in subclass
    QList<TriggerDetail> triggerable(QSanguosha::TriggerEvent event, RoomObject *room, const QVariant &data) const override;
};

// a nasty way for 'fake moves', usually used in the process of multi-card chosen
class FakeMoveRecordPrivate;
class FakeMoveRecord final : public GlobalRecord
{
public:
    FakeMoveRecord(const QString &skillName);
    ~FakeMoveRecord() final override;

    QList<TriggerDetail> triggerable(QSanguosha::TriggerEvent event, RoomObject *room, const QVariant &data) const final override;
    bool trigger(QSanguosha::TriggerEvent event, RoomObject *room, const TriggerDetail &detail, QVariant &data) const final override;

private:
    FakeMoveRecordPrivate *const d;
};

#endif
