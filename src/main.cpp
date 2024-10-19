// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "config-telly-skout.h"
#include "telly-skout-version.h"

#include "TellySkoutSettings.h"
#include "channelsmodel.h"
#include "channelsproxymodel.h"
#include "database.h"
#include "fetcher.h"
#include "groupsmodel.h"
#include "programsmodel.h"
#include "programsproxymodel.h"

#include <KAboutData>
#if HAVE_KCRASH
#include <KCrash>
#endif
#include <KLocalizedContext>
#include <KLocalizedString>

#include <QCommandLineParser>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QString>
#include <QTimer>

#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.breeze"));
#else
    QApplication app(argc, argv);
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
#endif

#if HAVE_KCRASH
    KCrash::initialize();
#endif

    KLocalizedString::setApplicationDomain("telly-skout");

    // about
    const QString applicationDescription = i18n("Convergent TV guide based on Kirigami");

    KAboutData about(QStringLiteral("telly-skout"),
                     i18n("Telly Skout"),
                     QStringLiteral(TELLY_SKOUT_VERSION_STRING),
                     applicationDescription,
                     KAboutLicense::LGPL_V2_1,
                     i18n("© 2020 KDE Community"));
    about.addAuthor(QStringLiteral("Plata"), QString(), QStringLiteral("plata.hill@kdemail.net"));
    KAboutData::setApplicationData(about);

    QGuiApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("org.kde.telly-skout")));

    // command line parser
    QCommandLineParser parser;
    about.setupCommandLine(&parser);

    QCommandLineOption selfTestOpt(QStringLiteral("self-test"), QStringLiteral("internal, for automated testing"));
    parser.addOption(selfTestOpt);

    parser.process(app);
    about.processCommandLine(&parser);

    // trigger fetching of favorites before loading QML such that e.g. network requests can already run in the background
    Database::instance();
    Fetcher::instance().fetchFavorites();

    // register qml types
    qRegisterMetaType<ChannelId>();
    QMetaType::registerConverter<ChannelId, QString>([](const ChannelId &channelId) {
        return channelId.value();
    });
    QMetaType::registerConverter<QString, ChannelId>([](const QString &str) {
        return ChannelId(str);
    });
    qRegisterMetaType<GroupId>();
    QMetaType::registerConverter<GroupId, QString>([](const GroupId &groupId) {
        return groupId.value();
    });
    QMetaType::registerConverter<QString, GroupId>([](const QString &str) {
        return GroupId(str);
    });
    qRegisterMetaType<ProgramId>();
    QMetaType::registerConverter<ProgramId, QString>([](const ProgramId &programId) {
        return programId.value();
    });
    QMetaType::registerConverter<QString, ProgramId>([](const QString &str) {
        return ProgramId(str);
    });

    // setup engine
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

    QObject::connect(&app, &QCoreApplication::aboutToQuit, TellySkoutSettings::self(), &TellySkoutSettings::save);

    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    if (parser.isSet(selfTestOpt)) {
        QTimer::singleShot(std::chrono::milliseconds(250), &app, &QCoreApplication::quit);
    }

    return app.exec();
}
