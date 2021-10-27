/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.TellySkout 1.0

Kirigami.ScrollablePage {
    id: root

    title: "Select Favorites"

    property string lastCountry: ""

    Kirigami.PlaceholderMessage {
        visible: countryList.count === 0

        width: Kirigami.Units.gridUnit * 20
        anchors.centerIn: parent

        Controls.BusyIndicator {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }
        text: i18n("Loading countries...")
    }

    ListView {
        id: countryList

        anchors.fill: parent

        model: CountriesModel { }

        delegate: CountryListDelegate {
        }
    }

    Component.onCompleted: {
        Fetcher.fetchCountries()
    }
}
