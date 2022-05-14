// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14
import org.kde.kirigami 2.19 as Kirigami

Kirigami.ScrollablePage {
    title: i18n("Settings")

    Kirigami.FormLayout {
        Kirigami.Heading {
            Kirigami.FormData.isSection: true
            text: i18n("Program")
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Delete old after")

            Controls.SpinBox {
                id: deleteProgramAfter

                value: _settings.deleteProgramAfter
                onValueModified: _settings.deleteProgramAfter = value
            }

            Controls.Label {
                text: i18n("days")
            }

        }

        RowLayout {
            Kirigami.FormData.label: i18n("Fetcher")

            Controls.ComboBox {
                id: fetcher

                model: ["TV Spielfilm", "xmltv.se"]
                currentIndex: _settings.fetcher
                onCurrentIndexChanged: _settings.fetcher = currentIndex
            }

        }

    }

}
