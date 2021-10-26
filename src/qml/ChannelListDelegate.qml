/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.TellySkout 1.0

Kirigami.SwipeListItem {
    id: listItem

    property var listView
    property bool sortable: false

    contentItem: RowLayout {
        Kirigami.ListItemDragHandle {
            listItem: listItem
            listView: listItem.listView
            onMoveRequested: sortable ? listView.model.move(oldIndex, newIndex) : {}
            visible: listItem.sortable
        }

        Kirigami.Icon {
            source: model.channel.refreshing ? "view-refresh" : model.channel.image === "" ? "rss" : Fetcher.image(model.channel.image)
        }

        Controls.Label {
            Layout.fillWidth: true
            height: Math.max(implicitHeight, Kirigami.Units.iconSizes.smallMedium)
            text: model.channel.displayName || model.channel.name
            color: listItem.checked || (listItem.pressed && !listItem.checked && !listItem.sectionDelegate) ? listItem.activeTextColor : listItem.textColor
        }
    }

    actions: [
        Kirigami.Action {
            readonly property string favoriteIcon: "favorite"
            readonly property string noFavoriteIcon: "list-add"

            icon.name: model.channel.favorite ? favoriteIcon : noFavoriteIcon
            text: i18n("Favorite")

            onTriggered: {
                if (model.channel.favorite) {
                    channelsModel.setFavorite(model.channel.url, false)
                    icon.name = noFavoriteIcon
                } else {
                    channelsModel.setFavorite(model.channel.url, true)
                    icon.name = favoriteIcon
                }
            }
        }
    ]
}
