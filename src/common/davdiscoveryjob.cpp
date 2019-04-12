/*
    Copyright (c) 2018 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include "davdiscoveryjob.h"

#include "davmanager.h"
#include "davprotocolbase.h"
#include "daverror.h"
#include "utils.h"
#include "davjob.h"

using namespace KDAV2;

DavDiscoveryJob::DavDiscoveryJob(const DavUrl &davUrl, const QString &wellKnownSuffix, QObject *parent)
    : DavJobBase(parent), mUrl(davUrl)
{
    auto url = davUrl.url();
    if (!url.toString().contains("/.well-known/")) {
        url.setPath(url.path() + "/.well-known/" + wellKnownSuffix);
        mUrl.setUrl(url);
    }
}

void DavDiscoveryJob::start()
{
    QDomDocument document;

    QDomElement propfindElement = document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("propfind"));
    document.appendChild(propfindElement);

    QDomElement propElement = document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("prop"));
    propfindElement.appendChild(propElement);

    propElement.appendChild(document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("current-user-principal")));
    propElement.appendChild(document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("principal-URL")));

    DavJob *job = DavManager::self()->createPropFindJob(mUrl.url(), document, QStringLiteral("0"));
    connect(job, &DavJob::result, this, &DavDiscoveryJob::davJobFinished);
}

QUrl DavDiscoveryJob::url() const
{
    return mUrl.url();
}

void DavDiscoveryJob::davJobFinished(KJob *job)
{
    DavJob *davJob = qobject_cast<DavJob *>(job);

    if (davJob->error()) {
        //Retry on the root uri on 404, otherwise fail
        if (davJob->responseCode() == 404 && davJob->url().path() != QStringLiteral("/")) {
            auto url = mUrl.url();
            url.setPath("/");
            mUrl.setUrl(url);
            start();
            return;
        }
        setLatestResponseCode(davJob->responseCode());
        setError(ERR_PROBLEM_WITH_REQUEST);
        setJobErrorText(davJob->errorText());
        setJobError(davJob->error());
        setErrorTextFromDavError();

        emitResult();
        return;
    }
    mUrl.setUrl(davJob->url());

    const QDomDocument document = davJob->response();
    const QDomElement multistatusElement = document.documentElement();


    const QString principalHref = [&] {
        QDomElement responseElement = Utils::firstChildElementNS(multistatusElement, QStringLiteral("DAV:"), QStringLiteral("response"));
        while (!responseElement.isNull()) {

            const QDomElement propstatElement = [&] {
                // check for the valid propstat, without giving up on first error
                const QDomNodeList propstats = responseElement.elementsByTagNameNS(QStringLiteral("DAV:"), QStringLiteral("propstat"));
                for (int i = 0; i < propstats.length(); ++i) {
                    const QDomElement propstatCandidate = propstats.item(i).toElement();
                    const QDomElement statusElement = Utils::firstChildElementNS(propstatCandidate, QStringLiteral("DAV:"), QStringLiteral("status"));
                    if (statusElement.text().contains(QLatin1String("200"))) {
                        return propstatCandidate;
                    }
                }
                return QDomElement{};
            }();

            if (propstatElement.isNull()) {
                responseElement = Utils::nextSiblingElementNS(responseElement, QStringLiteral("DAV:"), QStringLiteral("response"));
                continue;
            }

            // extract home sets
            const QDomElement propElement = Utils::firstChildElementNS(propstatElement, QStringLiteral("DAV:"), QStringLiteral("prop"));

            // Trying to get the principal url, given either by current-user-principal or principal-URL
            QDomElement urlHolder = Utils::firstChildElementNS(propElement, QStringLiteral("DAV:"), QStringLiteral("current-user-principal"));
            if (urlHolder.isNull()) {
                urlHolder = Utils::firstChildElementNS(propElement, QStringLiteral("DAV:"), QStringLiteral("principal-URL"));
            }

            if (!urlHolder.isNull()) {
                // Getting the href that will be used for the next round
                const QDomElement hrefElement = Utils::firstChildElementNS(urlHolder, QStringLiteral("DAV:"), QStringLiteral("href"));
                if (!hrefElement.isNull()) {
                    return hrefElement.text();
                }
            }

            responseElement = Utils::nextSiblingElementNS(responseElement, QStringLiteral("DAV:"), QStringLiteral("response"));
        }
        return QString{};
    }();

    QUrl principalUrl(mUrl.url());

    if (principalHref.startsWith(QLatin1Char('/'))) {
        // principalHref is only a path, use request url to complete
        principalUrl.setPath(principalHref, QUrl::TolerantMode);
    } else {
        // href is a complete url
        principalUrl = QUrl::fromUserInput(principalHref);
        principalUrl.setUserName(mUrl.url().userName());
        principalUrl.setPassword(mUrl.url().password());
    }

    mUrl.setUrl(principalUrl);
    emitResult();
}
