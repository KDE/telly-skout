import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14
import org.kde.TellySkout 1.0
import org.kde.kirigami 2.12 as Kirigami

Kirigami.ScrollablePage {
    id: root

    property string lastCountry: ""

    title: i18n("Select Favorites")
    Component.onCompleted: {
        Fetcher.fetchCountries();
    }

    Kirigami.PlaceholderMessage {
        visible: countryList.count === 0
        width: Kirigami.Units.gridUnit * 20
        anchors.centerIn: parent
        text: i18n("Loading countries...")

        Controls.BusyIndicator {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }

    }

    ListView {
        id: countryList

        anchors.fill: parent

        model: CountriesModel {
        }

        delegate: CountryListDelegate {
        }

    }

}
