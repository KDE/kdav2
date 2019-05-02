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

#ifndef KDAV2_DAVJOB_H
#define KDAV2_DAVJOB_H

#include <memory>

#include "kpimkdav2_export.h"

#include <KCoreAddons/KJob>
#include <QDomDocument>
#include <QUrl>
#include <QNetworkReply>

class DavJobPrivate;

namespace KDAV2
{
class KPIMKDAV2_EXPORT DavJob : public KJob
{
    Q_OBJECT

public:
    explicit DavJob(QNetworkReply *reply, QUrl url, QObject *parent = nullptr);
    ~DavJob();

    virtual void start() Q_DECL_OVERRIDE;

    QDomDocument response() const;
    QByteArray data() const;
    QUrl url() const;
    QNetworkReply::NetworkError responseCode() const;
    int httpStatusCode() const;
    QString getLocationHeader() const;
    QString getETagHeader() const;
    QString getContentTypeHeader() const;

private:
    void connectToReply(QNetworkReply *reply);
    std::unique_ptr<DavJobPrivate> d;
};

}

#endif
