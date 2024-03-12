// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.TellySkout
import org.kde.kirigami as Kirigami

Kirigami.SwipeListItem {
    padding: 0

    contentItem: Controls.ItemDelegate {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        text: model.group.name
        icon.name: model.group.refreshing ? "view-refresh" : ""
        onClicked: {
            lastGroup = model.group.id;
            Fetcher.fetchGroup(model.group.url, model.group.id);
            while (pageStack.depth > 1)pageStack.pop()
            pageStack.push("qrc:/ChannelListPage.qml", {
                "title": i18n("Channels") + " (" + i18n(model.group.name) + ")",
                "onlyFavorites": false,
                "groupFilter": lastGroup
            });
        }
    }

}
