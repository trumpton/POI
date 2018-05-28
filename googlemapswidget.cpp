#include <QDebug>
#include <QFile>
#include <QMessageBox>

#include "googlemapswidget.h"

GoogleMapsWidget::GoogleMapsWidget(QWidget *parent) :
   WebAccess(parent)
{

    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(initialise2(bool))) ;

#ifdef WEBENGINE
    connect(this, SIGNAL(authenticationRequired(const QUrl &, QAuthenticator*)),
            SLOT(authenticationRequired(const QUrl &, QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(const QUrl &, QAuthenticator *, const QString &)),
            SLOT(proxyAuthenticationRequired(const QUrl &, QAuthenticator *, const QString &)));
#endif

    dLat=0 ; dLon=0 ; iZoom=0 ;
}

GoogleMapsWidget::~GoogleMapsWidget()
{
}

void GoogleMapsWidget::initialise(QString key, QString workingUuid)
{
    // Save uuid
    cachedWorkingCollectionUuid = workingUuid ;

    // Load the html for the page
    QUrl url = QUrl("qrc:///googlemaps/google_maps.html") ;
    QFile f(":/googlemaps/google_maps.html") ;
    f.open(QFile::ReadOnly|QFile::Text) ;
    QTextStream in(&f) ;
    QString html = in.readAll() ;
    f.close() ;
    html = html.replace(QString("{APIKEY}"), key) ;
    setHtml(html, url) ;

}

void GoogleMapsWidget::initialise2(bool ok)
{
    // If initialise hasn't been called, ignore the signal
    if (cachedWorkingCollectionUuid.isEmpty()) return ;

    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::loadFinished() =>" ;
    qDebug() << "bool: " << (ok?"true":"false") ;
    if (ok) {

#ifdef WEBENGINE
        this->page()->setWebChannel(&channel) ;
        channel.registerObject("GoogleMapsWidget", this) ;
#else
        this->page()->mainFrame()->addToJavaScriptWindowObject("GoogleMapsWebViewWidget", this) ;
#endif

        // Finally, initialise the javascript (connect to channel & register workingcollectionuuid)
        initialiseJavascript(cachedWorkingCollectionUuid) ;


    } else {
        QMessageBox::information(NULL, "Error", "Error, unable to load Google Maps") ;
    }
}

void GoogleMapsWidget::clearCookies()
{
}

//
// Authentication
//
void GoogleMapsWidget::authenticationRequired(const QUrl &requestUrl, QAuthenticator *auth)
{
    Q_UNUSED(requestUrl) ;
    Q_UNUSED(auth) ;
    return ;
}

void GoogleMapsWidget::proxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth, const QString &proxyHost)
{
    Q_UNUSED(requestUrl);
    Q_UNUSED(auth) ;
    Q_UNUSED(proxyHost) ;
    return ;
}

//
// Map Status
//

double GoogleMapsWidget::getLat()
{
    return dLat ;
}

double GoogleMapsWidget::getLon()
{
    return dLon ;
}

int GoogleMapsWidget::getZoom()
{
    return iZoom ;
}

//
// Javascript access functions
//

void GoogleMapsWidget::runJavaScript(QString command)
{
#ifdef WEBENGINE
    page()->runJavaScript(command) ;
#else
    page()->mainFrame()->evaluateJavaScript(command) ;
#endif
}


//
// Collection identification
//

void GoogleMapsWidget::initialiseJavascript(QString uuid)
{
    QString str = QString("initialise(\"%1\");").arg(uuid) ;

    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::initialiseJavascript() =>" ;
    qDebug() << str ;

    runJavaScript(str) ;
}


//
// Javascript navigaion functions
//

void GoogleMapsWidget::gotoCoordinates(double lat, double lon, int zoom)
{

    QString str = QString("gotoCoordinates(%1, %2, %3);").arg(lat).arg(lon).arg(zoom) ;

    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::gotoCoordinates() =>" ;
    qDebug() << str ;

    dLat = lat ; dLon = lon ; iZoom = zoom ;
    runJavaScript(str) ;
}

//
// Javascript marker set / management functions
//

void GoogleMapsWidget::setMarker(QString uuid, QString collectionuuid, double east, double north, QString address)
{


    removeMarker(uuid) ;
    QString str =
      QString("setMarker(\"%1\", \"%2\", %3, %4, \"%5\");").
            arg(uuid).
            arg(collectionuuid).
            arg(east).arg(north).
            arg(address) ;
    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::setMarker() =>" ;
    qDebug() << str ;

    runJavaScript(str) ;
}

void GoogleMapsWidget::setMarkerCollection(QString uuid, QString collectionuuid)
{


    QString str =
      QString("setMarkerCollection(\"%1\", \"%2\");").
            arg(uuid).
            arg(collectionuuid) ;
    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::setMarkerCollection() =>" ;
    qDebug() << str ;

    runJavaScript(str) ;
}

void GoogleMapsWidget::selectMarker(QString uuid)
{

    QString str = QString("selectMarker(\"%1\");").arg(uuid) ;

    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::selectMarker() =>" ;
    qDebug() << str ;
    runJavaScript(str) ;
}


void GoogleMapsWidget::seekToMarker(QString uuid, int zoom)
{

    QString str = QString("seekToMarker(\"%1\",%2);").arg(uuid).arg(zoom) ;

    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::seekToMarker() =>" ;
    qDebug() << str ;
    runJavaScript(str) ;
}

void GoogleMapsWidget::removeMarker(QString uuid)
{

    QString str = QString("removeMarker(\"%1\");").arg(uuid) ;

    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::removeMarker() =>" ;
    qDebug() << str ;
    runJavaScript(str) ;
}

void GoogleMapsWidget::removeAllMarkers()
{

    QString str = QString("removeAllMarkers();") ;
    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::removeAllMarkers() =>" ;
    qDebug() << str ;
    runJavaScript(str) ;
}


//
// Javascript signal handlers
//
void GoogleMapsWidget::jsmapMoved(double lat, double lon, int zoom)
{
    dLat = lat ; dLon = lon ; iZoom = zoom ;
    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::jsmapMoved() =>" ;
    qDebug() << "lat = " << dLat << ", lon = " << dLon << ", zoom = " << iZoom ;
    emit mapMoved(dLat, dLon, iZoom) ;
}

void GoogleMapsWidget::jsmarkerMoved(QString uuid, QString collectionuuid, double lat, double lon)
{
    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::jsmarkerMoved() =>" ;
    qDebug() << "uuid = " << uuid ;
    qDebug() << "collectionuuid = " << collectionuuid ;
    qDebug() << "lat/lon = " << lat << "," << lon ;
    geocodeMarker(uuid, collectionuuid, true) ;
    emit markerMoved(uuid, collectionuuid, lat, lon) ;
}


void GoogleMapsWidget::jsmarkerSelected(QString uuid, QString collectionuuid)
{
    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::jsmarkerSelected() =>" ;
    qDebug() << "uuid = " << uuid ;
    qDebug() << "collectionuuid = " << collectionuuid ;

    // TODO: Check if geocoding as actually necessary here
    // As marker is geocoded once placed, and when moved
    // geocodeMarker(uuid, collectionuuid, false) ;

    emit markerSelected(uuid, collectionuuid) ;
}




//
// Geocoding
//

void GoogleMapsWidget::geocodeMarker(QString uuid, QString collectionuuid, bool forcegeocode)
{

    QString str = QString("geocodeMarker(\"%1\",\"%2\",%3);").arg(uuid).arg(collectionuuid).arg((forcegeocode?"true":"false")) ;

    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::geocodeMarker() =>" ;
    qDebug() << str ;
    runJavaScript(str) ;
}

void GoogleMapsWidget::jsmarkerGeocoded(QString uuid, QString collectionuuid, QString formattedaddress, QString door, QString street, QString town, QString state, QString country, QString postcode)
{
    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::jsmarkerGeocoded() =>" ;
    qDebug() << "uuid = " << uuid ;
    qDebug() << "collectionuuid = " << collectionuuid ;
    qDebug() << "address = " << formattedaddress << ", door = " << door << ", street = " << street << ", town= " << town << ", state = " << state << ", country = " << country << ", postcode = " << postcode;
    emit markerGeocoded(uuid, collectionuuid, formattedaddress, door, street, town, state, country, postcode) ;
}




//
// Address search
//

// Search Functions
void GoogleMapsWidget::searchLocation(QString address)
{
    QString str = QString("searchLocation(\"%1\");").arg(address.replace(' ','+')) ;

    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::searchLocation() =>" ;
    qDebug() << str ;

    runJavaScript(str) ;
}

void GoogleMapsWidget::jsSearchResultsReady(QString placeid, double lat, double lon, QString address, QString phone)
{
    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::jsSearchResults() =>" ;
    qDebug() << "placeid: " << placeid ;
    qDebug() << "lat/lon: " << lat << ", " << lon;
    qDebug() << "address:" << address ;
    qDebug() << "phone:" << phone ;
    emit searchResultsReady(lat, lon, address, phone) ;
}

void GoogleMapsWidget::jsSearchFailed(QString error)
{
    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::jsSearchFailed() =>" ;
    qDebug() << "error: " << error ;
    emit searchFailed(error) ;
}

