// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import Qt.labs.platform
import QtQuick
import QtQuick.Layouts
import org.kde.TellySkout
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: root

    property bool sortingChanged: false

    title: i18nc("@title", "Sort Favorites")
    Component.onDestruction: {
        if (sortingChanged) {
            channelsModel.save();
            sortingChanged = false;
        }
    }

    ListView {
        id: channelList

        anchors.fill: parent
        model: channelsModel
        currentIndex: -1 // do not select first list item
        reuseItems: true

        Kirigami.LoadingPlaceholder {
            visible: channelList.count === 0
            anchors.centerIn: parent
            text: i18n("Loading channels...")
        }

        ChannelsModel {
            id: channelsModel

            onlyFavorites: true
            onRowsMoved: {
                root.sortingChanged = true;
            }
        }

        delegate: ChannelListDelegate {
            listView: channelList
            sortable: true
        }
    }
}
