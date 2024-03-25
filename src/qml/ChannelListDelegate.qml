// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import QtQuick.Templates as Templates
import org.kde.TellySkout
import org.kde.kirigami as Kirigami

Templates.ItemDelegate {
    id: delegate

    property bool sortable: false
    property var listView

    width: parent ? parent.width : implicitWidth
    height: listItem.implicitHeight

    Controls.ItemDelegate {
        id: listItem

        down: false
        hoverEnabled: false
        width: parent.width

        contentItem: RowLayout {
            Kirigami.ListItemDragHandle {
                listItem: listItem
                listView: delegate.listView
                onMoveRequested: (oldIndex, newIndex) => {
                    if (sortable)
                        listView.model.move(oldIndex, newIndex);

                }
                visible: delegate.sortable
            }

            Kirigami.Icon {
                source: model.channel.refreshing ? "view-refresh" : model.channel.image === "" ? "tv" : Fetcher.image(model.channel.image)
            }

            Controls.Label {
                Layout.fillWidth: true
                height: Math.max(implicitHeight, Kirigami.Units.iconSizes.smallMedium)
                text: model.channel.displayName || model.channel.name
            }

            Controls.ToolButton {
                display: Controls.AbstractButton.IconOnly
                Controls.ToolTip.text: text
                Controls.ToolTip.visible: hovered
                icon.name: checked ? "favorite" : "list-add"
                text: i18nc("@info:tooltip", "Favorite")
                checkable: true
                checked: model.channel.favorite
                onToggled: channelsModel.setFavorite(model.channel.id, checked)
            }

        }

    }

}
