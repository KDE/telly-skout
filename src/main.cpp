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
#include "database.h"
#include "entriesmodel.h"
#include "feedgroupsmodel.h"
#include "feedsmodel.h"
#include "feedsproxymodel.h"
#include "fetcher.h"
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
                     i18n("Feed Reader"),
                     KAboutLicense::GPL,
                     i18n("© 2020 KDE Community"));
    about.addAuthor(i18n("Plata"), QString(), QStringLiteral("plata@example.com"));
    KAboutData::setApplicationData(about);

    qmlRegisterType<FeedsModel>("org.kde.TellyScout", 1, 0, "FeedsModel");
    qmlRegisterType<FeedGroupsModel>("org.kde.TellyScout", 1, 0, "FeedGroupsModel");
    qmlRegisterType<FeedsProxyModel>("org.kde.TellyScout", 1, 0, "FeedsProxyModel");

    qmlRegisterUncreatableType<EntriesModel>("org.kde.TellyScout", 1, 0, "EntriesModel", QStringLiteral("Get from Feed"));

    qmlRegisterSingletonInstance("org.kde.TellyScout", 1, 0, "Fetcher", &Fetcher::instance());
    qmlRegisterSingletonInstance("org.kde.TellyScout", 1, 0, "Database", &Database::instance());

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    KLocalizedString::setApplicationDomain("telly-scout");

    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("RSS/Atom Feed Reader"));
    QCommandLineOption addFeedOption(QStringList() << QStringLiteral("a") << QStringLiteral("addfeed"),
                                     i18n("Adds a new feed to database."),
                                     i18n("feed URL"),
                                     QStringLiteral("none"));
    parser.addOption(addFeedOption);

    about.setupCommandLine(&parser);
    parser.process(app);
    QString feedURL = parser.value(addFeedOption);
    if (feedURL != QStringLiteral("none"))
        Database::instance().addFeed(feedURL);
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

    return app.exec();
}
