/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.TellyScout 1.0

Kirigami.ScrollablePage {
    id: page

    property var channel

    title: channel.displayName || channel.name
    supportsRefreshing: true

    onRefreshingChanged:
        if(refreshing) {
            channel.refresh()
        }

    Connections {
        target: channel
        function onRefreshingChanged(refreshing) {
            if(!refreshing)
                page.refreshing = refreshing
        }
    }

    contextualActions: [
        Kirigami.Action {
            iconName: "help-about-symbolic"
            text: i18n("Details")
            onTriggered: {
                while(pageStack.depth > 2)
                    pageStack.pop()
                pageStack.push("qrc:/ChannelDetailsPage.qml", {"channel": channel})
            }
        }
    ]

    actions.main: Kirigami.Action {
        iconName: "view-refresh"
        text: i18n("Refresh Channel")
        onTriggered: page.refreshing = true
        visible: !Kirigami.Settings.isMobile || programList.count === 0
    }

    Kirigami.PlaceholderMessage {
        visible: programList.count === 0

        width: Kirigami.Units.gridUnit * 20
        anchors.centerIn: parent

        text: channel.errorId === 0 ? i18n("No Programs available") : i18n("Error (%1): %2", channel.errorId, channel.errorString)
        icon.name: channel.errorId === 0 ? "" : "data-error"
    }

    ListView {
        id: programList
        visible: count !== 0
        model: page.channel.programs

        delegate: ProgramListDelegate { channelTitle : channel.displayName || channel.name }
    }
}
