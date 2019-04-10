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

#include "davjob.h"

#include "davmanager.h"
#include "libkdav2_debug.h"

#include <QTextStream>

using namespace KDAV2;

class DavJobPrivate {
public:
    DavJobPrivate();

    QByteArray data;
    QDomDocument doc;
    QUrl url;

    QString location;
    QString etag;
    QString contentType;
    int responseCode = 0;
};

DavJobPrivate::DavJobPrivate()
{
}

DavJob::DavJob()
    : KJob()
    , d(std::unique_ptr<DavJobPrivate>(new DavJobPrivate()))
{
}

DavJob::DavJob(QNetworkReply *reply, QUrl url, QObject *parent)
    : KJob(parent),
    d(std::unique_ptr<DavJobPrivate>(new DavJobPrivate()))
{
    d->url = url;
    connectToReply(reply);
}

DavJob::~DavJob()
{
}

void DavJob::connectToReply(QNetworkReply *reply)
{
    QObject::connect(reply, &QNetworkReply::readyRead, this, [=] () {
        d->data.append(reply->readAll());
    });
    QObject::connect(reply, static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error), this, [=] (QNetworkReply::NetworkError error) {
        qCWarning(KDAV2_LOG) << "Error " << error << reply->errorString() << "HTTP Status code " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qCWarning(KDAV2_LOG) << "Error " << reply->readAll();
    });
    QObject::connect(reply, &QNetworkReply::metaDataChanged, this, [=] () {
        qCDebug(KDAV2_LOG) << "Metadata changed: " << reply->rawHeaderPairs();
        d->location = reply->rawHeader("Location");
        d->etag = reply->rawHeader("ETag");
        //"text/x-vcard; charset=utf-8" -> "text/x-vcard"
        d->contentType = reply->rawHeader("Content-Type").split(';').first();
    });
    QObject::connect(reply, &QNetworkReply::finished, this, [=] () {
        //This is a workaround for QNetworkAccessManager::setRedirectPolicy(QNetworkRequest::UserVerifiedRedirectPolicy),
        //which does not seem to work with multiple redirects.
        const auto possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if(!possibleRedirectUrl.isEmpty()) {
            qCDebug(KDAV2_LOG) << "Redirecting to " << possibleRedirectUrl;
            auto request = reply->request();
            request.setUrl(possibleRedirectUrl);
            reply->disconnect(this);

            //Set in QWebdav
            const auto requestData = reply->property("requestData").toByteArray();
            d->data.clear();

            auto redirectReply = [&] {
                if (reply->property("isPut").toBool()) {
                    return DavManager::networkAccessManager()->put(request, requestData);
                }
                return DavManager::networkAccessManager()->sendCustomRequest(request, request.attribute(QNetworkRequest::CustomVerbAttribute).toByteArray(), requestData);
            }();
            redirectReply->setProperty("requestData", requestData);
            connectToReply(redirectReply);
            return;
        }

        //Could have changed due to redirects
        d->url = reply->url();

        d->doc.setContent(d->data, true);

        if (KDAV2_LOG().isDebugEnabled()) {
            QTextStream stream(stdout, QIODevice::WriteOnly);
            d->doc.save(stream, 2);
        }

        d->responseCode = reply->error();
        if (d->responseCode) {
            setError(KJob::UserDefinedError);
            setErrorText(reply->errorString());
        }
        emitResult();
    });

}

void DavJob::start()
{
}

QDomDocument DavJob::response()
{
    return d->doc;
}

QByteArray DavJob::data()
{
    return d->data;
}

QUrl DavJob::url()
{
    return d->url;
}

QString DavJob::getLocationHeader()
{
    return d->location;
}

QString DavJob::getETagHeader()
{
    return d->etag;
}

QString DavJob::getContentTypeHeader()
{
    return d->contentType;
}

int DavJob::responseCode()
{
    return d->responseCode;
}
