// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#include "xmltvsefetcher.h"

#include "database.h"
#include "programdata.h"

#include <QDateTime>
#include <QDebug>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QString>
#include <QtXml>

XmlTvSeFetcher::XmlTvSeFetcher()
{
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
    if (!Database::instance().channelExists(channelId)) {
        const QString url = "http://xmltv.xmltv.se/" + channelId.value();

        Q_EMIT startedFetchingChannel(channelId);

        ChannelData data;
        data.m_id = channelId;
        data.m_name = name;
        data.m_url = url;
        data.m_image = "https://gitlab.com/xmltv-se/open-source/channel-logos/-/raw/master/vector/" + channelId.value() + "_color.svg?inline=false";
        Database::instance().addChannel(data, countryId);

        Q_EMIT channelUpdated(channelId);
    }
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
        connect(reply, &QNetworkReply::finished, this, [this, channelId, reply]() {
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

                processChannel(docElem);
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

    Database::instance().addCountry(id, name, url);

    Q_EMIT countryUpdated(id);
}

void XmlTvSeFetcher::processChannel(const QDomElement &channel)
{
    QDomNodeList programs = channel.elementsByTagName("programme");
    const QDomNamedNodeMap &attributes = programs.at(0).attributes();

    const ChannelId channelId = ChannelId(attributes.namedItem("channel").toAttr().value());
    if (programs.count() > 0) {
        for (int i = 0; i < programs.count(); i++) {
            processProgram(programs.at(i));
        }

        Q_EMIT channelUpdated(channelId);
    }
}

void XmlTvSeFetcher::processProgram(const QDomNode &program)
{
    ProgramData data;

    const QDomNamedNodeMap &attributes = program.attributes();
    data.m_channelId = ChannelId(attributes.namedItem("channel").toAttr().value());
    const QString &startTimeString = attributes.namedItem("start").toAttr().value();
    QDateTime startTime = QDateTime::fromString(startTimeString, "yyyyMMddHHmmss +0000");
    startTime.setTimeSpec(Qt::UTC);
    data.m_startTime = startTime;
    // channel + start time can be used as ID
    data.m_id = ProgramId(data.m_channelId.value() + "_" + QString::number(startTime.toSecsSinceEpoch()));
    const QString &stopTimeString = attributes.namedItem("stop").toAttr().value();
    QDateTime stopTime = QDateTime::fromString(stopTimeString, "yyyyMMddHHmmss +0000");
    stopTime.setTimeSpec(Qt::UTC);
    data.m_stopTime = stopTime;
    data.m_title = program.namedItem("title").toElement().text();
    data.m_subtitle = program.namedItem("sub-title").toElement().text();
    data.m_description = program.namedItem("desc").toElement().text();
    data.m_category = program.namedItem("category").toElement().text();

    data.m_descriptionFetched = true;

    Database::instance().addProgram(data);
}
