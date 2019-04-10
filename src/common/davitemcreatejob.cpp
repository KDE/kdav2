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

#include "davitemcreatejob.h"

#include "davitemfetchjob.h"
#include "davmanager.h"
#include "daverror.h"
#include "davjob.h"

#include "libkdav2_debug.h"

using namespace KDAV2;

DavItemCreateJob::DavItemCreateJob(const DavItem &item, QObject *parent)
    : DavJobBase(parent), mItem(item)
{
}

void DavItemCreateJob::start()
{
    auto job = DavManager::self()->createCreateJob(mItem.data(), itemUrl(), mItem.contentType().toLatin1());
    connect(job, &DavJob::result, this, &DavItemCreateJob::davJobFinished);
}

DavItem DavItemCreateJob::item() const
{
    return mItem;
}

QUrl DavItemCreateJob::itemUrl() const
{
    return mItem.url().url();
}

void DavItemCreateJob::davJobFinished(KJob *job)
{
    auto storedJob = static_cast<DavJob*>(job);

    if (storedJob->error()) {
        setLatestResponseCode(storedJob->responseCode());
        setError(ERR_ITEMCREATE);
        setJobErrorText(storedJob->errorText());
        setJobError(storedJob->error());
        setErrorTextFromDavError();
        emitResult();
        return;
    }

    mItem.setUrl(DavUrl(storedJob->url(), mItem.url().protocol()));

    DavItemFetchJob *fetchJob = new DavItemFetchJob(mItem);
    connect(fetchJob, &DavItemFetchJob::result, this, &DavItemCreateJob::itemRefreshed);
    fetchJob->start();
}

void DavItemCreateJob::itemRefreshed(KJob *job)
{
    if (!job->error()) {
        DavItemFetchJob *fetchJob = qobject_cast<DavItemFetchJob *>(job);
        mItem.setEtag(fetchJob->item().etag());
    }
    emitResult();
}

