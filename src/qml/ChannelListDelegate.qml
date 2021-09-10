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

    leftPadding: 0
    rightPadding: 0

    contentItem: Kirigami.BasicListItem {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        text: model.channel.displayName || model.channel.name
        icon: model.channel.refreshing ? "view-refresh" : model.channel.image === "" ? "rss" : Fetcher.image(model.channel.image)
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
