#include "audio.h"
#include "mainwindow.h"
#include "server.h"
#include "settings.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QStyleFactory>
#include <QTranslator>

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "-server") == 0) {
        new QCoreApplication(argc, argv);
    } else {
        new QApplication(argc, argv);
        QCoreApplication::addLibraryPath(QCoreApplication::applicationDirPath() + "/plugins");

#ifdef Q_OS_OSX
        if (QStyleFactory::keys().contains("Fusion", Qt::CaseInsensitive))
            qApp->setStyle(QStyleFactory::create("Fusion"));
#endif
    }

#ifdef QT_NO_DEBUG
    QDir::setCurrent(qApp->applicationDirPath());
#endif

#ifdef Q_OS_LINUX
    QDir dir(QString("lua"));
    if (dir.exists() && (dir.exists(QString("config.lua")))) {
        // things look good and use current dir
    } else {
        QDir::setCurrent(qApp->applicationFilePath().replace("games", "share"));
    }
#endif

    // initialize random seed for later use
    qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

    QTranslator qt_translator, translator;
    qt_translator.load("qt_zh_CN.qm");
    translator.load("sanguosha.qm");

    qApp->installTranslator(&qt_translator);
    qApp->installTranslator(&translator);

    Sanguosha = new Engine;
    Config.init();

    if (qApp->arguments().contains("-server")) {
        Server *server = new Server(qApp);
        printf("Server is starting on port %u\n", Config.ServerPort);

        if (server->listen())
            printf("Starting successfully\n");
        else
            printf("Starting failed!\n");

        return qApp->exec();
    }

    QFile file("sanguosha.qss");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        qApp->setStyleSheet(stream.readAll());
    }

    qApp->setFont(Config.AppFont);

#ifdef AUDIO_SUPPORT
    Audio::init();
#endif

    MainWindow *main_window = new MainWindow;

    Sanguosha->setParent(main_window);
    main_window->show();

    foreach (QString arg, qApp->arguments()) {
        if (arg.startsWith("-connect:")) {
            arg.remove("-connect:");
            Config.HostAddress = arg;
            Config.setValue("HostUrl", arg);

            main_window->startConnection();
            break;
        }
    }

    int execResult = qApp->exec();
    delete qApp;
    return execResult;
}
