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
    implicitHeight: 20
    color: "transparent"
    Rectangle {
            id: borderTop
            visible: program.title !== ""
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
    Controls.Label {
        id: label
        width: parent.implicitWidth
        leftPadding: 10
        topPadding: 10
        rightPadding: 10
        bottomPadding: 10
        height: parent.implicitHeight * (program.stop - program.start) / 60
        text: program.title !== "" ? "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "</b><br>" + program.title : ""
        wrapMode: Text.Wrap
    }
}
