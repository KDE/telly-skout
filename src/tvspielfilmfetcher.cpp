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
    data.m_id = GroupId(QStringLiteral("tvspielfilm.germany"));
    data.m_name = i18n("Germany");
    data.m_url = QStringLiteral("https://www.tvspielfilm.de/tv-programm/sendungen");

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
        [this, callback](const QByteArray &data) {
            static const QRegularExpression reChannelList(QStringLiteral("<select name=\\\"channel\\\">.*</select>"),
                                                          QRegularExpression::DotMatchesEverythingOption);
            const QRegularExpressionMatch matchChannelList = reChannelList.match(QString::fromUtf8(data));
            if (matchChannelList.hasMatch()) {
                QMap<ChannelId, ChannelData> channels;
                const QString channelList = matchChannelList.captured(0);

                static const QRegularExpression reChannel(
                    QStringLiteral("<option\\s+data-group=\\\"(.*?)\\\"\\s+value=\\\"(.*?)\\\">(&nbsp;&nbsp;)?(.*?)</option>"),
                    QRegularExpression::DotMatchesEverythingOption);
                QRegularExpressionMatchIterator it = reChannel.globalMatch(channelList);
                while (it.hasNext()) {
                    const QRegularExpressionMatch channelMatch = it.next();
                    const ChannelId id = ChannelId(channelMatch.captured(2));

                    // exclude groups (e.g. "alle Sender" or "g:1")
                    if (id.value().length() > 0 && !id.value().contains(QStringLiteral("g:")) && !id.value().contains(QStringLiteral("label="))) {
                        const QString name = channelMatch.captured(4);
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
        data.m_url = QStringLiteral("https://www.tvspielfilm.de/tv-programm/sendungen/") + name.toLower().replace(QLatin1Char(' '), QLatin1Char('-'))
            + QStringLiteral(",") + channelId.value() + QStringLiteral(".html");

        data.m_image = QStringLiteral("https://a2.tvspielfilm.de/images/tv/sender/mini/") + channelId.value().toLower() + QStringLiteral(".png");

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
                callback(processDescription(QString::fromUtf8(data), url));
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
    const QString url = QStringLiteral("https://www.tvspielfilm.de/tv-programm/sendungen/?time=day&channel=") + channelId.value() + QStringLiteral("&date=")
        + date.toString(QStringLiteral("yyyy-MM-dd")) + QStringLiteral("&page=") + QString::number(page);

    qDebug() << "Starting to fetch program for " << channelId.value() << "(" << url << ")";

    m_provider.get(
        QUrl(url),
        [this, channelId, date, page, programs, callback, errorCallback, url](const QByteArray &data) {
            QVector<ProgramData> allPrograms(programs);
            processChannel(QString::fromUtf8(data), url, channelId, allPrograms);

            // fetch next page
            static const QRegularExpression reNextPage(
                QStringLiteral("<ul class=\\\"pagination__items\\\">.*</ul>\\s*<a href=\\\"(.*?)\\\".*class=\\\"js-track-link pagination__link "
                               "pagination__link--next\\\""),
                QRegularExpression::DotMatchesEverythingOption);
            const QRegularExpressionMatch matchNextPage = reNextPage.match(QString::fromUtf8(data));
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

void TvSpielfilmFetcher::processChannel(const QString &infoTable, const QString &url, const ChannelId &channelId, QVector<ProgramData> &programs)
{
    // column with title + description URL + start/stop time
    const QString reDescriptionUrl(QStringLiteral("<a href=\\\"(https://www.tvspielfilm.de/tv-programm/sendung/.*?\\.html)\\\""));
    const QString reTitle(QStringLiteral("<strong>(.*?)</strong>"));
    const QString reDateTime(QStringLiteral("class=\\\"progressbar-info\\\".*?data-rel-start=\\\"(\\d+)\\\".*?data-rel-end=\\\"(\\d+)\\\""));
    const QString reMainCol(QStringLiteral("<td class=\\\"col-3\\\">.*?") + reDescriptionUrl + QStringLiteral(".*?") + reTitle + QStringLiteral(".*?")
                            + reDateTime + QStringLiteral(".*?</td>"));

    // column with category
    const QString reCategory(QStringLiteral("<span>(.*?)</span>"));
    const QString reCategoryCol(QStringLiteral("<td class=\\\"col-4\\\">.*?") + reCategory + QStringLiteral(".*?</td>"));

    static const QRegularExpression reProgram(QStringLiteral("<tr class=\\\"hover\\\">.*?") + reMainCol + QStringLiteral(".*?") + reCategoryCol
                                                  + QStringLiteral(".*?</tr>"),
                                              QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = reProgram.globalMatch(infoTable);
    while (it.hasNext()) {
        const QRegularExpressionMatch match = it.next();
        const ProgramData programData = processProgram(match, url, channelId, !it.hasNext());
        if (!programs.empty()) {
            ProgramData &previousProgamData = programs.last();
            // sometimes, there can be multiple programs for the same time (e.g. different news per region of a local channel)
            // show this as alternative in the title
            if (programData.m_startTime == previousProgamData.m_startTime) {
                previousProgamData.m_title += QStringLiteral(" / ") + programData.m_title;
            }
            // it can happen that the stop time is incorrect (i.e. after the next program has already started)
            // overwrite stop time with start time of next program
            // only if the previous program is really previous:
            // As days are fetched from future to past to be able to stop early,
            // the last program of the next day is the "previous" program of the first program of the current day.
            else if (programData.m_startTime < previousProgamData.m_stopTime && previousProgamData.m_startTime < programData.m_startTime) {
                previousProgamData.m_stopTime = programData.m_startTime;
            } else {
                programs.push_back(programData);
            }
        } else {
            programs.push_back(programData);
        }
    }
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
        const ProgramId programId = ProgramId(channelId.value() + QStringLiteral("_") + QString::number(startTime.toSecsSinceEpoch()));

        programData.m_id = programId;
        programData.m_url = descriptionUrl;
        programData.m_channelId = channelId;
        programData.m_startTime = startTime.toLocalTime();
        programData.m_stopTime = stopTime.toLocalTime();
        programData.m_title = title;
        programData.m_subtitle = QStringLiteral("");
        programData.m_description = QStringLiteral("");
        programData.m_descriptionFetched = false;
        programData.m_categories.push_back(category);
    } else {
        qWarning() << "Failed to parse program " << url;
    }

    return programData;
}

QString TvSpielfilmFetcher::processDescription(const QString &descriptionPage, const QString &url)
{
    QString description;

    // description
    {
        static const QRegularExpression re(QStringLiteral("<section class=\\\"broadcast-detail__description\\\">.*?(<p>.*?)\\s*</section>"),
                                           QRegularExpression::DotMatchesEverythingOption);
        const QRegularExpressionMatch match = re.match(descriptionPage);
        if (match.hasMatch()) {
            QString descriptionHtml = match.captured(1);
            descriptionHtml.replace(QStringLiteral("<p>"), QStringLiteral(""));
            descriptionHtml.replace(QStringLiteral("</p>"), QStringLiteral("<br>"));
            description += descriptionHtml;
        }
    }

    // original title
    {
        static const QRegularExpression re(QStringLiteral("<dt>Originaltitel:</dt>\\s*<dd>(.*?)</dd>"), QRegularExpression::DotMatchesEverythingOption);
        const QRegularExpressionMatch match = re.match(descriptionPage);
        if (match.hasMatch()) {
            if (!description.isEmpty()) {
                description += QStringLiteral("<br>");
            }
            description += i18n("Original title: %1", match.captured(1));
        }
    }

    // country
    {
        static const QRegularExpression re(QStringLiteral("<dt>Land:</dt>\\s*<dd>(.*?)</dd>"), QRegularExpression::DotMatchesEverythingOption);
        const QRegularExpressionMatch match = re.match(descriptionPage);
        if (match.hasMatch()) {
            if (!description.isEmpty()) {
                description += QStringLiteral("<br>");
            }
            description += i18n("Country: %1", match.captured(1));
        }
    }

    // year
    {
        static const QRegularExpression re(QStringLiteral("<dt>Jahr:</dt>\\s*<dd>(.*?)</dd>"), QRegularExpression::DotMatchesEverythingOption);
        const QRegularExpressionMatch match = re.match(descriptionPage);
        if (match.hasMatch()) {
            if (!description.isEmpty()) {
                description += QStringLiteral("<br>");
            }
            description += i18n("Year: %1", match.captured(1));
        }
    }

    // duration
    {
        static const QRegularExpression re(QStringLiteral("<dt>Länge:</dt>\\s*<dd>(.*?)</dd>"), QRegularExpression::DotMatchesEverythingOption);
        const QRegularExpressionMatch match = re.match(descriptionPage);
        if (match.hasMatch()) {
            if (!description.isEmpty()) {
                description += QStringLiteral("<br>");
            }
            description += i18n("Duration: %1", match.captured(1));
        }
    }

    // FSK
    {
        static const QRegularExpression re(QStringLiteral("<dt>FSK:</dt>\\s*<dd>(.*?)</dd>"), QRegularExpression::DotMatchesEverythingOption);
        const QRegularExpressionMatch match = re.match(descriptionPage);
        if (match.hasMatch()) {
            if (!description.isEmpty()) {
                description += QStringLiteral("<br>");
            }
            description += i18n("FSK: %1", match.captured(1));
        }
    }

    // cast + crew
    {
        static const QRegularExpression reActors(QStringLiteral("<dl class=\\\"actors\\\"(.*?)</dl>"), QRegularExpression::DotMatchesEverythingOption);
        const QRegularExpressionMatch actorsMatch = reActors.match(descriptionPage);
        if (actorsMatch.hasMatch()) {
            if (!description.isEmpty()) {
                description += QStringLiteral("<br>");
            }

            static const QRegularExpression re(QStringLiteral("<dt.*?>(.*?)</dt>.*?<dd.*?>(.*?)\\s*</dd>"), QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatchIterator it = re.globalMatch(actorsMatch.captured(1));
            while (it.hasNext()) {
                const QRegularExpressionMatch match = it.next();
                const QString role = match.captured(1).trimmed();
                static const QRegularExpression reNames(
                    QStringLiteral("\\s*(<a\\s*href=\\\".*?\\\"\\s*target=\\\".*?\\\"\\s*title=\\\".*?\\\">)?(.+?)(</a>)?\\s*(,|$)"),
                    QRegularExpression::DotMatchesEverythingOption);
                QString names = QStringLiteral("");
                QRegularExpressionMatchIterator namesIt = reNames.globalMatch(match.captured(2));
                while (namesIt.hasNext()) {
                    if (!names.isEmpty()) {
                        names += QStringLiteral(", ");
                    }
                    const QRegularExpressionMatch namesMatch = namesIt.next();
                    names += namesMatch.captured(2).trimmed();
                }
                if (role == QStringLiteral("&nbsp;") && names == QStringLiteral("&nbsp;")) {
                    // split cast and crew into 2 sections
                    description += QStringLiteral("<br>");
                } else {
                    if (role.endsWith(QLatin1Char(':'))) {
                        description += QStringLiteral("<br>") + role + QLatin1Char(' ') + names;
                    } else {
                        description += QStringLiteral("<br>") + role + QStringLiteral(" - ") + names;
                    }
                }
            }
        }
    }

    if (description.isEmpty()) {
        qWarning() << "Failed to parse program description from" << url;
    }

    return description;
}

bool TvSpielfilmFetcher::programExists(const ChannelId &channelId, const QDate &date)
{
    const QDateTime lastTime(date, QTime(23, 59, 59));

    return Database::instance().programExists(channelId, lastTime);
}

#include "moc_tvspielfilmfetcher.cpp"
