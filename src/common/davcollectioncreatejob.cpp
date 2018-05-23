/*
    Copyright (c) 2010 Grégory Oestreicher <greg@kamago.net>
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

#include "davcollectioncreatejob.h"

#include "davcollectionfetchjob.h"
#include "davcollectionmodifyjob.h"
#include "daverror.h"
#include "davjob.h"
#include "davmanager.h"

#include <QColor>
#include <QMetaEnum>

using namespace KDAV2;

DavCollectionCreateJob::DavCollectionCreateJob(const DavCollection &collection, QObject *parent)
    : DavJobBase(parent), mCollection(collection), mRedirectCount(0)
{
}

void DavCollectionCreateJob::start()
{
    auto protocol = mCollection.url().protocol();
    switch (protocol) {
        case CalDav:
            // This is a calendar, use the MKCALENDAR request
            createCalendar();
            break;
        case CardDav:
            // This is an addressbook, use the extended MKCOL request
            createAddressbook();
            break;
        default: {
            // This is a normal collection
            auto job = DavManager::self()->createMkColJob(collectionUrl());
            connect(job, &DavJob::result, this, &DavCollectionCreateJob::collectionCreated);
        }
    }
}

DavCollection DavCollectionCreateJob::collection() const
{
    return mCollection;
}

QUrl DavCollectionCreateJob::collectionUrl() const
{
    return mCollection.url().url();
}

static QUrl assembleUrl(QUrl existingUrl, const QString &location)
{
    if (location.isEmpty()) {
        return existingUrl;
    } else if (location.startsWith(QLatin1Char('/'))) {
        auto url = existingUrl;
        url.setPath(location, QUrl::TolerantMode);
        return url;
    } else {
        return QUrl::fromUserInput(location);
    }
    return {};
}

void DavCollectionCreateJob::collectionCreated(KJob *job)
{
    auto *storedJob        = qobject_cast<DavJob *>(job);
    const int responseCode = storedJob->responseCode();

    if (responseCode == 301 || responseCode == 302 || responseCode == 307 || responseCode == 308) {
        if (mRedirectCount > 4) {
            setLatestResponseCode(responseCode);
            setError(UserDefinedError + responseCode);
            emitResult();
        } else {
            auto url = assembleUrl(storedJob->url(), storedJob->getLocationHeader());
            QUrl _collectionUrl(url);
            _collectionUrl.setUserInfo(collectionUrl().userInfo());
            mCollection.setUrl(DavUrl(_collectionUrl, mCollection.url().protocol()));

            ++mRedirectCount;
            start();
        }

        return;
    }

    if (storedJob->error()) {
        setLatestResponseCode(responseCode);
        setError(ERR_COLLECTIONCREATE);
        setJobErrorText(storedJob->errorText());
        setJobError(storedJob->error());
        setErrorTextFromDavError();

        emitResult();
        return;
    }

    auto url = assembleUrl(storedJob->url(), storedJob->getLocationHeader());
    url.setUserInfo(collectionUrl().userInfo());

    DavCollectionModifyJob *modifyJob =
        new DavCollectionModifyJob(DavUrl(url, mCollection.url().protocol()), this);

    modifyJob->setProperty(QStringLiteral("displayname"), mCollection.displayName());

    connect(modifyJob, &DavCollectionFetchJob::result, this, &DavCollectionCreateJob::collectionModified);
    modifyJob->start();
}

void DavCollectionCreateJob::collectionModified(KJob *job)
{
    if (job->error()) {
        setError(ERR_PROBLEM_WITH_REQUEST);
        setErrorTextFromDavError();
        emitResult();
        return;
    }

    DavCollectionFetchJob *fetchJob = new DavCollectionFetchJob(mCollection, this);
    connect(fetchJob, &DavCollectionFetchJob::result, this, &DavCollectionCreateJob::collectionRefreshed);
    fetchJob->start();
}

void DavCollectionCreateJob::collectionRefreshed(KJob *job)
{
    if (job->error()) {
        setError(ERR_PROBLEM_WITH_REQUEST);
        setErrorTextFromDavError();
        emitResult();
        return;
    }

    DavCollectionFetchJob *fetchJob = qobject_cast<DavCollectionFetchJob *>(job);
    mCollection                     = fetchJob->collection();

    emitResult();
}

void DavCollectionCreateJob::createCalendar()
{
    // clang-format off
    /* Create a query like this:
     *
     *  <C:mkcalendar xmlns:D='DAV:'xmlns:C='urn:ietf:params:xml:ns:caldav' xmlns:ICAL="http://apple.com/ns/ical/">
     *    <D:set>
     *      <D:prop>
     *        <D:displayname>Test Calendar</D:displayname>
     *        <ICAL:calendar-color>#24b0a3ff</ICAL:calendar-color>
     *        <C:supported-calendar-component-set>
     *          <C:comp name="VEVENT"/>
     *          <C:comp name="VJOURNAL"/>
     *          <C:comp name="VTODO"/>
     *        </C:supported-calendar-component-set>
     *      </D:prop>
     *    </D:set>
     *  </C:mkcalendar>
     */
    // clang-format on

    QDomDocument document;

    auto mkcalElement = document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("mkcalendar"));
    document.appendChild(mkcalElement);
    auto setElement = mkcalElement.appendChild(
        document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("set")));
    auto propElement =
        setElement.appendChild(document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("prop")));

    if (!mCollection.displayName().isEmpty()) {
        auto displayNameElement = propElement.appendChild(
            document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("displayname")));
        displayNameElement.appendChild(document.createTextNode(mCollection.displayName()));
    }

    if (mCollection.color().isValid()) {
        auto colorElement = propElement.appendChild(document.createElementNS(
            QStringLiteral("http://apple.com/ns/ical/"), QStringLiteral("calendar-color")));
        colorElement.appendChild(document.createTextNode(mCollection.color().name() + "FF"));
    }

    auto compSetElement = propElement.appendChild(document.createElementNS(
        QStringLiteral("urn:ietf:params:xml:ns:caldav"), QStringLiteral("supported-calendar-component-set")));

    auto supportedComp = mCollection.contentTypes();

    if (supportedComp.testFlag(DavCollection::Events)) {
        auto compElement = document.createElementNS(
            QStringLiteral("urn:ietf:params:xml:ns:caldav"), QStringLiteral("comp"));
        compElement.setAttribute(QStringLiteral("name"), QStringLiteral("VEVENT"));
        compSetElement.appendChild(compElement);
    }

    if (supportedComp.testFlag(DavCollection::Todos)) {
        auto compElement = document.createElementNS(
            QStringLiteral("urn:ietf:params:xml:ns:caldav"), QStringLiteral("comp"));
        compElement.setAttribute(QStringLiteral("name"), QStringLiteral("VTODO"));
        compSetElement.appendChild(compElement);
    }

    if (supportedComp.testFlag(DavCollection::FreeBusy)) {
        auto compElement = document.createElementNS(
            QStringLiteral("urn:ietf:params:xml:ns:caldav"), QStringLiteral("comp"));
        compElement.setAttribute(QStringLiteral("name"), QStringLiteral("VFREEBUSY"));
        compSetElement.appendChild(compElement);
    }

    if (supportedComp.testFlag(DavCollection::Journal)) {
        auto compElement = document.createElementNS(
            QStringLiteral("urn:ietf:params:xml:ns:caldav"), QStringLiteral("comp"));
        compElement.setAttribute(QStringLiteral("name"), QStringLiteral("VJOURNAL"));
        compSetElement.appendChild(compElement);
    }

    auto job = DavManager::self()->createMkCalendarJob(collectionUrl(), document);
    // Skip the modification
    connect(job, &DavJob::result, this, &DavCollectionCreateJob::collectionModified);
}

void DavCollectionCreateJob::createAddressbook()
{
    // clang-format off
    /* Create a query like this:
     *
     * <D:mkcol xmlns:D="DAV:" xmlns:C="urn:ietf:params:xml:ns:carddav">
     *   <D:set>
     *     <D:prop>
     *       <D:resourcetype>
     *         <D:collection/>
     *         <C:addressbook/>
     *       </D:resourcetype>
     *       <D:displayname>Lisa's Contacts</D:displayname>
     *     </D:prop>
     *   </D:set>
     * </D:mkcol>
     */
    // clang-format on

    QDomDocument document;

    auto mkcolElement = document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("mkcol"));
    document.appendChild(mkcolElement);
    auto setElement = mkcolElement.appendChild(
        document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("set")));
    auto propElement =
        setElement.appendChild(document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("prop")));

    auto resourceTypeElement = propElement.appendChild(
        document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("resourcetype")));

    resourceTypeElement.appendChild(
        document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("collection")));

    resourceTypeElement.appendChild(document.createElementNS(
        QStringLiteral("urn:ietf:params:xml:ns:carddav"), QStringLiteral("addressbook")));

    if (!mCollection.displayName().isEmpty()) {
        auto displayNameElement = propElement.appendChild(
            document.createElementNS(QStringLiteral("DAV:"), QStringLiteral("displayname")));
        displayNameElement.appendChild(document.createTextNode(mCollection.displayName()));
    }

    auto job = DavManager::self()->createMkColJob(collectionUrl(), document);
    // Skip the modification
    connect(job, &DavJob::result, this, &DavCollectionCreateJob::collectionModified);
}
