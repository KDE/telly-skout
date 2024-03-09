// SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "../src/database.h"

#include <QStandardPaths>
#include <QTest>

#include <algorithm>

class DatabaseTest : public QObject
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

    void testAddGroup()
    {
        GroupData data;
        data.m_id = GroupId(QStringLiteral("Group1"));
        data.m_name = QStringLiteral("Group 1");
        data.m_url = QStringLiteral("GroupUrl1");

        Database::instance().addGroup(data);

        QCOMPARE(Database::instance().groupCount(), 1);
        const QVector<GroupData> groups = Database::instance().groups();
        const GroupData &group1 = groups.at(0);
        QCOMPARE(group1.m_id.value(), QStringLiteral("Group1"));
        QCOMPARE(group1.m_name, QStringLiteral("Group 1"));
        QCOMPARE(group1.m_url, QStringLiteral("GroupUrl1"));
    }

    void testAddGroups()
    {
        GroupData data2;
        data2.m_id = GroupId(QStringLiteral("Group2"));
        data2.m_name = QStringLiteral("Group 2");
        data2.m_url = QStringLiteral("GroupUrl2");
        GroupData data3;
        data3.m_id = GroupId(QStringLiteral("Group3"));
        data3.m_name = QStringLiteral("Group 3");
        data3.m_url = QStringLiteral("GroupUrl3");
        QVector<GroupData> data;
        data.push_back(data2);
        data.push_back(data3);

        Database::instance().addGroups(data);

        QCOMPARE(Database::instance().groupCount(), 3);
        const QVector<GroupData> groups = Database::instance().groups();
        const GroupData &group1 = groups.at(0);
        QCOMPARE(group1.m_id.value(), QStringLiteral("Group1"));
        QCOMPARE(group1.m_name, QStringLiteral("Group 1"));
        QCOMPARE(group1.m_url, QStringLiteral("GroupUrl1"));
        const GroupData &group2 = groups.at(1);
        QCOMPARE(group2.m_id.value(), QStringLiteral("Group2"));
        QCOMPARE(group2.m_name, QStringLiteral("Group 2"));
        QCOMPARE(group2.m_url, QStringLiteral("GroupUrl2"));
        const GroupData &group3 = groups.at(2);
        QCOMPARE(group3.m_id.value(), QStringLiteral("Group3"));
        QCOMPARE(group3.m_name, QStringLiteral("Group 3"));
        QCOMPARE(group3.m_url, QStringLiteral("GroupUrl3"));
    }

    void testGroupExists()
    {
        QCOMPARE(Database::instance().groupExists(GroupId(QStringLiteral("Group1"))), true);
        QCOMPARE(Database::instance().groupExists(GroupId(QStringLiteral("Group2"))), true);
        QCOMPARE(Database::instance().groupExists(GroupId(QStringLiteral("Group3"))), true);

        QCOMPARE(Database::instance().groupExists(GroupId(QStringLiteral("Group0"))), false);
    }

    void testAddChannel()
    {
        ChannelData data;
        data.m_id = ChannelId(QStringLiteral("Channel1"));
        data.m_name = QStringLiteral("Channel 1");
        data.m_url = QStringLiteral("Channel1Url");
        data.m_image = QStringLiteral("Channel1Image");

        Database::instance().addChannel(data, GroupId(QStringLiteral("Group1")));

        QCOMPARE(Database::instance().channelCount(), 1);
        const QVector<ChannelData> channels = Database::instance().channels(false);
        const ChannelData &channel1 = channels.at(0);
        QCOMPARE(channel1.m_id.value(), QStringLiteral("Channel1"));
        QCOMPARE(channel1.m_name, QStringLiteral("Channel 1"));
        QCOMPARE(channel1.m_url, QStringLiteral("Channel1Url"));
        QCOMPARE(channel1.m_image, QStringLiteral("Channel1Image"));
    }

    void testGroupsForChannel()
    {
        const QVector<GroupData> groups = Database::instance().groups(ChannelId(QStringLiteral("Channel1")));
        QCOMPARE(groups.size(), 1);
        QCOMPARE(groups.at(0).m_id.value(), QStringLiteral("Group1"));
    }

    void testAddChannels()
    {
        ChannelData data2;
        data2.m_id = ChannelId(QStringLiteral("Channel2"));
        data2.m_name = QStringLiteral("Channel 2");
        data2.m_url = QStringLiteral("Channel2Url");
        data2.m_image = QStringLiteral("Channel2Image");
        ChannelData data3;
        data3.m_id = ChannelId(QStringLiteral("Channel3"));
        data3.m_name = QStringLiteral("Channel 3");
        data3.m_url = QStringLiteral("Channel3Url");
        data3.m_image = QStringLiteral("Channel3Image");
        QList<ChannelData> data;
        data.push_back(data2);
        data.push_back(data3);

        Database::instance().addChannels(data, GroupId(QStringLiteral("Group1")));

        QCOMPARE(Database::instance().channelCount(), 3);
        const QVector<ChannelData> channels = Database::instance().channels(false);
        const ChannelData &channel1 = channels.at(0);
        QCOMPARE(channel1.m_id.value(), QStringLiteral("Channel1"));
        QCOMPARE(channel1.m_name, QStringLiteral("Channel 1"));
        QCOMPARE(channel1.m_url, QStringLiteral("Channel1Url"));
        QCOMPARE(channel1.m_image, QStringLiteral("Channel1Image"));
        const ChannelData &channel2 = channels.at(1);
        QCOMPARE(channel2.m_id.value(), QStringLiteral("Channel2"));
        QCOMPARE(channel2.m_name, QStringLiteral("Channel 2"));
        QCOMPARE(channel2.m_url, QStringLiteral("Channel2Url"));
        QCOMPARE(channel2.m_image, QStringLiteral("Channel2Image"));
        const ChannelData &channel3 = channels.at(2);
        QCOMPARE(channel3.m_id.value(), QStringLiteral("Channel3"));
        QCOMPARE(channel3.m_name, QStringLiteral("Channel 3"));
        QCOMPARE(channel3.m_url, QStringLiteral("Channel3Url"));
        QCOMPARE(channel3.m_image, QStringLiteral("Channel3Image"));
    }

    void testChannelExists()
    {
        QCOMPARE(Database::instance().channelExists(ChannelId(QStringLiteral("Channel1"))), true);
        QCOMPARE(Database::instance().channelExists(ChannelId(QStringLiteral("Channel2"))), true);
        QCOMPARE(Database::instance().channelExists(ChannelId(QStringLiteral("Channel3"))), true);

        QCOMPARE(Database::instance().channelExists(ChannelId(QStringLiteral("Channel0"))), false);
    }

    void testChannel()
    {
        const ChannelData &channel1 = Database::instance().channel(ChannelId(QStringLiteral("Channel1")));
        QCOMPARE(channel1.m_id.value(), QStringLiteral("Channel1"));
        QCOMPARE(channel1.m_name, QStringLiteral("Channel 1"));
        QCOMPARE(channel1.m_url, QStringLiteral("Channel1Url"));
        QCOMPARE(channel1.m_image, QStringLiteral("Channel1Image"));
    }

    void testAddFavorite()
    {
        QCOMPARE(Database::instance().favoriteCount(), 0);
        Database::instance().addFavorite(ChannelId(QStringLiteral("Channel1")));
        QCOMPARE(Database::instance().favoriteCount(), 1);
        Database::instance().addFavorite(ChannelId(QStringLiteral("Channel2")));
        QCOMPARE(Database::instance().favoriteCount(), 2);
    }

    void testFavorites()
    {
        const QVector<ChannelId> favorites = Database::instance().favorites();
        QCOMPARE(favorites.at(0).value(), QStringLiteral("Channel1"));
        QCOMPARE(favorites.at(1).value(), QStringLiteral("Channel2"));
    }

    void testChannelsOnlyFavorites()
    {
        const QVector<ChannelData> channels = Database::instance().channels(true);
        QCOMPARE(channels.size(), 2);
        QCOMPARE(channels.at(0).m_id.value(), QStringLiteral("Channel1"));
        QCOMPARE(channels.at(1).m_id.value(), QStringLiteral("Channel2"));
    }

    void testSortFavorites()
    {
        QVector<ChannelId> data;
        data.push_back(ChannelId(QStringLiteral("Channel2")));
        data.push_back(ChannelId(QStringLiteral("Channel1")));
        Database::instance().sortFavorites(data);
        const QVector<ChannelId> favorites = Database::instance().favorites();
        QCOMPARE(favorites.size(), 2);
        QCOMPARE(favorites.at(0).value(), QStringLiteral("Channel2"));
        QCOMPARE(favorites.at(1).value(), QStringLiteral("Channel1"));
    }

    void testIsFavorite()
    {
        QCOMPARE(Database::instance().isFavorite(ChannelId(QStringLiteral("Channel1"))), true);
        QCOMPARE(Database::instance().isFavorite(ChannelId(QStringLiteral("Channel2"))), true);

        QCOMPARE(Database::instance().isFavorite(ChannelId(QStringLiteral("Channel3"))), false);

        QCOMPARE(Database::instance().isFavorite(ChannelId(QStringLiteral("Channel0"))), false);
    }

    void testRemoveFavorite()
    {
        QCOMPARE(Database::instance().favoriteCount(), 2);
        Database::instance().removeFavorite(ChannelId(QStringLiteral("Channel2")));
        QCOMPARE(Database::instance().favoriteCount(), 1);
        QCOMPARE(Database::instance().isFavorite(ChannelId(QStringLiteral("Channel1"))), true);
        QCOMPARE(Database::instance().isFavorite(ChannelId(QStringLiteral("Channel2"))), false);
    }

    void testClearFavorites()
    {
        QCOMPARE(Database::instance().favoriteCount(), 1);
        Database::instance().clearFavorites();
        QCOMPARE(Database::instance().favoriteCount(), 0);
        QCOMPARE(Database::instance().isFavorite(ChannelId(QStringLiteral("Channel1"))), false);
    }

    void testAddProgram()
    {
        ProgramData data;
        data.m_id = ProgramId(QStringLiteral("Program1"));
        data.m_url = QStringLiteral("Program1Url");
        data.m_channelId = ChannelId(QStringLiteral("Channel1"));
        data.m_startTime = QDateTime::fromString(QStringLiteral("2022-12-28T00:00:00"), Qt::ISODate);
        data.m_stopTime = QDateTime::fromString(QStringLiteral("2022-12-28T01:00:00"), Qt::ISODate);
        data.m_title = QStringLiteral("Program1Title");
        data.m_subtitle = QStringLiteral("Program1Subtitle");
        data.m_description = QStringLiteral("Program1Description");
        data.m_descriptionFetched = true;
        data.m_categories = {QStringLiteral("Category1")};

        Database::instance().addProgram(data);

        QCOMPARE(Database::instance().programCount(ChannelId(QStringLiteral("Channel1"))), 1);
        const QVector<ProgramData> programs = Database::instance().programs(ChannelId(QStringLiteral("Channel1")));
        const ProgramData &program1 = programs.at(0);
        QCOMPARE(program1.m_id.value(), QStringLiteral("Program1"));
        QCOMPARE(program1.m_url, QStringLiteral("Program1Url"));
        QCOMPARE(program1.m_channelId.value(), QStringLiteral("Channel1"));
        QCOMPARE(program1.m_startTime, QDateTime::fromString(QStringLiteral("2022-12-28T00:00:00"), Qt::ISODate));
        QCOMPARE(program1.m_stopTime, QDateTime::fromString(QStringLiteral("2022-12-28T01:00:00"), Qt::ISODate));
        QCOMPARE(program1.m_title, QStringLiteral("Program1Title"));
        QCOMPARE(program1.m_subtitle, QStringLiteral("Program1Subtitle"));
        QCOMPARE(program1.m_description, QStringLiteral("Program1Description"));
        QCOMPARE(program1.m_descriptionFetched, true);
        QCOMPARE(program1.m_categories, {QStringLiteral("Category1")});
    }

    void testAddPrograms()
    {
        ProgramData data2;
        data2.m_id = ProgramId(QStringLiteral("Program2"));
        data2.m_url = QStringLiteral("Program2Url");
        data2.m_channelId = ChannelId(QStringLiteral("Channel1"));
        data2.m_startTime = QDateTime::fromString(QStringLiteral("2022-12-28T01:00:00"), Qt::ISODate);
        data2.m_stopTime = QDateTime::fromString(QStringLiteral("2022-12-28T02:00:00"), Qt::ISODate);
        data2.m_title = QStringLiteral("Program2Title");
        data2.m_subtitle = QStringLiteral("Program2Subtitle");
        data2.m_description = QStringLiteral("Program2Description");
        data2.m_descriptionFetched = false;
        data2.m_categories = {QStringLiteral("Category2")};
        ProgramData data3;
        data3.m_id = ProgramId(QStringLiteral("Program3"));
        data3.m_url = QStringLiteral("Program3Url");
        data3.m_channelId = ChannelId(QStringLiteral("Channel2"));
        data3.m_startTime = QDateTime::fromString(QStringLiteral("2022-12-28T02:00:00"), Qt::ISODate);
        data3.m_stopTime = QDateTime::fromString(QStringLiteral("2022-12-28T03:00:00"), Qt::ISODate);
        data3.m_title = QStringLiteral("Program3Title");
        data3.m_subtitle = QStringLiteral("Program3Subtitle");
        data3.m_description = QStringLiteral("Program3Description");
        data3.m_descriptionFetched = true;
        data3.m_categories = {QStringLiteral("Category1"), QStringLiteral("Category2")};
        QVector<ProgramData> data;
        data.push_back(data2);
        data.push_back(data3);

        Database::instance().addPrograms(data);

        QCOMPARE(Database::instance().programCount(ChannelId(QStringLiteral("Channel1"))), 2);
        QCOMPARE(Database::instance().programCount(ChannelId(QStringLiteral("Channel2"))), 1);
        const QVector<ProgramData> programsChannel1 = Database::instance().programs(ChannelId(QStringLiteral("Channel1")));
        const ProgramData &program1 = programsChannel1.at(0);
        QCOMPARE(program1.m_id.value(), QStringLiteral("Program1"));
        QCOMPARE(program1.m_url, QStringLiteral("Program1Url"));
        QCOMPARE(program1.m_channelId.value(), QStringLiteral("Channel1"));
        QCOMPARE(program1.m_startTime, QDateTime::fromString(QStringLiteral("2022-12-28T00:00:00"), Qt::ISODate));
        QCOMPARE(program1.m_stopTime, QDateTime::fromString(QStringLiteral("2022-12-28T01:00:00"), Qt::ISODate));
        QCOMPARE(program1.m_title, QStringLiteral("Program1Title"));
        QCOMPARE(program1.m_subtitle, QStringLiteral("Program1Subtitle"));
        QCOMPARE(program1.m_description, QStringLiteral("Program1Description"));
        QCOMPARE(program1.m_descriptionFetched, true);
        QCOMPARE(program1.m_categories, {QStringLiteral("Category1")});
        const ProgramData &program2 = programsChannel1.at(1);
        QCOMPARE(program2.m_id.value(), QStringLiteral("Program2"));
        QCOMPARE(program2.m_url, QStringLiteral("Program2Url"));
        QCOMPARE(program2.m_channelId.value(), QStringLiteral("Channel1"));
        QCOMPARE(program2.m_startTime, QDateTime::fromString(QStringLiteral("2022-12-28T01:00:00"), Qt::ISODate));
        QCOMPARE(program2.m_stopTime, QDateTime::fromString(QStringLiteral("2022-12-28T02:00:00"), Qt::ISODate));
        QCOMPARE(program2.m_title, QStringLiteral("Program2Title"));
        QCOMPARE(program2.m_subtitle, QStringLiteral("Program2Subtitle"));
        QCOMPARE(program2.m_description, QStringLiteral("Program2Description"));
        QCOMPARE(program2.m_descriptionFetched, false);
        QCOMPARE(program2.m_categories, {QStringLiteral("Category2")});
        const QVector<ProgramData> programsChannel2 = Database::instance().programs(ChannelId(QStringLiteral("Channel2")));
        const ProgramData &program3 = programsChannel2.at(0);
        QCOMPARE(program3.m_id.value(), QStringLiteral("Program3"));
        QCOMPARE(program3.m_url, QStringLiteral("Program3Url"));
        QCOMPARE(program3.m_channelId.value(), QStringLiteral("Channel2"));
        QCOMPARE(program3.m_startTime, QDateTime::fromString(QStringLiteral("2022-12-28T02:00:00"), Qt::ISODate));
        QCOMPARE(program3.m_stopTime, QDateTime::fromString(QStringLiteral("2022-12-28T03:00:00"), Qt::ISODate));
        QCOMPARE(program3.m_title, QStringLiteral("Program3Title"));
        QCOMPARE(program3.m_subtitle, QStringLiteral("Program3Subtitle"));
        QCOMPARE(program3.m_description, QStringLiteral("Program3Description"));
        QCOMPARE(program3.m_descriptionFetched, true);
        QVector<QString> sortedCategories = program3.m_categories;
        std::sort(sortedCategories.begin(), sortedCategories.end());
        QCOMPARE(sortedCategories.at(0), QStringLiteral("Category1"));
        QCOMPARE(sortedCategories.at(1), QStringLiteral("Category2"));
    }

    void testProgramExists()
    {
        QCOMPARE(Database::instance().programExists(ChannelId(QStringLiteral("Channel1")),
                                                    QDateTime::fromString(QStringLiteral("2022-12-28T02:00:00"), Qt::ISODate)),
                 true);
        QCOMPARE(Database::instance().programExists(ChannelId(QStringLiteral("Channel1")),
                                                    QDateTime::fromString(QStringLiteral("2022-12-28T02:00:01"), Qt::ISODate)),
                 false);
    }

    void testUpdateProgramDescription()
    {
        Database::instance().updateProgramDescription(ProgramId(QStringLiteral("Program2")), QStringLiteral("Program2DescriptionUpdated"));
        const QVector<ProgramData> programsChannel1 = Database::instance().programs(ChannelId(QStringLiteral("Channel1")));
        const ProgramData &program2 = programsChannel1.at(1);
        QCOMPARE(program2.m_description, QStringLiteral("Program2DescriptionUpdated"));
        QCOMPARE(program2.m_descriptionFetched, true);
    }

    void testPrograms()
    {
        const QMap<ChannelId, QVector<ProgramData>> programs = Database::instance().programs();
        const QVector<ProgramData> programsChannel1 = programs[ChannelId(QStringLiteral("Channel1"))];
        QCOMPARE(programsChannel1.size(), 2);
        QCOMPARE(programsChannel1.at(0).m_id.value(), QStringLiteral("Program1"));
        QCOMPARE(programsChannel1.at(1).m_id.value(), QStringLiteral("Program2"));
        const QVector<ProgramData> programsChannel2 = programs[ChannelId(QStringLiteral("Channel2"))];
        QCOMPARE(programsChannel2.size(), 1);
        QCOMPARE(programsChannel2.at(0).m_id.value(), QStringLiteral("Program3"));
    }
};

QTEST_GUILESS_MAIN(DatabaseTest)

#include "databasetest.moc"
