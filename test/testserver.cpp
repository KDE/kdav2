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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <KDAV2/Utils>
#include <KDAV2/DavCollectionDeleteJob>
#include <KDAV2/DavCollectionModifyJob>
#include <KDAV2/DavCollectionsFetchJob>
#include <KDAV2/DavItemFetchJob>
#include <KDAV2/DavItemCreateJob>
#include <KDAV2/DavItemsFetchJob>
#include <KDAV2/DavItemModifyJob>
#include <KDAV2/DavItemDeleteJob>
#include <KDAV2/DavItemsListJob>
#include <KDAV2/Utils>

#include <QtCore/QDebug>
#include <QtCore/QStringList>
#include <QCoreApplication>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QUrl mainUrl(QStringLiteral("https://apps.kolabnow.com/addressbooks/test1%40kolab.org"));
    mainUrl.setUserName(QStringLiteral("test1@kolab.org"));
    mainUrl.setPassword(QStringLiteral("Welcome2KolabSystems"));
    KDAV2::DavUrl davUrl(mainUrl, KDAV2::CardDav);

    auto *job = new KDAV2::DavCollectionsFetchJob(davUrl);
    job->exec();
    if (job->error()) {
        qWarning() << "DavCollectionsFetchJob failed " << job->errorString();
    } else {
        qInfo() << "DavCollectionsFetchJob succeeded";
    }

    for(const auto collection : job->collections()) {
        qInfo() << collection.displayName() << "PRIVS: " << collection.privileges();
        auto collectionUrl = collection.url();
        int anz = -1;
        {
            auto itemListJob = new KDAV2::DavItemsListJob(collectionUrl);
            itemListJob->exec();
            anz = itemListJob->items().size();
            qInfo() << "items:" << itemListJob->items().size();
            for (const auto &item : itemListJob->items()) {
                qInfo() << item.url().url() << item.contentType() << item.data();
                auto itemFetchJob = new KDAV2::DavItemFetchJob(item);
                itemFetchJob->exec();
                const auto fetchedItem = itemFetchJob->item();
                qInfo() << fetchedItem.contentType() << fetchedItem.data();

                auto itemsFetchJob = new KDAV2::DavItemsFetchJob(collectionUrl, QStringList() << item.url().toDisplayString());
                itemsFetchJob->exec();
                if (itemsFetchJob->item(item.url().toDisplayString()).contentType() != fetchedItem.contentType()) {       //itemsfetchjob do not get contentType
                    qInfo() << "Fetched same item but got different contentType:" << itemsFetchJob->item(item.url().toDisplayString()).contentType();
                }

                if (itemsFetchJob->item(item.url().toDisplayString()).data() != fetchedItem.data()) {
                    qInfo() << "Fetched same item but got different data:" << itemsFetchJob->item(item.url().toDisplayString()).data();
                }
            }
        }
        {
            qInfo() << "second run: (should be empty).";
            auto itemListJob = new KDAV2::DavItemsListJob(collectionUrl);
            itemListJob->exec();
            if (itemListJob->items().size() != anz) {
                qInfo() << "Items have added/deleted on server.";
            }
        }
    }

    {
        QUrl url(QStringLiteral("https://apps.kolabnow.com/addressbooks/test1%40kolab.org/cbbf386d-7e9b-4e72-947d-0b813ea9b347/"));
        url.setUserInfo(mainUrl.userInfo());
        KDAV2::DavUrl collectionUrl(url, KDAV2::CardDav);
        auto collectionDeleteJob = new KDAV2::DavCollectionDeleteJob(collectionUrl);
        collectionDeleteJob->exec();
        if (collectionDeleteJob->error()) {
            qWarning() << "Delete job failed: " << collectionDeleteJob->errorString();
        }
    }

    {
        QUrl url(QStringLiteral("https://apps.kolabnow.com/addressbooks/test1%40kolab.org/9290e784-c876-412f-8385-be292d64b2c6/"));
        url.setUserInfo(mainUrl.userInfo());
        KDAV2::DavUrl testCollectionUrl(url, KDAV2::CardDav);
        auto collectionModifyJob = new KDAV2::DavCollectionModifyJob(testCollectionUrl);
        collectionModifyJob->setProperty(QStringLiteral("displayname"), QStringLiteral("test234"));
        collectionModifyJob->exec();
        if (collectionModifyJob->error()) {
            qWarning() << "Modify job failed:" << collectionModifyJob->errorString();
        }
    }

    //create element with "wrong put url" test if we get the correct url back
    {
        QUrl url(QStringLiteral("https://apps.kolabnow.com/addressbooks/test1%40kolab.org/9290e784-c876-412f-8385-be292d64b2c6/xxx.vcf"));
        url.setUserInfo(mainUrl.userInfo());
        KDAV2::DavUrl testItemUrl(url, KDAV2::CardDav);
        QByteArray data = "BEGIN:VCARD\r\nVERSION:3.0\r\nPRODID:-//Kolab//iRony DAV Server 0.3.1//Sabre//Sabre VObject 2.1.7//EN\r\nUID:12345678-1234-1234-1234-123456789abc\r\nFN:John Doe\r\nN:Doe;John;;;\r\nEMAIL;TYPE=INTERNET;TYPE=HOME:john.doe@example.com\r\nREV;VALUE=DATE-TIME:20161221T145611Z\r\nEND:VCARD\r\n";
        KDAV2::DavItem item(testItemUrl, QStringLiteral("text/x-vcard"), data, QString());
        auto createJob = new KDAV2::DavItemCreateJob(item);
        createJob->exec();
        if (createJob->error()) {
            qWarning() << "Create job failed: " << createJob->errorString();
        }
        if (createJob->item().url().toDisplayString() != QStringLiteral("https://apps.kolabnow.com/addressbooks/test1%40kolab.org/9290e784-c876-412f-8385-be292d64b2c6/12345678-1234-1234-1234-123456789abc.vcf")) {
            qWarning() << "unexpected url" << createJob->item().url().url();
        }
    }

    {
        QUrl url(QStringLiteral("https://apps.kolabnow.com/addressbooks/test1%40kolab.org/9290e784-c876-412f-8385-be292d64b2c6/12345678-1234-1234-1234-123456789abc.vcf"));
        url.setUserInfo(mainUrl.userInfo());
        KDAV2::DavUrl testItemUrl(url, KDAV2::CardDav);
        QByteArray data = "BEGIN:VCARD\r\nVERSION:3.0\r\nPRODID:-//Kolab//iRony DAV Server 0.3.1//Sabre//Sabre VObject 2.1.7//EN\r\nUID:12345678-1234-1234-1234-123456789abc\r\nFN:John2 Doe\r\nN:Doe;John2;;;\r\nEMAIL;TYPE=INTERNET;TYPE=HOME:john2.doe@example.com\r\nREV;VALUE=DATE-TIME:20161221T145611Z\r\nEND:VCARD\r\n";
        KDAV2::DavItem item(testItemUrl, QStringLiteral("text/x-vcard"), data, QString());
        auto modifyJob = new KDAV2::DavItemModifyJob(item);
        modifyJob->exec();
        if (modifyJob->error()) {
            qWarning() << "Modify job failed " << modifyJob->errorString();
        }
    }

    {
        QUrl url(QStringLiteral("https://apps.kolabnow.com/addressbooks/test1%40kolab.org/9290e784-c876-412f-8385-be292d64b2c6/12345678-1234-1234-1234-123456789abc.vcf"));
        url.setUserInfo(mainUrl.userInfo());
        KDAV2::DavUrl testItemUrl(url, KDAV2::CardDav);
        QByteArray data = "BEGIN:VCARD\r\nVERSION:3.0\r\nPRODID:-//Kolab//iRony DAV Server 0.3.1//Sabre//Sabre VObject 2.1.7//EN\r\nUID:12345678-1234-1234-1234-123456789abc\r\nFN:John2 Doe\r\nN:Doe;John2;;;\r\nEMAIL;TYPE=INTERNET;TYPE=HOME:john2.doe@example.com\r\nREV;VALUE=DATE-TIME:20161221T145611Z\r\nEND:VCARD\r\n";
        KDAV2::DavItem item(testItemUrl, QStringLiteral("text/x-vcard"), data, QString());
        auto deleteJob = new KDAV2::DavItemDeleteJob(item);
        deleteJob->exec();
        if (deleteJob->error()) {
            qWarning() << "Delete job failed " << deleteJob->errorString();
        }
    }
}
