// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Window
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.TellySkout

FormCard.FormCardPage {
    title: i18nc("@title", "Settings")

    FormCard.FormHeader {
        title: i18nc("@title:group", "Appearance")
    }

    FormCard.FormCard {
        FormCard.FormSpinBoxDelegate {
            label: i18nc("@option:spinbox height of the program", "Program height:")
            from: 0
            to: Screen.height
            value: TellySkoutSettings.programHeight
            onValueChanged: TellySkoutSettings.programHeight = value
            textFromValue: (value, locale) => {
                return i18nc("Number in px/min", "%1px/min", value);
            }
            valueFromText: (text, locale) => {
                return text.substring(0, text.length - 6);
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormSpinBoxDelegate {
            label: i18nc("@option:spinbox", "Column width:")
            from: 0
            to: Screen.width
            value: TellySkoutSettings.columnWidth
            onValueChanged: TellySkoutSettings.columnWidth = value
            textFromValue: (value, locale) => {
                return i18nc("Number in px", "%1px", value);
            }
            valueFromText: (text, locale) => {
                return text.substring(0, text.length - 2);
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormSpinBoxDelegate {
            label: i18nc("@option:spinbox", "Font size:")
            from: 1
            to: 200
            value: TellySkoutSettings.fontSize
            onValueChanged: TellySkoutSettings.fontSize = value
            textFromValue: (value, locale) => {
                return i18nc("Number in px", "%1px", value);
            }
            valueFromText: (text, locale) => {
                return text.substring(0, text.length - 2);
            }
        }

        FormCard.FormSwitchDelegate {
            text: i18nc("@option:check show date selection", "Show date selection:")
            checked: TellySkoutSettings.showDateSelection
            onCheckedChanged: TellySkoutSettings.showDateSelection = checked
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group Programs as TV programs", "Programs")
    }

    FormCard.FormCard {
        FormCard.FormSpinBoxDelegate {
            label: i18nc("@label:spinbox", "Delete old programs after:")
            value: TellySkoutSettings.deleteProgramAfter
            onValueChanged: TellySkoutSettings.deleteProgramAfter = value
            textFromValue: (value, locale) => {
                return i18ncp("%1 is a number, completes the setting 'Delete old programs after:'", "%1 day", "%1 days", value);
            }
            valueFromText: (text, locale) => {
                return text.substring(0, text.length - 4);
            }
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormComboBoxDelegate {
            id: fetcher

            text: i18n("Fetcher:")
            description: i18nc("@option:combobox", "The fetcher which loads the programs.")
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: TellySkoutSettings.fetcher
            onCurrentIndexChanged: TellySkoutSettings.fetcher = currentIndex
            editable: false
            model: ["TV Spielfilm", "XMLTV"]
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormSpinBoxDelegate {
            label: i18nc("@label:spinbox", "Prefetch:")
            visible: fetcher.currentIndex === 0 // only for TV Spielfilm
            value: TellySkoutSettings.tvSpielfilmPrefetch
            onValueChanged: TellySkoutSettings.tvSpielfilmPrefetch = value
            textFromValue: (value, locale) => {
                return i18ncp("%1 is a number, completes the setting 'Prefetch:'", "%1 day", "%1 days", value);
            }
            valueFromText: (text, locale) => {
                return text.substring(0, text.length - 4);
            }
        }

        FormCard.AbstractFormDelegate {
            id: fileDelegate

            background: null
            visible: fetcher.currentIndex === 1 // only for XMLTV
            text: i18nc("@label:textbox", "File:")

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Controls.Label {
                    text: fileDelegate.text
                }

                RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Controls.TextField {
                        id: xmltvFile

                        text: TellySkoutSettings.xmltvFile
                        onAccepted: TellySkoutSettings.xmltvFile = text
                        Layout.fillWidth: true
                    }

                    Controls.Button {
                        icon.name: 'file-search-symbolic'
                        onClicked: fileDialog.open()
                        display: Controls.Button.IconOnly
                        text: i18nc("@action:button", "Open file dialog")
                    }

                    FileDialog {
                        id: fileDialog

                        nameFilters: [i18n("XML files (*.xml)"), i18n("All files (*)")]
                        fileMode: FileDialog.OpenFile
                        onAccepted: {
                            // remove prefixed "file://"
                            const path = selectedFile.toString().replace(/^(file:\/{2})/, "");
                            xmltvFile.text = path;
                            TellySkoutSettings.xmltvFile = path;
                        }
                    }
                }
            }
        }
    }
}
