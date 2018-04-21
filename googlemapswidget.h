#ifndef GOOGLEMAPSWIDGET_H
#define GOOGLEMAPSWIDGET_H

#include "WebAccess.h"
#include "configuration.h"

#include <QObject>
#include <QString>


class GoogleMapsWidget: public WebAccess
{
    // Enable slots and signals
    Q_OBJECT

private:
    QString apiKey ;
    QString cachedWorkingCollectionUuid ;

signals:
    void mapMoved(double lat, double lon, int zoom) ;
    void markerMoved(const QString& uuid, const QString& collectionuuid, const double lat, const double lon) ;
    void markerGeocoded(const QString& uuid, const QString& collectionuuid, const QString& address) ;
    void markerSelected(const QString& uuid, const QString& collectionuuid) ;
    void searchResultsReady(const QString& placeid, const double lat, const double lon, const QString& address, const QString& phone) ;
    void searchFailed(const QString& error) ;

public slots:
    void initialise2(bool ok) ;
    void jsmarkerSelected(QString uuid, QString collectionuuid) ;
    void jsmarkerMoved(QString uuid, QString collectionuuid, double lat, double lon) ;
    void jsmarkerGeocoded(QString uuid, QString collectionuuid, QString address) ;
    void jsSearchResultsReady(QString placeid, double lat, double lon, QString address, QString phone) ;
    void jsmapMoved(double lat, double lon, int zoom) ;
    void jsSearchFailed(QString error) ;

    void authenticationRequired(const QUrl &requestUrl, QAuthenticator *auth) ;
    void proxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth, const QString &proxyHost) ;

public:
    explicit GoogleMapsWidget(QWidget *parent = 0);
    ~GoogleMapsWidget() ;

    // Initialise key, workingcollection and load web page
    void initialise(QString apiKey, QString workingUuid) ;
    void clearCookies() ;

    // Search Functions
    void searchLocation(QString address) ;
    void searchLatLon(double lat, double lon) ;

    // Navigation Management
    void gotoCoordinates(double east, double north, int zoom=16) ;
    double getLat() ;
    double getLon() ;
    int getZoom() ;

    // Marker Management
    void setMarker(QString uuid, QString collectionuuid, double lat, double lon, QString address) ;
    void removeMarker(QString uuid) ;
    void removeAllMarkers() ;
    void setMarkerCollection(QString uuid, QString collectionuuid) ;
    bool queryMarker(QString uuid, QString& collectionuuid, double *lat, double *lon, QString& addresstxt) ;
    void selectMarker(QString uuid) ;
    void geocodeMarker(QString uuid, bool forcegeocode=true) ;
    void seekToMarker(QString uuid, int zoom) ;

private:
    // Initialise Javascript and set working collection uuid
    void initialiseJavascript(QString uuid) ;

    void runJavaScript(QString command) ;

#ifdef WEBENGINE
    QWebChannel channel ;
#else
    QNetworkAccessManager manager ;
#endif

    double dLat, dLon ;
    int iZoom ;
};

#endif // GOOGLEMAPSWIDGET_H
