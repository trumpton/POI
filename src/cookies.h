#ifndef COOKIES_H
#define COOKIES_H

#include <QString>
#include <QNetworkCookieJar>

class Cookies : public QNetworkCookieJar {

public:
    Cookies() ;
    ~Cookies() ;
    bool load(QString identifier) ;
    bool save() ;

private:
    QString cachedIdentifier ;

};


#endif // COOKIES_H
