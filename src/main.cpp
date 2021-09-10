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

#include "TellySkoutSettings.h"
#include "channelsmodel.h"
#include "channelsproxymodel.h"
#include "channelstablemodel.h"
#include "countriesmodel.h"
#include "database.h"
#include "fetcher.h"
#include "programsmodel.h"
#include "telly-skout-version.h"

#ifdef Q_OS_ANDROID
Q_DECL_EXPORT
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    // about
    QCoreApplication::setOrganizationName(QStringLiteral("KDE"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    QCoreApplication::setApplicationName(QStringLiteral("Telly Skout"));

    KAboutData about(QStringLiteral("telly-skout"),
                     i18n("Telly Skout"),
                     QStringLiteral(TELLY_SKOUT_VERSION_STRING),
                     i18n("Channel Reader"),
                     KAboutLicense::GPL,
                     i18n("© 2020 KDE Community"));
    about.addAuthor(i18n("Plata"), QString(), QStringLiteral("plata@example.com"));
    KAboutData::setApplicationData(about);

    // command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription(i18n("Convergent EPG based on Kirigami"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    about.processCommandLine(&parser);

    // register qml types
    qmlRegisterType<CountriesModel>("org.kde.TellySkout", 1, 0, "CountriesModel");
    qmlRegisterType<ChannelsModel>("org.kde.TellySkout", 1, 0, "ChannelsModel");
    qmlRegisterType<ChannelsProxyModel>("org.kde.TellySkout", 1, 0, "ChannelsProxyModel");
    qmlRegisterType<ChannelsTableModel>("org.kde.TellySkout", 1, 0, "ChannelsTableModel");

    qmlRegisterUncreatableType<ProgramsModel>("org.kde.TellySkout", 1, 0, "ProgramsModel", QStringLiteral("Get from Channel"));

    qmlRegisterSingletonInstance("org.kde.TellySkout", 1, 0, "Fetcher", &Fetcher::instance());
    qmlRegisterSingletonInstance("org.kde.TellySkout", 1, 0, "Database", &Database::instance());

    // setup engine
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    KLocalizedString::setApplicationDomain("telly-skout");

    engine.rootContext()->setContextProperty(QStringLiteral("_aboutData"), QVariant::fromValue(about));

    TellySkoutSettings settings;

    engine.rootContext()->setContextProperty(QStringLiteral("_settings"), &settings);

    QObject::connect(&app, &QCoreApplication::aboutToQuit, &settings, &TellySkoutSettings::save);

    Database::instance();

    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
