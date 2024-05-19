// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.TellySkout
import org.kde.kirigami as Kirigami

Rectangle {
    id: root

    property int channelIdx
    property var dialog
    property int pxPerMin
    property date startTime
    property date stopTime
    property real currentTimestamp

    function updateDialog() {
        if (program !== undefined) {
            if (!program.descriptionFetched)
                Fetcher.fetchProgramDescription(program.channelId, program.id, program.url);

            root.dialog.program = program;
        }
    }

    // start always at startTime, even if program starts earlier
    // stop always at stopTime, even if the program runs longer
    height: (Math.min(program.stop, stopTime) - Math.max(program.start, startTime)) / 60000 * pxPerMin
    color: channelIdx % 2 == 0 ? "transparent" : Kirigami.Theme.alternateBackgroundColor
    border.color: Kirigami.Theme.textColor
    Component.onCompleted: {
        // update dialog if it is open (for this program)
        if (root.dialog.visible && root.dialog.programId === program.id)
            updateDialog();

    }

    // hightlight running program
    Rectangle {
        color: Kirigami.Theme.focusColor
        visible: (program.start <= currentTimestamp) && (program.stop >= currentTimestamp)
        height: (currentTimestamp - program.start) / 60000 * root.pxPerMin
        border.width: 0

        anchors {
            left: root.left
            right: root.right
            top: root.top
            margins: root.border.width
        }

    }

    Text {
        anchors.fill: parent
        text: "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "</b> " + program.title
        font.pixelSize: _settings.fontSize
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

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (program !== undefined) {
                updateDialog();
                root.dialog.open();
            }
        }
    }

}
