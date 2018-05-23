/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "utils.h"

#include "enums.h"

#include "davitem.h"
#include "davmanager.h"
#include "davprotocolbase.h"
#include "davurl.h"

#include <QColor>
#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QString>

#include "libkdav2_debug.h"

using namespace KDAV2;

QDomElement Utils::firstChildElementNS(const QDomElement &parent, const QString &namespaceUri, const QString &tagName)
{
    for (QDomNode child = parent.firstChild(); !child.isNull(); child = child.nextSibling()) {
        if (child.isElement()) {
            const QDomElement elt = child.toElement();
            if (tagName.isEmpty() || (elt.tagName() == tagName && elt.namespaceURI() == namespaceUri)) {
                return elt;
            }
        }
    }

    return QDomElement();
}

QDomElement Utils::nextSiblingElementNS(const QDomElement &element, const QString &namespaceUri, const QString &tagName)
{
    for (QDomNode sib = element.nextSibling(); !sib.isNull(); sib = sib.nextSibling()) {
        if (sib.isElement()) {
            const QDomElement elt = sib.toElement();
            if (tagName.isEmpty() || (elt.tagName() == tagName && elt.namespaceURI() == namespaceUri)) {
                return elt;
            }
        }
    }

    return QDomElement();
}

Privileges Utils::extractPrivileges(const QDomElement &element)
{
    Privileges final = None;
    QDomElement privElement = firstChildElementNS(element, QStringLiteral("DAV:"), QStringLiteral("privilege"));

    while (!privElement.isNull()) {
        QDomElement child = privElement.firstChildElement();

        while (!child.isNull()) {
            final |= parsePrivilege(child);
            child = child.nextSiblingElement();
        }

        privElement = Utils::nextSiblingElementNS(privElement, QStringLiteral("DAV:"), QStringLiteral("privilege"));
    }

    return final;
}

Privileges Utils::parsePrivilege(const QDomElement &element)
{
    Privileges final = None;

    if (!element.childNodes().isEmpty()) {
        // This is an aggregate privilege, parse each of its children
        QDomElement child = element.firstChildElement();
        while (!child.isNull()) {
            final |= parsePrivilege(child);
            child = child.nextSiblingElement();
        }
    } else {
        // This is a normal privilege
        const QString privname = element.localName();

        if (privname == QLatin1String("read")) {
            final |= KDAV2::Read;
        } else if (privname == QLatin1String("write")) {
            final |= KDAV2::Write;
        } else if (privname == QLatin1String("write-properties")) {
            final |= KDAV2::WriteProperties;
        } else if (privname == QLatin1String("write-content")) {
            final |= KDAV2::WriteContent;
        } else if (privname == QLatin1String("unlock")) {
            final |= KDAV2::Unlock;
        } else if (privname == QLatin1String("read-acl")) {
            final |= KDAV2::ReadAcl;
        } else if (privname == QLatin1String("read-current-user-privilege-set")) {
            final |= KDAV2::ReadCurrentUserPrivilegeSet;
        } else if (privname == QLatin1String("write-acl")) {
            final |= KDAV2::WriteAcl;
        } else if (privname == QLatin1String("bind")) {
            final |= KDAV2::Bind;
        } else if (privname == QLatin1String("unbind")) {
            final |= KDAV2::Unbind;
        } else if (privname == QLatin1String("all")) {
            final |= KDAV2::All;
        }
    }

    return final;
}

QLatin1String Utils::protocolName(Protocol protocol)
{
    QLatin1String protocolName("");

    switch (protocol) {
    case KDAV2::CalDav:
        protocolName = QLatin1String("CalDav");
        break;
    case KDAV2::CardDav:
        protocolName = QLatin1String("CardDav");
        break;
    case KDAV2::GroupDav:
        protocolName = QLatin1String("GroupDav");
        break;
    }

    return protocolName;
}

Protocol Utils::protocolByName(const QString &name)
{
    Protocol protocol = KDAV2::CalDav;

    if (name == QLatin1String("CalDav")) {
        protocol = KDAV2::CalDav;
    } else if (name == QLatin1String("CardDav")) {
        protocol = KDAV2::CardDav;
    } else if (name == QLatin1String("GroupDav")) {
        protocol = KDAV2::GroupDav;
    } else {
        qCCritical(KDAV2_LOG) << "Unexpected protocol name : " << name;
    }

    return protocol;
}

QString Utils::createUniqueId()
{
    qint64 time = QDateTime::currentMSecsSinceEpoch() / 1000;
    int r = qrand() % 1000;
    QString id = QLatin1String("R") + QString::number(r);
    QString uid = QString::number(time) + QLatin1String(".") + id;
    return uid;
}

QString Utils::contactsMimeType(Protocol protocol)
{
    QString ret;

    if (protocol == KDAV2::CardDav) {
        ret = QStringLiteral("text/vcard");
    } else if (protocol == KDAV2::GroupDav) {
        ret = QStringLiteral("text/x-vcard");
    }

    return ret;
}

bool Utils::extractCollection(const QDomElement &response, DavUrl davUrl, DavCollection &collection)
{
    QDomElement propstatElement;

    // check for the valid propstat, without giving up on first error
    {
        const QDomNodeList propstats =
            response.elementsByTagNameNS(QStringLiteral("DAV:"), QStringLiteral("propstat"));
        for (int i = 0; i < propstats.length(); ++i) {
            const QDomElement propstatCandidate = propstats.item(i).toElement();
            const QDomElement statusElement     = Utils::firstChildElementNS(
                propstatCandidate, QStringLiteral("DAV:"), QStringLiteral("status"));
            if (statusElement.text().contains(QStringLiteral("200"))) {
                propstatElement = propstatCandidate;
            }
        }
    }

    if (propstatElement.isNull()) {
        return false;
    }

    // extract url
    const QDomElement hrefElement =
        Utils::firstChildElementNS(response, QStringLiteral("DAV:"), QStringLiteral("href"));

    if (hrefElement.isNull()) {
        return false;
    }


    QString href = hrefElement.text();
    if (!href.endsWith(QLatin1Char('/'))) {
        href.append(QLatin1Char('/'));
    }

    QUrl url = davUrl.url();
    url.setUserInfo(QString());
    if (href.startsWith(QLatin1Char('/'))) {
        // href is only a path, use request url to complete
        url.setPath(href, QUrl::TolerantMode);
    } else {
        // href is a complete url
        url = QUrl::fromUserInput(href);
    }

    // extract display name
    const QDomElement propElement =
        Utils::firstChildElementNS(propstatElement, QStringLiteral("DAV:"), QStringLiteral("prop"));
    const QDomElement displaynameElement =
        Utils::firstChildElementNS(propElement, QStringLiteral("DAV:"), QStringLiteral("displayname"));
    const QString displayName = displaynameElement.text();

    // Extract CTag
    const QDomElement CTagElement = Utils::firstChildElementNS(
        propElement, QStringLiteral("http://calendarserver.org/ns/"), QStringLiteral("getctag"));
    QString CTag;
    if (!CTagElement.isNull()) {
        CTag = CTagElement.text();
    }

    // extract calendar color if provided
    const QDomElement colorElement = Utils::firstChildElementNS(
        propElement, QStringLiteral("http://apple.com/ns/ical/"), QStringLiteral("calendar-color"));
    QColor color;
    if (!colorElement.isNull()) {
        QString colorValue = colorElement.text();
        if(colorValue[0] == '#' && colorValue.size() == 9) {
            // Put the alpha part at the beginning for Qt:
            // Qt wants #AARRGGBB instead of #RRGGBBAA
            colorValue = QStringLiteral("#") + colorValue.right(2) + colorValue.mid(1, 6);
        }

        if (QColor::isValidColor(colorValue)) {
            color.setNamedColor(colorValue);
        }
    }

    // extract allowed content types
    const DavCollection::ContentTypes contentTypes =
        DavManager::self()->davProtocol(davUrl.protocol())->collectionContentTypes(propstatElement);

    auto _url = url;
    _url.setUserInfo(davUrl.url().userInfo());
    collection = DavCollection(DavUrl(_url, davUrl.protocol()), displayName, contentTypes);

    collection.setCTag(CTag);
    if (color.isValid()) {
        collection.setColor(color);
    }

    // extract privileges
    const QDomElement currentPrivsElement = Utils::firstChildElementNS(
        propElement, QStringLiteral("DAV:"), QStringLiteral("current-user-privilege-set"));
    if (currentPrivsElement.isNull()) {
        // Assume that we have all privileges
        collection.setPrivileges(KDAV2::All);
    } else {
        Privileges privileges = Utils::extractPrivileges(currentPrivsElement);
        collection.setPrivileges(privileges);
    }

    qCDebug(KDAV2_LOG) << url.toDisplayString() << "PRIVS: " << collection.privileges();

    return true;
}
