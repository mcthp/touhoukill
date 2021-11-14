#ifndef _LUA_WRAPPER_H
#define _LUA_WRAPPER_H

#include <QMetaObject>
#include <QObject>
#include <QPointer>

class LuaStatePrivate;
class LuaMultiThreadEnvironment;
class LuaMultiThreadEnvironmentPrivate;
class Package;
class CardFace;
class Skill;
struct lua_State;

class LuaState final : public QObject
{
    Q_OBJECT

public:
    ~LuaState() override;

    lua_State *data();

    // Helper functions
    bool pushCardFace(const QString &name);
    bool pushCardFaces();
    QStringList cardFaceNames() const;
    bool pushSkill(const QString &name);
    bool pushSkills();
    QStringList skillNames() const;
    bool pushPackage(const QString &name);
    bool pushPackages();
    QStringList packageNames() const;

private:
    LuaStatePrivate *d;
    friend class LuaMultiThreadEnvironment;

    LuaState();
    Q_DISABLE_COPY_MOVE(LuaState)
};

class LuaStatePointer final
{
public:
    LuaStatePointer(const LuaStatePointer &) = default;
    LuaStatePointer &operator=(const LuaStatePointer &) = default;
    inline LuaStatePointer(LuaState *p)
        : d(p)
    {
    }

    inline LuaState *data() const
    {
        return d.data();
    }

    inline LuaState *operator->() const
    {
        return d.data();
    }

    inline LuaState &operator*() const
    {
        Q_ASSERT(!d.isNull());
        return *(d.data());
    }

    inline operator LuaState *() const
    {
        return d.data();
    }

    inline bool isNull() const
    {
        return d.isNull();
    }

    inline operator lua_State *() const
    {
        if (d.isNull())
            return nullptr;

        return d->data();
    }

private:
    LuaStatePointer() = delete;
    QPointer<LuaState> d;
};

class LuaMultiThreadEnvironment final
{
public:
    static LuaStatePointer luaStateForCurrentThread();

    static const QList<const Package *> &packages();
    static const QList<const CardFace *> &cardFaces();
    static const QList<const Skill *> &skills();

    static const QString &luaVersion();
    static const QString &luaCopyright();

private:
    LuaMultiThreadEnvironmentPrivate *d;
    Q_DISABLE_COPY_MOVE(LuaMultiThreadEnvironment)

    LuaMultiThreadEnvironment();
    static LuaMultiThreadEnvironment *self();
};

namespace BuiltinExtension {

// Should this be here? Maybe Engine should be responsible for this instead
// Exit program when a pure-server program runs, and disable connecting to any server (except for localhost) on main window
// Does nothing when debug is on
void disableConnectToServer();

} // namespace BuiltinExtension
#endif
