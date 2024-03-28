// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import Qt.labs.platform
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.TellySkout
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    property string lastChannel: ""
    property string groupFilter: ""
    property bool sortable: false
    property bool showOnlyFavorites: false
    property bool sortingChanged: false

    title: i18n("Channels")
    Component.onDestruction: {
        if (sortable && sortingChanged) {
            channelsModel.save();
            sortingChanged = false;
        }
    }

    Kirigami.PlaceholderMessage {
        visible: channelList.count === 0
        width: Kirigami.Units.gridUnit * 20
        anchors.centerIn: parent
        text: i18n("Loading channels...")

        Controls.BusyIndicator {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }

    }

    ListView {
        id: channelList

        anchors.fill: parent
        model: root.showOnlyFavorites ? channelsModel : proxyModel
        currentIndex: -1 // do not select first list item
        reuseItems: true

        ChannelsProxyModel {
            id: proxyModel

            onlyFavorites: false
            group: root.groupFilter
            sourceModel: channelsModel
        }

        ChannelsModel {
            id: channelsModel

            onlyFavorites: root.showOnlyFavorites
            onRowsMoved: {
                if (root.sortable)
                    root.sortingChanged = true;

            }
        }

        delegate: ChannelListDelegate {
            listView: channelList
            sortable: root.sortable
        }

    }

}
