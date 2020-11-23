/*
    Copyright (c) 2016 Sandro Knauß <sknauss@kde.org>

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

#include "daverror.h"

using namespace KDAV2;

Error::Error()
    : mErrorNumber(NO_ERR)
    , mHttpStatusCode(0)
    , mResponseCode(0)
    , mJobErrorCode(0)
{
}

Error::Error(ErrorNumber errNo, int httpStatusCode, int responseCode, const QString &errorText, int jobErrorCode)
    : mErrorNumber(errNo)
    , mHttpStatusCode(httpStatusCode)
    , mResponseCode(responseCode)
    , mErrorText(errorText)
    , mJobErrorCode(jobErrorCode)
{
}

ErrorNumber Error::errorNumber() const
{
    return mErrorNumber;
}

QString Error::errorText() const
{
    return mErrorText;
}

int Error::jobErrorCode() const
{
    return mJobErrorCode;
}

int Error::httpStatusCode() const
{
    return mHttpStatusCode;
}

int Error::responseCode() const
{
    return mResponseCode;
}

QString Error::description() const
{
    switch (mErrorNumber) {
        case ERR_PROBLEM_WITH_REQUEST: {
            // User-side error
            QString err;
            if (mHttpStatusCode == 401) {
                err = QStringLiteral("Invalid username/password");
            } else if (mHttpStatusCode == 403) {
                err = QStringLiteral("Access forbidden");
            } else if (mHttpStatusCode == 404) {
                err = QStringLiteral("Resource not found");
            } else {
                err = QStringLiteral("HTTP error");
            }
            return QStringLiteral("There was a problem with the request.\n"
                            "%1 (%2).").arg(err).arg(mHttpStatusCode);
        }
        case ERR_NO_MULTIGET:
            return QStringLiteral("Protocol for the collection does not support MULTIGET");
        case ERR_SERVER_UNRECOVERABLE:
            return QStringLiteral("The server encountered an error that prevented it from completing your request: %1 (%2)").arg(mErrorText).arg(mHttpStatusCode);
        case ERR_COLLECTIONDELETE:
            return QStringLiteral("There was a problem with the request. The collection has not been deleted from the server.\n"
                            "%1 (%2).").arg(mErrorText).arg(mHttpStatusCode);
        case ERR_COLLECTIONFETCH:
            return QStringLiteral("Invalid responses from backend");
        case ERR_COLLECTIONFETCH_XQUERY_SETFOCUS:
            return QStringLiteral("Error setting focus for XQuery");
        case ERR_COLLECTIONFETCH_XQUERY_INVALID:
            return QStringLiteral("Invalid XQuery submitted by DAV implementation");
        case ERR_COLLECTIONMODIFY:
            return QStringLiteral("There was a problem with the request. The collection has not been modified on the server.\n"
                        "%1 (%2).").arg(mErrorText).arg(mHttpStatusCode);
        case ERR_COLLECTIONMODIFY_NO_PROPERITES:
            return QStringLiteral("No properties to change or remove");
        case ERR_COLLECTIONMODIFY_RESPONSE: {
            auto result = QStringLiteral("There was an error when modifying the properties");
            if (!mErrorText.isEmpty()) {
                result.append(QStringLiteral("\nThe server returned more information:\n%1").arg(mErrorText));
            }
            return result;
        }
        case ERR_COLLECTIONCREATE:
            return QStringLiteral("There was an error when creating the collection");
        case ERR_ITEMCREATE:
            return QStringLiteral("There was a problem with the request. The item has not been created on the server.\n"
                        "%1 (%2).").arg(mErrorText).arg(mHttpStatusCode);
        case ERR_ITEMDELETE:
            return QStringLiteral("There was a problem with the request. The item has not been deleted from the server.\n"
                        "%1 (%2).").arg(mErrorText).arg(mHttpStatusCode);
        case ERR_ITEMMODIFY:
            return QStringLiteral("There was a problem with the request. The item was not modified on the server.\n"
                        "%1 (%2).").arg(mErrorText).arg(mHttpStatusCode);
        case ERR_ITEMLIST:
            return QStringLiteral("There was a problem with the request.");
        case ERR_ITEMLIST_NOMIMETYPE:
            return QStringLiteral("There was a problem with the request. The requested mimetypes are not supported.");
        case NO_ERR:
            break;
    }
    return mErrorText;
}
