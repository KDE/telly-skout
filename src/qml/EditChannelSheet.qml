/*
* SPDX-FileCopyrightText: 2021 Dimitris Kardarakos <dimkard@posteo.net>
*
* SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14
import org.kde.kirigami 2.12 as Kirigami
import org.kde.TellyScout 1.0 as TellyScout

Kirigami.OverlaySheet {
    id: root

    property var channel

    header: Kirigami.Heading {
        text: i18n("Edit Channel")
    }

    onChannelChanged: groupCombo.currentIndex = (root.channel !== undefined) ? groupCombo.indexOfValue(root.channel.groupName) : groupCombo.indexOfValue("")

    contentItem: Kirigami.FormLayout {
        Controls.TextField {
            id: displayName

            text: (root.channel !== undefined) ? (channel.displayName || channel.name) : ""
            Layout.fillWidth: true
            Kirigami.FormData.label: i18n("Display Name:")
        }
    }

    footer: RowLayout {
        Item {
            Layout.fillWidth: true
        }

        Controls.ToolButton {
            text: i18n("OK")

            onClicked: {
                TellyScout.Database.editChannel(channel.url, displayName.text, false);
                root.close();
            }
        }

        Controls.ToolButton {
            text: i18n("Cancel")

            onClicked: root.close()
        }
    }
}
