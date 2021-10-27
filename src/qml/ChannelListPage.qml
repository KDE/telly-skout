/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import Qt.labs.platform 1.1
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.TellySkout 1.0

Kirigami.ScrollablePage {
    id: root

    title: countryFilter ? i18n("Channels") + " (" + i18n(countryFilter) + ")" : i18n("Channels")

    property string lastChannel: ""
    property alias groupFilter: proxyModel.groupName
    property alias countryFilter: proxyModel.country
    property bool sortable: false
    property bool onlyFavorites: false

    Kirigami.PlaceholderMessage {
        visible: channelList.count === 0

        width: Kirigami.Units.gridUnit * 20
        anchors.centerIn: parent

        Controls.BusyIndicator {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }
        text: i18n("Loading channels...")
    }

    ListView {
        id: channelList

        anchors.fill: parent

        model: root.onlyFavorites ? channelsModel : proxyModel
        delegate: Kirigami.DelegateRecycler {
            width: parent ? parent.width : implicitWidth
            sourceComponent: delegateComponent
        }

        ChannelsProxyModel {
            id: proxyModel
            groupName: ""
            country: ""
            sourceModel: channelsModel
        }

        ChannelsModel {
            id: channelsModel
            onlyFavorites: root.onlyFavorites
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
