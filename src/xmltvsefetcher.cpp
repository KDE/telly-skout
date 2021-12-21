#include "xmltvsefetcher.h"

#include "database.h"

#include <QDateTime>
#include <QDebug>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QString>
#include <QtXml>

XmlTvSeFetcher::XmlTvSeFetcher()
{
}

void XmlTvSeFetcher::fetchFavorites()
{
    qDebug() << "Starting to fetch favorites";

    Q_EMIT startedFetchingFavorites();

    QSqlQuery query;
    query.prepare(QStringLiteral("SELECT channel FROM Favorites;"));
    Database::instance().execute(query);
    while (query.next()) {
        const ChannelId channelId = ChannelId(query.value(QStringLiteral("channel")).toString());
        fetchProgram(channelId);
    }

    Q_EMIT finishedFetchingFavorites();
}

void XmlTvSeFetcher::fetchCountries()
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
            Q_EMIT errorFetching(Error(reply->error(), reply->errorString()));
        } else {
            QByteArray data = reply->readAll();

            QDomDocument versionXML;

            if (!versionXML.setContent(data)) {
                qWarning() << "Failed to parse XML";
            }

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

void XmlTvSeFetcher::fetchCountry(const QString &url, const CountryId &countryId)
{
    qDebug() << "Starting to fetch country (" << countryId.value() << ", " << url << ")";

    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, url, countryId, reply]() {
        if (reply->error()) {
            qWarning() << "Error fetching country";
            qWarning() << reply->errorString();
            Q_EMIT errorFetchingCountry(countryId, Error(reply->error(), reply->errorString()));
        } else {
            QByteArray data = reply->readAll();

            QDomDocument versionXML;

            if (!versionXML.setContent(data)) {
                qWarning() << "Failed to parse XML";
            }

            QDomNodeList nodes = versionXML.elementsByTagName("channel");

            for (int i = 0; i < nodes.count(); i++) {
                QDomNode elm = nodes.at(i);
                if (elm.isElement()) {
                    const QDomNamedNodeMap &attributes = elm.attributes();
                    const ChannelId id = ChannelId(attributes.namedItem("id").toAttr().value());

                    const QString &name = elm.firstChildElement("display-name").text();

                    fetchChannel(id, name, countryId);
                }
            }
        }
        delete reply;
        Q_EMIT countryUpdated(countryId);
    });
}

void XmlTvSeFetcher::fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url)
{
    Q_UNUSED(channelId)
    Q_UNUSED(programId)
    Q_UNUSED(url)

    // nothing to be done (already fetched as part of the program)
}

void XmlTvSeFetcher::fetchChannel(const ChannelId &channelId, const QString &name, const CountryId &countryId)
{
    const QString url = "http://xmltv.xmltv.se/" + channelId.value();

    Q_EMIT startedFetchingChannel(channelId);

    // story channel per country (ignore if it exists already)
    QSqlQuery countryQuery;
    countryQuery.prepare(QStringLiteral("INSERT OR IGNORE INTO CountryChannels VALUES (:id, :country, :channel);"));
    countryQuery.bindValue(QStringLiteral(":id"), countryId.value() + "_" + channelId.value());
    countryQuery.bindValue(QStringLiteral(":country"), countryId.value());
    countryQuery.bindValue(QStringLiteral(":channel"), channelId.value());
    Database::instance().execute(countryQuery);

    // if channel is unknown, store it
    QSqlQuery queryChannelExists;
    queryChannelExists.prepare(QStringLiteral("SELECT COUNT (id) FROM Channels WHERE id=:id;"));
    queryChannelExists.bindValue(QStringLiteral(":id"), channelId.value());
    Database::instance().execute(queryChannelExists);
    queryChannelExists.next();

    if (queryChannelExists.value(0).toInt() == 0) {
        ChannelData data;
        data.m_id = channelId;
        data.m_name = name;
        data.m_url = url;
        data.m_image = "https://gitlab.com/xmltv-se/open-source/channel-logos/-/raw/master/vector/" + channelId.value() + "_color.svg?inline=false";
        Database::instance().addChannel(data, countryId);
    }

    Q_EMIT channelUpdated(channelId);
}

void XmlTvSeFetcher::fetchProgram(const ChannelId &channelId)
{
    const QString url = "http://xmltv.xmltv.se/" + channelId.value();

    QDate today = QDate::currentDate();
    QDate yesterday = QDate::currentDate().addDays(-1);
    QDate tomorrow = QDate::currentDate().addDays(1);
    QSet<QDate> days{yesterday, today, tomorrow};
    for (auto day : days) {
        // check if program is available already
        const QDateTime utcTime(day, QTime(), Qt::UTC);
        const qint64 lastTime = utcTime.addDays(1).toSecsSinceEpoch() - 1;

        if (Database::instance().programExists(channelId, lastTime)) {
            continue;
        }

        const QString urlDay = url + "_" + day.toString("yyyy-MM-dd") + ".xml"; // e.g. http://xmltv.xmltv.se/3sat.de_2021-07-29.xml
        qDebug() << "Starting to fetch program for " << channelId.value() << "(" << urlDay << ")";

        QNetworkRequest request((QUrl(urlDay)));
        QNetworkReply *reply = get(request);
        connect(reply, &QNetworkReply::finished, this, [this, channelId, url, reply]() {
            if (reply->error()) {
                qWarning() << "Error fetching channel";
                qWarning() << reply->errorString();
                Q_EMIT errorFetchingChannel(channelId, Error(reply->error(), reply->errorString()));
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

void XmlTvSeFetcher::processCountry(const QDomElement &country)
{
    const CountryId id = CountryId(country.attributes().namedItem("id").toAttr().value());
    const QString &name = country.text();

    Q_EMIT startedFetchingCountry(id);

    // http://xmltv.xmltv.se/channels-Germany.xml
    const QString url = "http://xmltv.xmltv.se/channels-" + id.value() + ".xml";

    // if country is unknown, store it
    QSqlQuery queryCountryExists;
    queryCountryExists.prepare(QStringLiteral("SELECT COUNT () FROM Channels WHERE id=:id;"));
    queryCountryExists.bindValue(QStringLiteral(":id"), id.value());
    Database::instance().execute(queryCountryExists);
    queryCountryExists.next();

    if (queryCountryExists.value(0).toInt() == 0) {
        Database::instance().addCountry(id, name, url);
    }

    Q_EMIT countryUpdated(id);
}

void XmlTvSeFetcher::processChannel(const QDomElement &channel, const QString &url)
{
    QDomNodeList programs = channel.elementsByTagName("programme");
    const QDomNamedNodeMap &attributes = programs.at(0).attributes();

    const ChannelId channelId = ChannelId(attributes.namedItem("channel").toAttr().value());
    if (programs.count() > 0) {
        for (int i = 0; i < programs.count(); i++) {
            processProgram(programs.at(i), url);
        }

        Q_EMIT channelUpdated(channelId);
    }
}

void XmlTvSeFetcher::processProgram(const QDomNode &program, const QString &url)
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
