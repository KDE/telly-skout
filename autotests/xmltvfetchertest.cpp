// SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "../src/xmltvfetcher.h"

#include "../src/TellySkoutSettings.h"
#include "../src/database.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QString>
#include <QTest>

class XmltvFetcherTest : public QObject
{
    Q_OBJECT

private:
    const QString m_dataPath = QFINDTESTDATA("data/xmltvfetcher/test.xml");

private Q_SLOTS:
    void initTestCase()
    {
        // xmltv: : Europe/Berlin (UTC+1), DB: UTC
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

        TellySkoutSettings::setXmltvFile(m_dataPath);
    }

    void testFetchGroups()
    {
        XmltvFetcher fetcher;
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
        QCOMPARE(group.m_id.value(), "xmltv");
        QCOMPARE(group.m_name, "XMLTV");
        QCOMPARE(group.m_url, m_dataPath);
    }

    void testFetchGroup()
    {
        XmltvFetcher fetcher;
        const GroupData group{GroupId("xmltv"), "XMLTV", m_dataPath};
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
        QCOMPARE(callbackCalled, true);
        QCOMPARE(errorCallbackCalled, false);
        QCOMPARE(data.size(), 2);
    }

    void testFetchProgram()
    {
        const ChannelId channelId("channel1");

        XmltvFetcher fetcher;
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
        QCOMPARE(callbackCalled, true);
        QCOMPARE(errorCallbackCalled, false);
        QCOMPARE(data.size(), 2);

        QCOMPARE(data.at(0).m_id, ProgramId("channel1_1672182000"));
        QCOMPARE(data.at(0).m_url, "");
        QCOMPARE(data.at(0).m_channelId, channelId);
        QCOMPARE(data.at(0).m_startTime, QDateTime::fromString("2022-12-28T01:00:00", Qt::ISODate));
        QCOMPARE(data.at(0).m_stopTime, QDateTime::fromString("2022-12-28T07:00:00", Qt::ISODate));
        QCOMPARE(data.at(0).m_title, "Title 1");
        QCOMPARE(data.at(0).m_subtitle, "Subtitle 1");
        QCOMPARE(data.at(0).m_description, "Description 1");
        QCOMPARE(data.at(0).m_descriptionFetched, true);
        QCOMPARE(data.at(0).m_categories.at(0), "Category 1");

        QCOMPARE(data.at(1).m_id, ProgramId("channel1_1672203600"));
        QCOMPARE(data.at(1).m_url, "");
        QCOMPARE(data.at(1).m_channelId, channelId);
        QCOMPARE(data.at(1).m_startTime, QDateTime::fromString("2022-12-28T07:00:00", Qt::ISODate));
        QCOMPARE(data.at(1).m_stopTime, QDateTime::fromString("2022-12-28T10:00:00", Qt::ISODate));
        QCOMPARE(data.at(1).m_title, "Title 2");
        QCOMPARE(data.at(1).m_subtitle, "Subtitle 2");
        QCOMPARE(data.at(1).m_description, "Description 2");
        QCOMPARE(data.at(1).m_descriptionFetched, true);
        QCOMPARE(data.at(1).m_categories.at(0), "Category 2");
    }

    void testFetchProgramDescription()
    {
        const ChannelId channelId("channel1");
        ProgramId programId;

        XmltvFetcher fetcher;
        QString data;
        bool callbackCalled = false;
        bool errorCallbackCalled = false;
        fetcher.fetchProgramDescription(
            channelId,
            ProgramId("channel1_1672182000"),
            "",
            [&data, &callbackCalled](const QString &description) {
                data = description;
                callbackCalled = true;
            },
            [&errorCallbackCalled](Error) {
                errorCallbackCalled = true;
            });
        // nothing done, description already known after fetchProgram()
        QCOMPARE(callbackCalled, false);
        QCOMPARE(errorCallbackCalled, false);
        QCOMPARE(data, "");
    }
};

QTEST_GUILESS_MAIN(XmltvFetcherTest)

#include "xmltvfetchertest.moc"
