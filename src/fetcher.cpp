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
#include <QtXml>

#include <Syndication/Syndication>

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
    const QString urlToday = url + "_2021-07-27.xml"; // TODO: get date
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

      QDomNodeList nodes = versionXML.elementsByTagName("title");
      qDebug() << "programs";
      QVector<QString> programs;
      for (int i = 0; i < nodes.count(); i++) {
        QDomNode elm = nodes.at(i);
        if (elm.isElement()) {
          qDebug() << elm.toElement().tagName() << " = "
                   << elm.toElement().text();
          programs.push_back(elm.toElement().text());
        }
      }

     // feed = channel, entry = Sendung
            processChannel(programs, url);

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

void Fetcher::processChannel(const QVector<QString>& programs, const QString &url)
{
    if (programs.isEmpty()) {
        return;
    }

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

    unsigned int id = 0;
    for (const auto &program : programs) {
        processProgram(program, id, url);
        ++id;
    }

    Q_EMIT feedUpdated(url);
}

void Fetcher::processProgram(const QString& program, unsigned int id, const QString &url)
{
    qDebug() << "Processing" << program;
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
    query.bindValue(QStringLiteral(":title"), program);
    QDateTime current = QDateTime::currentDateTime();
    query.bindValue(QStringLiteral(":created"), current.toSecsSinceEpoch()); // TODO
    query.bindValue(QStringLiteral(":updated"), current.toSecsSinceEpoch()); // TODO
    query.bindValue(QStringLiteral(":link"), url);

        query.bindValue(QStringLiteral(":content"), "Test 123"); // TODO

    Database::instance().execute(query);

    //for (const auto &author : entry->authors()) {
        processAuthor(url, id);
    //}

    //for (const auto &enclosure : entry->enclosures()) {
        processEnclosure(url, id);
    //}*/
}

void Fetcher::processAuthor(const QString &url, unsigned int id)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("INSERT INTO Authors VALUES(:feed, :id, :name, :uri, :email);"));
    query.bindValue(QStringLiteral(":feed"), url);
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":name"), "Author"); // TODO
    query.bindValue(QStringLiteral(":uri"), "URI");// TODO
    query.bindValue(QStringLiteral(":email"), "author@example.com");// TODO
    Database::instance().execute(query);
}

void Fetcher::processEnclosure(const QString &feedUrl, unsigned int id)
{
    QSqlQuery query;
    query.prepare(QStringLiteral("INSERT INTO Enclosures VALUES (:feed, :id, :duration, :size, :title, :type, :url);"));
    query.bindValue(QStringLiteral(":feed"), feedUrl);
    query.bindValue(QStringLiteral(":id"), id);
    query.bindValue(QStringLiteral(":duration"), 3);// TODO
    query.bindValue(QStringLiteral(":size"), 500);// TODO
    query.bindValue(QStringLiteral(":title"), "Title");// TODO
    query.bindValue(QStringLiteral(":type"), "Type");// TODO
    query.bindValue(QStringLiteral(":url"), feedUrl);// TODO
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
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/") + QString::fromStdString(QCryptographicHash::hash(url.toUtf8(), QCryptographicHash::Md5).toHex().toStdString());
}

QNetworkReply *Fetcher::get(QNetworkRequest &request)
{
    request.setRawHeader("User-Agent", "telly-scout/0.1; Syndication");
    return manager->get(request);
}
