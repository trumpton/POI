#include "cookies.h"

#include <QSettings>
#include <QNetworkCookie>
#include <QDebug>

Cookies::Cookies()
{}

Cookies::~Cookies()
{
    save() ;
}

bool Cookies::load(QString identifier)
{
    cachedIdentifier = identifier ;
    QSettings settings("trumpton.uk", QString("Cookies-") + cachedIdentifier);
    int size = settings.value("size").toInt() ;
    for (int i=0; i<size; i++) {
        QByteArray r = settings.value(QString::number(i)).toByteArray() ;
        qDebug() << "Loading Cookie: r=" << QString(r) << "\n";
        QList<QNetworkCookie> c = QNetworkCookie::parseCookies(r) ;
        if (c.size()>0) insertCookie(c.at(0)) ;
    }
    return true ;
}

bool Cookies::save()
{
    QSettings settings("trumpton.uk", QString("Cookies-") + cachedIdentifier);
    QList<QNetworkCookie> list = allCookies() ;
    settings.setValue("size", QString::number(list.size())) ;
    for (int i=0; i<list.size(); i++) {
        QNetworkCookie c = list.at(i) ;
            settings.setValue(QString::number(i), c.toRawForm()) ;
            qDebug() << "Saving Cookie: r=" << QString(c.toRawForm()) << "\n" ;
    }
    return true ;
}
