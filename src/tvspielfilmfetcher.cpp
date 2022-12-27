// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#include "tvspielfilmfetcher.h"

#include "database.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QDebug>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QString>
#include <QtXml>

TvSpielfilmFetcher::TvSpielfilmFetcher()
{
}

void TvSpielfilmFetcher::fetchGroups()
{
    const GroupId id = GroupId("tvspielfilm.germany");
    const QString name = i18n("Germany");

    Q_EMIT startedFetchingGroup(id);

    const QString url = "https://www.tvspielfilm.de/tv-programm/sendungen";

    Database::instance().addGroup(id, name, url);

    Q_EMIT groupUpdated(id);
}

void TvSpielfilmFetcher::fetchGroup(const QString &url, const GroupId &groupId)
{
    qDebug() << "Starting to fetch group (" << groupId.value() << ", " << url << ")";

    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, url, groupId, reply]() {
        if (reply->error()) {
            qWarning() << "Error fetching group";
            qWarning() << reply->errorString();
            Q_EMIT errorFetchingGroup(groupId, Error(reply->error(), reply->errorString()));
        } else {
            QByteArray data = reply->readAll();

            static QRegularExpression re("<select name=\\\"channel\\\">.*</select>");
            re.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch match = re.match(data);
            if (match.hasMatch()) {
                const QString matched = match.captured(0);

                QDomDocument channelsXml;

                if (!channelsXml.setContent(matched)) {
                    qWarning() << "Failed to parse XML";
                }

                QDomNodeList channelNodes = channelsXml.elementsByTagName("option");

                for (int i = 0; i < channelNodes.count(); i++) {
                    QDomNode channelNode = channelNodes.at(i);
                    if (channelNode.isElement()) {
                        const QDomNamedNodeMap &attributes = channelNode.attributes();
                        const ChannelId id = ChannelId(attributes.namedItem("value").toAttr().value());

                        // exclude groups (e.g. "alle Sender" or "g:1")
                        if (id.value().length() > 0 && !id.value().contains("g:")) {
                            const QString &name = channelNode.toElement().text();
                            fetchChannel(id, name, groupId);
                        }
                    }
                }
            }
        }
        delete reply;
        Q_EMIT groupUpdated(groupId);
    });
}

void TvSpielfilmFetcher::fetchChannel(const ChannelId &channelId, const QString &name, const GroupId &group)
{
    if (!Database::instance().channelExists(channelId)) {
        ChannelData data;
        data.m_id = channelId;
        data.m_name = name;

        // https://www.tvspielfilm.de/tv-programm/sendungen/das-erste,ARD.html
        data.m_url = "https://www.tvspielfilm.de/tv-programm/sendungen/" + name.toLower().replace(' ', '-') + "," + channelId.value() + ".html";

        Q_EMIT startedFetchingChannel(data.m_id);

        data.m_image = "https://a2.tvspielfilm.de/images/tv/sender/mini/" + channelId.value().toLower() + ".webp";
        Database::instance().addChannel(data, group);

        Q_EMIT channelUpdated(channelId);
    }
}

void TvSpielfilmFetcher::fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url)
{
    qDebug() << "Starting to fetch description for" << programId.value() << "(" << url << ")";
    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, channelId, programId, url, reply]() {
        if (reply->error()) {
            qWarning() << "Error fetching program description";
            qWarning() << reply->errorString();
        } else {
            QByteArray data = reply->readAll();
            processDescription(data, url, programId);

            Q_EMIT channelUpdated(channelId);
        }
        delete reply;
    });
}

void TvSpielfilmFetcher::fetchProgram(const ChannelId &channelId)
{
    QDate today = QDate::currentDate();
    QDate yesterday = QDate::currentDate().addDays(-1);
    QDate tomorrow = QDate::currentDate().addDays(1);
    const QVector<QDate> days{tomorrow, today, yesterday}; // backwards such that we can stop early (see below)
    for (auto day : days) {
        // check if program is available already
        const QDateTime utcTime(day, QTime(), Qt::UTC);
        const qint64 lastTime = utcTime.addDays(1).toSecsSinceEpoch() - 1;

        if (Database::instance().programExists(channelId, lastTime)) {
            return; // assume that programs from previous days are available
        }

        // https://www.tvspielfilm.de/tv-programm/sendungen/?date=2021-11-09&time=day&channel=ARD
        const QString url = "https://www.tvspielfilm.de/tv-programm/sendungen/?time=day&channel=" + channelId.value();
        const QString urlDay = url + "&date=" + day.toString("yyyy-MM-dd") + "&page=1";
        QVector<ProgramData> programs;
        fetchProgram(channelId, urlDay, programs);
    }
}

void TvSpielfilmFetcher::fetchProgram(const ChannelId &channelId, const QString &url, QVector<ProgramData> &programs)
{
    qDebug() << "Starting to fetch program for " << channelId.value() << "(" << url << ")";

    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, channelId, url, programs, reply]() {
        if (reply->error()) {
            qWarning() << "Error fetching channel";
            qWarning() << reply->errorString();
            Q_EMIT errorFetchingChannel(channelId, Error(reply->error(), reply->errorString()));
        } else {
            QByteArray data = reply->readAll();
            QVector<ProgramData> allPrograms(programs);
            allPrograms.append(processChannel(data, url, channelId));

            // fetch next page
            static QRegularExpression reNextPage(
                "<ul class=\\\"pagination__items\\\">.*</ul>\\s*<a href=\\\"(.*?)\\\".*class=\\\"js-track-link pagination__link pagination__link--next\\\"");
            reNextPage.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch matchNextPage = reNextPage.match(data);
            if (matchNextPage.hasMatch()) {
                fetchProgram(channelId, matchNextPage.captured(1), allPrograms);
            } else {
                // all pages processed, update DB + GUI
                Database::instance().addPrograms(allPrograms);
                Q_EMIT channelUpdated(channelId);
            }
        }
        delete reply;
    });
}

QVector<ProgramData> TvSpielfilmFetcher::processChannel(const QString &infoTable, const QString &url, const ChannelId &channelId)
{
    QVector<ProgramData> programs;

    // column with date and time
    const QString reTime("<strong>(\\d\\d:\\d\\d) - (\\d\\d:\\d\\d)</strong>");
    const QString reDate("<span>.*? (\\d\\d\\.\\d\\d\\.)</span>");
    const QString reDateTimeCol("<td class=\\\"col-2\\\">.*?" + reTime + ".*?" + reDate + ".*?</td>");

    // column with title + description URL
    const QString reDescriptionUrl("<a href=\\\"(https://www.tvspielfilm.de/tv-programm/sendung/.*?\\.html)\\\"");
    const QString reTitle("<strong>(.*?)</strong>");
    const QString reTitleCol("<td class=\\\"col-3\\\">.*?" + reDescriptionUrl + ".*?" + reTitle + ".*?</td>");

    // column with category
    const QString reCategory("<span>(.*?)</span>");
    const QString reCategoryCol("<td class=\\\"col-4\\\">.*?" + reCategory + ".*?</td>");

    QRegularExpression reProgram("<tr class=\\\"hover\\\">.*?" + reDateTimeCol + ".*?" + reTitleCol + ".*?" + reCategoryCol + ".*?</tr>");
    reProgram.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = reProgram.globalMatch(infoTable);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        const ProgramData programData = processProgram(match, url, channelId, !it.hasNext());
        if (!programs.empty()) {
            // sometimes, there can be multiple programs for the same time (e.g. different news per region of a local channel)
            // show this as alternative in the title
            ProgramData &previousProgamData = programs.last();
            if (programData.m_startTime < previousProgamData.m_stopTime) {
                previousProgamData.m_title += " / " + programData.m_title;
            } else {
                programs.push_back(programData);
            }
        } else {
            programs.push_back(programData);
        }
    }

    return programs;
}

ProgramData TvSpielfilmFetcher::processProgram(const QRegularExpressionMatch &programMatch, const QString &url, const ChannelId &channelId, bool isLast)
{
    Q_UNUSED(isLast)

    ProgramData programData;

    if (programMatch.hasMatch()) {
        const QString date = programMatch.captured(3);
        const QDateTime startTime = QDateTime::fromString(QString::number(QDate::currentDate().year()) + date + programMatch.captured(1), "yyyydd.MM.HH:mm");
        QDateTime stopTime = QDateTime::fromString(QString::number(QDate::currentDate().year()) + date + programMatch.captured(2), "yyyydd.MM.HH:mm");
        // ends after midnight
        if (stopTime < startTime) {
            stopTime = stopTime.addDays(1);
        }
        const QString descriptionUrl = programMatch.captured(4);
        const QString title = programMatch.captured(5);
        const QString category = programMatch.captured(6);

        // channel + start time can be used as ID
        const ProgramId programId = ProgramId(channelId.value() + "_" + QString::number(startTime.toSecsSinceEpoch()));

        programData.m_id = programId;
        programData.m_url = descriptionUrl;
        programData.m_channelId = channelId;
        programData.m_startTime = startTime;
        programData.m_stopTime = stopTime;
        programData.m_title = title;
        programData.m_subtitle = "";
        programData.m_description = "";
        programData.m_descriptionFetched = false;
        programData.m_categories.push_back(category);
    } else {
        qWarning() << "Failed to parse program " << url;
    }

    return programData;
}

void TvSpielfilmFetcher::processDescription(const QString &descriptionPage, const QString &url, const ProgramId &programId)
{
    static QRegularExpression reDescription("<section class=\\\"broadcast-detail__description\\\">.*?<p>(.*?)</p>");
    reDescription.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = reDescription.match(descriptionPage);
    if (match.hasMatch()) {
        const QString description = match.captured(1);

        Database::instance().updateProgramDescription(programId, description);
    } else {
        qWarning() << "Failed to parse program description from" << url;
    }
}
