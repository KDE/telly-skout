/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "fetcher.h"

#include "database.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QtXml>

Fetcher::Fetcher()
{
    manager = new QNetworkAccessManager(this);
    manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    manager->setStrictTransportSecurityEnabled(true);
    manager->enableStrictTransportSecurityStore(true);
}

void Fetcher::fetchFavorites()
{
    qDebug() << "Starting to fetch favorites";

    Q_EMIT startedFetchingFavorites();

    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT channel FROM Favorites;"));
    Database::instance().execute(query);
    while (query.next()) {
        const QString &channelId = query.value(QStringLiteral("channel")).toString();
        fetchProgram(channelId);
    }

    Q_EMIT finishedFetchingFavorites();
}

void Fetcher::fetchChannels()
{
    // http://xmltv.se/countries.xml
    const QString url = "http://xmltv.se/countries.xml";
    qDebug() << "Starting to fetch countries (" << url << ")";

    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, url, reply]() {
        if (reply->error()) {
            qWarning() << "Error fetching countries";
            qWarning() << reply->errorString();
            Q_EMIT error(url, reply->error(), reply->errorString()); // TODO: error handling for countries fetching (see channel.cpp)
        } else {
            QByteArray data = reply->readAll();

            QDomDocument versionXML;

            if (!versionXML.setContent(data)) {
                qWarning() << "Failed to parse XML";
            }

            // print out the element names of all elements that are direct children
            // of the outermost element.
            QDomElement docElem = versionXML.documentElement();

            QDomNodeList nodes = versionXML.elementsByTagName("country");
            for (int i = 0; i < nodes.count(); i++) {
                QDomNode elm = nodes.at(i);
                if (elm.isElement()) {
                    const QDomElement &countryElement = elm.toElement();
                    processCountry(countryElement);
                }
            }
        }
        delete reply;
    });
}

void Fetcher::fetchCountry(const QString &url, const QString &countryId)
{
    qDebug() << "Starting to fetch country (" << countryId << ", " << url << ")";

    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, url, countryId, reply]() {
        if (reply->error()) {
            qWarning() << "Error fetching country";
            qWarning() << reply->errorString();
            Q_EMIT error(url, reply->error(), reply->errorString()); // TODO: error handling for country fetching (see channel.cpp)
        } else {
            QByteArray data = reply->readAll();

            QDomDocument versionXML;

            if (!versionXML.setContent(data)) {
                qWarning() << "Failed to parse XML";
            }

            // print out the element names of all elements that are direct children
            // of the outermost element.
            QDomElement docElem = versionXML.documentElement();

            QDomNodeList nodes = versionXML.elementsByTagName("channel");

            for (int i = 0; i < nodes.count(); i++) {
                QDomNode elm = nodes.at(i);
                if (elm.isElement()) {
                    const QDomNamedNodeMap &attributes = elm.attributes();
                    const QString &id = attributes.namedItem("id").toAttr().value();

                    const QString &name = elm.firstChildElement("display-name").text();

                    fetchChannel(id, name, countryId);
                }
            }
        }
        delete reply;
    });
}

void Fetcher::fetchChannel(const QString &channelId, const QString &name, const QString &country)
{
    const QString url = "http://xmltv.xmltv.se/" + channelId;

    Q_EMIT startedFetchingChannel(channelId);

    // story channel per country (ignore if it exists already)
    QSqlQuery countryQuery;
    countryQuery.prepare(QStringLiteral("INSERT OR IGNORE INTO CountryChannels VALUES (:id, :country, :channel);"));
    countryQuery.bindValue(QStringLiteral(":id"), country + "_" + channelId);
    countryQuery.bindValue(QStringLiteral(":country"), country);
    countryQuery.bindValue(QStringLiteral(":channel"), channelId);
    Database::instance().execute(countryQuery);

    // if channel is unknown, store it
    QSqlQuery queryChannelExists;
    queryChannelExists.prepare(QStringLiteral("SELECT COUNT (id) FROM Channels WHERE id=:id;"));
    queryChannelExists.bindValue(QStringLiteral(":id"), channelId);
    Database::instance().execute(queryChannelExists);
    queryChannelExists.next();

    if (queryChannelExists.value(0).toInt() == 0) {
        const QString image = "https://gitlab.com/xmltv-se/open-source/channel-logos/-/raw/master/vector/" + channelId + "_color.svg?inline=false";
        Database::instance().addChannel(channelId, name, url, country, image);

        Q_EMIT channelUpdated(channelId);
    }
}

void Fetcher::fetchProgram(const QString &channelId)
{
    const QString url = "http://xmltv.xmltv.se/" + channelId;

    QDate today = QDate::currentDate();
    QDate yesterday = QDate::currentDate().addDays(-1);
    QDate tomorrow = QDate::currentDate().addDays(1);
    QSet<QDate> days{yesterday, today, tomorrow};
    for (auto day : days) {
        // check if program is available already
        const QDateTime utcTime(day, QTime(), Qt::UTC);
        const qint64 lastTime = utcTime.addDays(1).toSecsSinceEpoch() - 1;
        QSqlQuery queryProgramAvailable;
        queryProgramAvailable.prepare(QStringLiteral("SELECT COUNT (id) FROM Programs WHERE channel=:channel AND stop>=:lastTime;"));
        queryProgramAvailable.bindValue(QStringLiteral(":channel"), "http://xmltv.xmltv.se/" + channelId); // TODO use channel ID in Programs
        queryProgramAvailable.bindValue(QStringLiteral(":lastTime"), lastTime);
        Database::instance().execute(queryProgramAvailable);
        queryProgramAvailable.next();

        if (queryProgramAvailable.value(0).toInt() > 0) {
            continue;
        }

        const QString urlDay = url + "_" + day.toString("yyyy-MM-dd") + ".xml"; // e.g. http://xmltv.xmltv.se/3sat.de_2021-07-29.xml
        qDebug() << "Starting to fetch program for " << channelId << "(" << urlDay << ")";

        QNetworkRequest request((QUrl(urlDay)));
        QNetworkReply *reply = get(request);
        connect(reply, &QNetworkReply::finished, this, [this, channelId, url, reply]() {
            if (reply->error()) {
                qWarning() << "Error fetching channel";
                qWarning() << reply->errorString();
                Q_EMIT error(channelId, reply->error(), reply->errorString());
            } else {
                QByteArray data = reply->readAll();

                QDomDocument versionXML;

                if (!versionXML.setContent(data)) {
                    qWarning() << "Failed to parse XML";
                }

                QDomElement docElem = versionXML.documentElement();

                processChannel(docElem, url);
            }
            delete reply;
        });
    }
}

void Fetcher::processCountry(const QDomElement &country)
{
    const QString &id = country.attributes().namedItem("id").toAttr().value();
    const QString &name = country.text();

    Q_EMIT startedFetchingCountry(id);

    // http://xmltv.xmltv.se/channels-Germany.xml
    const QString url = "http://xmltv.xmltv.se/channels-" + id + ".xml";

    // if country is unknown, store it
    QSqlQuery queryCountryExists;
    queryCountryExists.prepare(QStringLiteral("SELECT COUNT (id) FROM Channels WHERE id=:id;"));
    queryCountryExists.bindValue(QStringLiteral(":id"), id);
    Database::instance().execute(queryCountryExists);
    queryCountryExists.next();

    if (queryCountryExists.value(0).toInt() == 0) {
        Database::instance().addCountry(id, name, url);
    }

    fetchCountry(url, id);

    Q_EMIT countryUpdated(id);
}

void Fetcher::processChannel(const QDomElement &channel, const QString &url)
{
    QDomNodeList programs = channel.elementsByTagName("programme");
    const QDomNamedNodeMap &attributes = programs.at(0).attributes();

    const QString &channelId = attributes.namedItem("channel").toAttr().value();
    if (programs.count() > 0) {
        for (int i = 0; i < programs.count(); i++) {
            processProgram(programs.at(i), url);
        }

        Q_EMIT channelUpdated(channelId);
    }
}

void Fetcher::processProgram(const QDomNode &program, const QString &url)
{
    const QDomNamedNodeMap &attributes = program.attributes();
    const QString &channel = attributes.namedItem("channel").toAttr().value();
    const QString &startTimeString = attributes.namedItem("start").toAttr().value();
    QDateTime startTime = QDateTime::fromString(startTimeString, "yyyyMMddHHmmss +0000");
    startTime.setTimeSpec(Qt::UTC);
    // channel + start time can be used as ID
    const QString id = channel + "_" + QString::number(startTime.toSecsSinceEpoch());
    const QString &stopTimeString = attributes.namedItem("stop").toAttr().value();
    QDateTime stopTime = QDateTime::fromString(stopTimeString, "yyyyMMddHHmmss +0000");
    stopTime.setTimeSpec(Qt::UTC);
    const QString &title = program.namedItem("title").toElement().text();
    const QString &subtitle = program.namedItem("sub-title").toElement().text();
    const QString &description = program.namedItem("desc").toElement().text();
    const QString &category = program.namedItem("category").toElement().text();

    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT (id) FROM Programs WHERE id=:id;"));
    query.bindValue(QStringLiteral(":id"), id);
    Database::instance().execute(query);
    query.next();

    if (query.value(0).toInt() != 0) {
        return;
    }

    query.prepare(QStringLiteral("INSERT INTO Programs VALUES (:id, :channel, :start, :stop, :title, :subtitle, :description, :category);"));
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":channel"), url);
    query.bindValue(QStringLiteral(":start"), startTime.toSecsSinceEpoch());
    query.bindValue(QStringLiteral(":stop"), stopTime.toSecsSinceEpoch());
    query.bindValue(QStringLiteral(":title"), title);
    query.bindValue(QStringLiteral(":subtitle"), subtitle);
    query.bindValue(QStringLiteral(":description"), description);
    query.bindValue(QStringLiteral(":category"), category);

    Database::instance().execute(query);
}

QString Fetcher::image(const QString &url)
{
    QString path = filePath(url);
    if (QFileInfo::exists(path)) {
        return path;
    }

    download(url);

    return QLatin1String("");
}

void Fetcher::download(const QString &url)
{
    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, url, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray data = reply->readAll();
            QFile file(filePath(url));
            file.open(QIODevice::WriteOnly);
            file.write(data);
            file.close();
        }
        Q_EMIT imageDownloadFinished(url);

        delete reply;
    });
}

void Fetcher::removeImage(const QString &url)
{
    qDebug() << "Remove image: " << filePath(url);
    QFile(filePath(url)).remove();
}

QString Fetcher::filePath(const QString &url)
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/")
        + QString::fromStdString(QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5).toHex().toStdString());
}

QNetworkReply *Fetcher::get(QNetworkRequest &request)
{
    request.setRawHeader("User-Agent", "telly-skout/0.1");
    return manager->get(request);
}
