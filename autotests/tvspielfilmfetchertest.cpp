// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "../src/tvspielfilmfetcher.h"

#include <QFile>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QString>
#include <QTest>

namespace
{
class MockQNetworkReply : public QNetworkReply
{
    Q_OBJECT
public:
    explicit MockQNetworkReply(const QString &dataFileName, QObject *parent = nullptr)
        : QNetworkReply(parent)
    {
        setError(NetworkError::NoError, QString());
        setFinished(true);

        m_data.setFileName(QFINDTESTDATA(dataFileName));
        m_data.open(QIODevice::ReadOnly);
    }

    qint64 readData(char *data, qint64 maxlen) override
    {
        return m_data.read(data, maxlen);
    }

    void abort() override
    {
    }

private:
    QFile m_data;
};

class MockQNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit MockQNetworkAccessManager(QObject *parent = nullptr)
        : QNetworkAccessManager(parent)
    {
    }

    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData = nullptr) override
    {
        Q_UNUSED(op);
        Q_UNUSED(outgoingData);
        if (m_replies.contains(request.url().toString())) {
            QNetworkReply *reply = m_replies[request.url().toString()];
            reply->open(QIODevice::ReadOnly);
            return reply;
        } else {
            qWarning() << request.url() << m_replies;
        }
        return nullptr;
    }

    void registerReply(const QString &url, QNetworkReply *reply)
    {
        m_replies[url] = reply;
    }

private:
    QHash<QString, QNetworkReply *> m_replies;
};
}

class TvSpielfilmFetcherTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        // TV Spielfilm: Europe/Berlin (UTC+1), DB: UTC
        // check that start/stop times are displayed correctly in Europe/Athens (UTC+2 = EET-2)
        qputenv("TZ", "EET-2");
        QStandardPaths::setTestModeEnabled(true);
    }

    void testFetchGroups()
    {
        MockQNetworkAccessManager nam;
        TvSpielfilmFetcher fetcher(&nam);
        QVector<GroupData> data;
        bool callbackCalled = false;
        bool errorCallbackCalled = false;
        fetcher.fetchGroups(
            [&data, &callbackCalled](const QVector<GroupData> &groups) {
                data = groups;
                callbackCalled = true;
            },
            [&errorCallbackCalled](Error) {
                errorCallbackCalled = true;
            });
        QCOMPARE(callbackCalled, true);
        QCOMPARE(errorCallbackCalled, false);
        QCOMPARE(data.size(), 1);
        const GroupData &group = data.at(0);
        QCOMPARE(group.m_id.value(), QStringLiteral("tvspielfilm.germany"));
        QCOMPARE(group.m_name, QStringLiteral("Germany"));
        QCOMPARE(group.m_url, QStringLiteral("https://www.tvspielfilm.de/tv-programm/sendungen"));
    }

    void testFetchGroup()
    {
        QNetworkReply *reply = new MockQNetworkReply(QStringLiteral("data/tvspielfilmfetcher/channels.html"));
        MockQNetworkAccessManager nam;
        nam.registerReply(QStringLiteral("https://www.tvspielfilm.de/tv-programm/sendungen"), reply);
        TvSpielfilmFetcher fetcher(&nam);
        const GroupData group{GroupId(QStringLiteral("tvspielfilm.germany")),
                              QStringLiteral("Germany"),
                              QStringLiteral("https://www.tvspielfilm.de/tv-programm/sendungen")};
        QList<ChannelData> data;
        bool callbackCalled = false;
        bool errorCallbackCalled = false;
        fetcher.fetchGroup(
            group.m_url,
            group.m_id,
            [&data, &callbackCalled](const QList<ChannelData> &channels) {
                data = channels;
                callbackCalled = true;
            },
            [&errorCallbackCalled](Error) {
                errorCallbackCalled = true;
            });
        Q_EMIT reply->finished();
        QCOMPARE(callbackCalled, true);
        QCOMPARE(errorCallbackCalled, false);
        QCOMPARE(data.size(), 212);
    }

    void testFetchProgram()
    {
        const ChannelId channelId(QStringLiteral("SWR"));

        QNetworkReply *replyYesterday = new MockQNetworkReply(QStringLiteral("data/tvspielfilmfetcher/empty.html"));
        QNetworkReply *replyTodayPage1 = new MockQNetworkReply(QStringLiteral("data/tvspielfilmfetcher/swr-page1.html"));
        QNetworkReply *replyTodayPage2 = new MockQNetworkReply(QStringLiteral("data/tvspielfilmfetcher/swr-page2.html"));
        QNetworkReply *replyTomorrow = new MockQNetworkReply(QStringLiteral("data/tvspielfilmfetcher/empty.html"));
        MockQNetworkAccessManager nam;

        const QString yesterday = QDate::currentDate().addDays(-1).toString(QStringLiteral("yyyy-MM-dd"));
        const QString today = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
        const QString tomorrow = QDate::currentDate().addDays(1).toString(QStringLiteral("yyyy-MM-dd"));

        const QString url = QStringLiteral("https://www.tvspielfilm.de/tv-programm/sendungen/?time=day&channel=") + channelId.value();
        nam.registerReply(url + QStringLiteral("&date=") + yesterday + QStringLiteral("&page=1"), replyYesterday);
        nam.registerReply(url + QStringLiteral("&date=") + today + QStringLiteral("&page=1"), replyTodayPage1);
        nam.registerReply(url + QStringLiteral("&date=") + today + QStringLiteral("&page=2"),
                          replyTodayPage2); // link for 2022-12-28 page 2 hard coded in swr-page1.html
        nam.registerReply(url + QStringLiteral("&date=") + tomorrow + QStringLiteral("&page=1"), replyTomorrow);

        TvSpielfilmFetcher fetcher(&nam);
        QVector<ProgramData> data;
        bool callbackCalled = false;
        bool errorCallbackCalled = false;
        fetcher.fetchProgram(
            channelId,
            [&data, &callbackCalled](const QVector<ProgramData> &programs) {
                data = programs;
                callbackCalled = true;
            },
            [&errorCallbackCalled](Error) {
                errorCallbackCalled = true;
            });
        Q_EMIT replyTomorrow->finished();
        Q_EMIT replyTodayPage1->finished();
        Q_EMIT replyTodayPage2->finished();
        Q_EMIT replyYesterday->finished();
        QCOMPARE(callbackCalled, true);
        QCOMPARE(errorCallbackCalled, false);
        QCOMPARE(data.size(), 3);

        QCOMPARE(data.at(0).m_id, ProgramId(QStringLiteral("SWR_1672182000")));
        QCOMPARE(data.at(0).m_url, QStringLiteral("https://www.tvspielfilm.de/tv-programm/sendung/description1.html"));
        QCOMPARE(data.at(0).m_channelId, channelId);
        QCOMPARE(data.at(0).m_startTime, QDateTime::fromString(QStringLiteral("2022-12-28T01:00:00"), Qt::ISODate));
        QCOMPARE(data.at(0).m_stopTime, QDateTime::fromString(QStringLiteral("2022-12-28T07:00:00"), Qt::ISODate));
        QCOMPARE(data.at(0).m_title, QStringLiteral("Title 1"));
        QCOMPARE(data.at(0).m_subtitle, QStringLiteral(""));
        QCOMPARE(data.at(0).m_description, QStringLiteral(""));
        QCOMPARE(data.at(0).m_descriptionFetched, false);
        QCOMPARE(data.at(0).m_categories.at(0), QStringLiteral("Category 1"));

        QCOMPARE(data.at(1).m_id, ProgramId(QStringLiteral("SWR_1672203600")));
        QCOMPARE(data.at(1).m_url, QStringLiteral("https://www.tvspielfilm.de/tv-programm/sendung/description2.html"));
        QCOMPARE(data.at(1).m_channelId, channelId);
        QCOMPARE(data.at(1).m_startTime, QDateTime::fromString(QStringLiteral("2022-12-28T07:00:00"), Qt::ISODate));
        QCOMPARE(data.at(1).m_stopTime, QDateTime::fromString(QStringLiteral("2022-12-28T10:00:00"), Qt::ISODate));
        QCOMPARE(data.at(1).m_title, QStringLiteral("Title 2"));
        QCOMPARE(data.at(1).m_subtitle, QStringLiteral(""));
        QCOMPARE(data.at(1).m_description, QStringLiteral(""));
        QCOMPARE(data.at(1).m_descriptionFetched, false);
        QCOMPARE(data.at(1).m_categories.at(0), QStringLiteral("Category 2"));

        QCOMPARE(data.at(2).m_id, ProgramId(QStringLiteral("SWR_1672214400")));
        QCOMPARE(data.at(2).m_url, QStringLiteral("https://www.tvspielfilm.de/tv-programm/sendung/description3.html"));
        QCOMPARE(data.at(2).m_channelId, channelId);
        QCOMPARE(data.at(2).m_startTime, QDateTime::fromString(QStringLiteral("2022-12-28T10:00:00"), Qt::ISODate));
        QCOMPARE(data.at(2).m_stopTime, QDateTime::fromString(QStringLiteral("2022-12-28T13:00:00"), Qt::ISODate));
        QCOMPARE(data.at(2).m_title, QStringLiteral("Title 3"));
        QCOMPARE(data.at(2).m_subtitle, QStringLiteral(""));
        QCOMPARE(data.at(2).m_description, QStringLiteral(""));
        QCOMPARE(data.at(2).m_descriptionFetched, false);
        QCOMPARE(data.at(2).m_categories.at(0), QStringLiteral("Category 3"));
    }

    void testFetchProgramDescription()
    {
        const ChannelId channelId(QStringLiteral("SWR"));
        ProgramId programId;

        QNetworkReply *reply = new MockQNetworkReply(QStringLiteral("data/tvspielfilmfetcher/description.html"));
        MockQNetworkAccessManager nam;
        nam.registerReply(QStringLiteral("https://www.tvspielfilm.de/tv-programm/sendung/description1.html"), reply);
        TvSpielfilmFetcher fetcher(&nam);
        QString data;
        bool callbackCalled = false;
        bool errorCallbackCalled = false;
        fetcher.fetchProgramDescription(
            channelId,
            ProgramId(QStringLiteral("channel1_1672182000")),
            QStringLiteral("https://www.tvspielfilm.de/tv-programm/sendung/description1.html"),
            [&data, &callbackCalled](const QString &description) {
                data = description;
                callbackCalled = true;
            },
            [&errorCallbackCalled](Error) {
                errorCallbackCalled = true;
            });
        Q_EMIT reply->finished();
        QCOMPARE(callbackCalled, true);
        QCOMPARE(errorCallbackCalled, false);
        QCOMPARE(data, QStringLiteral("Description"));
    }
};

QTEST_GUILESS_MAIN(TvSpielfilmFetcherTest)

#include "tvspielfilmfetchertest.moc"
