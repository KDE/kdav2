/*
    Copyright (c) 2009 Grégory Oestreicher <greg@kamago.net>

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

#include "davprotocolbase.h"

#include <QVariant>

using namespace KDAV2;

XMLQueryBuilder::~XMLQueryBuilder()
{
}

void XMLQueryBuilder::setParameter(const QString &key, const QVariant &value)
{
    mParameters[key] = value;
}

QVariant XMLQueryBuilder::parameter(const QString &key) const
{
    QVariant ret;
    if (mParameters.contains(key)) {
        ret = mParameters.value(key);
    }
    return ret;
}

DavProtocolBase::~DavProtocolBase()
{
}

QString DavProtocolBase::principalHomeSet() const
{
    return QString();
}

QString DavProtocolBase::principalHomeSetNS() const
{
    return QString();
}
