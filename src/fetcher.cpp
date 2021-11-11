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

void Fetcher::fetchCountries()
{
    const QString id = "tvspielfilm.germany";
    const QString name = i18n("Germany (TV Spielfilm)");

    Q_EMIT startedFetchingCountry(id);

    const QString url = "https://www.tvspielfilm.de/tv-programm/sendungen";

    // if country is unknown, store it
    QSqlQuery queryCountryExists;
    queryCountryExists.prepare(QStringLiteral("SELECT COUNT (id) FROM Channels WHERE id=:id;"));
    queryCountryExists.bindValue(QStringLiteral(":id"), id);
    Database::instance().execute(queryCountryExists);
    queryCountryExists.next();

    if (queryCountryExists.value(0).toInt() == 0) {
        Database::instance().addCountry(id, name, url);
    }

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
    // https://www.tvspielfilm.de/tv-programm/sendungen/das-erste,ARD.html
    const QString url = "https://www.tvspielfilm.de/tv-programm/sendungen/" + name.toLower().replace(' ', '-') + "," + channelId + ".html";

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
        // TODO: https://a2.tvspielfilm.de/images/tv/sender/mini/sprite_web_optimized_1616508904.webp
        const QString image = "https://a2.tvspielfilm.de/images/tv/sender/mini/" + channelId.toLower() + ".webp";
        Database::instance().addChannel(channelId, name, url, country, image);
    }

    Q_EMIT channelUpdated(channelId);
}

void Fetcher::fetchProgram(const QString &channelId)
{
    // https://www.tvspielfilm.de/tv-programm/sendungen/?date=2021-11-09&time=day&channel=ARD
    const QString url = "https://www.tvspielfilm.de/tv-programm/sendungen/?time=day&channel=" + channelId;

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
        queryProgramAvailable.bindValue(QStringLiteral(":channel"), channelId);
        queryProgramAvailable.bindValue(QStringLiteral(":lastTime"), lastTime);
        Database::instance().execute(queryProgramAvailable);
        queryProgramAvailable.next();

        if (queryProgramAvailable.value(0).toInt() > 0) {
            continue;
        }

        // TODO: better way to determine max page (see <ul class="pagination__items">)
        // we only need last page of yesterday + first of tomorrow
        for (int page = 1; page < 3; ++page) {
            const QString urlDay = url + "&date=" + day.toString("yyyy-MM-dd") + "&page=" + QString::number(page);
            qDebug() << "Starting to fetch program for " << channelId << "(" << urlDay << ")";

            QNetworkRequest request((QUrl(urlDay)));
            QNetworkReply *reply = get(request);
            connect(reply, &QNetworkReply::finished, this, [this, channelId, url, reply, page]() {
                if (reply->error()) {
                    qWarning() << "Error fetching channel";
                    qWarning() << reply->errorString();
                    // error only if there's no data at all (page 1)
                    if (page == 1) {
                        Q_EMIT error(channelId, reply->error(), reply->errorString());
                    }
                } else {
                    QByteArray data = reply->readAll();

                    // table with program info
                    QRegularExpression reInfoTable("<table class=\\\"info-table\\\">(.*?)</table>");
                    reInfoTable.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
                    QRegularExpressionMatch match = reInfoTable.match(data);
                    if (match.hasMatch()) {
                        const QString infoTable = match.captured(0);
                        processChannel(infoTable, url, channelId);
                    } else {
                        qWarning() << "Failed to parse " << url;
                    }
                }
                delete reply;
            });
        }
    }
}

void Fetcher::fetchDescription(const QString &channelId, const QString &programId, const QString &descriptionUrl)
{
    QNetworkRequest request((QUrl(descriptionUrl)));
    QNetworkReply *reply = get(request);
    connect(reply, &QNetworkReply::finished, this, [this, channelId, programId, descriptionUrl, reply]() {
        if (reply->error()) {
            qWarning() << "Error fetching program description";
            qWarning() << reply->errorString();
        } else {
            QByteArray data = reply->readAll();
            processDescription(data, descriptionUrl, programId);
            Q_EMIT channelUpdated(channelId);
        }
        delete reply;
    });
}

void Fetcher::processChannel(const QString &infoTable, const QString &url, const QString &channelId)
{
    QRegularExpression reProgram("<tr class=\\\"hover\\\">(.*?)</tr>");
    reProgram.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator it = reProgram.globalMatch(infoTable);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        const QString row = match.captured(0);
        processProgram(row, url, channelId);

        Q_EMIT channelUpdated(channelId);
    }
}

void Fetcher::processProgram(const QString &programRow, const QString &url, const QString &channelId)
{
    // column with title + description URL
    QString title = "";
    QString descriptionUrl = "";
    QRegularExpression reTitleCol("<td class=\\\"col-3\\\">(.*?)</td>");
    reTitleCol.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch titleColMatch = reTitleCol.match(programRow);
    if (titleColMatch.hasMatch()) {
        const QString titleCol = titleColMatch.captured(0);

        // title
        QRegularExpression reTitle("<strong>(.*?)</strong>");
        reTitle.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch titleMatch = reTitle.match(titleCol);
        if (titleMatch.hasMatch()) {
            title = titleMatch.captured(1);
        } else {
            qWarning() << "Failed to parse title " << url;
            return;
        }

        // description URL
        QRegularExpression reDescriptionUrl("<a href=\\\"(https://www.tvspielfilm.de/tv-programm/sendung/.*?\\.html)\\\"");
        reDescriptionUrl.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch descriptionUrlMatch = reDescriptionUrl.match(titleCol);
        if (descriptionUrlMatch.hasMatch()) {
            descriptionUrl = descriptionUrlMatch.captured(1);
        } else {
            qWarning() << "Failed to parse description URL " << url;
            // not critical -> no return
        }
    } else {
        qWarning() << "Failed to parse title column " << url;
        return;
    }

    // column with date and time
    QDateTime startTime;
    QDateTime stopTime;
    QRegularExpression reDateTimeCol("<td class=\\\"col-2\\\">(.*?)</td>");
    reDateTimeCol.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch dateTimeColMatch = reDateTimeCol.match(programRow);
    if (dateTimeColMatch.hasMatch()) {
        const QString dateTimeCol = dateTimeColMatch.captured(0);

        // date
        QString date = "";
        QRegularExpression reDate("<span>(.*?) (\\d\\d\\.\\d\\d\\.)</span>");
        reDate.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch dateMatch = reDate.match(dateTimeCol);
        if (dateMatch.hasMatch()) {
            date = dateMatch.captured(2);
        } else {
            qWarning() << "Failed to parse date " << url;
            return;
        }

        // time
        QRegularExpression reTime("<strong>(\\d\\d:\\d\\d) - (\\d\\d:\\d\\d)</strong>");
        reTime.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch timeMatch = reTime.match(dateTimeCol);
        if (timeMatch.hasMatch()) {
            startTime = QDateTime::fromString(QString::number(QDate::currentDate().year()) + date + timeMatch.captured(1), "yyyydd.MM.HH:mm");
            stopTime = QDateTime::fromString(QString::number(QDate::currentDate().year()) + date + timeMatch.captured(2), "yyyydd.MM.HH:mm");
            // ends after midnight
            if (stopTime < startTime) {
                stopTime = stopTime.addDays(1);
            }
        } else {
            qWarning() << "Failed to parse time " << url;
            return;
        }
    } else {
        qWarning() << "Failed to parse date and time " << url;
        return;
    }

    // column with category
    QString category = "";
    QRegularExpression reCategoryCol("<td class=\\\"col-4\\\">(.*?)</td>");
    reCategoryCol.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch categoryColMatch = reCategoryCol.match(programRow);
    if (categoryColMatch.hasMatch()) {
        const QString categoryCol = categoryColMatch.captured(0);

        // category
        QRegularExpression reCategory("<span>(.*?)</span>");
        reCategory.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch categoryMatch = reCategory.match(categoryCol);
        if (categoryMatch.hasMatch()) {
            category = categoryMatch.captured(1);
        } else {
            qWarning() << "Failed to parse category " << url;
            // not critical -> no return
        }
    } else {
        qWarning() << "Failed to parse category column " << url;
        return;
    }

    // channel + start time can be used as ID
    const QString programId = channelId + "_" + QString::number(startTime.toSecsSinceEpoch());

    QSqlQuery query;
    query.prepare(QStringLiteral("INSERT OR IGNORE INTO Programs VALUES (:id, :channel, :start, :stop, :title, :subtitle, :description, :category);"));
    query.bindValue(QStringLiteral(":id"), programId);
    query.bindValue(QStringLiteral(":channel"), channelId);
    query.bindValue(QStringLiteral(":start"), startTime.toSecsSinceEpoch());
    query.bindValue(QStringLiteral(":stop"), stopTime.toSecsSinceEpoch());
    query.bindValue(QStringLiteral(":title"), title);
    query.bindValue(QStringLiteral(":subtitle"), ""); // TODO
    query.bindValue(QStringLiteral(":description"), ""); // set in fetchDescription()
    query.bindValue(QStringLiteral(":category"), category);

    Database::instance().execute(query);

    // fetchDescription(channelId, programId, descriptionUrl); // TODO on demand? (way too slow)
}

void Fetcher::processDescription(const QString &descriptionPage, const QString &url, const QString &programId)
{
    QRegularExpression reDescription("<section class=\\\"broadcast-detail__description\\\">.*?<p>(.*?)</p>");
    reDescription.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = reDescription.match(descriptionPage);
    if (match.hasMatch()) {
        const QString description = match.captured(1);

        QSqlQuery query;
        query.prepare(QStringLiteral("UPDATE Programs SET description=:description WHERE id=:id;"));
        query.bindValue(QStringLiteral(":id"), programId);
        query.bindValue(QStringLiteral(":description"), description);

        Database::instance().execute(query);
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
