import QtQuick 2.14
import QtQuick.Controls 2.15 as Controls
import Qt.labs.platform 1.1
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.TellySkout 1.0

Kirigami.Page {
    id: root

    title: i18n("Favorites")

    property int windowHeight: 0

    padding: 0

    Row
    {
        id: header
        x: -channelTable.Controls.ScrollBar.horizontal.position * channelTable.contentWidth
        Repeater
        {
            model: proxyModel
            delegate: Column
            {
                width: 200
                Rectangle {
                    color: Kirigami.Theme.backgroundColor
                    width: parent.width
                    height: 30
                    border.color: Kirigami.Theme.textColor

                    Text
                    {
                        text: modelData.name
                        color: Kirigami.Theme.textColor
                    }
                }
            }
        }
    }

    Controls.ScrollView {
        id: channelTable

        readonly property int pxPerMin: 5
        readonly property var date: new Date()

        width: parent.width
        height: parent.height - header.height
        anchors.top: header.bottom
        contentHeight: 24 * 60 * pxPerMin
        Row
        {
            id: content
            Repeater
            {
                model: proxyModel
                delegate: Column
                {
                    width: 200
                    Repeater
                    {
                        model: ProgramsProxyModel {
                            id: proxyProgramModel
                            start: new Date(channelTable.date.getFullYear(), channelTable.date.getMonth(), channelTable.date.getDate()) // today 00:00h
                            stop: new Date(channelTable.date.getFullYear(), channelTable.date.getMonth(), channelTable.date.getDate(), 23, 59, 0) // today 23:59h
                            sourceModel: modelData.programsModel
                        }
                        delegate: ChannelTableDelegate {
                            overlay: overlaySheet
                            pxPerMin: channelTable.pxPerMin
                            startTime: proxyProgramModel.start
                            stopTime: proxyProgramModel.stop
                        }
                    }
                }
            }
        }

        Component.onCompleted: {
            // scroll to current time
            var today = new Date()
            today.setHours(0)
            today.setMinutes(0)
            today.setSeconds(0)
            const now = new Date()
            // offset [s] to 00:00h
            const offsetS = (now.getTime() - today.getTime()) / 1000
            // center in window (vertically)
            Controls.ScrollBar.vertical.position = offsetS / (24 * 60 * 60) - (windowHeight / 2) / channelTable.contentHeight
        }
    }

    ChannelsProxyModel {
        id: proxyModel
        groupName: "favorite"
        country: ""
        sourceModel: channelsModel
    }

    ChannelsModel {
        id: channelsModel
    }

    Kirigami.OverlaySheet {
        id: overlaySheet
        property alias text: overlaySheetText.text
        Text {
            id: overlaySheetText
            Layout.fillWidth: true
            color: Kirigami.Theme.textColor
            wrapMode: Text.WordWrap
        }
    }

    Component.onCompleted: {
        Fetcher.fetchFavorites()
    }
}
