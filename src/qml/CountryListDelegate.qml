/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.TellyScout 1.0

Kirigami.SwipeListItem {

    leftPadding: 0
    rightPadding: 0

    contentItem: Kirigami.BasicListItem {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        text: model.country.name
        icon: model.country.refreshing ? "view-refresh" : "rss"

        onClicked: {
            lastCountry = model.country.id
            while(pageStack.depth > 1)
                pageStack.pop()
            pageStack.push("qrc:/ChannelListPage.qml", {groupFilter: "", countryFilter: lastCountry})
        }
    }
}
