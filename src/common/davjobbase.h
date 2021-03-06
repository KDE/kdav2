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

#ifndef KDAV2_DAVJOBBASE_H
#define KDAV2_DAVJOBBASE_H

#include <memory>
#include "kpimkdav2_export.h"
#include <KJob>
#include "daverror.h"

struct DavJobBasePrivate;

namespace KDAV2
{
class Error;
class DavJob;

/**
 * @short base class for the jobs used by the resource.
 */
class KPIMKDAV2_EXPORT DavJobBase : public KJob
{
    Q_OBJECT

public:
    explicit DavJobBase(QObject *parent = nullptr);
    ~DavJobBase();

    /**
     * Get the latest http status code.
     *
     * If no response code has been set then 0 will be returned, but will
     * be meaningless unless error() is non-zero. In that case this means
     * that the latest error was not at the HTTP level.
     */
    unsigned int latestHttpStatusCode() const;

    /**
     * Get the response code.
     *
     * This is a QNetworkReply::NetworkError
     *
     * If no response code has been set then 0 will be returned, but will
     * be meaningless unless error() is non-zero. In that case this means
     * that the latest error was not at the HTTP level.
     *
     */
    unsigned int latestResponseCode() const;

    /**
     * Check if the job can be retried later.
     *
     * This will return true for transient errors, i.e. if the response code
     * is either zero and error() is set or if the HTTP response code hints
     * at a temporary error.
     *
     * The HTTP response codes considered retryable are:
     *   - 401
     *   - 402
     *   - 407
     *   - 408
     *   - 423
     *   - 429
     *   - 501 to 504, inclusive
     *   - 507
     *   - 511
     */
    bool canRetryLater() const;

    /**
     * Check if the job failed because of a conflict
     */
    bool hasConflict() const;

    /**
     * Returns a instance of the KDAV2:Error to be able to translate the error
     */
    Error davError() const;

protected:
    void setErrorTextFromDavError();
    void setDavError(const Error &error);

    /**
     * Set the error of this job from a failed DavJob (executed by this job).
     */
    void setErrorFromJob(DavJob*, ErrorNumber jobErrorCode = ERR_PROBLEM_WITH_REQUEST);
private:
    std::unique_ptr<DavJobBasePrivate> d;
};

}

#endif
