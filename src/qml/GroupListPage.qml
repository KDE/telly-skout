// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14
import org.kde.TellySkout 1.0
import org.kde.kirigami 2.19 as Kirigami

Kirigami.ScrollablePage {
    id: root

    property string lastGroup: ""

    title: i18n("Select Favorites")
    Component.onCompleted: {
        Fetcher.fetchGroups();
    }

    Kirigami.PlaceholderMessage {
        visible: groupList.count === 0
        width: Kirigami.Units.gridUnit * 20
        anchors.centerIn: parent
        text: i18n("Loading groups...")

        Controls.BusyIndicator {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }

    }

    ListView {
        id: groupList

        anchors.fill: parent

        model: GroupsModel {
        }

        delegate: GroupListDelegate {
        }

    }

}
