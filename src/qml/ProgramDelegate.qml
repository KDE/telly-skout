// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.TellySkout
import org.kde.kirigami as Kirigami

Controls.ItemDelegate {
    id: root

    required property var program
    required property Kirigami.PromptDialog dialog
    required property int pxPerMin
    required property date startTime
    required property date stopTime
    required property real currentTimestamp

    function updateDialog(): void {
        if (root.program !== undefined) {
            if (!root.program.descriptionFetched)
                Fetcher.fetchProgramDescription(root.program.channelId, root.program.id, root.program.url);
            root.dialog.program = root.program;
        }
    }

    leftPadding: 0
    topPadding: 0
    rightPadding: 0
    bottomPadding: 0

    leftInset: 0
    topInset: 0
    rightInset: 0
    bottomInset: 0

    // start always at startTime, even if program starts earlier
    // stop always at stopTime, even if the program runs longer
    Layout.preferredHeight: (Math.min(root.program.stop, root.stopTime) - Math.max(root.program.start, root.startTime)) / 60000 * root.pxPerMin

    Component.onCompleted: {
        // update dialog if it is open (for this program)
        if (root.dialog.visible && root.dialog.programId === root.program.id)
            updateDialog();
    }

    Text {
        anchors.fill: parent
        text: "<b>" + root.program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "</b> " + root.program.title
        font.pixelSize: TellySkoutSettings.fontSize
        wrapMode: Text.Wrap
        elide: Text.ElideRight // avoid that text overlaps into next program
        // indicate if program is over
        color: root.program.stop >= root.currentTimestamp ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
        leftPadding: 3
        topPadding: 3
        rightPadding: 3
        bottomPadding: 3
        visible: height >= root.pxPerMin * 4 // do not show for short programs to avoid that text overlaps into next program
    }

    onClicked: {
        if (root.program !== undefined) {
            updateDialog();
            root.dialog.open();
        }
    }

    // hightlight running program
    background: Item {
        Rectangle {
            color: Kirigami.Theme.focusColor
            visible: root.program.stop >= root.currentTimestamp
            height: (root.currentTimestamp - root.program.start) / 60000 * root.pxPerMin
            border.width: 0

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
        }

        Kirigami.Separator {
            anchors.bottom: parent.bottom
            width: parent.width
            weight: Kirigami.Separator.Weight.Light
        }
    }
}
