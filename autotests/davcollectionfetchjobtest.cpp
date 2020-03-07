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

#include "davcollectionfetchjobtest.h"

#include <KDAV2/DavCollectionFetchJob>

#include <QColor>
#include <QTest>

void DavCollectionFetchJobTest::runAddressbookTest()
{
    QUrl url(QStringLiteral("https://apps.kolabnow.com/addressbooks/test1%40kolab.org/7511b26d-198c-41b0-8cbf-78923ee1ca8c"));
    url.setUserName("test1@kolab.org");
    url.setPassword("Welcome2KolabSystems");
    KDAV2::DavUrl testCollectionUrl(url, KDAV2::CalDav);
    KDAV2::DavCollection testCollection;
    testCollection.setUrl(testCollectionUrl);
    auto collectionFetchJob = new KDAV2::DavCollectionFetchJob(testCollection);
    collectionFetchJob->exec();

    QCOMPARE(collectionFetchJob->error(), 0);

    QCOMPARE(testCollection.CTag(), QString());

    KDAV2::DavCollection resultCollection = collectionFetchJob->collection();

    QVERIFY(!resultCollection.CTag().isEmpty());
    QCOMPARE(resultCollection.displayName(), QLatin1String("Contacts"));
}

void DavCollectionFetchJobTest::runCalendarTest()
{
    QUrl url(QStringLiteral("https://apps.kolabnow.com/calendars/test1%40kolab.org/52abfc74-2441-49b5-80ef-f058d0fa4bd0"));
    url.setUserName("test1@kolab.org");
    url.setPassword("Welcome2KolabSystems");
    KDAV2::DavUrl testCollectionUrl(url, KDAV2::CalDav);
    KDAV2::DavCollection testCollection;
    testCollection.setUrl(testCollectionUrl);
    auto collectionFetchJob = new KDAV2::DavCollectionFetchJob(testCollection);
    collectionFetchJob->exec();

    QCOMPARE(collectionFetchJob->error(), 0);

    QCOMPARE(testCollection.CTag(), QString());
    QCOMPARE(testCollection.color(), QColor());

    KDAV2::DavCollection resultCollection = collectionFetchJob->collection();

    QVERIFY(!resultCollection.CTag().isEmpty());
    QCOMPARE(resultCollection.displayName(), QLatin1String("Calendar"));
    QCOMPARE(resultCollection.color().name(), QLatin1String("#cc0000"));
}

QTEST_GUILESS_MAIN(DavCollectionFetchJobTest)
