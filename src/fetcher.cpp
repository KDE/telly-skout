/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QTextDocumentFragment>

#include "database.h"
#include "fetcher.h"

Fetcher::Fetcher()
{
    manager = new QNetworkAccessManager(this);
    manager->setRedirectPolicy(QNetworkRequest::NoLessSafeRedirectPolicy);
    manager->setStrictTransportSecurityEnabled(true);
    manager->enableStrictTransportSecurityStore(true);
}

void Fetcher::fetchCountry(const QString &url)
{
    qDebug() << "Starting to fetch" << url;

    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, url, reply]() {
        if (reply->error()) {
            qWarning() << "Error fetching country";
            qWarning() << reply->errorString();
            Q_EMIT error(url, reply->error(), reply->errorString());
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

            QVector<QString> mCountries;
            for (int i = 0; i < nodes.count(); i++) {
                QDomNode elm = nodes.at(i);
                if (elm.isElement()) {
                    const QDomNamedNodeMap &attributes = elm.attributes();
                    const QString &id = attributes.namedItem("id").toAttr().value();

                    const QString &name = elm.firstChildElement("display-name").text();

                    fetchChannel(id, name);
                }
            }
        }
        delete reply;
    });
}

void Fetcher::fetchChannel(const QString &channelId, const QString &name)
{
    const QString url = "http://xmltv.xmltv.se/" + channelId;

    Q_EMIT startedFetchingChannel(url);

    // if channel is unknown, store it
    QSqlQuery queryChannelExists;
    queryChannelExists.prepare(QStringLiteral("SELECT COUNT (url) FROM Channels WHERE url=:url;"));
    queryChannelExists.bindValue(QStringLiteral(":url"), url);
    Database::instance().execute(queryChannelExists);
    queryChannelExists.next();

    if (queryChannelExists.value(0).toInt() == 0) {
        QSqlQuery queryInsertChannel;
        queryInsertChannel.prepare(QStringLiteral(
            "INSERT INTO Channels VALUES (:name, :url, :image, :link, :description, :deleteAfterCount, :deleteAfterType, :subscribed, :lastUpdated, "
            ":notify, :favorite, :displayName);"));
        queryInsertChannel.bindValue(QStringLiteral(":name"), name);
        queryInsertChannel.bindValue(QStringLiteral(":url"), url);
        queryInsertChannel.bindValue(QStringLiteral(":image"), QLatin1String(""));
        queryInsertChannel.bindValue(QStringLiteral(":link"), QLatin1String(""));
        queryInsertChannel.bindValue(QStringLiteral(":description"), QLatin1String(""));
        queryInsertChannel.bindValue(QStringLiteral(":deleteAfterCount"), 0);
        queryInsertChannel.bindValue(QStringLiteral(":deleteAfterType"), 0);
        queryInsertChannel.bindValue(QStringLiteral(":subscribed"), QDateTime::currentDateTime().toSecsSinceEpoch());
        queryInsertChannel.bindValue(QStringLiteral(":lastUpdated"), 0);
        queryInsertChannel.bindValue(QStringLiteral(":notify"), false);
        queryInsertChannel.bindValue(QStringLiteral(":favorite"), false);
        queryInsertChannel.bindValue(QStringLiteral(":displayName"), QLatin1String(""));
        Database::instance().execute(queryInsertChannel);
    } else {
        // fetch complete program only for favorites
        QSqlQuery queryIsFavorite;
        queryIsFavorite.prepare(QStringLiteral("SELECT COUNT (url) FROM Channels WHERE url=:url AND favorite=TRUE;"));
        queryIsFavorite.bindValue(QStringLiteral(":url"), url);
        Database::instance().execute(queryIsFavorite);
        queryIsFavorite.next();

        if (queryIsFavorite.value(0).toInt() == 1) {
            QDateTime current = QDateTime::currentDateTime();
            const QString urlToday = url + "_" + current.toString("yyyy-MM-dd") + ".xml"; // e.g. http://xmltv.xmltv.se/3sat.de_2021-07-29.xml
            qDebug() << "Starting to fetch" << urlToday;

            Q_EMIT startedFetchingChannel(urlToday);

            QNetworkRequest request((QUrl(urlToday)));
            QNetworkReply *reply = get(request);
            connect(reply, &QNetworkReply::finished, this, [this, url, reply]() {
                if (reply->error()) {
                    qWarning() << "Error fetching channel";
                    qWarning() << reply->errorString();
                    Q_EMIT error(url, reply->error(), reply->errorString());
                } else {
                    QByteArray data = reply->readAll();

                    QDomDocument versionXML;

                    if (!versionXML.setContent(data)) {
                        qWarning() << "Failed to parse XML";
                    }

                    // print out the element names of all elements that are direct children
                    // of the outermost element.
                    /*QDomElement docElem = versionXML.documentElement();

                    QDomNodeList nodes = versionXML.elementsByTagName("country");
                    qDebug() << "Countries";
                    QVector<QString> mCountries;
                    for (int i = 0; i < nodes.count(); i++) {
                      QDomNode elm = nodes.at(i);
                      if (elm.isElement()) {
                        qDebug() << elm.toElement().tagName() << " = "
                                 << elm.toElement().text();
                        mCountries.push_back(elm.toElement().text());
                      }
                    }*/

                    QDomElement docElem = versionXML.documentElement();

                    // channel = channel, program = Sendung
                    processChannel(docElem, url);

                    // https://gitlab.com/tabos/tvguide/-/blob/master/src/tvguide-assistant.c
                }
                delete reply;
            });
        }
    }
}

void Fetcher::fetchAll()
{
    /*QSqlQuery query;
    query.prepare(QStringLiteral("SELECT url FROM Channels;")); // url for channel e.g. http://xmltv.xmltv.se/3sat.de
    Database::instance().execute(query);
    while (query.next()) {
        fetch(query.value(0).toString());
    }*/

    // http://xmltv.se/countries.xml
    const QString url = "http://xmltv.se/countries.xml";
    qDebug() << "Starting to fetch" << url;

    // Q_EMIT startedFetchingChannel(url);

    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, url, reply]() {
        if (reply->error()) {
            qWarning() << "Error fetching countries";
            qWarning() << reply->errorString();
            Q_EMIT error(url, reply->error(), reply->errorString());
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

void Fetcher::processCountry(const QDomElement &country)
{
    const QString &id = country.attributes().namedItem("id").toAttr().value();

    // http://xmltv.xmltv.se/channels-Germany.xml
    const QString url = "http://xmltv.xmltv.se/channels-" + id + ".xml";

    QSqlQuery query;
    query.prepare(QStringLiteral("INSERT INTO Countries VALUES(:channel, :id, :name, :url, :email);"));
    query.bindValue(QStringLiteral(":channel"), "channel"); // TODO
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":name"), country.text());
    query.bindValue(QStringLiteral(":url"), url);
    query.bindValue(QStringLiteral(":email"), "country@example.com"); // TODO
    Database::instance().execute(query);

    fetchCountry(url);
}

void Fetcher::processChannel(const QDomElement &channel, const QString &url)
{
    QDomNodeList programs = channel.elementsByTagName("programme");
    const QDomNamedNodeMap &attributes = programs.at(0).attributes();

    const QString &channelId = attributes.namedItem("channel").toAttr().value();
    if (programs.count() > 0) {
        QSqlQuery query;
        query.prepare(QStringLiteral("UPDATE Channels SET image=:image, link=:link, description=:description, lastUpdated=:lastUpdated WHERE url=:url;"));
        query.bindValue(QStringLiteral(":url"), url);
        query.bindValue(QStringLiteral(":link"), url);
        query.bindValue(QStringLiteral(":description"), ""); // TODO

        QDateTime current = QDateTime::currentDateTime();
        query.bindValue(QStringLiteral(":lastUpdated"), current.toSecsSinceEpoch());
        Database::instance().execute(query);

        /* for (auto &country : channel->countries()) {
             processCountry(country, QLatin1String(""), url);
         }*/

        /*QString image;
        if (channel->image()->url().startsWith(QStringLiteral("/"))) {
            image = QUrl(url).adjusted(QUrl::RemovePath).toString() + channel->image()->url();
        } else {
            image = channel->image()->url();
        }
        query.bindValue(QStringLiteral(":image"), image);
        Database::instance().execute(query);

        qDebug() << "Updated channel title:" << channel->title();*/

        Q_EMIT channelDetailsUpdated(url, channelId, "", url, "", current); // TODO

        for (int i = 0; i < programs.count(); i++) {
            processProgram(programs.at(i), url);
        }

        Q_EMIT channelUpdated(url);
    }
}

void Fetcher::processProgram(const QDomNode &program, const QString &url)
{
    const QDomNamedNodeMap &attributes = program.attributes();
    const QString &channel = attributes.namedItem("channel").toAttr().value();
    const QString &startTimeString = attributes.namedItem("start").toAttr().value();
    QDateTime startTime = QDateTime::fromString(startTimeString, "yyyyMMddHHmmss +0000");
    // channel + start time can be used as ID
    const QString id = channel + startTime.toSecsSinceEpoch();
    const QString &stopTimeString = attributes.namedItem("stop").toAttr().value();
    QDateTime stopTime = QDateTime::fromString(stopTimeString, "yyyyMMddHHmmss +0000");
    const QString &title = program.namedItem("title").toElement().text();
    const QString &subtitle = program.namedItem("sub-title").toElement().text();
    const QString &description = program.namedItem("desc").toElement().text();
    const QString &category = program.namedItem("category").toElement().text();

    qDebug() << "Processing" << title;
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT (id) FROM Programs WHERE id=:id;"));
    query.bindValue(QStringLiteral(":id"), id);
    Database::instance().execute(query);
    query.next();

    if (query.value(0).toInt() != 0) {
        return;
    }

    query.prepare(QStringLiteral("INSERT INTO Programs VALUES (:channel, :id, :title, :content, :created, :updated, :link, 0);"));
    query.bindValue(QStringLiteral(":channel"), url);
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":title"), title);
    query.bindValue(QStringLiteral(":content"), description);
    query.bindValue(QStringLiteral(":created"), startTime.toSecsSinceEpoch());
    query.bindValue(QStringLiteral(":updated"), stopTime.toSecsSinceEpoch());
    query.bindValue(QStringLiteral(":link"), url);

    Database::instance().execute(query);

    // for (const auto &country : program->countries()) {
    //    processCountry(url, id);
    //}

    // for (const auto &enclosure : program->enclosures()) {
    //    processEnclosure(url, id);
    //}*/
}

void Fetcher::processEnclosure(const QString &channelUrl, unsigned int id)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("INSERT INTO Enclosures VALUES (:channel, :id, :duration, :size, :title, :type, :url);"));
    query.bindValue(QStringLiteral(":channel"), channelUrl);
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":duration"), 3); // TODO
    query.bindValue(QStringLiteral(":size"), 500); // TODO
    query.bindValue(QStringLiteral(":title"), "Title"); // TODO
    query.bindValue(QStringLiteral(":type"), "Type"); // TODO
    query.bindValue(QStringLiteral(":url"), channelUrl); // TODO
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
        QByteArray data = reply->readAll();
        QFile file(filePath(url));
        file.open(QIODevice::WriteOnly);
        file.write(data);
        file.close();

        Q_EMIT imageDownloadFinished(url);
        delete reply;
    });
}

void Fetcher::removeImage(const QString &url)
{
    qDebug() << filePath(url);
    QFile(filePath(url)).remove();
}

QString Fetcher::filePath(const QString &url)
{
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/")
        + QString::fromStdString(QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5).toHex().toStdString());
}

QNetworkReply *Fetcher::get(QNetworkRequest &request)
{
    request.setRawHeader("User-Agent", "telly-scout/0.1");
    return manager->get(request);
}
