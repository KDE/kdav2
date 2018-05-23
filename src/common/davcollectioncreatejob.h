/*
    Copyright (c) 2010 Grégory Oestreicher <greg@kamago.net>
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

#ifndef KDAV2_DAVCOLLECTIONCREATEJOB_H
#define KDAV2_DAVCOLLECTIONCREATEJOB_H

#include "kpimkdav2_export.h"

#include "davjobbase.h"
#include "davcollection.h"
#include "davurl.h"

namespace KDAV2
{

/**
 * @short A job that creates a DAV collection.
 */
class KPIMKDAV2_EXPORT DavCollectionCreateJob : public DavJobBase
{
    Q_OBJECT

public:
    /**
     * Creates a new dav collection create job.
     *
     * @param collection The collection that shall be created.
     * @param parent The parent object.
     */
    DavCollectionCreateJob(const DavCollection &collection, QObject *parent = nullptr);

    /**
     * Starts the job.
     */
    void start() Q_DECL_OVERRIDE;

    /**
     * Returns the created DAV item including the correct identifier url
     * and current etag information.
     */
    DavCollection collection() const;

    QUrl collectionUrl() const;

private Q_SLOTS:
    void collectionCreated(KJob *);
    void collectionModified(KJob *);
    void collectionRefreshed(KJob *);

private:
    DavCollection mCollection;
    int mRedirectCount;

    void createCalendar();
    void createAddressbook();
};

}

#endif
