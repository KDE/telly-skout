// SPDX-FileCopyrightText: none
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Dialogs 1.0
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

                model: ["TV Spielfilm", "xmltv.se", "XMLTV"]
                currentIndex: _settings.fetcher
                onCurrentIndexChanged: _settings.fetcher = currentIndex
            }

        }

        RowLayout {
            visible: fetcher.currentIndex == 2 // only for XMLTV

            Controls.TextField {
                id: xmltvFile

                Kirigami.FormData.label: i18n("File")
                text: _settings.xmltvFile
                onAccepted: _settings.xmltvFile = text
            }

            Controls.Button {
                icon.name: 'file-search-symbolic'
                onClicked: fileDialog.open()
            }

            FileDialog {
                id: fileDialog

                nameFilters: [i18n("XML files (*.xml)"), i18n("All files (*)")]
                selectExisting: true
                selectMultiple: false
                onAccepted: {
                    // remove prefixed "file://"
                    const path = fileUrl.toString().replace(/^(file:\/{2})/, "");
                    xmltvFile.text = path;
                    _settings.xmltvFile = path;
                }
            }

        }

    }

}
