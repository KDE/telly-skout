// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14
import org.kde.TellySkout 1.0
import org.kde.kirigami 2.19 as Kirigami

Rectangle {
    id: root

    property int channelIdx
    property var overlay
    property int pxPerMin
    property date startTime
    property date stopTime
    property real currentTimestamp

    function updateOverlay() {
        if (program !== undefined) {
            if (!program.descriptionFetched)
                Fetcher.fetchProgramDescription(program.channelId, program.id, program.url);

            var categoryText = "";
            if (program.categories.length)
                categoryText = "<br><i>" + program.categories.join(' ') + "</i>";

            var descriptionText = "";
            if (program.descriptionFetched && program.description)
                descriptionText = "<br><br>" + program.description;

            root.overlay.text = program !== undefined ? "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "-" + program.stop.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + " " + program.title + "</b>" + categoryText + descriptionText : "";
            root.overlay.programId = program.id;
        }
    }

    width: 200
    // start always at startTime, even if program starts earlier
    // stop always at stopTime, even if the program runs longer
    height: (Math.min(program.stop, stopTime) - Math.max(program.start, startTime)) / 60000 * pxPerMin
    color: channelIdx % 2 == 0 ? "transparent" : Kirigami.Theme.alternateBackgroundColor
    border.color: "transparent"

    // hightlight running program
    Rectangle {
        width: parent.width
        color: Kirigami.Theme.focusColor
        visible: (program.start <= currentTimestamp) && (program.stop >= currentTimestamp)
        height: (currentTimestamp - program.start) / 60000 * root.pxPerMin
        Component.onCompleted: {
            // update overlay if it is open (for this program)
            if (root.overlay.sheetOpen && root.overlay.programId === program.id)
                updateOverlay();

        }
    }

    // border
    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: Kirigami.Theme.textColor
    }

    Text {
        anchors.fill: parent
        text: "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "</b> " + program.title
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
                updateOverlay();
                root.overlay.open();
            }
        }
    }

}
