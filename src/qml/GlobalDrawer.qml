/**
 * SPDX-FileCopyrightText: 2021 Dimitris Kardarakos <dimkard@posteo.net>
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import org.kde.kirigami 2.12 as Kirigami
import org.kde.TellyScout 1.0 as TellyScout

Kirigami.GlobalDrawer {
    id: root

    property var channelsPage

    isMenu: true
    actions: [
        Kirigami.Action {
            text: i18n("Favorites")
            iconName: "rss"
            onTriggered: {
                pageStack.layers.clear()
                pageStack.clear()
                pageStack.push(root.channelsPage, {groupFilter: "Favorites"})
            }
        },
        Kirigami.Action {
            text: i18n("All Channels")
            iconName: "rss"
            onTriggered: {
                pageStack.layers.clear()
                pageStack.clear()
                pageStack.push(root.channelsPage, {groupFilter: ""})
            }
        },
        Kirigami.Action {
            id: channelGroups
            iconName: "edit-group"
            text: i18n("Channel Groups")
            children: [configureGroupsAction]
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

    Kirigami.Action {
        id: configureGroupsAction

        text: i18n("Configure Groups")
        iconName: "settings-configure"
        onTriggered: {
            pageStack.clear()
            pageStack.insertPage(0, root.channelsPage, {groupFilter: ""})
            pageStack.layers.clear()
            pageStack.layers.push(groupsList)
        }
    }

    Instantiator {
        model: groupsModel

        delegate: Kirigami.Action {
            text: model.name

            onTriggered: {
                pageStack.layers.clear()
                pageStack.clear()
                pageStack.push(root.channelsPage, {groupFilter: text})
            }
        }

        onObjectAdded: {
            channelGroups.children.push(object)
        }

        onObjectRemoved: {
            channelGroups.children = [];
            channelGroups.children.push(configureGroupsAction);
        }
    }

    Component {
        id: groupsList
        GroupsListPage {
            channelGroupsModel: groupsModel
        }
    }

    TellyScout.ChannelGroupsModel {
        id: groupsModel
    }
}
