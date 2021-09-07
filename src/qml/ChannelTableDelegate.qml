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

Rectangle {
    width: 200
    implicitWidth: 200
    implicitHeight: 7
    color: "transparent"
    Rectangle {
            id: borderTop
            visible: isFirst
            width: parent.width
            height: 1
            anchors.top: parent.top
            color: Kirigami.Theme.textColor
    }
    Rectangle {
            id: borderRight
            width: 1
            height: parent.height
            anchors.right: parent.right
            color: Kirigami.Theme.textColor
    }
    Text {
        id: text
        width: parent.implicitWidth
        leftPadding: 3
        topPadding: 3
        rightPadding: 3
        bottomPadding: 3
        height: parent.implicitHeight * (program.stop - program.start) / 60
        text: isFirst ? "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "</b> " + program.title : ""
        wrapMode: Text.Wrap
        MouseArea {
            anchors.fill: parent
            onClicked: {
                overlay.open()
            }
        }
    }

    Kirigami.OverlaySheet {
        id: overlay
        Controls.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "-" + program.stop.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "</b><br><br>" + program.description
        }
    }
}
