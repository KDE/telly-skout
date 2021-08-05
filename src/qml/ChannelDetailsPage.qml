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

Kirigami.ScrollablePage {
    id: detailsPage

    property QtObject channel;

    title: i18nc("<Channel Name> - Details", "%1 - Details", channel.displayName || channel.name)

    ColumnLayout {
        Kirigami.Icon {
            source: Fetcher.image(channel.image)
            height: 200
            width: height
        }
        Kirigami.Heading {
            text: channel.displayName || channel.name
        }
        Kirigami.Heading {
            text: channel.description;
            level: 3
        }
        Controls.Label {
            text: i18nc("by <author(s)>", "by %1", channel.authors[0].name)
            visible: channel.authors.length !== 0
        }
        Controls.Label {
            text: "<a href='%1'>%1</a>".arg(channel.link)
            onLinkActivated: Qt.openUrlExternally(link)
        }
        Controls.Label {
            text: i18n("Subscribed since: %1", channel.subscribed.toLocaleString(Qt.locale(), Locale.ShortFormat))
        }
        Controls.Label {
            text: i18n("last updated: %1", channel.lastUpdated.toLocaleString(Qt.locale(), Locale.ShortFormat))
        }
        Controls.Label {
            text: i18n("%1 posts, %2 unread", channel.entryCount, channel.unreadEntryCount)
        }
    }
}
