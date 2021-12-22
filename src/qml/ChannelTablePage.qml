import Qt.labs.platform 1.1
import QtQuick 2.14
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.14
import org.kde.TellySkout 1.0
import org.kde.kirigami 2.19 as Kirigami

Kirigami.Page {
    id: root

    property int windowHeight: 0
    property real currentTimestamp: 0

    function updateTime() {
        var now = new Date();
        currentTimestamp = now.getTime();
    }

    title: i18n("Favorites")
    padding: 0
    Component.onCompleted: {
        Fetcher.fetchFavorites();
        updateTime();
    }

    Timer {
        interval: 60000
        repeat: true
        running: true
        onTriggered: updateTime()
    }

    Kirigami.PlaceholderMessage {
        visible: contentRepeater.count === 0
        width: Kirigami.Units.gridUnit * 20
        icon.name: "rss"
        anchors.centerIn: parent
        text: i18n("Please select favorites")
    }

    Row {
        id: header

        x: -channelTable.Controls.ScrollBar.horizontal.position * channelTable.contentWidth
        visible: contentRepeater.count !== 0
        z: 100 // TODO: remove workaround for mobile (channelTable "anchors.top: header.bottom" not respected)

        Repeater {
            id: headerRepeater

            model: proxyModel

            delegate: Column {
                width: 200

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

    Controls.ScrollView {
        id: channelTable

        readonly property int pxPerMin: 5
        readonly property var date: new Date()

        visible: contentRepeater.count !== 0
        width: parent.width
        height: parent.height - header.height
        anchors.top: header.bottom
        contentHeight: 24 * 60 * pxPerMin
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
            Controls.ScrollBar.vertical.position = offsetS / (24 * 60 * 60) - (windowHeight / 2) / channelTable.contentHeight;
        }

        Row {
            id: content

            Repeater {
                id: contentRepeater

                model: proxyModel

                delegate: Column {
                    id: column

                    property int idx: index

                    width: 200

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

                            start: new Date(channelTable.date.getFullYear(), channelTable.date.getMonth(), channelTable.date.getDate()) // today 00:00h
                            stop: new Date(channelTable.date.getFullYear(), channelTable.date.getMonth(), channelTable.date.getDate(), 23, 59, 0) // today 23:59h
                            sourceModel: modelData.programsModel
                        }

                        delegate: ChannelTableDelegate {
                            channelIdx: column.idx
                            overlay: overlaySheet
                            pxPerMin: channelTable.pxPerMin
                            startTime: proxyProgramModel.start
                            stopTime: proxyProgramModel.stop
                            currentTimestamp: root.currentTimestamp
                        }

                    }

                }

            }

        }

    }

    ChannelsProxyModel {
        id: proxyModel

        onlyFavorites: true
        country: ""
        sourceModel: channelsModel
    }

    ChannelsModel {
        id: channelsModel

        onlyFavorites: true
    }

    Kirigami.OverlaySheet {
        id: overlaySheet

        property string programId: ""
        property alias text: overlaySheetText.text

        Text {
            id: overlaySheetText

            Layout.fillWidth: true
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
        }

    }

}
