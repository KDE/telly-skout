import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.TellySkout 1.0

Rectangle
{
    id: root

    property var overlay
    property int pxPerMin
    property date startTime
    property date stopTime

    width: 200
    // start always at startTime, even if program starts earlier
    // stop always at stopTime, even if the program runs longer
    height: (Math.min(program.stop, stopTime) - Math.max(program.start, startTime)) / 60000 * pxPerMin
    color: "transparent"
    border.color: Kirigami.Theme.textColor

    // hightlight running program
    Rectangle
    {
        z: -1
        width: parent.width
        color: Kirigami.Theme.focusColor

        Component.onCompleted: {
            const now = new Date().getTime()
            visible = (program.start <= now) && (program.stop >= now)
            height = (now - program.start) / 60000 * root.pxPerMin
        }
    }

    Text
    {
        anchors.fill: parent
        text: "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "</b> " + program.title
        wrapMode: Text.Wrap
        elide: Text.ElideRight // avoid that text overlaps into next program
        color: Kirigami.Theme.textColor
        leftPadding: 3
        topPadding: 3
        rightPadding: 3
        bottomPadding: 3
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (program !== undefined) {
                root.overlay.text = program !== undefined ? "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "-" + program.stop.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + " " + program.title + "</b><br><br>" + program.description : ""
                root.overlay.open()
            }
        }
    }
}
