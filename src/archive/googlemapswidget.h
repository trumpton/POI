#ifndef GOOGLEMAPSWIDGET_H
#define GOOGLEMAPSWIDGET_H

#include "configuration.h"

#include <QtWebEngineWidgets>
#include <QtWebChannel/QtWebChannel>

#include <QObject>
#include <QString>


class MapsWidget: public QWebEngineView
{
    // Enable slots and signals
    Q_OBJECT

private:
    QString apiKey ;
    QString cachedWorkingCollectionUuid ;
    QString cachedFileCollectionUuid ;
    QString cachedTrackCollectionUuid ;

signals:
    void mapMoved(double lat, double lon, int zoom) ;
    void markerMoved(const QString& uuid, const QString& collectionuuid, const double lat, const double lon) ;
    void markerGeocoded(const QString& uuid, const QString& collectionuuid, const QString& formattedaddress, QString &door, QString &street, QString& town, QString& state, QString& country, QString& postcode) ;
    void markerSelected(const QString& uuid, const QString& collectionuuid) ;
    void searchResultsReady(const double lat, const double lon, const QString& address, const QString& phone) ;
    void searchFailed(const QString& error) ;

public slots:
    void initialise2(bool ok) ;
    void jsmarkerSelected(QString uuid, QString collectionuuid) ;
    void jsmarkerMoved(QString uuid, QString collectionuuid, double lat, double lon) ;
    void jsmarkerGeocoded(QString uuid, QString collectionuuid, QString address, QString door, QString street, QString town, QString state, QString country, QString postcode) ;
    void jsSearchResultsReady(QString placeid, double lat, double lon, QString address, QString phone) ;
    void jsmapMoved(double lat, double lon, int zoom) ;
    void jsSearchFailed(QString error) ;
    void jsDebug(QString message) ;

    void authenticationRequired(const QUrl &requestUrl, QAuthenticator *auth) ;
    void proxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth, const QString &proxyHost) ;

public:
    explicit MapsWidget(QWidget *parent = 0);
    ~MapsWidget() ;

    // Initialise key, workingcollection and load web page
    void initialise(QString apiKey, QString initialWorkingUuid, QString initialFileUuid, QString initialTrackUuid) ;
    void registerUuids(QString workingUuid, QString fileUuid, QString trackUuid) ;
    void clearCookies() ;

    // Show Route Lines
    void showTracks(bool enabled) ;

    // Search Functions
    void searchLocation(QString address) ;

    // Geocode Functions
    void geocodeMarker(QString uuid, QString collectionuuid, bool forcegeocode=true) ;

    // Navigation Management
    void gotoCoordinates(double east, double north, int zoom=16) ;
    double getLat() ;
    double getLon() ;
    int getZoom() ;

    // Marker Management
    void setMarker(QString uuid, QString collectionuuid, double lat, double lon, QString address, int sequence, bool drop=false) ;
    void removeMarker(QString uuid) ;
    void removeAllMarkers() ;
    void setMarkerCollection(QString uuid, QString collectionuuid) ;
    bool queryMarker(QString uuid, QString& collectionuuid, double *lat, double *lon, QString& addresstxt) ;
    void selectMarker(QString uuid) ;
    void seekToMarker(QString uuid, int zoom) ;

private:
    // Initialise Javascript and set working collection uuid
    void initialiseJavascript() ;
    void runJavaScript(QString command) ;

    QWebChannel channel ;
    double dLat, dLon ;
    int iZoom ;
};

#endif // GOOGLEMAPSWIDGET_H
