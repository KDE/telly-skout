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

    Kirigami.PlaceholderMessage {
        visible: channelList.count === 0

        width: Kirigami.Units.gridUnit * 20
        icon.name: "rss"
        anchors.centerIn: parent

        text: i18n("No Channels added yet")
    }

    ListView {
        id: channelList

        anchors.fill: parent

        model: ChannelsProxyModel {
            id: proxyModel
            groupName: ""
            country: ""
            sourceModel: channelsModel
        }

        delegate: ChannelListDelegate {
        }

        ChannelsModel {
            id: channelsModel
        }
    }
}
