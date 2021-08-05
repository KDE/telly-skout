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

Kirigami.ScrollablePage {
    id: page

    property QtObject program
    property string channelTitle

    title: channelTitle

    ColumnLayout {
        Kirigami.Heading {
            text: program.title
        }

        Controls.Label {
            text: page.program.content
            baseUrl: page.program.baseUrl
            textFormat: Text.RichText
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            onLinkActivated: Qt.openUrlExternally(link)
            onWidthChanged: text = program.adjustedContent(width, font.pixelSize)
            font.pointSize: _settings && !(_settings.articleFontUseSystem) ? _settings.articleFontSize : Kirigami.Units.fontMetrics.font.pointSize
        }
    }

    actions.main: Kirigami.Action {
        text: i18n("Open in Browser")
        icon.name: "globe"
        onTriggered: Qt.openUrlExternally(program.link)
    }
}
