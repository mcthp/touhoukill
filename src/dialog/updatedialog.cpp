#include "updatedialog.h"
#include "engine.h"
#include "settings.h"

#include <QApplication>
#include <QFile>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QProgressBar>
#include <QPushButton>
#include <QString>
#include <QUrl>
#include <QVBoxLayout>

#if QT_VERSION >= 0x050600
#include <QVersionNumber>
#endif
#ifdef Q_OS_WIN
#include <QWinTaskbarButton>
#include <QWinTaskbarProgress>
#endif

UpdateDialog::UpdateDialog(QWidget *parent)
    : QDialog(parent)
    , bar(new QProgressBar)
    , lbl(new QLabel)
    , downloadManager(new QNetworkAccessManager(this))
    , scriptReply(NULL)
    , packReply(NULL)
    , taskbarButton(NULL)
    , m_finishedScript(false)
    , m_finishedPack(false)
    , m_busy(false)
{
    setWindowTitle(tr("New Version Available"));

    bar->setMaximum(10000);

    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget(lbl);
    layout->addWidget(bar);

    QPushButton *yesBtn = new QPushButton(tr("Yes"));
    connect(yesBtn, &QPushButton::clicked, [yesBtn]() -> void { yesBtn->setDisabled(true); });

    QPushButton *noBtn = new QPushButton(tr("No"));
    connect(noBtn, &QPushButton::clicked, [this]() -> void { QDialog::reject(); });
    connect(yesBtn, &QPushButton::clicked, [noBtn]() -> void { noBtn->setDisabled(true); });

    connect(yesBtn, &QPushButton::clicked, this, &UpdateDialog::startDownload);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(yesBtn);
    hlayout->addWidget(noBtn);

    layout->addLayout(hlayout);

    setLayout(layout);
}

void UpdateDialog::checkForUpdate()
{
    QNetworkRequest req;
#if QT_VERSION >= 0x050600
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif

    req.setUrl(QUrl("https://www.touhousatsu.rocks/TouhouKillUpdate0.9.json"));

    QNetworkReply *reply = downloadManager->get(req);
    connect(reply, (void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error), this, &UpdateDialog::updateError);
    connect(reply, &QNetworkReply::finished, this, &UpdateDialog::updateInfoReceived);
}

void UpdateDialog::updateError(QNetworkReply::NetworkError)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply != NULL)
        disconnect(reply, &QNetworkReply::finished, this, 0);
}

void UpdateDialog::updateInfoReceived()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (reply == NULL)
        return;

    QByteArray arr = reply->readAll();

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(arr, &err);

    if (err.error != QJsonParseError::NoError) {
        return;
    }
    if (!doc.isObject()) {
        qDebug() << "error document when parsing update data";
        return;
    }

    QJsonObject ob;
    QJsonObject fullOb = doc.object();
    ob = fullOb.value("Global").toObject();
    if (!ob.contains("LatestVersion") || !ob.value("LatestVersion").isString()) {
        qDebug() << "LatestVersion field is incorrect";
        return;
    }

    QString latestVersion = ob.value("LatestVersion").toString();

#if QT_VERSION >= 0x050600
    QVersionNumber ver = QVersionNumber::fromString(ob.value("LatestVersionNumber").toString());
#else
    QString ver = ob.value("LatestVersionNumber").toString();
#endif

    if (latestVersion > Sanguosha->getVersionNumber()) {
        // there is a new version available now!!
        QString from = QString("From") + Sanguosha->getVersionNumber();
        if (ob.contains(from))
            parseUpdateInfo(latestVersion, ver, ob.value(from).toObject());
        else {
#if QT_VERSION >= 0x050600
            QVersionNumber pref = QVersionNumber::commonPrefix(Sanguosha->getQVersionNumber(), ver);
            from = QString("From") + pref.toString();
            if (ob.contains(from))
                parseUpdateInfo(latestVersion, ver, ob.value(from).toObject());
            else
#endif
                parseUpdateInfo(latestVersion, ver, ob.value("FullPack").toObject());
        }
    }
}

#if QT_VERSION >= 0x050600
void UpdateDialog::parseUpdateInfo(const QString &v, const QVersionNumber &vn, const QJsonObject &ob)
#else
void UpdateDialog::parseUpdateInfo(const QString &v, const QString &vn, const QJsonObject &ob)
#endif
{
#if defined(Q_OS_WIN)
    QJsonValue value = ob.value("Win");
#elif defined(Q_OS_ANDROID)
    QJsonValue value = ob.value("And");
#elif defined(Q_OS_MACOS)
    QJsonValue value = ob.value("Mac");
#else
    QJsonValue value = ob.value("Oth");
#endif
    if (value.isString()) {
        QMessageBox mbox(this);
        mbox.setTextFormat(Qt::RichText);
        mbox.setText(tr("New Version %1(%3) available.<br/>"
                        "But we don\'t support auto-updating from %2 to %1 on this platform.<br/>"
                        "Please download the full package from <a href=\"%4\">Here</a>.")
                         .arg(v)
                         .arg(Sanguosha->getVersionNumber())
#if QT_VERSION >= 0x050600
                         .arg(vn.toString())
#else
                         .arg(vn)
#endif
                         .arg(value.toString()));
        mbox.setWindowTitle(tr("New Version Avaliable"));
        mbox.setIcon(QMessageBox::Information);
        mbox.setStandardButtons(QMessageBox::Ok);
        mbox.exec();
    } else if (value.isObject()) {
        QJsonObject updateOb = value.toObject();
#ifndef Q_OS_ANDROID
        QString updateScript = updateOb.value("UpdateScript").toString();
#else
        QString updateScript = "jni";
#endif
        QString packKey = "UpdatePack";
        QString hashKey = "UpdatePackHash";

        QString updatePack = updateOb.value(packKey).toString();
        QJsonObject updateHash = updateOb.value(hashKey).toObject();
        if (!updateScript.isEmpty() && !updatePack.isEmpty() && !updateHash.isEmpty()) {
            setInfo(v, vn, updatePack, updateHash, updateScript);
            exec();
        }
    }
}

#if QT_VERSION >= 0x050600
void UpdateDialog::setInfo(const QString &v, const QVersionNumber &vn, const QString &updatePackOrAddress, const QJsonObject &updateHash, const QString &updateScript)
#else
void UpdateDialog::setInfo(const QString &v, const QString &vn, const QString &updatePackOrAddress, const QJsonObject &updateHash, const QString &updateScript)
#endif
{
    lbl->setText(tr("New Version %1(%3) available.\n"
                    "We support auto-updating from %2 to %1 on this platform.\n"
                    "Click 'Yes' to update now.")
                     .arg(v)
                     .arg(Sanguosha->getVersionNumber())
#if QT_VERSION >= 0x050600
                     .arg(vn.toString()));
#else
                     .arg(vn));
#endif

    m_updateScript = updateScript;
    m_updatePack = updatePackOrAddress;
    m_updateHash = updateHash;
}

void UpdateDialog::startUpdate()
{
#ifdef Q_OS_WIN
    taskbarButton->progress()->hide();
#endif
// we should run update script and then exit this main program.
#if defined(Q_OS_WIN)
    QStringList arg;
    arg << "UpdateScript.vbs" << QString::number(QCoreApplication::applicationPid());
    QProcess::startDetached("wscript", arg, QCoreApplication::applicationDirPath());
#elif defined(Q_OS_ANDROID)
// call JNI to install the package
#else
    QStringList arg;
    arg << "-c" << ("\"./UpdateScript.sh " + QString::number(QCoreApplication::applicationPid()) + "\"");
    QProcess::startDetached("sh", arg, QCoreApplication::applicationDirPath());
#endif

    QCoreApplication::exit(0);
    QDialog::accept();
}

bool UpdateDialog::packHashVerify(const QByteArray &arr)
{
    static const QMap<QString, QCryptographicHash::Algorithm> algorithms {std::make_pair<QString, QCryptographicHash::Algorithm>("MD5", QCryptographicHash::Md5),
                                                                          std::make_pair<QString, QCryptographicHash::Algorithm>("SHA1", QCryptographicHash::Sha1)};

    foreach (const QString &str, algorithms.keys()) {
        if (m_updateHash.contains(str)) {
            QString hash = m_updateHash.value(str).toString();
            QByteArray calculatedHash = QCryptographicHash::hash(arr, algorithms.value(str));
            if (hash.toUpper() != QString::fromLatin1(calculatedHash.toHex()).toUpper())
                return false;
        }
    }

    return true;
}

void UpdateDialog::startDownload()
{
    if (m_updatePack.isEmpty() || m_updateScript.isEmpty()) {
        QMessageBox::critical(this, tr("Update Error"), tr("An error occurred when downloading packages.\nURL is empty."));
        QDialog::reject();
        return;
    }

    m_busy = true;

    QNetworkRequest reqPack;
#if QT_VERSION >= 0x050600
    reqPack.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
    reqPack.setUrl(QUrl(m_updatePack));
    packReply = downloadManager->get(reqPack);
    connect(packReply, &QNetworkReply::downloadProgress, this, &UpdateDialog::downloadProgress);
    connect(packReply, (void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error), this, &UpdateDialog::errPack);
    connect(packReply, &QNetworkReply::finished, this, &UpdateDialog::finishedPack);

#ifndef Q_OS_ANDROID
    QNetworkRequest reqScript;
#if QT_VERSION >= 0x050600
    reqScript.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
#endif
    reqScript.setUrl(QUrl(m_updateScript));
    scriptReply = downloadManager->get(reqScript);
    connect(scriptReply, (void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error), this, &UpdateDialog::errScript);
    connect(scriptReply, &QNetworkReply::finished, this, &UpdateDialog::finishedScript);
#else
    m_finishedScript = true;
#endif

#ifdef Q_OS_WIN
    taskbarButton->progress()->reset();
    taskbarButton->progress()->show();
#endif
}

void UpdateDialog::downloadProgress(quint64 downloaded, quint64 total)
{
    bar->setValue(10000 * downloaded / total);
#ifdef Q_OS_WIN
    taskbarButton->progress()->setValue(10000 * downloaded / total);
#endif
}

void UpdateDialog::finishedScript()
{
#if defined(Q_OS_WIN)
    QString suffix = ".vbs";
#else
    QString suffix = ".sh";
#endif
    QByteArray arr = scriptReply->readAll();
    QFile file;
    file.setFileName(QString("UpdateScript") + suffix);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    file.write(arr);
    file.close();

#ifdef Q_OS_UNIX
    file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner | QFile::ReadGroup | QFile::ExeGroup | QFile::ReadOther | QFile::ExeOther);
#endif

    m_finishedScript = true;
    if (m_finishedPack && m_finishedScript)
        startUpdate();
}

void UpdateDialog::errScript()
{
#ifdef Q_OS_WIN
    taskbarButton->progress()->hide();
#endif
    if (scriptReply != NULL) {
        disconnect(scriptReply, (void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error), this, &UpdateDialog::errScript);
        disconnect(scriptReply, &QNetworkReply::finished, this, &UpdateDialog::finishedScript);
    }
    if (packReply != NULL) {
        disconnect(packReply, &QNetworkReply::downloadProgress, this, &UpdateDialog::downloadProgress);
        disconnect(packReply, (void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error), this, &UpdateDialog::errPack);
        disconnect(packReply, &QNetworkReply::finished, this, &UpdateDialog::finishedPack);
    }

    QMessageBox::critical(this, tr("Update Error"), tr("An error occurred when downloading packages.\nCannot download the update script."));
    QDialog::reject();
}

void UpdateDialog::finishedPack()
{
#if defined(Q_OS_WIN)
    QString suffix = ".7z";
#elif defined(Q_OS_ANDROID)
    QString suffix = ".apk";
#else
    QString suffix = ".tar.xz";
#endif
    QByteArray arr = packReply->readAll();

    if (!packHashVerify(arr)) {
        QMessageBox::critical(this, tr("Update Error"), tr("An error occurred when downloading packages.\nDownload pack checksum mismatch."));
#ifdef Q_OS_WIN
        taskbarButton->progress()->hide();
#endif
        QDialog::reject();
        return;
    }

    QFile file;
    file.setFileName(QString("UpdatePack") + suffix);
    file.open(QIODevice::WriteOnly | QIODevice::Truncate);
    file.write(arr);
    file.close();

    m_finishedPack = true;

    if (m_finishedPack && m_finishedScript)
        startUpdate();
}

void UpdateDialog::errPack()
{
#ifdef Q_OS_WIN
    taskbarButton->progress()->hide();
#endif
    if (scriptReply != NULL) {
        disconnect(scriptReply, (void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error), this, &UpdateDialog::errScript);
        disconnect(scriptReply, &QNetworkReply::finished, this, &UpdateDialog::finishedScript);
    }
    if (packReply != NULL) {
        disconnect(packReply, &QNetworkReply::downloadProgress, this, &UpdateDialog::downloadProgress);
        disconnect(packReply, (void (QNetworkReply::*)(QNetworkReply::NetworkError))(&QNetworkReply::error), this, &UpdateDialog::errPack);
        disconnect(packReply, &QNetworkReply::finished, this, &UpdateDialog::finishedPack);
    }

    QMessageBox::critical(this, tr("Update Error"), tr("An error occurred when downloading packages.\nCannot download the update pack."));
    QDialog::reject();
}

void UpdateDialog::accept()
{
}

void UpdateDialog::reject()
{
    if (!m_busy)
        QDialog::reject();
}

void UpdateDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);
#ifdef Q_OS_WIN
    taskbarButton = new QWinTaskbarButton(this);
    taskbarButton->setWindow(windowHandle());
    QWinTaskbarProgress *prog = taskbarButton->progress();
    prog->setVisible(false);
    prog->setMinimum(0);
    prog->reset();
    prog->setMaximum(10000);
#endif
}
