/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickView>
#include <QString>
#include <QStringList>

#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>

#include "TellyScoutSettings.h"
#include "channelsmodel.h"
#include "channelsproxymodel.h"
#include "countriesmodel.h"
#include "database.h"
#include "fetcher.h"
#include "programsmodel.h"
#include "telly-scout-version.h"

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationName(QStringLiteral("Telly Scout"));

    KAboutData about(QStringLiteral("telly-scout"),
                     i18n("Telly Scout"),
                     QStringLiteral(TELLY_SCOUT_VERSION_STRING),
                     i18n("Channel Reader"),
                     KAboutLicense::GPL,
                     i18n("© 2020 KDE Community"));
    about.addAuthor(i18n("Plata"), QString(), QStringLiteral("plata@example.com"));
    KAboutData::setApplicationData(about);

    qmlRegisterType<CountriesModel>("org.kde.TellyScout", 1, 0, "CountriesModel");
    qmlRegisterType<ChannelsModel>("org.kde.TellyScout", 1, 0, "ChannelsModel");
    qmlRegisterType<ChannelsProxyModel>("org.kde.TellyScout", 1, 0, "ChannelsProxyModel");

    qmlRegisterUncreatableType<ProgramsModel>("org.kde.TellyScout", 1, 0, "ProgramsModel", QStringLiteral("Get from Channel"));

    qmlRegisterSingletonInstance("org.kde.TellyScout", 1, 0, "Fetcher", &Fetcher::instance());
    qmlRegisterSingletonInstance("org.kde.TellyScout", 1, 0, "Database", &Database::instance());

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    KLocalizedString::setApplicationDomain("telly-scout");

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("RSS/Atom Channel Reader"));
    QCommandLineOption addChannelOption(QStringList() << QStringLiteral("a") << QStringLiteral("addchannel"),
                                        i18n("Adds a new channel to database."),
                                        i18n("channel URL"),
                                        QStringLiteral("none"));
    parser.addOption(addChannelOption);

    about.setupCommandLine(&parser);
    parser.process(app);
    QString channelURL = parser.value(addChannelOption);
    if (channelURL != QStringLiteral("none"))
        Database::instance().addChannel(channelURL, "", "", "", ""); // TODO: remove
    about.processCommandLine(&parser);

    engine.rootContext()->setContextProperty(QStringLiteral("_aboutData"), QVariant::fromValue(about));

    TellyScoutSettings settings;

    engine.rootContext()->setContextProperty(QStringLiteral("_settings"), &settings);

    QObject::connect(&app, &QCoreApplication::aboutToQuit, &settings, &TellyScoutSettings::save);

    Database::instance();

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    Fetcher::instance().fetchAll();

    return app.exec();
}
