#include <QEventLoop>
#include "tomtomwidget.h"
#define URL "https://mydrive.tomtom.com/en_gb/"

TomTomWidget::TomTomWidget(QWidget *parent) :
    WebAccess(parent)
{
#ifndef WEBENGINE
    cookies.load("POI-Editor") ;
    this->page()->networkAccessManager()->setCookieJar(&cookies);
#endif
}

TomTomWidget::~TomTomWidget()
{
}

void TomTomWidget::initialise()
{
    connect(this->page(),SIGNAL(unsupportedContent(QNetworkReply*)),this,SLOT(jsUnsupportedContent(QNetworkReply*)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(jsLoadFinished(bool))) ;

    setUrl(QUrl(URL)) ;
}

void TomTomWidget::clearCookies()
{
}

void TomTomWidget::jsLoadFinished(bool ok)
{
}

// Download unsupported content
void TomTomWidget::jsUnsupportedContent(QNetworkReply *reply)
{
    // TODO: Not Required???
    qDebug()<<"Unsupported Content: " << reply->url().toString();
    if (reply->error() == QNetworkReply::NoError) {
        QString url = reply->url().toString() ;
        QRegExp rx("personal_info") ;
        if (rx.indexIn(url)>=0) {

            QEventLoop loop;
            QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
            loop.exec();
            QString response=reply->readAll();
            QString err = reply->errorString() ;

            // TODO: Parse response and store
            {
                QString description = "new entry [23]>+44123456" ;
                double lat=1, lon=1 ;
                qDebug() << "Exported: " << description << "." ;
                emit exportPoi(description, lat, lon) ;
            }
        }
    }
}
