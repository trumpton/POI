#ifndef TOMTOMWIDGET_H
#define TOMTOMWIDGET_H

#include "WebAccess.h"
#include "configuration.h"

#include <QList>
#include <QObject>
#include <QNetworkReply>
#include <QNetworkCookieJar>

#ifndef WEBENGINE
#include "cookies.h"
#endif

class TomTomWidget : public WebAccess
{
    Q_OBJECT

public:
    explicit TomTomWidget(QWidget *parent = 0);
    ~TomTomWidget() ;
    void initialise() ;
    void clearCookies() ;

signals:
    void exportPoi(QString description, double lat, double lon) ;

private slots:
    void jsLoadFinished(bool ok) ;
    void jsUnsupportedContent(QNetworkReply *reply) ;

private:
    bool importFavorites(QString surl) ;

#ifndef WEBENGINE
    Cookies cookies ;
#endif

};

#endif // TOMTOMWIDGET_H
