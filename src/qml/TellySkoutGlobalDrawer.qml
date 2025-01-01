// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as Controls
import org.kde.TellySkout
import org.kde.kirigami as Kirigami

Kirigami.GlobalDrawer {
    id: root

    required property int windowHeight

    isMenu: true
    actions: [
        Kirigami.Action {
            text: i18n("Favorites")
            icon.name: "view-calendar-day"
            onTriggered: {
                pageStack.layers.clear();
                if (pageStack.currentItem.title !== i18n("Favorites")) {
                    pageStack.clear();
                    pageStack.push("qrc:/qml/FavoritesPage.qml", {
                        "windowHeight": root.windowHeight
                    });
                    Fetcher.fetchFavorites();
                }
            }
            enabled: pageStack.layers.depth > 1 || pageStack.currentItem.title !== i18n("Favorites")
        },
        Kirigami.Action {
            text: i18n("Select Favorites")
            icon.name: "favorite"
            onTriggered: {
                pageStack.layers.clear();
                if (pageStack.currentItem.title !== i18n("Select Favorites")) {
                    pageStack.clear();
                    pageStack.push("qrc:/qml/GroupListPage.qml");
                }
            }
            enabled: pageStack.layers.depth > 1 || pageStack.currentItem.title !== i18n("Select Favorites")
        },
        Kirigami.Action {
            text: i18n("Sort Favorites")
            icon.name: "view-sort"
            onTriggered: {
                pageStack.layers.clear();
                if (pageStack.currentItem.title !== i18n("Sort Favorites")) {
                    pageStack.clear();
                    pageStack.push("qrc:/qml/SortFavoritesPage.qml");
                }
            }
            enabled: pageStack.layers.depth > 1 || pageStack.currentItem.title !== i18n("Sort Favorites")
        },
        Kirigami.Action {
            text: i18n("Settings")
            icon.name: "settings-configure"
            onTriggered: pageStack.layers.push("qrc:/qml/SettingsPage.qml")
            enabled: pageStack.layers.currentItem.title !== i18n("Settings")
        },
        Kirigami.Action {
            text: i18n("About")
            icon.name: "help-about-symbolic"
            onTriggered: pageStack.layers.push(Qt.createComponent('org.kde.kirigamiaddons.formcard', 'AboutPage'))
            enabled: pageStack.layers.currentItem.title !== i18n("About Telly Skout")
        }
    ]
}
