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

#include "davitemdeletejob.h"

#include "davitemfetchjob.h"
#include "davmanager.h"
#include "daverror.h"

#include <KIO/DeleteJob>
#include <KIO/Job>

using namespace KDAV;

DavItemDeleteJob::DavItemDeleteJob(const DavItem &item, QObject *parent)
    : DavJobBase(parent), mItem(item), mFreshResponseCode(-1)
{
}

void DavItemDeleteJob::start()
{
    KIO::DeleteJob *job = KIO::del(mItem.url().url(), KIO::HideProgressInfo | KIO::DefaultFlags);
    job->addMetaData(QStringLiteral("PropagateHttpHeader"), QStringLiteral("true"));
    job->addMetaData(QStringLiteral("customHTTPHeader"), QStringLiteral("If-Match: ") + mItem.etag());
    job->addMetaData(QStringLiteral("cookies"), QStringLiteral("none"));
    job->addMetaData(QStringLiteral("no-auth-prompt"), QStringLiteral("true"));

    connect(job, &KIO::DeleteJob::result, this, &DavItemDeleteJob::davJobFinished);
}

DavItem DavItemDeleteJob::freshItem() const
{
    return mFreshItem;
}

int DavItemDeleteJob::freshResponseCode() const
{
    return mFreshResponseCode;
}

void DavItemDeleteJob::davJobFinished(KJob *job)
{
    KIO::DeleteJob *deleteJob = qobject_cast<KIO::DeleteJob *>(job);

    if (deleteJob->error() && deleteJob->error() != KIO::ERR_NO_CONTENT) {
        const int responseCode = deleteJob->queryMetaData(QStringLiteral("responsecode")).isEmpty() ?
                                 0 :
                                 deleteJob->queryMetaData(QStringLiteral("responsecode")).toInt();

        if (responseCode != 404 && responseCode != 410) {
            setLatestResponseCode(responseCode);
            setError(ERR_ITEMDELETE);
            setJobErrorText(deleteJob->errorText());
            setJobError(deleteJob->error());
            setErrorTextFromDavError();
        }

        if (hasConflict()) {
            DavItemFetchJob *fetchJob = new DavItemFetchJob(mItem);
            connect(fetchJob, &DavItemFetchJob::result, this, &DavItemDeleteJob::conflictingItemFetched);
            fetchJob->start();
            return;
        }
    }

    emitResult();
}

void DavItemDeleteJob::conflictingItemFetched(KJob *job)
{
    DavItemFetchJob *fetchJob = qobject_cast<DavItemFetchJob *>(job);
    mFreshResponseCode = fetchJob->latestResponseCode();

    if (!job->error()) {
        mFreshItem = fetchJob->item();
    }

    emitResult();
}

