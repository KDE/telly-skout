// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14
import org.kde.TellySkout 1.0
import org.kde.kirigami 2.19 as Kirigami

Kirigami.SwipeListItem {
    leftPadding: 0
    rightPadding: 0

    contentItem: Kirigami.BasicListItem {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        text: model.group.name
        icon: model.group.refreshing ? "view-refresh" : ""
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
