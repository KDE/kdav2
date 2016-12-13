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

#ifndef DAVPRINCIPALHOMESETSFETCHJOB_H
#define DAVPRINCIPALHOMESETSFETCHJOB_H

#include "libkdav_export.h"

#include "davjobbase.h"
#include "davutils.h"

#include <kjob.h>

#include <QtCore/QStringList>

/**
 * @short A job that fetches home sets for a principal.
 */
class LIBKDAV_EXPORT DavPrincipalHomeSetsFetchJob : public DavJobBase
{
    Q_OBJECT

public:
    /**
     * Creates a new dav principals home sets fetch job.
     *
     * @param url The DAV url of the DAV principal.
     * @param parent The parent object.
     */
    explicit DavPrincipalHomeSetsFetchJob(const DavUtils::DavUrl &url, QObject *parent = Q_NULLPTR);

    /**
     * Starts the job.
     */
    void start() Q_DECL_OVERRIDE;

    /**
     * Returns the found home sets.
     */
    QStringList homeSets() const;

private Q_SLOTS:
    void davJobFinished(KJob *);

private:
    /**
     * Start the fetch process.
     *
     * There may be two rounds necessary if the first request
     * does not returns the home sets, but only the current-user-principal
     * or the principal-URL. The bool flag is here to prevent requesting
     * those last two on each request, as they are only fetched in
     * the first round.
     *
     * @param fetchHomeSetsOnly If set to true the request will not include
     *        the current-user-principal and principal-URL props.
     */
    void fetchHomeSets(bool fetchHomeSetsOnly);

    DavUtils::DavUrl mUrl;
    QStringList mHomeSets;
};

#endif
