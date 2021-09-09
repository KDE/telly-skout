/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls

import org.kde.kirigami 2.12 as Kirigami
import org.kde.TellySkout 1.0 as TellySkout

Kirigami.ApplicationWindow {
    id: root

    title: "Telly Skout"

    pageStack.initialPage: channelTable

    globalDrawer: TellySkoutGlobalDrawer {
        channelsTablePage: channelTable
    }

    Component {
        id: aboutPage
        Kirigami.AboutPage {
            aboutData: _aboutData
        }
    }

    contextDrawer: Kirigami.ContextDrawer {
        id: contextDrawer
    }

    ChannelTablePage  {
        id: channelTable
    }

}
