// SPDX-FileCopyrightText: 2023 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "../src/fetcher.h"

#include "../src/database.h"
#include "../src/fetcherimpl.h"

#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

namespace
{
class MockFetcherImpl : public FetcherImpl
{
    Q_OBJECT
public:
    virtual ~MockFetcherImpl() = default;

    void fetchGroups(std::function<void(const QVector<GroupData> &)> callback = nullptr, std::function<void(const Error &)> errorCallback = nullptr) override
    {
        Q_UNUSED(errorCallback)

        QVector<GroupData> groups;
        GroupData data;
        data.m_id = GroupId(QStringLiteral("TestGroup"));
        groups.push_back(data);
        callback(groups);
    }

    void fetchGroup(const QString &url,
                    const GroupId &groupId,
                    std::function<void(const QList<ChannelData> &)> callback = nullptr,
                    std::function<void(const Error &)> errorCallback = nullptr) override
    {
        Q_UNUSED(url)
        Q_UNUSED(groupId)
        Q_UNUSED(errorCallback)

        QList<ChannelData> channels;
        ChannelData data;
        data.m_id = ChannelId(QStringLiteral("TestChannel"));
        channels.push_back(data);
        callback(channels);
    }

    void fetchProgram(const ChannelId &channelId,
                      std::function<void(const QVector<ProgramData> &)> callback = nullptr,
                      std::function<void(const Error &)> errorCallback = nullptr) override
    {
        Q_UNUSED(channelId)
        Q_UNUSED(errorCallback)

        QVector<ProgramData> programs;
        ProgramData data;
        data.m_id = ProgramId(QStringLiteral("TestProgram"));
        data.m_channelId = ChannelId(QStringLiteral("TestChannel"));
        programs.push_back(data);
        callback(programs);
    }

    void fetchProgramDescription(const ChannelId &channelId,
                                 const ProgramId &programId,
                                 const QString &url,
                                 std::function<void(const QString &)> callback = nullptr,
                                 std::function<void(const Error &)> errorCallback = nullptr) override
    {
        Q_UNUSED(channelId)
        Q_UNUSED(programId)
        Q_UNUSED(url)
        Q_UNUSED(errorCallback)

        callback(QStringLiteral("TestDescription"));
    }

    QString image(const QString &url, std::function<void()> callback = nullptr, std::function<void(const Error &)> errorCallback = nullptr) override
    {
        Q_UNUSED(url)
        Q_UNUSED(errorCallback)

        callback();

        return QStringLiteral("TestImage");
    }

    QString imagePath(const QString &url) override
    {
        Q_UNUSED(url)

        return QStringLiteral("TestImagePath");
    }
};
}

class FetcherTest : public QObject
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

        std::unique_ptr<FetcherImpl> fetcherImpl(new MockFetcherImpl());
        Fetcher::instance().setImpl(std::move(fetcherImpl));
    }

    void testFetchGroups()
    {
        Fetcher::instance().fetchGroups();
        QCOMPARE(Database::instance().groupCount(), 1);
        const QVector<GroupData> groups = Database::instance().groups();
        QCOMPARE(groups.at(0).m_id.value(), QStringLiteral("TestGroup"));
    }

    void testFetchGroup()
    {
        QSignalSpy groupUpdatedSpy(&Fetcher::instance(), SIGNAL(groupUpdated(const GroupId &)));
        QVERIFY(groupUpdatedSpy.isValid());
        QCOMPARE(groupUpdatedSpy.count(), 0);
        Fetcher::instance().fetchGroup(QString(), QString());
        QCOMPARE(groupUpdatedSpy.count(), 1);
        QCOMPARE(Database::instance().channelCount(), 1);
        const QVector<ChannelData> channels = Database::instance().channels(false);
        QCOMPARE(channels.at(0).m_id.value(), QStringLiteral("TestChannel"));
    }

    void testFetchFavorites()
    {
        Database::instance().addFavorite(ChannelId(QStringLiteral("TestChannel")));
        QSignalSpy channelUpdatedSpy(&Fetcher::instance(), SIGNAL(channelUpdated(const ChannelId &)));
        QVERIFY(channelUpdatedSpy.isValid());
        QCOMPARE(channelUpdatedSpy.count(), 0);
        Fetcher::instance().fetchFavorites();
        QCOMPARE(channelUpdatedSpy.count(), 1);
        QCOMPARE(Database::instance().programCount(ChannelId(QStringLiteral("TestChannel"))), 1);
        const QVector<ProgramData> programs = Database::instance().programs(ChannelId(QStringLiteral("TestChannel")));
        QCOMPARE(programs.at(0).m_id.value(), QStringLiteral("TestProgram"));
    }

    void testFetchProgramDescription()
    {
        QSignalSpy channelUpdatedSpy(&Fetcher::instance(), SIGNAL(channelUpdated(const ChannelId &)));
        QVERIFY(channelUpdatedSpy.isValid());
        QCOMPARE(channelUpdatedSpy.count(), 0);
        Fetcher::instance().fetchProgramDescription(QStringLiteral("TestChannel"), QStringLiteral("TestProgram"), QString());
        QCOMPARE(channelUpdatedSpy.count(), 1);
        const QVector<ProgramData> programs = Database::instance().programs(ChannelId(QStringLiteral("TestChannel")));
        QCOMPARE(programs.at(0).m_description, QStringLiteral("TestDescription"));
    }

    void testImage()
    {
        QSignalSpy imageDownloadFinishedSpy(&Fetcher::instance(), SIGNAL(imageDownloadFinished(QString)));
        QVERIFY(imageDownloadFinishedSpy.isValid());
        QCOMPARE(imageDownloadFinishedSpy.count(), 0);
        QCOMPARE(Fetcher::instance().image(QString()), QStringLiteral("TestImage"));
        QCOMPARE(imageDownloadFinishedSpy.count(), 1);
    }
};

QTEST_GUILESS_MAIN(FetcherTest)

#include "fetchertest.moc"
