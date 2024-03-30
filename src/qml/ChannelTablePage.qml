// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import Qt.labs.platform
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.TellySkout
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: root

    readonly property int columnWidth: _settings.columnWidth
    property int windowHeight: 0
    property real currentTimestamp: new Date().getTime()

    title: i18n("Favorites")
    padding: 0

    Timer {
        interval: 60000
        repeat: true
        running: true
        onTriggered: currentTimestamp = new Date().getTime()
    }

    Kirigami.PlaceholderMessage {
        visible: contentRepeater.count === 0
        width: Kirigami.Units.gridUnit * 20
        icon.name: "favorite"
        anchors.centerIn: parent
        text: i18n("Please select favorites")
    }

    Row {
        id: header

        x: -channelTable.contentX
        visible: contentRepeater.count !== 0
        z: 100 // TODO: remove workaround for mobile (channelTable "anchors.top: header.bottom" not respected)

        Repeater {
            id: headerRepeater

            model: channelsModel

            delegate: Column {
                width: columnWidth

                Rectangle {
                    color: Kirigami.Theme.backgroundColor
                    width: parent.width
                    height: 30
                    border.color: Kirigami.Theme.textColor

                    Text {
                        text: modelData.name
                        color: Kirigami.Theme.textColor
                        anchors.centerIn: parent
                    }

                }

            }

        }

    }

    Flickable {
        id: channelTable

        readonly property int pxPerMin: _settings.programHeight
        readonly property var date: new Date()
        readonly property var start: new Date(date.getFullYear(), date.getMonth(), date.getDate()) // today 00:00h
        readonly property var stop: new Date(date.getFullYear(), date.getMonth(), date.getDate(), 23, 59, 0) // today 23:59h

        width: parent.width
        height: parent.height - header.height
        anchors.top: header.bottom
        visible: contentRepeater.count !== 0
        contentHeight: 24 * 60 * pxPerMin
        contentWidth: content.width
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        Component.onCompleted: {
            // scroll to current time
            var today = new Date();
            today.setHours(0);
            today.setMinutes(0);
            today.setSeconds(0);
            const now = new Date();
            // offset [s] to 00:00h
            const offsetS = (now.getTime() - today.getTime()) / 1000;
            // center in window (vertically)
            contentY = (offsetS / (24 * 60 * 60)) * contentHeight - (windowHeight / 2);
        }

        Row {
            id: content

            Repeater {
                id: contentRepeater

                model: channelsModel

                delegate: Column {
                    id: column

                    property int idx: index

                    width: root.columnWidth

                    // show info if program is not available
                    Rectangle {
                        width: parent.width
                        height: channelTable.contentHeight
                        visible: programRepeater.count === 0
                        color: Kirigami.Theme.negativeBackgroundColor
                        border.color: Kirigami.Theme.textColor

                        Text {
                            anchors.centerIn: parent
                            text: i18n("not available")
                            wrapMode: Text.Wrap
                            color: Kirigami.Theme.textColor
                        }

                    }

                    Repeater {
                        id: programRepeater

                        model: ProgramsProxyModel {
                            id: proxyProgramModel

                            start: channelTable.start
                            stop: channelTable.stop
                            sourceModel: modelData.programsModel
                        }

                        delegate: ChannelTableDelegate {
                            channelIdx: column.idx
                            overlay: overlaySheet
                            pxPerMin: channelTable.pxPerMin
                            width: root.columnWidth
                            startTime: channelTable.start
                            stopTime: channelTable.stop
                            currentTimestamp: root.currentTimestamp
                        }

                    }

                }

            }

        }

        Controls.ScrollBar.vertical: Controls.ScrollBar {
        }

        Controls.ScrollBar.horizontal: Controls.ScrollBar {
        }
    }

    ChannelsModel {
        id: channelsModel

        onlyFavorites: true
    }

    Kirigami.OverlaySheet {
        id: overlaySheet

        property var programId // persistent ID even if program is deleted
        property var program

        onProgramChanged: {
            if (program)
                programId = program.id;

        }

        Text {
            id: overlaySheetText

            property alias program: overlaySheet.program
            property string categoryText: (program && program.categories.length) > 0 ? "<br><i>" + program.categories.join(' ') + "</i>" : ""
            property string descriptionText: (program && program.descriptionFetched && program.description) ? "<br><br>" + program.description : ""

            text: program ? "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "-" + program.stop.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + " " + program.title + "</b>" + categoryText + descriptionText : ""
            Layout.fillWidth: true
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
        }

    }

}
