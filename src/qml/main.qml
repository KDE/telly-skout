// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as Controls
import org.kde.TellySkout
import org.kde.config as KConfig
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ApplicationWindow {
    id: root

    title: "Telly Skout"
    pageStack.initialPage: ChannelTablePage {
        windowHeight: root.height
    }

    globalDrawer: TellySkoutGlobalDrawer {
        windowHeight: root.height
    }

    contextDrawer: Kirigami.ContextDrawer {}

    KConfig.WindowStateSaver {
        configGroupName: "Main"
    }
}
