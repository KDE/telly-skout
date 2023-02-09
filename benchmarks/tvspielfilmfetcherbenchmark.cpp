// SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "../src/tvspielfilmfetcher.h"

#include <QTest>

#include <QFile>
#include <QHash>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QString>

#include <algorithm>

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

class TvSpielfilmFetcherBenchmark : public QObject
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
        QBENCHMARK {
            fetcher.fetchGroups([&data](const QVector<GroupData> &groups) {
                data = groups;
            });
        }
        QCOMPARE(data.size(), 1);
    }

    void testFetchGroup()
    {
        QNetworkReply *reply = new MockQNetworkReply("data/tvspielfilmfetcher/channels.html");
        MockQNetworkAccessManager nam;
        nam.registerReply("https://www.tvspielfilm.de/tv-programm/sendungen", reply);
        TvSpielfilmFetcher fetcher(&nam);
        const GroupData group{GroupId("tvspielfilm.germany"), "Germany", "https://www.tvspielfilm.de/tv-programm/sendungen"};
        QList<ChannelData> data;
        QBENCHMARK {
            fetcher.fetchGroup(group.m_url, group.m_id, [&data](const QList<ChannelData> &channels) {
                data = channels;
            });
            Q_EMIT reply->finished();
        }
        QCOMPARE(data.size(), 212);
    }

    void testFetchProgram()
    {
        const ChannelId channelId("SWR");

        QNetworkReply *replyYesterday = new MockQNetworkReply("data/tvspielfilmfetcher/empty.html");
        QNetworkReply *replyToday = new MockQNetworkReply("data/tvspielfilmfetcher/swr.html");
        QNetworkReply *replyTomorrow = new MockQNetworkReply("data/tvspielfilmfetcher/empty.html");
        MockQNetworkAccessManager nam;

        const QString yesterday = QDate::currentDate().addDays(-1).toString("yyyy-MM-dd");
        const QString today = QDate::currentDate().toString("yyyy-MM-dd");
        const QString tomorrow = QDate::currentDate().addDays(1).toString("yyyy-MM-dd");

        const QString url = "https://www.tvspielfilm.de/tv-programm/sendungen/?time=day&channel=" + channelId.value();
        nam.registerReply(url + "&date=" + yesterday + "&page=1", replyYesterday);
        nam.registerReply(url + "&date=" + today + "&page=1", replyToday);
        nam.registerReply(url + "&date=" + tomorrow + "&page=1", replyTomorrow);

        TvSpielfilmFetcher fetcher(&nam);
        size_t numPrograms = 0;
        QBENCHMARK {
            fetcher.fetchProgram(channelId, [&numPrograms](const QVector<ProgramData> &programs) {
                numPrograms = std::max(numPrograms, static_cast<size_t>(programs.size()));
            });
            Q_EMIT replyTomorrow->finished();
            Q_EMIT replyToday->finished();
            Q_EMIT replyYesterday->finished();
        }
        QCOMPARE(numPrograms, 60 * 24 - 1);
    }

    void testFetchProgramDescription()
    {
        const ChannelId channelId("SWR");
        ProgramId programId;

        QNetworkReply *reply = new MockQNetworkReply("data/tvspielfilmfetcher/description.html");
        MockQNetworkAccessManager nam;
        nam.registerReply("https://www.tvspielfilm.de/tv-programm/sendung/description1.html", reply);
        TvSpielfilmFetcher fetcher(&nam);
        QBENCHMARK {
            fetcher.fetchProgramDescription(channelId, ProgramId("channel1_1672182000"), "https://www.tvspielfilm.de/tv-programm/sendung/description1.html");
            Q_EMIT reply->finished();
        }
    }
};

QTEST_GUILESS_MAIN(TvSpielfilmFetcherBenchmark)

#include "tvspielfilmfetcherbenchmark.moc"