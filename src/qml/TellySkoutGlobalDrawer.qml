// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import org.kde.TellySkout 1.0 as TellySkout
import org.kde.kirigami 2.19 as Kirigami

Kirigami.GlobalDrawer {
    id: root

    isMenu: true
    actions: [
        Kirigami.Action {
            text: i18n("Favorites")
            iconName: "view-calendar-day"
            onTriggered: {
                pageStack.layers.clear();
                pageStack.clear();
                pageStack.push("qrc:/ChannelTablePage.qml", {
                    "windowHeight": root.parent.height
                });
            }
        },
        Kirigami.Action {
            text: i18n("Select Favorites")
            iconName: "favorite"
            onTriggered: {
                pageStack.layers.clear();
                pageStack.clear();
                pageStack.push("qrc:/GroupListPage.qml");
            }
        },
        Kirigami.Action {
            text: i18n("Sort Favorites")
            iconName: "view-sort"
            onTriggered: {
                pageStack.layers.clear();
                pageStack.clear();
                pageStack.push("qrc:/ChannelListPage.qml", {
                    "sortable": true,
                    "onlyFavorites": true,
                    "groupFilter": ""
                });
            }
        },
        Kirigami.Action {
            text: i18n("Settings")
            iconName: "settings-configure"
            onTriggered: pageStack.layers.push("qrc:/SettingsPage.qml")
            enabled: pageStack.layers.currentItem.title !== i18n("Settings")
        },
        Kirigami.Action {
            text: i18n("About")
            iconName: "help-about-symbolic"
            onTriggered: pageStack.layers.push(aboutPage)
            enabled: pageStack.layers.currentItem.title !== i18n("About")
        }
    ]
}
