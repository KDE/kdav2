/*
    Copyright (c) 2018 Christian Mollekopf <mollekopf@kolabsys.com>

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
#pragma once

#include "kpimkdav2_export.h"

#include "davjobbase.h"
#include "davurl.h"

#include <KCoreAddons/KJob>

#include <QtCore/QStringList>

namespace KDAV2
{

/**
 * @short A job that discovers the principal url using well-known uri's
 *
 * It's an incomplete implementation of RFC 6764 so far.
 */
class KPIMKDAV2_EXPORT DavDiscoveryJob : public DavJobBase
{
    Q_OBJECT

public:
    /**
     * Discover the url of the server.
     *
     * @param url The DAV url of the server.
     * @param parent The parent object.
     */
    explicit DavDiscoveryJob(const DavUrl &url, const QString &wellKnownSuffix, QObject *parent = nullptr);

    /**
     * Starts the job.
     */
    void start() Q_DECL_OVERRIDE;

    /**
     * Returns the found principal url.
     */
    QUrl url() const;

private Q_SLOTS:
    void davJobFinished(KJob *);

private:
    DavUrl mUrl;
};

}
