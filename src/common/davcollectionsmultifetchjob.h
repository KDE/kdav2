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

#ifndef KDAV2_DAVCOLLECTIONSMULTIFETCHJOB_H
#define KDAV2_DAVCOLLECTIONSMULTIFETCHJOB_H

#include "kpimkdav2_export.h"

#include "davcollection.h"
#include "davurl.h"

#include <KCoreAddons/KJob>

namespace KDAV2
{

/**
 * @short A job that fetches all DAV collection.
 *
 * This job is used to fetch all DAV collection that are available
 * under a certain list of DAV urls.
 *
 * @note This class just combines multiple calls of DavCollectionsFetchJob
 *       into one job.
 */
class KPIMKDAV2_EXPORT DavCollectionsMultiFetchJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Creates a new dav collections multi fetch job.
     *
     * @param urls The list of DAV urls whose sub collections shall be fetched.
     * @param parent The parent object.
     */
    explicit DavCollectionsMultiFetchJob(const DavUrl::List &urls, QObject *parent = nullptr);

    /**
     * Starts the job.
     */
    void start() Q_DECL_OVERRIDE;

    /**
     * Returns the list of fetched DAV collections.
     */
    DavCollection::List collections() const;

Q_SIGNALS:
    /**
     * This signal is emitted every time a new collection has been discovered.
     *
     * @param collectionUrl The URL of the discovered collection
     * @param configuredUrl The URL given to the job
     */
    void collectionDiscovered(int protocol, const QString &collectionUrl, const QString &configuredUrl);

private Q_SLOTS:
    void davJobFinished(KJob *);

private:
    DavUrl::List mUrls;
    DavCollection::List mCollections;
    uint mSubJobCount;
};

}

#endif
