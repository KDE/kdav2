/*
    Copyright (c) 2014 Gregory Oestreicher <greg@kamago.net>

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

#include "davjobbase.h"

#include "davjob.h"

using namespace KDAV2;

struct DavJobBasePrivate {
    int mLatestHttpStatusCode{0};
    int mLatestResponseCode{0};
    int mJobErrorCode{0};
    QString mInternalErrorText;
};

DavJobBase::DavJobBase(QObject *parent)
    : KJob(parent)
    , d(std::unique_ptr<DavJobBasePrivate>(new DavJobBasePrivate()))
{
}

DavJobBase::~DavJobBase()
{
}

unsigned int DavJobBase::latestHttpStatusCode() const
{
    return d->mLatestHttpStatusCode;
}

unsigned int DavJobBase::latestResponseCode() const
{
    return d->mLatestResponseCode;
}

bool DavJobBase::canRetryLater() const
{
    switch (latestHttpStatusCode()) {
        case 0:
            // Likely a timeout or a connection failure.
            if (error()) {
                return true;
            }
            break;
        case 401: // Authentication required
        case 402: // Payment required
        case 407: // Proxy authentication required
        case 408: // Request timeout
        case 423: // Locked
        case 429: // Too many requests
        case 501:
        case 502:
        case 503:
        case 504:
        case 507: // Insufficient storage
        case 511: // Network authentication required
            return true;
        default:
            break;
    }
    return false;
}

bool DavJobBase::hasConflict() const
{
    return latestHttpStatusCode() == 412;
}

void DavJobBase::setLatestHttpStatusCode(unsigned int code)
{
    d->mLatestHttpStatusCode = code;
}

Error DavJobBase::davError() const
{
    return Error((KDAV2::ErrorNumber)error(), d->mLatestHttpStatusCode, d->mLatestResponseCode, d->mInternalErrorText, d->mJobErrorCode);
}

void DavJobBase::setJobErrorText(const QString &errorText)
{
    d->mInternalErrorText = errorText;
}

void DavJobBase::setJobError(unsigned int jobErrorCode)
{
    d->mJobErrorCode = jobErrorCode;
}

void DavJobBase::setErrorTextFromDavError()
{
    setErrorText(davError().errorText());
}

void DavJobBase::setDavError(const Error &error)
{
    setLatestHttpStatusCode(error.httpStatusCode());
    d->mLatestResponseCode = error.responseCode();
    setError(error.errorNumber());
    setJobErrorText(error.internalErrorText());
    setJobError(error.jobErrorCode());
    setErrorText(error.errorText());
}

void DavJobBase::setErrorFromJob(DavJob *job, ErrorNumber jobErrorCode)
{
    setDavError(Error{jobErrorCode, job->httpStatusCode(), job->responseCode(), job->errorText(), job->error()});
}
