import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import org.kde.TellySkout 1.0 as TellySkout
import org.kde.kirigami 2.12 as Kirigami

Kirigami.ApplicationWindow {
    id: root

    title: "Telly Skout"
    pageStack.initialPage: channelTable

    Component {
        id: aboutPage

        Kirigami.AboutPage {
            aboutData: _aboutData
        }

    }

    ChannelTablePage {
        id: channelTable

        windowHeight: root.height
    }

    globalDrawer: TellySkoutGlobalDrawer {
        channelsTablePage: channelTable
    }

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

}
