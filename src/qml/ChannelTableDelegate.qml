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
    id: root

    property var overlay

    z : -row // ensure that later rows do not hide content
    width: 200
    implicitWidth: 200
    implicitHeight: 7
    color: metaData.isRunning ? Kirigami.Theme.focusColor : "transparent"
    Rectangle {
            id: borderTop
            visible: metaData.isFirst !== undefined ? metaData.isFirst : false
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
        height: program !== undefined ? parent.implicitHeight * (program.stop - program.start) / 60 : parent.implicitHeight
        text: metaData.isFirst ? "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "</b> " + program.title : ""
        wrapMode: Text.Wrap
        color: Kirigami.Theme.textColor
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (program !== undefined) {
                    root.overlay.text = program !== undefined ? "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "-" + program.stop.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "</b><br><br>" + program.description : ""
                    root.overlay.open()
                }
            }
        }
    }
}
