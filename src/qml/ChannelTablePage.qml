/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.14
import QtQuick.Controls 2.15 as Controls
import Qt.labs.platform 1.1
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.TellySkout 1.0

Kirigami.Page {
    id: root

    title: i18n("Favorites (table)")

    property string lastChannel: ""


    /*supportsRefreshing: true
    onRefreshingChanged:
        if(refreshing)  {
            Fetcher.fetchAll()
            refreshing = false
        }*/

    contextualActions: [
        Kirigami.Action {
            text: i18n("Refresh all channels")
            iconName: "view-refresh"
            onTriggered: refreshing = true
            visible: !Kirigami.Settings.isMobile
        }
    ]

    Kirigami.PlaceholderMessage {
        visible: channelTable.count === 0

        width: Kirigami.Units.gridUnit * 20
        icon.name: "rss"
        anchors.centerIn: parent

        text: i18n("No Channels added yet")
    }

    Controls.VerticalHeaderView {
        id: verticalHeader
        syncView: channelTable
        anchors.top: channelTable.top
    }

    Controls.HorizontalHeaderView {
        id: horizontalHeader
        syncView: channelTable
        anchors.left: channelTable.left
    }

    TableView {
        id: channelTable
        anchors.fill: root
        width: root.width - verticalHeader.width
        height: root.height - horizontalHeader.height
        anchors.left: verticalHeader.right
        anchors.top: horizontalHeader.bottom
        columnSpacing: 0
        rowSpacing: 0
        clip: true

        model: ChannelsTableModel {}

        delegate: ChannelTableDelegate {
        }
    }
}
