// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import Qt.labs.platform
import QtQuick
import QtQuick.Layouts
import org.kde.TellySkout
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    property string lastChannel: ""
    property string groupFilter: ""

    title: i18nc("@title", "Channels")

    ListView {
        id: channelList

        anchors.fill: parent
        model: proxyModel
        currentIndex: -1 // do not select first list item
        reuseItems: true

        Kirigami.LoadingPlaceholder {
            visible: channelList.count === 0
            anchors.centerIn: parent
            text: i18n("Loading channels...")
        }

        ChannelsProxyModel {
            id: proxyModel

            onlyFavorites: false
            group: root.groupFilter
            sourceModel: channelsModel
        }

        ChannelsModel {
            id: channelsModel

            onlyFavorites: false
        }

        delegate: ChannelListDelegate {
            listView: channelList
            sortable: false
        }
    }
}
