/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2018 Rémi Nicole <minijackson@riseup.net>

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

#ifndef KDAV2_DAVCOLLECTIONFETCHJOB_H
#define KDAV2_DAVCOLLECTIONFETCHJOB_H

#include "kpimkdav2_export.h"

#include "davcollection.h"
#include "davjobbase.h"
#include "davurl.h"

namespace KDAV2
{

/**
 * @short A job that fetches a DAV collection from the DAV server.
 */
class KPIMKDAV2_EXPORT DavCollectionFetchJob : public DavJobBase
{
    Q_OBJECT

public:
    /**
     * Creates a new dav collection fetch job.
     *
     * @param item The collection that shall be fetched.
     * @param parent The parent object.
     */
    DavCollectionFetchJob(const DavCollection &collection, QObject *parent = nullptr);

    /**
     * Starts the job.
     */
    void start() Q_DECL_OVERRIDE;

    /**
     * Returns the fetched collection including current etag information.
     */
    DavCollection collection() const;

private Q_SLOTS:
    void davJobFinished(KJob *);

private:
    DavCollection mCollection;
};

}

#endif
