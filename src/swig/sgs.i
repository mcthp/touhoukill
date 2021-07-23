%module sgs

%{

#include <QObject>
#include <QVariant>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QByteArray>

typedef void *LuaQrcWrapper;
LuaQrcWrapper qrc = nullptr;

namespace BuiltinExtension {
QStringList names();
bool verifyChecksum(const QString &extensionName);
void disableConnectToServer();
} // namespace BuiltinExtension

%}

%include "naturalvar.i"

class LuaQrcWrapper {
private:
    LuaQrcWrapper() = delete;
    ~LuaQrcWrapper() = delete;
    LuaQrcWrapper(const LuaQrcWrapper &) = delete;
    LuaQrcWrapper &operator=(const LuaQrcWrapper&) = delete;
};

%extend LuaQrcWrapper {
    QByteArray contents(const QString &n) noexcept {
        QString fileName = n;
        if (!fileName.startsWith("qrc:"))
            return QByteArray();

        fileName = fileName.mid(3);
        QFile f(fileName);
        if (!f.exists())
            return QByteArray();

        f.open(QIODevice::ReadOnly);
        return f.readAll();
    }

    bool contains(const QString &n) noexcept {
        QString fileName = n;
        if (!fileName.startsWith("qrc:"))
            return false;

        fileName = fileName.mid(3);
        QFileInfo f(fileName);
        return f.exists();
    }
};

extern LuaQrcWrapper qrc;

class QObject {
public:
    QString objectName();
    void setObjectName(const char *name);
    bool inherits(const char *class_name);
    bool setProperty(const char *name, const QVariant &value);
    QVariant property(const char *name) const;
    void setParent(QObject *parent);
    void deleteLater();
};

class BuiltinExtension {
private:
    LuaQrcWrapper() = delete;
    ~LuaQrcWrapper() = delete;
    LuaQrcWrapper(const LuaQrcWrapper &) = delete;
    LuaQrcWrapper &operator=(const LuaQrcWrapper&) = delete;
public:
    static QStringList names();
    static bool verifyChecksum(const QString &extensionName);
    static void disableConnectToServer();
};
