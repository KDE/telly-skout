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

void Fetcher::fetch(const QString &url)
{
    QDateTime current = QDateTime::currentDateTime();
    const QString urlToday = url + "_" + current.toString("yyyy-MM-dd") + ".xml"; // e.g. http://xmltv.xmltv.se/3sat.de_2021-07-29.xml
    qDebug() << "Starting to fetch" << urlToday;

    Q_EMIT startedFetchingFeed(urlToday);

    QNetworkRequest request((QUrl(urlToday)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, url, reply]() {
        if (reply->error()) {
            qWarning() << "Error fetching feed";
            qWarning() << reply->errorString();
            Q_EMIT error(url, reply->error(), reply->errorString());
        } else {
            QByteArray data = reply->readAll();

            qDebug() << "XML download size:" << data.size() << "bytes";

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

            // feed = channel, entry = Sendung
            processChannel(docElem, url);

            // https://gitlab.com/tabos/tvguide/-/blob/master/src/tvguide-assistant.c
        }
        delete reply;
    });
}

void Fetcher::fetchAll()
{
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT url FROM Feeds;")); // url for channel e.g. http://xmltv.xmltv.se/3sat.de
    Database::instance().execute(query);
    while (query.next()) {
        fetch(query.value(0).toString());
    }
}

void Fetcher::processChannel(const QDomElement &channel, const QString &url)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("UPDATE Feeds SET name=:name, image=:image, link=:link, description=:description, lastUpdated=:lastUpdated WHERE url=:url;"));
    query.bindValue(QStringLiteral(":name"), "3Sat"); // TODO
    query.bindValue(QStringLiteral(":url"), url);
    query.bindValue(QStringLiteral(":link"), url);
    query.bindValue(QStringLiteral(":description"), ""); // TODO

    QDateTime current = QDateTime::currentDateTime();
    query.bindValue(QStringLiteral(":lastUpdated"), current.toSecsSinceEpoch());

    /* for (auto &author : feed->authors()) {
         processAuthor(author, QLatin1String(""), url);
     }*/

    /*QString image;
    if (feed->image()->url().startsWith(QStringLiteral("/"))) {
        image = QUrl(url).adjusted(QUrl::RemovePath).toString() + feed->image()->url();
    } else {
        image = feed->image()->url();
    }
    query.bindValue(QStringLiteral(":image"), image);
    Database::instance().execute(query);

    qDebug() << "Updated feed title:" << feed->title();*/

    Q_EMIT feedDetailsUpdated(url, "3Sat", "", url, "", current); // TODO

    QDomNodeList programs = channel.elementsByTagName("programme");
    for (int i = 0; i < programs.count(); i++) {
        processProgram(programs.at(i), url);
    }

    Q_EMIT feedUpdated(url);
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
    const QString &description = program.namedItem("description").toElement().text();
    const QString &category = program.namedItem("category").toElement().text();

    qDebug() << "Processing" << title;
    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT COUNT (id) FROM Entries WHERE id=:id;"));
    query.bindValue(QStringLiteral(":id"), id);
    Database::instance().execute(query);
    query.next();

    if (query.value(0).toInt() != 0) {
        return;
    }

    query.prepare(QStringLiteral("INSERT INTO Entries VALUES (:feed, :id, :title, :content, :created, :updated, :link, 0);"));
    query.bindValue(QStringLiteral(":feed"), url);
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":title"), title);
    query.bindValue(QStringLiteral(":created"), startTime.toSecsSinceEpoch());
    query.bindValue(QStringLiteral(":updated"), stopTime.toSecsSinceEpoch());
    query.bindValue(QStringLiteral(":link"), url);

    query.bindValue(QStringLiteral(":content"), description);

    Database::instance().execute(query);

    // for (const auto &author : entry->authors()) {
    //    processAuthor(url, id);
    //}

    // for (const auto &enclosure : entry->enclosures()) {
    //    processEnclosure(url, id);
    //}*/
}

void Fetcher::processAuthor(const QString &url, unsigned int id)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("INSERT INTO Authors VALUES(:feed, :id, :name, :uri, :email);"));
    query.bindValue(QStringLiteral(":feed"), url);
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":name"), "Author"); // TODO
    query.bindValue(QStringLiteral(":uri"), "URI"); // TODO
    query.bindValue(QStringLiteral(":email"), "author@example.com"); // TODO
    Database::instance().execute(query);
}

void Fetcher::processEnclosure(const QString &feedUrl, unsigned int id)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("INSERT INTO Enclosures VALUES (:feed, :id, :duration, :size, :title, :type, :url);"));
    query.bindValue(QStringLiteral(":feed"), feedUrl);
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":duration"), 3); // TODO
    query.bindValue(QStringLiteral(":size"), 500); // TODO
    query.bindValue(QStringLiteral(":title"), "Title"); // TODO
    query.bindValue(QStringLiteral(":type"), "Type"); // TODO
    query.bindValue(QStringLiteral(":url"), feedUrl); // TODO
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
