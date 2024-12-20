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
    readonly property bool isLoading: Fetcher.favoritesPercentage !== 100
    property int windowHeight: 0
    property real currentTimestamp: new Date().getTime()

    title: i18nc("@title", "Favorites")
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

    actions: [
        Kirigami.Action {
            text: i18n("Refetch")
            icon.name: "view-refresh"
            visible: contentRepeater.count !== 0
            onTriggered: Fetcher.fetchFavorites(true)
        }
    ]

    header: Controls.ToolBar {
        visible: contentRepeater.count !== 0 && !isLoading

        padding: 0

        contentItem: RowLayout {
            x: -channelTable.contentX
            spacing: 0

            Repeater {
                model: channelsModel

                delegate: RowLayout {
                    id: channelHeadDelegate

                    required property var modelData

                    Layout.maximumWidth: root.columnWidth
                    Layout.minimumWidth: root.columnWidth

                    Controls.Label {
                        text: modelData.name
                        padding: Kirigami.Units.mediumSpacing

                        Layout.fillWidth: true
                    }

                    Kirigami.Separator {
                        Layout.fillHeight: true
                        Layout.topMargin: Kirigami.Units.mediumSpacing
                        Layout.bottomMargin: Kirigami.Units.mediumSpacing
                    }
                }
            }
        }
    }

    Kirigami.Theme.colorSet: Kirigami.Theme.View
    Kirigami.Theme.inherit: true

    Flickable {
        id: channelTable

        readonly property int pxPerMin: _settings.programHeight
        readonly property var date: new Date()
        readonly property var start: new Date(date.getFullYear(), date.getMonth(), date.getDate()) // today 00:00h
        readonly property var stop: new Date(date.getFullYear(), date.getMonth(), date.getDate(), 23, 59, 0) // today 23:59h

        anchors.fill: parent

        visible: contentRepeater.count !== 0 && !isLoading
        contentHeight: 24 * 60 * pxPerMin
        contentWidth: content.implicitWidth
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

        RowLayout {
            id: content

            spacing: 0

            Repeater {
                id: contentRepeater

                model: channelsModel

                delegate: RowLayout {
                    id: channelDelegate

                    required property int index
                    required property var modelData

                    spacing: 0

                    Layout.maximumWidth: root.columnWidth
                    Layout.minimumWidth: root.columnWidth
                    Layout.fillHeight: true

                    Item {
                        Layout.fillWidth: true

                        implicitHeight: column.implicitHeight

                        // show info if program is not available
                        Kirigami.PlaceholderMessage {
                            text: i18nc("placeholder message", "Information not available")
                            visible: programRepeater.count === 0
                            anchors.centerIn: parent
                            width: parent.width - Kirigami.Units.gridUnits * 4
                        }

                        ColumnLayout {
                            id: column

                            anchors.fill: parent

                            spacing: 0

                            Repeater {
                                id: programRepeater

                                model: ProgramsProxyModel {
                                    id: proxyProgramModel

                                    start: channelTable.start
                                    stop: channelTable.stop
                                    sourceModel: channelDelegate.modelData.programsModel
                                }

                                delegate: ChannelTableDelegate {
                                    index: channelDelegate.index
                                    dialog: detailsDialog
                                    pxPerMin: channelTable.pxPerMin
                                    startTime: channelTable.start
                                    stopTime: channelTable.stop
                                    currentTimestamp: root.currentTimestamp

                                    Layout.fillWidth: true
                                }
                            }
                        }
                    }

                    Kirigami.Separator {
                        Layout.fillHeight: true
                    }
                }
            }
        }

        Controls.ScrollBar.vertical: Controls.ScrollBar {}

        Controls.ScrollBar.horizontal: Controls.ScrollBar {}
    }

    ChannelsModel {
        id: channelsModel

        onlyFavorites: true
    }

    Kirigami.PromptDialog {
        id: detailsDialog

        property var programId // persistent ID even if program is deleted
        property var program
        property string categoryText: (program && program.categories.length) > 0 ? "<br><i>" + program.categories.join(' ') + "</i>" : ""
        property string descriptionText: (program && program.descriptionFetched && program.description) ? "<br><br>" + program.description : ""

        onProgramChanged: {
            if (program)
                programId = program.id;
        }
        title: program ? program.title : ""
        subtitle: program ? "<b>" + program.start.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "-" + program.stop.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) + "</b>" + categoryText + descriptionText : ""
        standardButtons: Controls.Dialog.Close
    }

    Kirigami.LoadingPlaceholder {
        id: loadingPlaceholder

        visible: contentRepeater.count !== 0 && isLoading
        anchors.centerIn: parent
        determinate: true
        progressBar.value: Fetcher.favoritesPercentage
    }
}
