// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Dialogs 1.0
import QtQuick.Layouts 1.14
import QtQuick.Window 2.15
import org.kde.kirigami 2.19 as Kirigami

Kirigami.ScrollablePage {
    title: i18n("Settings")

    Kirigami.FormLayout {
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("UI")
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Program height")

            Controls.SpinBox {
                from: 0
                to: Screen.height
                editable: true
                value: _settings.programHeight
                onValueModified: _settings.programHeight = value
            }

            Controls.Label {
                text: i18n("px/min")
            }

        }

        RowLayout {
            Kirigami.FormData.label: i18n("Column width")

            Controls.SpinBox {
                from: 0
                to: Screen.width
                editable: true
                value: _settings.columnWidth
                onValueModified: _settings.columnWidth = value
            }

            Controls.Label {
                text: i18n("px")
            }

        }

        RowLayout {
            Kirigami.FormData.label: i18n("Font size")

            Controls.SpinBox {
                from: 1
                to: 200
                editable: true
                value: _settings.fontSize
                onValueModified: _settings.fontSize = value
            }

            Controls.Label {
                text: i18n("px")
            }

        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Program")
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

                model: ["TV Spielfilm", "XMLTV"]
                currentIndex: _settings.fetcher
                onCurrentIndexChanged: _settings.fetcher = currentIndex
            }

        }

        RowLayout {
            visible: fetcher.currentIndex == 1 // only for XMLTV

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
