// SPDX-FileCopyrightText: 2022 Plata Hill <plata.hill@kdemail.net>
// SPDX-License-Identifier: LGPL-2.1-or-later

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.TellySkout
import org.kde.kirigami as Kirigami

Controls.ItemDelegate {
    width: parent.width

    contentItem: RowLayout {
        Kirigami.Icon {
            source: model.group.refreshing ? "view-refresh" : "tv"
        }

        Controls.Label {
            Layout.fillWidth: true
            text: model.group.name

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    const lastGroup = model.group.id;
                    Fetcher.fetchGroup(model.group.url, model.group.id);
                    while (pageStack.depth > 1)
                        pageStack.pop();
                    pageStack.push("qrc:/qml/SelectFavoritesPage.qml", {
                        "title": i18nc("@title, as in 'TV channels'", "Channels") + " (" + i18n(model.group.name) + ")",
                        "groupFilter": lastGroup
                    });
                }
            }
        }
    }
}
