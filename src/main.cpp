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
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QString>

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
    QQuickStyle::setStyle(QStringLiteral("Material"));
#else
    QApplication app(argc, argv);
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
#endif

#if HAVE_KCRASH
    KCrash::initialize();
#endif

    // about
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationName(QStringLiteral("Telly Skout"));

    const QString applicationDescription = i18n("Convergent TV guide based on Kirigami");

    KAboutData about(QStringLiteral("telly-skout"),
                     i18n("Telly Skout"),
                     QStringLiteral(TELLY_SKOUT_VERSION_STRING),
                     applicationDescription,
                     KAboutLicense::GPL,
                     i18n("© 2020 KDE Community"));
    about.addAuthor("Plata", QString(), QStringLiteral("plata@example.com"));
    KAboutData::setApplicationData(about);

    // command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(applicationDescription);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    // register qml types
    qmlRegisterType<GroupsModel>("org.kde.TellySkout", 1, 0, "GroupsModel");
    qmlRegisterType<ChannelsModel>("org.kde.TellySkout", 1, 0, "ChannelsModel");
    qmlRegisterType<ChannelsProxyModel>("org.kde.TellySkout", 1, 0, "ChannelsProxyModel");
    qmlRegisterType<ProgramsProxyModel>("org.kde.TellySkout", 1, 0, "ProgramsProxyModel");

    qmlRegisterUncreatableType<ProgramsModel>("org.kde.TellySkout", 1, 0, "ProgramsModel", QStringLiteral("Get from Channel"));

    qmlRegisterSingletonInstance("org.kde.TellySkout", 1, 0, "Fetcher", &Fetcher::instance());

    // setup engine
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    KLocalizedString::setApplicationDomain("telly-skout");

    engine.rootContext()->setContextProperty(QStringLiteral("_aboutData"), QVariant::fromValue(about));

    engine.rootContext()->setContextProperty(QStringLiteral("_settings"), TellySkoutSettings::self());

    QObject::connect(&app, &QCoreApplication::aboutToQuit, TellySkoutSettings::self(), &TellySkoutSettings::save);

    Database::instance();

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
