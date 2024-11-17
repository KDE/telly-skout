// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.TellySkout
import org.kde.kirigami as Kirigami

Controls.ItemDelegate {
    id: root

    required property int index
    required property var program

    required property Kirigami.PromptDialog dialog
    required property int pxPerMin
    required property date startTime
    required property date stopTime
    required property real currentTimestamp

    function updateDialog(): void {
        if (program !== undefined) {
            if (!program.descriptionFetched)
                Fetcher.fetchProgramDescription(program.channelId, program.id, program.url);
            root.dialog.program = program;
        }
    }

    // start always at startTime, even if program starts earlier
    // stop always at stopTime, even if the program runs longer
    Layout.preferredHeight: (Math.min(program.stop, stopTime) - Math.max(program.start, startTime)) / 60000 * pxPerMin

    Component.onCompleted: {
        // update dialog if it is open (for this program)
        if (root.dialog.visible && root.dialog.programId === program.id)
            updateDialog();
    }

    Text {
        anchors.fill: parent
        text: "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "</b> " + program.title
        font.pixelSize: TellySkoutSettings.fontSize
        wrapMode: Text.Wrap
        elide: Text.ElideRight // avoid that text overlaps into next program
        // indicate if program is over
        color: program.stop >= new Date().getTime() ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
        leftPadding: 3
        topPadding: 3
        rightPadding: 3
        bottomPadding: 3
        visible: height >= root.pxPerMin * 4 // do not show for short programs to avoid that text overlaps into next program
    }

    onClicked: {
        if (program !== undefined) {
            updateDialog();
            root.dialog.open();
        }
    }

    background: Rectangle {
        color: root.index % 2 == 0 ? "transparent" : Kirigami.Theme.alternateBackgroundColor

        // hightlight running program
        Rectangle {
            color: Kirigami.Theme.focusColor
            visible: (program.start <= currentTimestamp) && (program.stop >= currentTimestamp)
            height: (currentTimestamp - program.start) / 60000 * root.pxPerMin
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
