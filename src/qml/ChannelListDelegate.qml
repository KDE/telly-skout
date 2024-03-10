// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14
import org.kde.TellySkout 1.0
import org.kde.kirigami 2.19 as Kirigami

Kirigami.SwipeListItem {
    id: listItem

    property var listView
    property bool sortable: false

    actions: [
        Kirigami.Action {
            icon.name: checked ? "favorite" : "list-add"
            text: i18n("Favorite")
            checkable: true
            checked: model.channel.favorite
            onToggled: channelsModel.setFavorite(model.channel.id, checked)
        }
    ]

    contentItem: RowLayout {
        Kirigami.ListItemDragHandle {
            listItem: listItem
            listView: listItem.listView
            onMoveRequested: (oldIndex, newIndex) => {
                if (sortable)
                    listView.model.move(oldIndex, newIndex);

            }
            visible: listItem.sortable
        }

        Kirigami.Icon {
            source: model.channel.refreshing ? "view-refresh" : model.channel.image === "" ? "tv" : Fetcher.image(model.channel.image)
        }

        Controls.Label {
            Layout.fillWidth: true
            height: Math.max(implicitHeight, Kirigami.Units.iconSizes.smallMedium)
            text: model.channel.displayName || model.channel.name
            color: listItem.checked || (listItem.pressed && !listItem.checked && !listItem.sectionDelegate) ? listItem.activeTextColor : listItem.textColor
        }

    }

}
