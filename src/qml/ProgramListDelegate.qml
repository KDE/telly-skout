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

    property string channelTitle

    leftPadding: 0
    rightPadding: 0

    contentItem: Kirigami.BasicListItem {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        text: model.program.title
        subtitle: model.program.updated.toLocaleString(Qt.locale(), Locale.ShortFormat) + (model.program.authors.length === 0 ? "" : " " + i18nc("by <author(s)>", "by") + " " + model.program.authors[0].name)
        reserveSpaceForIcon: false
        bold: !model.program.read

        onClicked: {
            while(pageStack.depth > 2)
                pageStack.pop()
            model.program.read = true
            pageStack.push("qrc:/ProgramPage.qml", {"program": model.program, "channelTitle" : channelTitle})
        }
    }
}
