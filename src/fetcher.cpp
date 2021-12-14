/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "fetcher.h"

#include "database.h"

#include <KLocalizedString>

#include <QCryptographicHash>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QVector>
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

    const QVector<QString> favoriteChannels = Database::instance().favoriteChannels();
    for (int i = 0; i < favoriteChannels.length(); i++) {
        fetchProgram(favoriteChannels.at(i));
    }

    Q_EMIT finishedFetchingFavorites();
}

void Fetcher::fetchCountries()
{
    const QString id = "tvspielfilm.germany";
    const QString name = i18n("Germany");

    Q_EMIT startedFetchingCountry(id);

    const QString url = "https://www.tvspielfilm.de/tv-programm/sendungen";

    Database::instance().addCountry(id, name, url);

    Q_EMIT countryUpdated(id);
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

            QRegularExpression re("<select name=\\\"channel\\\">.*</select>");
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
                        const QString &id = attributes.namedItem("value").toAttr().value();

                        // exclude groups (e.g. "alle Sender" or "g:1")
                        if (id.length() > 0 && !id.contains("g:")) {
                            const QString &name = channelNode.toElement().text();
                            fetchChannel(id, name, countryId);
                        }
                    }
                }
            }
        }
        delete reply;
        Q_EMIT countryUpdated(countryId);
    });
}

void Fetcher::fetchChannel(const QString &channelId, const QString &name, const QString &country)
{
    ChannelData data;
    data.m_id = channelId;

    // https://www.tvspielfilm.de/tv-programm/sendungen/das-erste,ARD.html
    data.m_url = "https://www.tvspielfilm.de/tv-programm/sendungen/" + name.toLower().replace(' ', '-') + "," + channelId + ".html";

    Q_EMIT startedFetchingChannel(data.m_id);

    // TODO: https://a2.tvspielfilm.de/images/tv/sender/mini/sprite_web_optimized_1616508904.webp
    data.m_image = "https://a2.tvspielfilm.de/images/tv/sender/mini/" + channelId.toLower() + ".webp";
    Database::instance().addChannel(data, country);

    Q_EMIT channelUpdated(channelId);
}

void Fetcher::fetchProgram(const QString &channelId)
{
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

        // https://www.tvspielfilm.de/tv-programm/sendungen/?date=2021-11-09&time=day&channel=ARD
        const QString url = "https://www.tvspielfilm.de/tv-programm/sendungen/?time=day&channel=" + channelId;
        const QString urlDay = url + "&date=" + day.toString("yyyy-MM-dd") + "&page=1";
        fetchProgram(channelId, urlDay);
    }
}

void Fetcher::fetchProgram(const QString &channelId, const QString &url)
{
    qDebug() << "Starting to fetch program for " << channelId << "(" << url << ")";

    QNetworkRequest request((QUrl(url)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, channelId, url, reply]() {
        if (reply->error()) {
            qWarning() << "Error fetching channel";
            qWarning() << reply->errorString();
            Q_EMIT error(channelId, reply->error(), reply->errorString());
        } else {
            QByteArray data = reply->readAll();
            processChannel(data, url, channelId);

            // fetch next page
            QRegularExpression reNextPage(
                "<ul class=\\\"pagination__items\\\">.*<a href=\\\"(.*?)\\\"\\s*class=\\\"js-track-link pagination__link pagination__link--next\\\"");
            reNextPage.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
            QRegularExpressionMatch matchNextPage = reNextPage.match(data);
            if (matchNextPage.hasMatch()) {
                fetchProgram(channelId, matchNextPage.captured(1));
            } else {
                // all pages processed, update GUI
                Q_EMIT channelUpdated(channelId);
            }
        }
        delete reply;
    });
}

void Fetcher::fetchDescription(const QString &channelId, const QString &programId, const QString &descriptionUrl, bool isLast)
{
    qDebug() << "Starting to fetch description for" << programId << "(" << descriptionUrl << ")";
    QNetworkRequest request((QUrl(descriptionUrl)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, channelId, programId, descriptionUrl, reply, isLast]() {
        if (reply->error()) {
            qWarning() << "Error fetching program description";
            qWarning() << reply->errorString();
        } else {
            QByteArray data = reply->readAll();
            processDescription(data, descriptionUrl, programId);

            // update GUI only for last description
            // updating for every description is too expensive
            if (isLast) {
                Q_EMIT channelUpdated(channelId);
            }
        }
        delete reply;
    });
}

void Fetcher::processChannel(const QString &infoTable, const QString &url, const QString &channelId)
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
        programs.push_back(processProgram(match, url, channelId, !it.hasNext()));
    }

    Database::instance().addPrograms(programs);
}

ProgramData Fetcher::processProgram(const QRegularExpressionMatch &programMatch, const QString &url, const QString &channelId, bool isLast)
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
        const QString programId = channelId + "_" + QString::number(startTime.toSecsSinceEpoch());

        programData.m_id = programId;
        programData.m_url = descriptionUrl;
        programData.m_channelId = channelId;
        programData.m_startTime = startTime;
        programData.m_stopTime = stopTime;
        programData.m_title = title;
        programData.m_subtitle = "";
        programData.m_description = "";
        programData.m_category = category;

        // TODO on demand? (way too slow)
        // fetchDescription(channelId, programId, descriptionUrl, isLast);
    } else {
        qWarning() << "Failed to parse program " << url;
    }

    return programData;
}

void Fetcher::processDescription(const QString &descriptionPage, const QString &url, const QString &programId)
{
    QRegularExpression reDescription("<section class=\\\"broadcast-detail__description\\\">.*?<p>(.*?)</p>");
    reDescription.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = reDescription.match(descriptionPage);
    if (match.hasMatch()) {
        const QString description = match.captured(1);

        Database::instance().updateProgramDescription(programId, description);
    } else {
        qWarning() << "Failed to parse program description from" << url;
    }
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
