// SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "../src/database.h"

#include <QStandardPaths>
#include <QTest>

class DatabaseBenchmark : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
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

    void testAddGroups()
    {
        QVector<GroupData> groups;

        for (int i = 0; i < 10000; ++i) {
            GroupData data;
            data.m_id = GroupId(QStringLiteral("Group") + QString::number(i));
            data.m_name = QStringLiteral("Group");
            data.m_url = QStringLiteral("GroupUrl");
            groups.push_back(data);
        }

        QBENCHMARK {
            Database::instance().addGroups(groups);
        }

        QCOMPARE(Database::instance().groupCount(), 10000);
    }

    void testGroups()
    {
        QBENCHMARK {
            const QVector<GroupData> groups = Database::instance().groups();
            QCOMPARE(groups.size(), 10000);
        }
    }

    void testAddChannels()
    {
        QList<ChannelData> channels;

        for (int i = 0; i < 10000; ++i) {
            ChannelData data;
            data.m_id = ChannelId(QStringLiteral("Channel") + QString::number(i));
            data.m_name = QStringLiteral("Channel");
            data.m_url = QStringLiteral("ChannelUrl");
            data.m_image = QStringLiteral("ChannelImage");
            channels.push_back(data);
        }

        QBENCHMARK {
            Database::instance().addChannels(channels, GroupId(QStringLiteral("Group1")));
        }

        QCOMPARE(Database::instance().channelCount(), 10000);
    }

    void testGroupsForChannel()
    {
        QBENCHMARK {
            const QVector<GroupData> groups = Database::instance().groups(ChannelId(QStringLiteral("Channel1")));
            QCOMPARE(groups.size(), 1);
        }
    }

    void testAddFavorite()
    {
        QBENCHMARK {
            for (int i = 0; i < 100; ++i) {
                Database::instance().addFavorite(ChannelId(QStringLiteral("Channel") + QString::number(i)));
            }
        }
    }

    void testFavorites()
    {
        QBENCHMARK {
            const QVector<ChannelId> favorites = Database::instance().favorites();
            QCOMPARE(favorites.size(), 100);
        }
    }

    void testChannelsOnlyFavorites()
    {
        QBENCHMARK {
            const QVector<ChannelData> channels = Database::instance().channels(true);
            QCOMPARE(channels.size(), 100);
        }
    }

    void testSortFavorites()
    {
        QVector<ChannelId> favorites;
        for (int i = 99; i >= 0; --i) {
            favorites.push_back(ChannelId(QStringLiteral("Channel") + QString::number(i)));
        }

        QBENCHMARK {
            Database::instance().sortFavorites(favorites);
        }
    }

    void testRemoveFavorite()
    {
        QBENCHMARK {
            Database::instance().removeFavorite(ChannelId(QStringLiteral("Channel50")));
        }
        QCOMPARE(Database::instance().favoriteCount(), 99);
    }

    void testAddPrograms()
    {
        QVector<ProgramData> programs;

        for (int i = 0; i < 10000; ++i) {
            ProgramData data;
            data.m_id = ProgramId(QStringLiteral("Program") + QString::number(i));
            data.m_url = QStringLiteral("ProgramUrl");
            data.m_channelId = ChannelId(QStringLiteral("Channel1"));
            data.m_startTime = QDateTime::fromString(QStringLiteral("2022-12-28T01:00:00"), Qt::ISODate).addSecs(i);
            data.m_stopTime = QDateTime::fromString(QStringLiteral("2022-12-28T02:00:00"), Qt::ISODate).addSecs(i);
            data.m_title = QStringLiteral("ProgramTitle");
            data.m_subtitle = QStringLiteral("ProgramSubtitle");
            data.m_description = QStringLiteral("ProgramDescription");
            data.m_descriptionFetched = true;
            data.m_categories = {QStringLiteral("Category1")};
            programs.push_back(data);
        }

        QBENCHMARK {
            Database::instance().addPrograms(programs);
        }

        QCOMPARE(Database::instance().programCount(ChannelId(QStringLiteral("Channel1"))), 10000);
    }

    void testPrograms()
    {
        QBENCHMARK {
            const QMap<ChannelId, QVector<ProgramData>> programs = Database::instance().programs();
            QCOMPARE(programs.size(), 1);
            QCOMPARE(programs[ChannelId(QStringLiteral("Channel1"))].size(), 10000);
        }
    }
};

QTEST_GUILESS_MAIN(DatabaseBenchmark)

#include "databasebenchmark.moc"
