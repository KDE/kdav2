/*
    Copyright (c) 2010 Grégory Oestreicher <greg@kamago.net>

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

#ifndef KDAV_DAVCOLLECTIONDELETEJOB_H
#define KDAV_DAVCOLLECTIONDELETEJOB_H

#include "libkdav_export.h"

#include "utils.h"

#include <KCoreAddons/KJob>

namespace KDAV
{

/**
 * @short A job that deletes a DAV collection.
 *
 * This job is used to delete a DAV collection at a certain URL.
 */
class LIBKDAV_EXPORT DavCollectionDeleteJob : public KJob
{
    Q_OBJECT

public:
    /**
     * Creates a new DAV collection delete job.
     *
     * @param url The dav url of the collection to delete
     * @param parent The parent object.
     */
    explicit DavCollectionDeleteJob(const Utils::DavUrl &url, QObject *parent = Q_NULLPTR);

    /**
     * Starts the job.
     */
    void start() Q_DECL_OVERRIDE;

private Q_SLOTS:
    void davJobFinished(KJob *);

private:
    Utils::DavUrl mUrl;
};

}

#endif

