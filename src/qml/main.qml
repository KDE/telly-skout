// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as Controls
import org.kde.TellySkout as TellySkout
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ApplicationWindow {
    id: root

    title: "Telly Skout"
    pageStack.initialPage: channelTable

    Component {
        id: aboutPage

        FormCard.AboutPage {
            aboutData: _aboutData
        }

    }

    ChannelTablePage {
        id: channelTable

        windowHeight: root.height
    }

    globalDrawer: TellySkoutGlobalDrawer {
    }

    contextDrawer: Kirigami.ContextDrawer {
    }

}
