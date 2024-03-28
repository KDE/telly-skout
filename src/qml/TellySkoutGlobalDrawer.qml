// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as Controls
import org.kde.TellySkout
import org.kde.kirigami as Kirigami

Kirigami.GlobalDrawer {
    id: root

    property var channelTablePage

    isMenu: true
    actions: [
        Kirigami.Action {
            text: i18n("Favorites")
            icon.name: "view-calendar-day"
            onTriggered: {
                pageStack.layers.clear();
                pageStack.clear();
                pageStack.push(channelTablePage);
                Fetcher.fetchFavorites();
            }
        },
        Kirigami.Action {
            text: i18n("Select Favorites")
            icon.name: "favorite"
            onTriggered: {
                pageStack.layers.clear();
                pageStack.clear();
                pageStack.push("qrc:/GroupListPage.qml");
            }
        },
        Kirigami.Action {
            text: i18n("Sort Favorites")
            icon.name: "view-sort"
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
            icon.name: "settings-configure"
            onTriggered: pageStack.layers.push("qrc:/SettingsPage.qml")
            enabled: pageStack.layers.currentItem.title !== i18n("Settings")
        },
        Kirigami.Action {
            text: i18n("About")
            icon.name: "help-about-symbolic"
            onTriggered: pageStack.layers.push(aboutPage)
            enabled: pageStack.layers.currentItem.title !== i18n("About")
        }
    ]
}
