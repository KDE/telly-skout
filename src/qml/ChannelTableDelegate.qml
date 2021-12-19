import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.TellySkout 1.0

Rectangle
{
    id: root

    property int channelIdx
    property var overlay
    property int pxPerMin
    property date startTime
    property date stopTime

    width: 200
    // start always at startTime, even if program starts earlier
    // stop always at stopTime, even if the program runs longer
    height: (Math.min(program.stop, stopTime) - Math.max(program.start, startTime)) / 60000 * pxPerMin
    color: channelIdx % 2 == 0? "transparent" : Kirigami.Theme.alternateBackgroundColor
    border.color: "transparent"

    // hightlight running program
    // TODO: update as time elapses
    Rectangle
    {
        width: parent.width
        color: Kirigami.Theme.focusColor

        Component.onCompleted: {
            const now = new Date().getTime()
            visible = (program.start <= now) && (program.stop >= now)
            height = (now - program.start) / 60000 * root.pxPerMin

            // update overlay if it is open (for this program)
            if (root.overlay.sheetOpen && root.overlay.programId === program.id)
            {
                updateOverlay()
            }
        }
    }

    // border
    Rectangle
    {
        anchors.fill: parent
        color: "transparent"
        border.color: Kirigami.Theme.textColor
    }

    Text
    {
        anchors.fill: parent
        text: "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "</b> " + program.title
        wrapMode: Text.Wrap
        elide: Text.ElideRight // avoid that text overlaps into next program
        // indicate if program is over
        color: program.stop >= new Date().getTime()? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
        leftPadding: 3
        topPadding: 3
        rightPadding: 3
        bottomPadding: 3
        visible: height >= root.pxPerMin * 4 // do not show for short programs to avoid that text overlaps into next program
    }

    MouseArea
    {
        anchors.fill: parent
        onClicked:
        {
            if (program !== undefined) {
                updateOverlay()
                root.overlay.open()
            }
        }
    }

    function updateOverlay()
    {
        if (program !== undefined) {
            if (program.description === "__NOT_LOADED__")
            {
                Fetcher.fetchProgramDescription(program.channelId, program.id, program.url)
            }

            var categoryText = ""
            if (program.category !== "")
            {
                categoryText = "<br><i>" + program.category + "</i>"
            }

            var descriptionText = ""
            if (program.description !== "" && program.description !== "__NOT_LOADED__")
            {
                descriptionText = "<br><br>" + program.description
            }

            root.overlay.text = program !== undefined ? "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "-" + program.stop.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + " " + program.title + "</b>" +  categoryText + descriptionText : ""
            root.overlay.programId = program.id
        }
    }

}
