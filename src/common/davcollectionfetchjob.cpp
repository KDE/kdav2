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

#include "davcollectionfetchjob.h"

#include <QString>

#include "daverror.h"
#include "davjob.h"
#include "davmanager.h"
#include "davprotocolbase.h"
#include "utils.h"

using namespace KDAV2;

DavCollectionFetchJob::DavCollectionFetchJob(const DavCollection &collection, QObject *parent)
    : DavJobBase(parent), mCollection(collection)
{
}

void DavCollectionFetchJob::start()
{
    const DavProtocolBase *protocol = DavManager::self()->davProtocol(mCollection.url().protocol());
    Q_ASSERT(protocol);
    XMLQueryBuilder::Ptr builder(protocol->collectionsQuery());

    auto job = DavManager::self()->createPropFindJob(
        mCollection.url().url(), builder->buildQuery(), /* depth = */ QStringLiteral("0"));
    connect(job, &DavJob::result, this, &DavCollectionFetchJob::davJobFinished);
}

DavCollection DavCollectionFetchJob::collection() const
{
    return mCollection;
}

void DavCollectionFetchJob::davJobFinished(KJob *job)
{
    auto *storedJob        = qobject_cast<DavJob *>(job);
    const int responseCode = storedJob->responseCode();

    if (storedJob->error()) {
        setLatestResponseCode(responseCode);
        setError(ERR_PROBLEM_WITH_REQUEST);
        setJobErrorText(storedJob->errorText());
        setJobError(storedJob->error());
        setErrorTextFromDavError();
    } else {
        /*
         * Extract data from a document like the following:
         *
         * <multistatus xmlns="DAV:">
         *   <response xmlns="DAV:">
         *     <href xmlns="DAV:">/path/to/collection/</href>
         *     <propstat xmlns="DAV:">
         *       <prop xmlns="DAV:">
         *         <displayname>collection name</displayname>
         *         <resourcetype>
         *           <collection/>
         *           <card:addressbook/>
         *         </resourcetype>
         *         <cs:getctag>ctag</cs:getctag>
         *       </prop>
         *       <status xmlns="DAV:">HTTP/1.1 200 OK</status>
         *     </propstat>
         *   </response>
         * </multistatus>
         */

        const QDomDocument document       = storedJob->response();

        const QDomElement documentElement = document.documentElement();
        QDomElement responseElement       = Utils::firstChildElementNS(
            documentElement, QStringLiteral("DAV:"), QStringLiteral("response"));

        // Validate that we got a valid PROPFIND response
        if (documentElement.localName().compare(QStringLiteral("multistatus"), Qt::CaseInsensitive) != 0) {
            setError(ERR_COLLECTIONFETCH);
            setErrorTextFromDavError();
            emitResult();
            return;
        }

        if (!Utils::extractCollection(responseElement, mCollection.url(), mCollection)) {
            setError(ERR_COLLECTIONFETCH);
            setErrorTextFromDavError();
            emitResult();
            return;
        }
    }

    emitResult();
}
