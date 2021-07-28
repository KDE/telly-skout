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

Kirigami.OverlaySheet {
    id: addSheet
    property string groupName: ""
    parent: applicationWindow().overlay
    header: Kirigami.Heading {
        text: i18n("Add Feed")
    }

    /*countryItem: ColumnLayout {
        Controls.Label {
            text: i18n("Country:")
        }
        // TODO: show combobox with content of http://xmltv.se/countries.xml
        Controls.TextField {
            id: urlField
            Layout.fillWidth: true
            text: "Germany"
        }
    }*/

    contentItem: ColumnLayout {
        Controls.Label {
            text: i18n("Channel:")
        }
        // TODO: show combobox with content of http://xmltv.xmltv.se/channels-Germany.xml
        // (replace Germany with selected country from above)
        Controls.TextField {
            id: urlField
            Layout.fillWidth: true
            text: "http://xmltv.xmltv.se/3sat.de"
        }
    }

    footer: RowLayout {
        Item {
            Layout.fillWidth: true
        }

        Controls.ToolButton {
            text: i18n("OK")
            enabled: urlField.text

            onClicked: {
                Database.addFeed(urlField.text, addSheet.groupName)
                addSheet.close()
            }
        }

        Controls.ToolButton {
            text: i18n("Cancel")

            onClicked: addSheet.close()
        }
    }
}
