#include <QDebug>
#include <QFile>
#include <QMessageBox>

#include "googlemapswidget.h"

// Format for QString::arg(double)
#define dbl(n) arg(n,20,'g',15)


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

void GoogleMapsWidget::initialise(QString key, QString workingUuid, QString trackUuid)
{
    // Save uuid
    cachedWorkingCollectionUuid = workingUuid ;
    cachedTrackCollectionUuid = trackUuid ;

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
        initialiseJavascript(cachedWorkingCollectionUuid, cachedTrackCollectionUuid) ;


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
//#ifdef WEBENGINE
    page()->runJavaScript(command) ;
//#else
//    page()->mainFrame()->evaluateJavaScript(command) ;
//#endif
}


//
// Collection identification
//

void GoogleMapsWidget::initialiseJavascript(QString workingUuid, QString trackUuid)
{
    QString str = QString("initialise(\"%1\", \"%2\");").arg(workingUuid).arg(trackUuid) ;

    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::initialiseJavascript() =>" ;
    qDebug() << str ;

    runJavaScript(str) ;
}


//
// Javascript navigaion functions
//

void GoogleMapsWidget::showTracks(bool enabled)
{
    QString str = QString("showTracks(%1);").arg(enabled?"true":"false") ;

    qDebug() << "" ;
    qDebug() << "GoogleMapsWidget::showTracks() =>" ;
    qDebug() << str ;
    runJavaScript(str) ;
}

void GoogleMapsWidget::gotoCoordinates(double lat, double lon, int zoom)
{

    QString str = QString("gotoCoordinates(%1, %2, %3);").dbl(lat).dbl(lon).arg(zoom) ;
    qDebug() << "GoogleMapsWidget::" << str ;
    dLat = lat ; dLon = lon ; iZoom = zoom ;
    runJavaScript(str) ;
}

//
// Javascript marker set / management functions
//

void GoogleMapsWidget::setMarker(QString uuid, QString collectionuuid, double east, double north, QString address, int sequence, bool drop)
{
    removeMarker(uuid) ;
    QString str = QString("setMarker(\"%1\", \"%2\", %3, %4, \"%5\", %6, %7);").
            arg(uuid).arg(collectionuuid).dbl(east).dbl(north).
            arg(address).arg(sequence).arg(drop?"true":"false");
    qDebug() << "GoogleMapsWidget::" << str ;

    runJavaScript(str) ;
}

void GoogleMapsWidget::setMarkerCollection(QString uuid, QString collectionuuid)
{
    QString str =
      QString("setMarkerCollection(\"%1\", \"%2\");").
            arg(uuid).
            arg(collectionuuid) ;
    qDebug() << "GoogleMapsWidget::" << str ;
    runJavaScript(str) ;
}

void GoogleMapsWidget::selectMarker(QString uuid)
{

    QString str = QString("selectMarker(\"%1\");").arg(uuid) ;
    qDebug() << "GoogleMapsWidget::" << str ;
    runJavaScript(str) ;
}


void GoogleMapsWidget::seekToMarker(QString uuid, int zoom)
{

    QString str = QString("seekToMarker(\"%1\",%2);").arg(uuid).arg(zoom) ;
    qDebug() << "GoogleMapsWidget::" << str ;
    runJavaScript(str) ;
}

void GoogleMapsWidget::removeMarker(QString uuid)
{

    QString str = QString("removeMarker(\"%1\");").arg(uuid) ;
    qDebug() << "GoogleMapsWidget::" << str ;
    runJavaScript(str) ;
}

void GoogleMapsWidget::removeAllMarkers()
{

    QString str = QString("removeAllMarkers();") ;
    qDebug() << "GoogleMapsWidget::" << str ;
    runJavaScript(str) ;
}


//
// Javascript signal handlers
//
void GoogleMapsWidget::jsmapMoved(double lat, double lon, int zoom)
{
    QString str = QString("jsMapMoved(%1,%2,%3").dbl(lat).dbl(lon).arg(zoom) ;
    qDebug() << "GoogleMapsWidget::" << str ;
    dLat = lat ; dLon = lon ; iZoom = zoom ;
    emit mapMoved(dLat, dLon, iZoom) ;
}

void GoogleMapsWidget::jsmarkerMoved(QString uuid, QString collectionuuid, double lat, double lon)
{
    QString str = QString("jsmarkerMoved(\"%1\",\"%2\",%3,%4);").arg(uuid).arg(collectionuuid).dbl(lat).dbl(lon) ;
    qDebug() << "GoogleMapsWidget::" << str ;
    emit markerMoved(uuid, collectionuuid, lat, lon) ;
}


void GoogleMapsWidget::jsmarkerSelected(QString uuid, QString collectionuuid)
{
    QString str = QString("jsmarkerSelected(\"%1\",\"%2\"").arg(uuid).arg(collectionuuid) ;
    qDebug() << "GoogleMapsWidget::" << str ;
    emit markerSelected(uuid, collectionuuid) ;
}


//
// Geocoding
//

void GoogleMapsWidget::geocodeMarker(QString uuid, QString collectionuuid, bool forcegeocode)
{

    QString str = QString("geocodeMarker(\"%1\",\"%2\",%3);").arg(uuid).arg(collectionuuid).arg((forcegeocode?"true":"false")) ;
    qDebug() << "GoogleMapsWidget::" << str ;
    runJavaScript(str) ;
}

void GoogleMapsWidget::jsmarkerGeocoded(QString uuid, QString collectionuuid, QString formattedaddress, QString door, QString street, QString town, QString state, QString country, QString postcode)
{
    QString str = QString("jsmarkerGeocoded(\"%1\",\"%2\",\"%3\",\"%4\",\"%5\",\"%6\",\"%7\",\"%8\",\"%9\")")
            .arg(uuid)
            .arg(collectionuuid)
            .arg(formattedaddress)
            .arg(door)
            .arg(street)
            .arg(town)
            .arg(state)
            .arg(country)
            .arg(postcode) ;
    qDebug() << "GoogleMapsWidget" << str ;
    emit markerGeocoded(uuid, collectionuuid, formattedaddress, door, street, town, state, country, postcode) ;
}




//
// Address search
//

// Search Functions
void GoogleMapsWidget::searchLocation(QString address)
{
    QString str = QString("searchLocation(\"%1\");").arg(address.replace(' ','+')) ;
    qDebug() << "GoogleMapsWidget::" << str ;
    runJavaScript(str) ;
}

void GoogleMapsWidget::jsSearchResultsReady(QString placeid, double lat, double lon, QString address, QString phone)
{
    QString str = QString("jsSearchResultsReady(\"%1\",%2,%3,\"%4\",\"%5\")").arg(placeid).dbl(lat).dbl(lon).arg(address).arg(phone) ;
    qDebug() << "GoogleMapsWidget::" << str ;
    emit searchResultsReady(lat, lon, address, phone) ;
}

void GoogleMapsWidget::jsSearchFailed(QString error)
{
    QString str = QString("jsSearchFailed(\"%1\")").arg(error) ;
    qDebug() << "GoogleMapsWidget::" << str ;
    emit searchFailed(error) ;
}

//
// Debug
//
void GoogleMapsWidget::jsDebug(QString message)
{
    qDebug() << QString("JS: ") << message ;

}
