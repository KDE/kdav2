/*
    Copyright (c) 2017 Sandro Knauß <sknauss@kde.org>
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

#include "davcollectioncreatejobtest.h"

#include <KDAV2/DavCollectionCreateJob>
#include <KDAV2/DavCollectionDeleteJob>

#include <QColor>
#include <QTest>

void DavCollectionCreateJobTest::initTestCase()
{
    QString addressbookUuid = QUuid::createUuid().toString();
    addressbookUrl = "https://apps.kolabnow.com/addressbooks/test1%40kolab.org/" + addressbookUuid;
    addressbookUrl.setUserName("test1@kolab.org");
    addressbookUrl.setPassword("Welcome2KolabSystems");

    QString calendarUuid = QUuid::createUuid().toString();
    calendarUrl = "https://apps.kolabnow.com/calendars/test1%40kolab.org/" + calendarUuid;
    calendarUrl.setUserName("test1@kolab.org");
    calendarUrl.setPassword("Welcome2KolabSystems");
}

void DavCollectionCreateJobTest::runNormalCollectionTest()
{
}

void DavCollectionCreateJobTest::runAddressbookTest()
{
    KDAV2::DavUrl testCollectionUrl(addressbookUrl, KDAV2::CardDav);
    KDAV2::DavCollection testCollection;

    testCollection.setDisplayName("Test AddressBook Collection");
    testCollection.setUrl(testCollectionUrl);

    auto collectionCreateJob = new KDAV2::DavCollectionCreateJob(testCollection);
    collectionCreateJob->exec();

    QCOMPARE(collectionCreateJob->error(), 0);

    KDAV2::DavCollection resultCollection = collectionCreateJob->collection();

    QVERIFY(!resultCollection.CTag().isEmpty());
    QCOMPARE(resultCollection.displayName(), QLatin1String("Test AddressBook Collection"));

    delete collectionCreateJob;
}

void DavCollectionCreateJobTest::runCalendarTest()
{
    KDAV2::DavUrl testCollectionUrl(calendarUrl, KDAV2::CalDav);
    KDAV2::DavCollection testCollection;

    testCollection.setDisplayName("Test Calendar Collection");
    testCollection.setUrl(testCollectionUrl);
    testCollection.setContentTypes(KDAV2::DavCollection::Events | KDAV2::DavCollection::Todos);
    testCollection.setColor("#123456");

    auto collectionCreateJob = new KDAV2::DavCollectionCreateJob(testCollection);
    collectionCreateJob->exec();

    QCOMPARE(collectionCreateJob->error(), 0);

    KDAV2::DavCollection resultCollection = collectionCreateJob->collection();

    QVERIFY(!resultCollection.CTag().isEmpty());
    QCOMPARE(resultCollection.displayName(), QLatin1String("Test Calendar Collection"));
    QCOMPARE(resultCollection.color().name(), QLatin1String("#123456"));
    QVERIFY(resultCollection.contentTypes().testFlag(KDAV2::DavCollection::Events));
    QVERIFY(resultCollection.contentTypes().testFlag(KDAV2::DavCollection::Todos));

    delete collectionCreateJob;
}

void DavCollectionCreateJobTest::cleanupTestCase()
{
    {
        KDAV2::DavUrl testCollectionUrl(addressbookUrl, KDAV2::CardDav);

        auto collectionDeleteJob = new KDAV2::DavCollectionDeleteJob(testCollectionUrl);
        collectionDeleteJob->exec();
        QCOMPARE(collectionDeleteJob->error(), 0);

        delete collectionDeleteJob;
    }

    {
        KDAV2::DavUrl testCollectionUrl(calendarUrl, KDAV2::CalDav);

        auto collectionDeleteJob = new KDAV2::DavCollectionDeleteJob(testCollectionUrl);
        collectionDeleteJob->exec();
        QCOMPARE(collectionDeleteJob->error(), 0);

        delete collectionDeleteJob;
    }
}

QTEST_GUILESS_MAIN(DavCollectionCreateJobTest)
