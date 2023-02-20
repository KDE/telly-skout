// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "tvspielfilmfetcher.h"

#include "TellySkoutSettings.h"
#include "database.h"

#include <KLocalizedString>

#include <QDate>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>
#include <QString>
#include <QTimeZone>

TvSpielfilmFetcher::TvSpielfilmFetcher(QNetworkAccessManager *nam)
    : NetworkFetcher(nam)
{
}

void TvSpielfilmFetcher::fetchGroups(std::function<void(const QVector<GroupData> &)> callback, std::function<void(const Error &)> errorCallback)
{
    Q_UNUSED(errorCallback);

    QVector<GroupData> groups;
    GroupData data;
    data.m_id = GroupId("tvspielfilm.germany");
    data.m_name = i18n("Germany");
    data.m_url = "https://www.tvspielfilm.de/tv-programm/sendungen";

    groups.push_back(data);

    if (callback) {
        callback(groups);
    }
}

void TvSpielfilmFetcher::fetchGroup(const QString &url,
                                    const GroupId &groupId,
                                    std::function<void(const QList<ChannelData> &)> callback,
                                    std::function<void(const Error &)> errorCallback)
{
    qDebug() << "Starting to fetch group (" << groupId.value() << ", " << url << ")";

    m_provider.get(
        QUrl(url),
        [this, callback](QByteArray data) {
            static QRegularExpression reChannelList("<select name=\\\"channel\\\">.*</select>");
            reChannelList.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch matchChannelList = reChannelList.match(data);
            if (matchChannelList.hasMatch()) {
                QMap<ChannelId, ChannelData> channels;
                const QString channelList = matchChannelList.captured(0);

                static QRegularExpression reChannel("<option.*?value=\\\"(.*?)\\\">&nbsp;&nbsp;(.*?)</option>");
                reChannel.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
                QRegularExpressionMatchIterator it = reChannel.globalMatch(channelList);
                while (it.hasNext()) {
                    QRegularExpressionMatch channelMatch = it.next();
                    const ChannelId id = ChannelId(channelMatch.captured(1));

                    // exclude groups (e.g. "alle Sender" or "g:1")
                    if (id.value().length() > 0 && !id.value().contains("g:")) {
                        const QString name = channelMatch.captured(2);
                        fetchChannel(id, name, channels);
                    }
                }

                if (callback) {
                    callback(channels.values());
                }
            }
        },
        [groupId, errorCallback](const Error &error) {
            qWarning() << "Error fetching group" << groupId.value();
            qWarning() << error.m_message;

            if (errorCallback) {
                errorCallback(error);
            }
        });
}

void TvSpielfilmFetcher::fetchChannel(const ChannelId &channelId, const QString &name, QMap<ChannelId, ChannelData> &channels)
{
    if (!channels.contains(channelId)) {
        ChannelData data;
        data.m_id = channelId;
        data.m_name = name;

        // https://www.tvspielfilm.de/tv-programm/sendungen/das-erste,ARD.html
        data.m_url = "https://www.tvspielfilm.de/tv-programm/sendungen/" + name.toLower().replace(' ', '-') + "," + channelId.value() + ".html";

        data.m_image = "https://a2.tvspielfilm.de/images/tv/sender/mini/" + channelId.value().toLower() + ".png";

        channels.insert(channelId, data);
    }
}

void TvSpielfilmFetcher::fetchProgramDescription(const ChannelId &channelId,
                                                 const ProgramId &programId,
                                                 const QString &url,
                                                 std::function<void(const QString &)> callback,
                                                 std::function<void(const Error &)> errorCallback)
{
    qDebug() << "Starting to fetch description for" << programId.value() << "(" << url << ")";

    m_provider.get(
        QUrl(url),
        [this, channelId, programId, url, callback](const QByteArray &data) {
            if (callback) {
                callback(processDescription(data, url));
            }
        },
        [channelId, programId, url, errorCallback](const Error &error) {
            qWarning() << "Error fetching program description for" << channelId.value() << "," << programId.value() << "(" << url << "):";
            qWarning() << error.m_message;

            if (errorCallback) {
                errorCallback(error);
            }
        });
}

void TvSpielfilmFetcher::fetchProgram(const ChannelId &channelId,
                                      std::function<void(const QVector<ProgramData> &)> callback,
                                      std::function<void(const Error &)> errorCallback)
{
    // backwards such that we can stop early (see below)
    const QDate lastDate = QDate::currentDate().addDays(TellySkoutSettings::tvSpielfilmPrefetch());

    QVector<ProgramData> programs;
    if (programExists(channelId, lastDate)) {
        // assume that programs from previous days are available
        if (callback) {
            callback(programs);
        }
    } else {
        fetchProgram(channelId, lastDate, 1, programs, callback, errorCallback);
    }
}

void TvSpielfilmFetcher::fetchProgram(const ChannelId &channelId,
                                      const QDate &date,
                                      unsigned int page,
                                      QVector<ProgramData> &programs,
                                      std::function<void(const QVector<ProgramData> &)> callback,
                                      std::function<void(const Error &)> errorCallback)
{
    // https://www.tvspielfilm.de/tv-programm/sendungen/?time=day&channel=ARD&date=2021-11-09&page=1
    const QString url = "https://www.tvspielfilm.de/tv-programm/sendungen/?time=day&channel=" + channelId.value() + "&date=" + date.toString("yyyy-MM-dd")
        + "&page=" + QString::number(page);

    qDebug() << "Starting to fetch program for " << channelId.value() << "(" << url << ")";

    m_provider.get(
        QUrl(url),
        [this, channelId, date, page, programs, callback, errorCallback, url](QByteArray data) {
            QVector<ProgramData> allPrograms(programs);
            allPrograms.append(processChannel(data, url, channelId));

            // fetch next page
            static QRegularExpression reNextPage(
                "<ul class=\\\"pagination__items\\\">.*</ul>\\s*<a href=\\\"(.*?)\\\".*class=\\\"js-track-link pagination__link pagination__link--next\\\"");
            reNextPage.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch matchNextPage = reNextPage.match(data);
            if (matchNextPage.hasMatch()) {
                fetchProgram(channelId, date, page + 1, allPrograms, callback, errorCallback);
            } else {
                // all pages for this day processed, continue with previous day (stop yesterday)
                const QDate previousDay = date.addDays(-1);
                if (QDate::currentDate().addDays(-1) <= previousDay && !programExists(channelId, previousDay)) {
                    fetchProgram(channelId, previousDay, 1, allPrograms, callback, errorCallback);
                } else {
                    // all pages for all days processed
                    if (callback) {
                        callback(allPrograms);
                    }
                }
            }
        },
        [channelId, errorCallback](const Error &error) {
            qWarning() << "Error fetching channel" << channelId.value();
            qWarning() << error.m_message;

            if (errorCallback) {
                errorCallback(error);
            }
        });
}

QVector<ProgramData> TvSpielfilmFetcher::processChannel(const QString &infoTable, const QString &url, const ChannelId &channelId)
{
    QVector<ProgramData> programs;

    // column with title + description URL + start/stop time
    const QString reDescriptionUrl("<a href=\\\"(https://www.tvspielfilm.de/tv-programm/sendung/.*?\\.html)\\\"");
    const QString reTitle("<strong>(.*?)</strong>");
    const QString reDateTime("class=\\\"progressbar-info\\\".*?data-rel-start=\\\"(\\d+)\\\".*?data-rel-end=\\\"(\\d+)\\\"");
    const QString reMainCol("<td class=\\\"col-3\\\">.*?" + reDescriptionUrl + ".*?" + reTitle + ".*?" + reDateTime + ".*?</td>");

    // column with category
    const QString reCategory("<span>(.*?)</span>");
    const QString reCategoryCol("<td class=\\\"col-4\\\">.*?" + reCategory + ".*?</td>");

    QRegularExpression reProgram("<tr class=\\\"hover\\\">.*?" + reMainCol + ".*?" + reCategoryCol + ".*?</tr>");
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
        const QString descriptionUrl = programMatch.captured(1);
        const QString title = programMatch.captured(2);

        const QDateTime startTime = QDateTime::fromSecsSinceEpoch(programMatch.captured(3).toInt());
        const QDateTime stopTime = QDateTime::fromSecsSinceEpoch(programMatch.captured(4).toInt());

        const QString category = programMatch.captured(5);

        // channel + start time can be used as ID
        const ProgramId programId = ProgramId(channelId.value() + "_" + QString::number(startTime.toSecsSinceEpoch()));

        programData.m_id = programId;
        programData.m_url = descriptionUrl;
        programData.m_channelId = channelId;
        programData.m_startTime = startTime.toLocalTime();
        programData.m_stopTime = stopTime.toLocalTime();
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

QString TvSpielfilmFetcher::processDescription(const QString &descriptionPage, const QString &url)
{
    static QRegularExpression reDescription("<section class=\\\"broadcast-detail__description\\\">.*?<p>(.*?)</p>");
    reDescription.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = reDescription.match(descriptionPage);
    if (match.hasMatch()) {
        return match.captured(1);
    } else {
        qWarning() << "Failed to parse program description from" << url;
    }
    return "";
}

bool TvSpielfilmFetcher::programExists(const ChannelId &channelId, const QDate &date)
{
    const QDateTime lastTime(date, QTime(23, 59, 59));

    return Database::instance().programExists(channelId, lastTime);
}
