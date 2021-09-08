/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.14
import QtQuick.Controls 2.15 as Controls
import Qt.labs.platform 1.1
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.TellySkout 1.0

Kirigami.Page {
    id: root

    title: i18n("Favorites")

    property string lastChannel: ""


    Kirigami.PlaceholderMessage {
        visible: false //channelTable.model().columnCount === 0

        width: Kirigami.Units.gridUnit * 20
        icon.name: "favorite"
        anchors.centerIn: parent

        text: i18n("Please select favorites.")
    }

    Controls.HorizontalHeaderView {
        id: horizontalHeader
        syncView: channelTable
        anchors.left: channelTable.left
    }

    TableView {
        id: channelTable
        width: root.width
        height: root.height - horizontalHeader.height
        anchors.left: horizontalHeader.left
        anchors.top: horizontalHeader.bottom
        columnSpacing: 0
        rowSpacing: 0
        clip: true

        model: ChannelsTableModel {}

        delegate: ChannelTableDelegate {
            overlay: overlaySheet
        }
    }

    Kirigami.OverlaySheet {
        id: overlaySheet
        property alias text: overlaySheetText.text
        Text {
            id: overlaySheetText
            Layout.fillWidth: true
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
        }
    }
}
