// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "../src/tvspielfilmfetcher.h"

#include "../src/database.h"

#include <QFile>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QString>
#include <QTest>

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

        Database::instance().execute(QStringLiteral("DELETE FROM \"Groups\";"));
        QCOMPARE(Database::instance().groupCount(), 0);
        Database::instance().execute(QStringLiteral("DELETE FROM Channels;"));
        QCOMPARE(Database::instance().channelCount(), 0);
        Database::instance().execute(QStringLiteral("DELETE FROM GroupChannels;"));
        Database::instance().execute(QStringLiteral("DELETE FROM Programs;"));
        Database::instance().execute(QStringLiteral("DELETE FROM ProgramCategories;"));
        Database::instance().execute(QStringLiteral("DELETE FROM Favorites;"));
        QCOMPARE(Database::instance().favoriteCount(), 0);
    }

    void testFetchGroups()
    {
        MockQNetworkAccessManager nam;
        TvSpielfilmFetcher fetcher(&nam);
        QSignalSpy groupUpdatedSpy(&fetcher, SIGNAL(groupUpdated(const GroupId &)));
        QVERIFY(groupUpdatedSpy.isValid());
        QCOMPARE(groupUpdatedSpy.count(), 0);
        fetcher.fetchGroups();
        QCOMPARE(groupUpdatedSpy.count(), 1);
        QCOMPARE(Database::instance().groupCount(), 1);
        const QVector<GroupData> groups = Database::instance().groups();
        const GroupData &group = groups.at(0);
        QCOMPARE(group.m_id.value(), "tvspielfilm.germany");
        QCOMPARE(group.m_name, "Germany");
        QCOMPARE(group.m_url, "https://www.tvspielfilm.de/tv-programm/sendungen");
    }

    void testFetchGroup()
    {
        QNetworkReply *reply = new MockQNetworkReply("data/tvspielfilmfetcher/channels.html");
        MockQNetworkAccessManager nam;
        nam.registerReply("https://www.tvspielfilm.de/tv-programm/sendungen", reply);
        TvSpielfilmFetcher fetcher(&nam);
        QSignalSpy groupUpdatedSpy(&fetcher, SIGNAL(groupUpdated(const GroupId &)));
        QVERIFY(groupUpdatedSpy.isValid());
        QCOMPARE(groupUpdatedSpy.count(), 0);
        const GroupData group{GroupId("tvspielfilm.germany"), "Germany", "https://www.tvspielfilm.de/tv-programm/sendungen"};
        fetcher.fetchGroup(group.m_url, group.m_id);
        Q_EMIT reply->finished();
        QCOMPARE(groupUpdatedSpy.count(), 1);
        QCOMPARE(Database::instance().channelCount(), 212);
    }

    void testFetchProgram()
    {
        const ChannelId channelId("SWR");

        QNetworkReply *replyYesterday = new MockQNetworkReply("data/tvspielfilmfetcher/empty.html");
        QNetworkReply *replyTodayPage1 = new MockQNetworkReply("data/tvspielfilmfetcher/swr-page1.html");
        QNetworkReply *replyTodayPage2 = new MockQNetworkReply("data/tvspielfilmfetcher/swr-page2.html");
        QNetworkReply *replyTomorrow = new MockQNetworkReply("data/tvspielfilmfetcher/empty.html");
        MockQNetworkAccessManager nam;

        const QString yesterday = QDate::currentDate().addDays(-1).toString("yyyy-MM-dd");
        const QString today = QDate::currentDate().toString("yyyy-MM-dd");
        const QString tomorrow = QDate::currentDate().addDays(1).toString("yyyy-MM-dd");

        const QString url = "https://www.tvspielfilm.de/tv-programm/sendungen/?time=day&channel=" + channelId.value();
        nam.registerReply(url + "&date=" + yesterday + "&page=1", replyYesterday);
        nam.registerReply(url + "&date=" + today + "&page=1", replyTodayPage1);
        nam.registerReply(url + "&date=2022-12-28&page=2", replyTodayPage2); // link for 2022-12-28 page 2 hard coded in swr-page1.html
        nam.registerReply(url + "&date=" + tomorrow + "&page=1", replyTomorrow);

        TvSpielfilmFetcher fetcher(&nam);
        QSignalSpy channelUpdatedSpy(&fetcher, SIGNAL(channelUpdated(const ChannelId &)));
        QVERIFY(channelUpdatedSpy.isValid());
        QCOMPARE(channelUpdatedSpy.count(), 0);
        fetcher.fetchProgram(channelId);
        Q_EMIT replyTomorrow->finished();
        Q_EMIT replyTodayPage1->finished();
        Q_EMIT replyTodayPage2->finished();
        Q_EMIT replyYesterday->finished();
        QCOMPARE(channelUpdatedSpy.count(), 3);
        QCOMPARE(Database::instance().programCount(channelId), 3);

        const auto programs = Database::instance().programs(channelId);
        QCOMPARE(programs.at(0).m_id, ProgramId("SWR_1672182000"));
        QCOMPARE(programs.at(0).m_url, "https://www.tvspielfilm.de/tv-programm/sendung/description1.html");
        QCOMPARE(programs.at(0).m_channelId, channelId);
        QCOMPARE(programs.at(0).m_startTime, QDateTime::fromString("2022-12-28T01:00:00", Qt::ISODate));
        QCOMPARE(programs.at(0).m_stopTime, QDateTime::fromString("2022-12-28T07:00:00", Qt::ISODate));
        QCOMPARE(programs.at(0).m_title, "Title 1");
        QCOMPARE(programs.at(0).m_subtitle, "");
        QCOMPARE(programs.at(0).m_description, "");
        QCOMPARE(programs.at(0).m_descriptionFetched, false);
        QCOMPARE(programs.at(0).m_categories.at(0), "Category 1");

        QCOMPARE(programs.at(1).m_id, ProgramId("SWR_1672203600"));
        QCOMPARE(programs.at(1).m_url, "https://www.tvspielfilm.de/tv-programm/sendung/description2.html");
        QCOMPARE(programs.at(1).m_channelId, channelId);
        QCOMPARE(programs.at(1).m_startTime, QDateTime::fromString("2022-12-28T07:00:00", Qt::ISODate));
        QCOMPARE(programs.at(1).m_stopTime, QDateTime::fromString("2022-12-28T10:00:00", Qt::ISODate));
        QCOMPARE(programs.at(1).m_title, "Title 2");
        QCOMPARE(programs.at(1).m_subtitle, "");
        QCOMPARE(programs.at(1).m_description, "");
        QCOMPARE(programs.at(1).m_descriptionFetched, false);
        QCOMPARE(programs.at(1).m_categories.at(0), "Category 2");

        QCOMPARE(programs.at(2).m_id, ProgramId("SWR_1672214400"));
        QCOMPARE(programs.at(2).m_url, "https://www.tvspielfilm.de/tv-programm/sendung/description3.html");
        QCOMPARE(programs.at(2).m_channelId, channelId);
        QCOMPARE(programs.at(2).m_startTime, QDateTime::fromString("2022-12-28T10:00:00", Qt::ISODate));
        QCOMPARE(programs.at(2).m_stopTime, QDateTime::fromString("2022-12-28T13:00:00", Qt::ISODate));
        QCOMPARE(programs.at(2).m_title, "Title 3");
        QCOMPARE(programs.at(2).m_subtitle, "");
        QCOMPARE(programs.at(2).m_description, "");
        QCOMPARE(programs.at(2).m_descriptionFetched, false);
        QCOMPARE(programs.at(2).m_categories.at(0), "Category 3");
    }

    void testFetchProgramDescription()
    {
        const ChannelId channelId("SWR");
        ProgramId programId;

        QNetworkReply *reply = new MockQNetworkReply("data/tvspielfilmfetcher/description.html");
        MockQNetworkAccessManager nam;
        nam.registerReply("https://www.tvspielfilm.de/tv-programm/sendung/description1.html", reply);
        TvSpielfilmFetcher fetcher(&nam);
        QSignalSpy channelUpdatedSpy(&fetcher, SIGNAL(channelUpdated(const ChannelId &)));
        QVERIFY(channelUpdatedSpy.isValid());
        QCOMPARE(channelUpdatedSpy.count(), 0);
        fetcher.fetchProgramDescription(channelId,
                                        Database::instance().programs(channelId).at(0).m_id,
                                        "https://www.tvspielfilm.de/tv-programm/sendung/description1.html");
        Q_EMIT reply->finished();
        QCOMPARE(channelUpdatedSpy.count(), 1);
        QCOMPARE(Database::instance().programs(channelId).at(0).m_description, "Description");
        QCOMPARE(Database::instance().programs(channelId).at(0).m_descriptionFetched, true);
    }
};

QTEST_GUILESS_MAIN(TvSpielfilmFetcherTest)

#include "tvspielfilmfetchertest.moc"
