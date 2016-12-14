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

#ifndef KDAV_DAVUTILS_AKONADI_H
#define KDAV_DAVUTILS_AKONADI_H

#include "utils.h"

#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtXml/QDomElement>

#include <AkonadiCore/Item>

class DavItem;
namespace Akonadi
{
class Collection;
class Item;
}

/**
 * @short A namespace that contains helper methods for DAV functionality.
 */
namespace Utils
{
/**
 * Returns the i18n'ed name of the given DAV @p protocol dialect.
 */
QString translatedProtocolName(Protocol protocol);

/**
 * Returns the protocol matching the given i18n'ed @p name. This is the opposite
 * of Utils::translatedProtocolName().
 */
Protocol protocolByTranslatedName(const QString &name);

/**
 * Creates a new DavItem from the Akonadi::Item @p item.
 *
 * The returned item will have no payload (DavItem::data() will return an empty
 * QByteArray) if the @p item payload is not recognized.
 */
DavItem createDavItem(const Akonadi::Item &item, const Akonadi::Collection &collection, const Akonadi::Item::List &dependentItems = Akonadi::Item::List());

/**
 * Parses the DAV data contained in @p source and puts it in @p target and @extraItems.
 */
bool parseDavData(const DavItem &source, Akonadi::Item &target, Akonadi::Item::List &extraItems);
}

#endif
