import QtQuick 2.14
import QtQuick.Controls 2.14 as Controls
import QtQuick.Layouts 1.14
import org.kde.TellySkout 1.0
import org.kde.kirigami 2.12 as Kirigami

Kirigami.SwipeListItem {
    leftPadding: 0
    rightPadding: 0

    contentItem: Kirigami.BasicListItem {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        text: model.country.name
        icon: model.country.refreshing ? "view-refresh" : ""
        onClicked: {
            lastCountry = model.country.id;
            Fetcher.fetchCountry(model.country.url, model.country.id);
            while (pageStack.depth > 1)pageStack.pop()
            pageStack.push("qrc:/ChannelListPage.qml", {
                "title": i18n("Channels") + " (" + i18n(model.country.name) + ")",
                "groupFilter": "",
                "countryFilter": lastCountry
            });
        }
    }

}
