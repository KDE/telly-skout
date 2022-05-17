// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

#include "xmltvfetcher.h"

#include "TellySkoutSettings.h"
#include "database.h"
#include "programdata.h"

#include <KLocalizedString>

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QString>

XmltvFetcher::XmltvFetcher()
    : m_doc("xmltv")
{
    const TellySkoutSettings settings;

    if (!open(settings.xmltvFile())) {
        qCritical() << "Failed to open" << settings.xmltvFile();
    }

    connect(&settings, &TellySkoutSettings::xmltvFileChanged, this, [this]() {
        const TellySkoutSettings settings;
        if (!open(settings.xmltvFile())) {
            qCritical() << "Failed to open" << settings.xmltvFile();
        }
    });
}

void XmltvFetcher::fetchGroups()
{
    const GroupId id = GroupId("xmltv");
    const QString name = i18n("XMLTV");

    Q_EMIT startedFetchingGroup(id);

    TellySkoutSettings settings;
    Database::instance().addGroup(id, name, settings.xmltvFile());

    Q_EMIT groupUpdated(id);
}

void XmltvFetcher::fetchGroup(const QString &url, const GroupId &groupId)
{
    qDebug() << "Starting to fetch group (" << groupId.value() << ", " << url << ")";

    QDomNodeList nodes = m_doc.elementsByTagName("channel");

    for (int i = 0; i < nodes.count(); i++) {
        QDomNode elm = nodes.at(i);
        if (elm.isElement()) {
            const QDomNamedNodeMap &attributes = elm.attributes();
            const ChannelId id = ChannelId(attributes.namedItem("id").toAttr().value());

            const QString &name = elm.firstChildElement("display-name").text();
            const QString &icon = elm.firstChildElement("icon").attributes().namedItem("src").toAttr().value();

            fetchChannel(id, name, groupId, icon);
        }
    }

    Q_EMIT groupUpdated(groupId);
}

void XmltvFetcher::fetchProgram(const ChannelId &channelId)
{
    QDomNodeList programs = m_doc.elementsByTagName("programme");
    for (int i = 0; i < programs.count(); i++) {
        const QDomNode &program = programs.at(i);
        const QDomNamedNodeMap &attributes = program.attributes();

        if (channelId.value() != attributes.namedItem("channel").toAttr().value()) {
            continue; // TODO: do not loop all programs for each channel
        }
        processProgram(program);
    }

    Q_EMIT channelUpdated(channelId);
}

void XmltvFetcher::fetchProgramDescription(const ChannelId &channelId, const ProgramId &programId, const QString &url)
{
    Q_UNUSED(channelId)
    Q_UNUSED(programId)
    Q_UNUSED(url)

    // nothing to be done (already fetched as part of the program)
}

bool XmltvFetcher::open(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    if (!m_doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();
    return true;
}

void XmltvFetcher::fetchChannel(const ChannelId &channelId, const QString &name, const GroupId &groupId, const QString &icon)
{
    if (!Database::instance().channelExists(channelId)) {
        Q_EMIT startedFetchingChannel(channelId);

        ChannelData data;
        data.m_id = channelId;
        data.m_name = name;
        data.m_url = "";
        data.m_image = icon;
        Database::instance().addChannel(data, groupId);

        Q_EMIT channelUpdated(channelId);
    }
}

void XmltvFetcher::processGroup(const QDomElement &group)
{
    const GroupId id = GroupId(group.attributes().namedItem("id").toAttr().value());
    const QString &name = group.text();

    Q_EMIT startedFetchingGroup(id);

    Database::instance().addGroup(id, name, "");

    Q_EMIT groupUpdated(id);
}

void XmltvFetcher::processProgram(const QDomNode &program)
{
    ProgramData data;

    const QDomNamedNodeMap &attributes = program.attributes();
    data.m_channelId = ChannelId(attributes.namedItem("channel").toAttr().value());
    const QString &startTimeString = attributes.namedItem("start").toAttr().value();
    QDateTime startTime = QDateTime::fromString(startTimeString.left(14), "yyyyMMddHHmmss");
    const int startTimeOffset = startTimeString.right(5).leftRef(3).toInt();
    startTime.setOffsetFromUtc(startTimeOffset * 3600);
    startTime = startTime.toUTC();
    data.m_startTime = startTime;
    // channel + start time can be used as ID
    data.m_id = ProgramId(data.m_channelId.value() + "_" + QString::number(startTime.toSecsSinceEpoch()));
    const QString &stopTimeString = attributes.namedItem("stop").toAttr().value();
    QDateTime stopTime = QDateTime::fromString(stopTimeString.left(14), "yyyyMMddHHmmss");
    const int stopTimeOffset = stopTimeString.right(5).leftRef(3).toInt();
    stopTime.setOffsetFromUtc(stopTimeOffset * 3600);
    stopTime = stopTime.toUTC();
    data.m_stopTime = stopTime;
    data.m_title = program.namedItem("title").toElement().text();
    data.m_subtitle = program.namedItem("sub-title").toElement().text();
    data.m_description = program.namedItem("desc").toElement().text();
    data.m_category = program.namedItem("category").toElement().text();

    data.m_descriptionFetched = true;

    Database::instance().addProgram(data);
}
