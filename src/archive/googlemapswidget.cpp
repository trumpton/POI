#include <QDebug>
#include <QFile>
#include <QMessageBox>

#include "googlemapswidget.h"

// Format for QString::arg(double)
#define dbl(n) arg(n,20,'g',15)


MapsWidget::MapsWidget(QWidget *parent) :
   QWebEngineView(parent)
{

    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(initialise2(bool))) ;
    connect(this, SIGNAL(authenticationRequired(const QUrl &, QAuthenticator*)),
            SLOT(authenticationRequired(const QUrl &, QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(const QUrl &, QAuthenticator *, const QString &)),
            SLOT(proxyAuthenticationRequired(const QUrl &, QAuthenticator *, const QString &)));
    dLat=0 ; dLon=0 ; iZoom=0 ;
}

MapsWidget::~MapsWidget()
{
}

void MapsWidget::initialise(QString key, QString initialWorkingUuid, QString initialFileUuid, QString initialTrackUuid)
{

    cachedWorkingCollectionUuid = initialWorkingUuid ;
    cachedFileCollectionUuid = initialFileUuid ;
    cachedTrackCollectionUuid = initialTrackUuid ;

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


void MapsWidget::initialise2(bool ok)
{
    // If initialise hasn't been called, ignore the signal
    if (cachedWorkingCollectionUuid.isEmpty()) return ;

    qDebug() << "" ;
    qDebug() << "MapsWidget::loadFinished() =>" ;
    qDebug() << "bool: " << (ok?"true":"false") ;
    if (ok) {

        this->page()->setWebChannel(&channel) ;
        channel.registerObject("MapsWidget", this) ;
        // Finally, initialise the javascript (connect to channel & register workingcollectionuuid)
        initialiseJavascript() ;
        registerUuids(cachedWorkingCollectionUuid, cachedFileCollectionUuid, cachedTrackCollectionUuid);

    } else {
        QMessageBox::information(NULL, "Error", "Error, unable to load Google Maps") ;
    }
}

void MapsWidget::clearCookies()
{
}

//
// Authentication
//
void MapsWidget::authenticationRequired(const QUrl &requestUrl, QAuthenticator *auth)
{
    Q_UNUSED(requestUrl) ;
    Q_UNUSED(auth) ;
    return ;
}

void MapsWidget::proxyAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth, const QString &proxyHost)
{
    Q_UNUSED(requestUrl);
    Q_UNUSED(auth) ;
    Q_UNUSED(proxyHost) ;
    return ;
}

//
// Map Status
//

double MapsWidget::getLat()
{
    return dLat ;
}

double MapsWidget::getLon()
{
    return dLon ;
}

int MapsWidget::getZoom()
{
    return iZoom ;
}

//
// Javascript access functions
//

void MapsWidget::runJavaScript(QString command)
{
    page()->runJavaScript(command) ;
}


//
// Collection identification
//
void MapsWidget::registerUuids(QString workingUuid, QString fileUuid, QString trackUuid)
{
    cachedWorkingCollectionUuid = workingUuid ;
    cachedFileCollectionUuid = fileUuid ;
    cachedTrackCollectionUuid = trackUuid ;
    QString str = QString("registerUuids(\"%1\", \"%2\", \"%3\");").arg(workingUuid).arg(fileUuid).arg(trackUuid) ;
    qDebug() << "MapsWidget::" << str ;
    runJavaScript(str) ;
}


void MapsWidget::initialiseJavascript()
{
    QString str = QString("initialise();") ;
    qDebug() << "MapsWidget::" << str ;
    runJavaScript(str) ;
}


//
// Javascript navigaion functions
//

void MapsWidget::showTracks(bool enabled)
{
    QString str = QString("showTracks(%1);").arg(enabled?"true":"false") ;
    qDebug() << "MapsWidget::" << str ;
    runJavaScript(str) ;
}

void MapsWidget::gotoCoordinates(double lat, double lon, int zoom)
{

    QString str = QString("gotoCoordinates(%1, %2, %3);").dbl(lat).dbl(lon).arg(zoom) ;
    qDebug() << "MapsWidget::" << str ;
    dLat = lat ; dLon = lon ; iZoom = zoom ;
    runJavaScript(str) ;
}

//
// Javascript marker set / management functions
//

void MapsWidget::setMarker(QString uuid, QString collectionuuid, double east, double north, QString address, int sequence, bool drop)
{
    removeMarker(uuid) ;
    QString str = QString("setMarker(\"%1\", \"%2\", %3, %4, \"%5\", %6, %7);").
            arg(uuid).arg(collectionuuid).dbl(east).dbl(north).
            arg(address).arg(sequence).arg(drop?"true":"false");
    qDebug() << "MapsWidget::" << str ;

    runJavaScript(str) ;
}

void MapsWidget::setMarkerCollection(QString uuid, QString collectionuuid)
{
    QString str =
      QString("setMarkerCollection(\"%1\", \"%2\");").
            arg(uuid).
            arg(collectionuuid) ;
    qDebug() << "MapsWidget::" << str ;
    runJavaScript(str) ;
}

void MapsWidget::selectMarker(QString uuid)
{

    QString str = QString("selectMarker(\"%1\");").arg(uuid) ;
    qDebug() << "MapsWidget::" << str ;
    runJavaScript(str) ;
}


void MapsWidget::seekToMarker(QString uuid, int zoom)
{

    QString str = QString("seekToMarker(\"%1\",%2);").arg(uuid).arg(zoom) ;
    qDebug() << "MapsWidget::" << str ;
    runJavaScript(str) ;
}

void MapsWidget::removeMarker(QString uuid)
{

    QString str = QString("removeMarker(\"%1\");").arg(uuid) ;
    qDebug() << "MapsWidget::" << str ;
    runJavaScript(str) ;
}

void MapsWidget::removeAllMarkers()
{

    QString str = QString("removeAllMarkers();") ;
    qDebug() << "MapsWidget::" << str ;
    runJavaScript(str) ;
}


//
// Javascript signal handlers
//
void MapsWidget::jsmapMoved(double lat, double lon, int zoom)
{
    QString str = QString("jsMapMoved(%1,%2,%3").dbl(lat).dbl(lon).arg(zoom) ;
    qDebug() << "MapsWidget::" << str ;
    dLat = lat ; dLon = lon ; iZoom = zoom ;
    emit mapMoved(dLat, dLon, iZoom) ;
}

void MapsWidget::jsmarkerMoved(QString uuid, QString collectionuuid, double lat, double lon)
{
    QString str = QString("jsmarkerMoved(\"%1\",\"%2\",%3,%4);").arg(uuid).arg(collectionuuid).dbl(lat).dbl(lon) ;
    qDebug() << "MapsWidget::" << str ;
    emit markerMoved(uuid, collectionuuid, lat, lon) ;
}


void MapsWidget::jsmarkerSelected(QString uuid, QString collectionuuid)
{
    QString str = QString("jsmarkerSelected(\"%1\",\"%2\"").arg(uuid).arg(collectionuuid) ;
    qDebug() << "MapsWidget::" << str ;
    emit markerSelected(uuid, collectionuuid) ;
}


//
// Geocoding
//

void MapsWidget::geocodeMarker(QString uuid, QString collectionuuid, bool forcegeocode)
{

    QString str = QString("geocodeMarker(\"%1\",\"%2\",%3);").arg(uuid).arg(collectionuuid).arg((forcegeocode?"true":"false")) ;
    qDebug() << "MapsWidget::" << str ;
    runJavaScript(str) ;
}

void MapsWidget::jsmarkerGeocoded(QString uuid, QString collectionuuid, QString formattedaddress, QString door, QString street, QString town, QString state, QString country, QString postcode)
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
    qDebug() << "MapsWidget" << str ;
    emit markerGeocoded(uuid, collectionuuid, formattedaddress, door, street, town, state, country, postcode) ;
}




//
// Address search
//

// Search Functions
void MapsWidget::searchLocation(QString address)
{
    QString str = QString("searchLocation(\"%1\");").arg(address.replace(' ','+')) ;
    qDebug() << "MapsWidget::" << str ;
    runJavaScript(str) ;
}

void MapsWidget::jsSearchResultsReady(QString placeid, double lat, double lon, QString address, QString phone)
{
    QString str = QString("jsSearchResultsReady(\"%1\",%2,%3,\"%4\",\"%5\")").arg(placeid).dbl(lat).dbl(lon).arg(address).arg(phone) ;
    qDebug() << "MapsWidget::" << str ;
    emit searchResultsReady(lat, lon, address, phone) ;
}

void MapsWidget::jsSearchFailed(QString error)
{
    QString str = QString("jsSearchFailed(\"%1\")").arg(error) ;
    qDebug() << "MapsWidget::" << str ;
    emit searchFailed(error) ;
}

//
// Debug
//
void MapsWidget::jsDebug(QString message)
{
    qDebug() << QString("JS: ") << message ;

}
