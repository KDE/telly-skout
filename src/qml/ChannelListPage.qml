// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

import Qt.labs.platform 1.1
import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14
import org.kde.TellySkout 1.0
import org.kde.kirigami 2.19 as Kirigami

Kirigami.ScrollablePage {
    id: root

    property string lastChannel: ""
    property alias onlyFavoritesFilter: proxyModel.onlyFavorites
    property alias groupFilter: proxyModel.group
    property bool sortable: false
    property bool onlyFavorites: false

    title: i18n("Channels")
    Component.onDestruction: {
        if (root.sortable)
            channelsModel.save();

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
        model: root.onlyFavorites ? channelsModel : proxyModel
        currentIndex: -1 // do not select first list item

        ChannelsProxyModel {
            id: proxyModel

            onlyFavorites: false
            group: ""
            sourceModel: channelsModel
        }

        ChannelsModel {
            id: channelsModel

            onlyFavorites: root.onlyFavorites
        }

        delegate: Kirigami.DelegateRecycler {
            width: parent ? parent.width : implicitWidth
            sourceComponent: delegateComponent
        }

    }

    Component {
        id: delegateComponent

        ChannelListDelegate {
            listView: channelList
            sortable: root.sortable
        }

    }

}
