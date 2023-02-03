// SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "../src/database.h"

#include <QStandardPaths>
#include <QTest>

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
        data.m_id = GroupId("Group1");
        data.m_name = "Group 1";
        data.m_url = "GroupUrl1";

        Database::instance().addGroup(data);

        QCOMPARE(Database::instance().groupCount(), 1);
        const QVector<GroupData> groups = Database::instance().groups();
        const GroupData &group1 = groups.at(0);
        QCOMPARE(group1.m_id.value(), "Group1");
        QCOMPARE(group1.m_name, "Group 1");
        QCOMPARE(group1.m_url, "GroupUrl1");
    }

    void testAddGroups()
    {
        GroupData data2;
        data2.m_id = GroupId("Group2");
        data2.m_name = "Group 2";
        data2.m_url = "GroupUrl2";
        GroupData data3;
        data3.m_id = GroupId("Group3");
        data3.m_name = "Group 3";
        data3.m_url = "GroupUrl3";
        QVector<GroupData> data;
        data.push_back(data2);
        data.push_back(data3);

        Database::instance().addGroups(data);

        QCOMPARE(Database::instance().groupCount(), 3);
        const QVector<GroupData> groups = Database::instance().groups();
        const GroupData &group1 = groups.at(0);
        QCOMPARE(group1.m_id.value(), "Group1");
        QCOMPARE(group1.m_name, "Group 1");
        QCOMPARE(group1.m_url, "GroupUrl1");
        const GroupData &group2 = groups.at(1);
        QCOMPARE(group2.m_id.value(), "Group2");
        QCOMPARE(group2.m_name, "Group 2");
        QCOMPARE(group2.m_url, "GroupUrl2");
        const GroupData &group3 = groups.at(2);
        QCOMPARE(group3.m_id.value(), "Group3");
        QCOMPARE(group3.m_name, "Group 3");
        QCOMPARE(group3.m_url, "GroupUrl3");
    }

    void testGroupExists()
    {
        QCOMPARE(Database::instance().groupExists(GroupId("Group1")), true);
        QCOMPARE(Database::instance().groupExists(GroupId("Group2")), true);
        QCOMPARE(Database::instance().groupExists(GroupId("Group3")), true);

        QCOMPARE(Database::instance().groupExists(GroupId("Group0")), false);
    }

    void testAddChannel()
    {
        ChannelData data;
        data.m_id = ChannelId("Channel1");
        data.m_name = "Channel 1";
        data.m_url = "Channel1Url";
        data.m_image = "Channel1Image";

        Database::instance().addChannel(data, GroupId("Group1"));

        QCOMPARE(Database::instance().channelCount(), 1);
        const QVector<ChannelData> channels = Database::instance().channels(false);
        const ChannelData &channel1 = channels.at(0);
        QCOMPARE(channel1.m_id.value(), "Channel1");
        QCOMPARE(channel1.m_name, "Channel 1");
        QCOMPARE(channel1.m_url, "Channel1Url");
        QCOMPARE(channel1.m_image, "Channel1Image");
    }

    void testGroupsForChannel()
    {
        const QVector<GroupData> groups = Database::instance().groups(ChannelId("Channel1"));
        QCOMPARE(groups.size(), 1);
        QCOMPARE(groups.at(0).m_id.value(), "Group1");
    }

    void testAddChannels()
    {
        ChannelData data2;
        data2.m_id = ChannelId("Channel2");
        data2.m_name = "Channel 2";
        data2.m_url = "Channel2Url";
        data2.m_image = "Channel2Image";
        ChannelData data3;
        data3.m_id = ChannelId("Channel3");
        data3.m_name = "Channel 3";
        data3.m_url = "Channel3Url";
        data3.m_image = "Channel3Image";
        QList<ChannelData> data;
        data.push_back(data2);
        data.push_back(data3);

        Database::instance().addChannels(data, GroupId("Group1"));

        QCOMPARE(Database::instance().channelCount(), 3);
        const QVector<ChannelData> channels = Database::instance().channels(false);
        const ChannelData &channel1 = channels.at(0);
        QCOMPARE(channel1.m_id.value(), "Channel1");
        QCOMPARE(channel1.m_name, "Channel 1");
        QCOMPARE(channel1.m_url, "Channel1Url");
        QCOMPARE(channel1.m_image, "Channel1Image");
        const ChannelData &channel2 = channels.at(1);
        QCOMPARE(channel2.m_id.value(), "Channel2");
        QCOMPARE(channel2.m_name, "Channel 2");
        QCOMPARE(channel2.m_url, "Channel2Url");
        QCOMPARE(channel2.m_image, "Channel2Image");
        const ChannelData &channel3 = channels.at(2);
        QCOMPARE(channel3.m_id.value(), "Channel3");
        QCOMPARE(channel3.m_name, "Channel 3");
        QCOMPARE(channel3.m_url, "Channel3Url");
        QCOMPARE(channel3.m_image, "Channel3Image");
    }

    void testChannelExists()
    {
        QCOMPARE(Database::instance().channelExists(ChannelId("Channel1")), true);
        QCOMPARE(Database::instance().channelExists(ChannelId("Channel2")), true);
        QCOMPARE(Database::instance().channelExists(ChannelId("Channel3")), true);

        QCOMPARE(Database::instance().channelExists(ChannelId("Channel0")), false);
    }

    void testChannel()
    {
        const ChannelData &channel1 = Database::instance().channel(ChannelId("Channel1"));
        QCOMPARE(channel1.m_id.value(), "Channel1");
        QCOMPARE(channel1.m_name, "Channel 1");
        QCOMPARE(channel1.m_url, "Channel1Url");
        QCOMPARE(channel1.m_image, "Channel1Image");
    }

    void testAddFavorite()
    {
        QCOMPARE(Database::instance().favoriteCount(), 0);
        Database::instance().addFavorite(ChannelId("Channel1"));
        QCOMPARE(Database::instance().favoriteCount(), 1);
        Database::instance().addFavorite(ChannelId("Channel2"));
        QCOMPARE(Database::instance().favoriteCount(), 2);
    }

    void testFavorites()
    {
        const QVector<ChannelId> favorites = Database::instance().favorites();
        QCOMPARE(favorites.at(0).value(), "Channel1");
        QCOMPARE(favorites.at(1).value(), "Channel2");
    }

    void testChannelsOnlyFavorites()
    {
        const QVector<ChannelData> channels = Database::instance().channels(true);
        QCOMPARE(channels.size(), 2);
        QCOMPARE(channels.at(0).m_id.value(), "Channel1");
        QCOMPARE(channels.at(1).m_id.value(), "Channel2");
    }

    void testSortFavorites()
    {
        QVector<ChannelId> data;
        data.push_back(ChannelId("Channel2"));
        data.push_back(ChannelId("Channel1"));
        Database::instance().sortFavorites(data);
        const QVector<ChannelId> favorites = Database::instance().favorites();
        QCOMPARE(favorites.size(), 2);
        QCOMPARE(favorites.at(0).value(), "Channel2");
        QCOMPARE(favorites.at(1).value(), "Channel1");
    }

    void testIsFavorite()
    {
        QCOMPARE(Database::instance().isFavorite(ChannelId("Channel1")), true);
        QCOMPARE(Database::instance().isFavorite(ChannelId("Channel2")), true);

        QCOMPARE(Database::instance().isFavorite(ChannelId("Channel3")), false);

        QCOMPARE(Database::instance().isFavorite(ChannelId("Channel0")), false);
    }

    void testRemoveFavorite()
    {
        QCOMPARE(Database::instance().favoriteCount(), 2);
        Database::instance().removeFavorite(ChannelId("Channel2"));
        QCOMPARE(Database::instance().favoriteCount(), 1);
        QCOMPARE(Database::instance().isFavorite(ChannelId("Channel1")), true);
        QCOMPARE(Database::instance().isFavorite(ChannelId("Channel2")), false);
    }

    void testClearFavorites()
    {
        QCOMPARE(Database::instance().favoriteCount(), 1);
        Database::instance().clearFavorites();
        QCOMPARE(Database::instance().favoriteCount(), 0);
        QCOMPARE(Database::instance().isFavorite(ChannelId("Channel1")), false);
    }

    void testAddProgram()
    {
        ProgramData data;
        data.m_id = ProgramId("Program1");
        data.m_url = "Program1Url";
        data.m_channelId = ChannelId("Channel1");
        data.m_startTime = QDateTime::fromString("2022-12-28T00:00:00", Qt::ISODate);
        data.m_stopTime = QDateTime::fromString("2022-12-28T01:00:00", Qt::ISODate);
        data.m_title = "Program1Title";
        data.m_subtitle = "Program1Subtitle";
        data.m_description = "Program1Description";
        data.m_descriptionFetched = true;
        data.m_categories = {"Category1"};

        Database::instance().addProgram(data);

        QCOMPARE(Database::instance().programCount(ChannelId("Channel1")), 1);
        const QVector<ProgramData> programs = Database::instance().programs(ChannelId("Channel1"));
        const ProgramData &program1 = programs.at(0);
        QCOMPARE(program1.m_id.value(), "Program1");
        QCOMPARE(program1.m_url, "Program1Url");
        QCOMPARE(program1.m_channelId.value(), "Channel1");
        QCOMPARE(program1.m_startTime, QDateTime::fromString("2022-12-28T00:00:00", Qt::ISODate));
        QCOMPARE(program1.m_stopTime, QDateTime::fromString("2022-12-28T01:00:00", Qt::ISODate));
        QCOMPARE(program1.m_title, "Program1Title");
        QCOMPARE(program1.m_subtitle, "Program1Subtitle");
        QCOMPARE(program1.m_description, "Program1Description");
        QCOMPARE(program1.m_descriptionFetched, true);
        QCOMPARE(program1.m_categories, {"Category1"});
    }

    void testAddPrograms()
    {
        ProgramData data2;
        data2.m_id = ProgramId("Program2");
        data2.m_url = "Program2Url";
        data2.m_channelId = ChannelId("Channel1");
        data2.m_startTime = QDateTime::fromString("2022-12-28T01:00:00", Qt::ISODate);
        data2.m_stopTime = QDateTime::fromString("2022-12-28T02:00:00", Qt::ISODate);
        data2.m_title = "Program2Title";
        data2.m_subtitle = "Program2Subtitle";
        data2.m_description = "Program2Description";
        data2.m_descriptionFetched = false;
        data2.m_categories = {"Category2"};
        ProgramData data3;
        data3.m_id = ProgramId("Program3");
        data3.m_url = "Program3Url";
        data3.m_channelId = ChannelId("Channel2");
        data3.m_startTime = QDateTime::fromString("2022-12-28T02:00:00", Qt::ISODate);
        data3.m_stopTime = QDateTime::fromString("2022-12-28T03:00:00", Qt::ISODate);
        data3.m_title = "Program3Title";
        data3.m_subtitle = "Program3Subtitle";
        data3.m_description = "Program3Description";
        data3.m_descriptionFetched = true;
        data3.m_categories = {"Category1", "Category2"};
        QVector<ProgramData> data;
        data.push_back(data2);
        data.push_back(data3);

        Database::instance().addPrograms(data);

        QCOMPARE(Database::instance().programCount(ChannelId("Channel1")), 2);
        QCOMPARE(Database::instance().programCount(ChannelId("Channel2")), 1);
        const QVector<ProgramData> programsChannel1 = Database::instance().programs(ChannelId("Channel1"));
        const ProgramData &program1 = programsChannel1.at(0);
        QCOMPARE(program1.m_id.value(), "Program1");
        QCOMPARE(program1.m_url, "Program1Url");
        QCOMPARE(program1.m_channelId.value(), "Channel1");
        QCOMPARE(program1.m_startTime, QDateTime::fromString("2022-12-28T00:00:00", Qt::ISODate));
        QCOMPARE(program1.m_stopTime, QDateTime::fromString("2022-12-28T01:00:00", Qt::ISODate));
        QCOMPARE(program1.m_title, "Program1Title");
        QCOMPARE(program1.m_subtitle, "Program1Subtitle");
        QCOMPARE(program1.m_description, "Program1Description");
        QCOMPARE(program1.m_descriptionFetched, true);
        QCOMPARE(program1.m_categories, {"Category1"});
        const ProgramData &program2 = programsChannel1.at(1);
        QCOMPARE(program2.m_id.value(), "Program2");
        QCOMPARE(program2.m_url, "Program2Url");
        QCOMPARE(program2.m_channelId.value(), "Channel1");
        QCOMPARE(program2.m_startTime, QDateTime::fromString("2022-12-28T01:00:00", Qt::ISODate));
        QCOMPARE(program2.m_stopTime, QDateTime::fromString("2022-12-28T02:00:00", Qt::ISODate));
        QCOMPARE(program2.m_title, "Program2Title");
        QCOMPARE(program2.m_subtitle, "Program2Subtitle");
        QCOMPARE(program2.m_description, "Program2Description");
        QCOMPARE(program2.m_descriptionFetched, false);
        QCOMPARE(program2.m_categories, {"Category2"});
        const QVector<ProgramData> programsChannel2 = Database::instance().programs(ChannelId("Channel2"));
        const ProgramData &program3 = programsChannel2.at(0);
        QCOMPARE(program3.m_id.value(), "Program3");
        QCOMPARE(program3.m_url, "Program3Url");
        QCOMPARE(program3.m_channelId.value(), "Channel2");
        QCOMPARE(program3.m_startTime, QDateTime::fromString("2022-12-28T02:00:00", Qt::ISODate));
        QCOMPARE(program3.m_stopTime, QDateTime::fromString("2022-12-28T03:00:00", Qt::ISODate));
        QCOMPARE(program3.m_title, "Program3Title");
        QCOMPARE(program3.m_subtitle, "Program3Subtitle");
        QCOMPARE(program3.m_description, "Program3Description");
        QCOMPARE(program3.m_descriptionFetched, true);
        QCOMPARE(program3.m_categories.at(0), "Category1");
        QCOMPARE(program3.m_categories.at(1), "Category2");
    }

    void testProgramExists()
    {
        QCOMPARE(Database::instance().programExists(ChannelId("Channel1"), QDateTime::fromString("2022-12-28T02:00:00", Qt::ISODate)), true);
        QCOMPARE(Database::instance().programExists(ChannelId("Channel1"), QDateTime::fromString("2022-12-28T02:00:01", Qt::ISODate)), false);
    }

    void testUpdateProgramDescription()
    {
        Database::instance().updateProgramDescription(ProgramId("Program2"), "Program2DescriptionUpdated");
        const QVector<ProgramData> programsChannel1 = Database::instance().programs(ChannelId("Channel1"));
        const ProgramData &program2 = programsChannel1.at(1);
        QCOMPARE(program2.m_description, "Program2DescriptionUpdated");
        QCOMPARE(program2.m_descriptionFetched, true);
    }

    void testPrograms()
    {
        const QMap<ChannelId, QVector<ProgramData>> programs = Database::instance().programs();
        const QVector<ProgramData> programsChannel1 = programs[ChannelId("Channel1")];
        QCOMPARE(programsChannel1.size(), 2);
        QCOMPARE(programsChannel1.at(0).m_id.value(), "Program1");
        QCOMPARE(programsChannel1.at(1).m_id.value(), "Program2");
        const QVector<ProgramData> programsChannel2 = programs[ChannelId("Channel2")];
        QCOMPARE(programsChannel2.size(), 1);
        QCOMPARE(programsChannel2.at(0).m_id.value(), "Program3");
    }
};

QTEST_GUILESS_MAIN(DatabaseTest)

#include "databasetest.moc"
