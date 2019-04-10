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

#include "daverror.h"

using namespace KDAV2;

class DavJobBasePrivate {
public:
    DavJobBasePrivate();

    int mLatestResponseCode;
    int mJobErrorCode;
    QString mInternalErrorText;
};

DavJobBasePrivate::DavJobBasePrivate()
    : mLatestResponseCode(0)
    , mJobErrorCode(0)
{
}

DavJobBase::DavJobBase(QObject *parent)
    : KJob(parent)
    , d(std::unique_ptr<DavJobBasePrivate>(new DavJobBasePrivate()))
{
}

DavJobBase::~DavJobBase()
{
}

unsigned int DavJobBase::latestResponseCode() const
{
    return d->mLatestResponseCode;
}

bool DavJobBase::canRetryLater() const
{
    switch (latestResponseCode()) {
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
    return latestResponseCode() == 412;
}

void DavJobBase::setLatestResponseCode(unsigned int code)
{
    d->mLatestResponseCode = code;
}

Error DavJobBase::davError() const
{
    return Error((KDAV2::ErrorNumber)error(), d->mLatestResponseCode, d->mInternalErrorText, d->mJobErrorCode);
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
    setError(error.errorNumber());
    setLatestResponseCode(error.responseCode());
    setJobErrorText(error.internalErrorText());
    setJobError(error.jobErrorCode());
}
