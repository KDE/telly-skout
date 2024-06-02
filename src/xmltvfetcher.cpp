// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "xmltvfetcher.h"

#include "TellySkoutSettings.h"
#include "programdata.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QDebug>
#include <QString>
#include <QStringView>

XmltvFetcher::XmltvFetcher()
    : m_doc(QStringLiteral("xmltv"))
{
    m_provider.get(QUrl::fromLocalFile(TellySkoutSettings::xmltvFile()), std::bind(&XmltvFetcher::open, this, std::placeholders::_1), [](const Error &error) {
        Q_UNUSED(error)
        qCritical() << "Failed to open" << TellySkoutSettings::xmltvFile();
    });

    connect(TellySkoutSettings::self(), &TellySkoutSettings::xmltvFileChanged, this, [this]() {
        m_provider.get(QUrl::fromLocalFile(TellySkoutSettings::xmltvFile()),
                       std::bind(&XmltvFetcher::open, this, std::placeholders::_1),
                       [](const Error &error) {
                           Q_UNUSED(error)
                           qCritical() << "Failed to open" << TellySkoutSettings::xmltvFile();
                       });
    });
}

void XmltvFetcher::fetchGroups(std::function<void(const QVector<GroupData> &)> callback, std::function<void(const Error &)> errorCallback)
{
    Q_UNUSED(errorCallback);

    QVector<GroupData> groups;

    GroupData data;
    data.m_id = GroupId(QStringLiteral("xmltv"));
    data.m_name = i18n("XMLTV");
    data.m_url = TellySkoutSettings::xmltvFile();

    groups.push_back(data);

    if (callback) {
        callback(groups);
    }
}

void XmltvFetcher::fetchGroup(const QString &url,
                              const GroupId &groupId,
                              std::function<void(const QList<ChannelData> &)> callback,
                              std::function<void(const Error &)> errorCallback)
{
    Q_UNUSED(errorCallback)

    qDebug() << "Starting to fetch group (" << groupId.value() << ", " << url << ")";

    QMap<ChannelId, ChannelData> channels;

    QDomNodeList nodes = m_doc.elementsByTagName(QStringLiteral("channel"));

    for (int i = 0; i < nodes.count(); i++) {
        QDomNode elm = nodes.at(i);
        if (elm.isElement()) {
            const QDomNamedNodeMap &attributes = elm.attributes();
            const ChannelId id = ChannelId(attributes.namedItem(QStringLiteral("id")).toAttr().value());

            const QString &name = elm.firstChildElement(QStringLiteral("display-name")).text();
            const QString &icon = elm.firstChildElement(QStringLiteral("icon")).attributes().namedItem(QStringLiteral("src")).toAttr().value();

            fetchChannel(id, name, icon, channels);
        }
    }

    if (callback) {
        callback(channels.values());
    }
}

void XmltvFetcher::fetchProgram(const ChannelId &channelId,
                                std::function<void(const QVector<ProgramData> &)> callback,
                                std::function<void(const Error &)> errorCallback)
{
    Q_UNUSED(errorCallback)

    QVector<ProgramData> programs;

    QDomNodeList programNodes = m_doc.elementsByTagName(QStringLiteral("programme"));
    for (int i = 0; i < programNodes.count(); i++) {
        const QDomNode &program = programNodes.at(i);
        const QDomNamedNodeMap &attributes = program.attributes();

        if (channelId.value() != attributes.namedItem(QStringLiteral("channel")).toAttr().value()) {
            continue; // TODO: do not loop all programs for each channel
        }
        programs.push_back(processProgram(program));
    }

    if (callback) {
        callback(programs);
    }
}

void XmltvFetcher::fetchProgramDescription(const ChannelId &channelId,
                                           const ProgramId &programId,
                                           const QString &url,
                                           std::function<void(const QString &)> callback,
                                           std::function<void(const Error &)> errorCallback)
{
    Q_UNUSED(channelId)
    Q_UNUSED(programId)
    Q_UNUSED(url)
    Q_UNUSED(callback)
    Q_UNUSED(errorCallback)

    // nothing to be done (already fetched as part of the program)
}

QString XmltvFetcher::image(const QString &url, std::function<void()> callback, std::function<void(const Error &)> errorCallback)
{
    Q_UNUSED(url);
    Q_UNUSED(callback)
    Q_UNUSED(errorCallback)

    return QStringLiteral("");
}

QString XmltvFetcher::imagePath(const QString &url)
{
    Q_UNUSED(url);
    return QStringLiteral("");
}

void XmltvFetcher::open(QByteArray data)
{
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if (!m_doc.setContent(data, &errorMsg, &errorLine, &errorColumn)) {
        qCritical() << "Could not read XML:" << errorMsg << "(l" << errorLine << ":" << errorColumn << ")";
    }
}

void XmltvFetcher::fetchChannel(const ChannelId &channelId, const QString &name, const QString &icon, QMap<ChannelId, ChannelData> &channels)
{
    if (!channels.contains(channelId)) {
        ChannelData data;
        data.m_id = channelId;
        data.m_name = name;
        data.m_url = QStringLiteral("");
        data.m_image = icon;

        channels.insert(channelId, data);
    }
}

ProgramData XmltvFetcher::processProgram(const QDomNode &program)
{
    ProgramData data;

    const QDomNamedNodeMap &attributes = program.attributes();
    data.m_channelId = ChannelId(attributes.namedItem(QStringLiteral("channel")).toAttr().value());
    const QString &startTimeString = attributes.namedItem(QStringLiteral("start")).toAttr().value();
    QDateTime startTime = QDateTime::fromString(startTimeString.left(14), QStringLiteral("yyyyMMddHHmmss"));
    const int startTimeOffset = QStringView{startTimeString}.right(5).left(3).toInt();
    startTime.setOffsetFromUtc(startTimeOffset * 3600);
    startTime = startTime.toUTC();
    data.m_startTime = startTime;
    // channel + start time can be used as ID
    data.m_id = ProgramId(data.m_channelId.value() + QStringLiteral("_") + QString::number(startTime.toSecsSinceEpoch()));
    const QString &stopTimeString = attributes.namedItem(QStringLiteral("stop")).toAttr().value();
    QDateTime stopTime = QDateTime::fromString(stopTimeString.left(14), QStringLiteral("yyyyMMddHHmmss"));
    const int stopTimeOffset = QStringView{stopTimeString}.right(5).left(3).toInt();
    stopTime.setOffsetFromUtc(stopTimeOffset * 3600);
    stopTime = stopTime.toUTC();
    data.m_stopTime = stopTime;
    data.m_title = program.namedItem(QStringLiteral("title")).toElement().text();
    data.m_subtitle = program.namedItem(QStringLiteral("sub-title")).toElement().text();
    data.m_description = program.namedItem(QStringLiteral("desc")).toElement().text();

    if (program.isElement()) {
        QDomNodeList categoryNodes = program.toElement().elementsByTagName(QStringLiteral("category"));
        for (int i = 0; i < categoryNodes.count(); i++) {
            data.m_categories.push_back(categoryNodes.at(i).toElement().text());
        }
    }

    data.m_descriptionFetched = true;

    return data;
}

#include "moc_xmltvfetcher.cpp"
