#include <QDebug>
#include <QFile>
#include <QMessageBox>

#include "mapswidget.h"

// Format for QString::arg(double)
#define dbl(n) arg(n,20,'g',15)


MapsWidget::MapsWidget(QWidget *parent) :
   QWebEngineView(parent)
{
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(initialise2(bool))) ;
    dLat=0 ; dLon=0 ; iZoom=0 ;
}

MapsWidget::~MapsWidget()
{
}

//
// Javascript access functions
//

void MapsWidget::runJavaScript(QString command)
{
    page()->runJavaScript(command) ;
}


// Initialise1: Set the webpage Loading
void MapsWidget::initialise(QString url,
                            QString bingKeyData,
                            QString geocodeType, QString hereId, QString hereCode,
                            int aerialTileZoom, QString aerialTileUrl,
                            int satelliteOverlayTileZoom, QString satelliteOverlayTileUrl,
                            int mapTileZoom, QString mapTileUrl,
                            int contourTileZoom, QString contourTileUrl,
                            int trailTileZoom, QString trailTileUrl,
                            QString workingUuid, QString fileUuid, QString trackUuid)
{

    // Save the keys & Geocode Option
    cachedGeocodeType = geocodeType ;
    cachedBingKey = bingKeyData ;
    cachedHereId = hereId ;
    cachedHereCode = hereCode ;

    // Save the map tile parameters
    cachedAerialTileZoom = aerialTileZoom ;
    cachedAerialTileUrl = aerialTileUrl ;
    cachedSatelliteOverlayTileZoom = satelliteOverlayTileZoom ;
    cachedSatelliteOverlayTileUrl = satelliteOverlayTileUrl ;
    cachedMapTileZoom = mapTileZoom ;
    cachedMapTileUrl = mapTileUrl ;
    cachedContourTileZoom = contourTileZoom ;
    cachedContourTileUrl = contourTileUrl ;
    cachedTrailTileZoom = trailTileZoom ;
    cachedTrailTileUrl = trailTileUrl ;

    // Save the uuids for later
    cachedWorkingCollectionUuid = workingUuid;
    cachedFileCollectionUuid = fileUuid;
    cachedTrackCollectionUuid = trackUuid;

    // Load the html for the page
    QUrl qu = QUrl(url) ;
    QFile f(url.replace("qrc:///",":/")) ;
    f.open(QFile::ReadOnly|QFile::Text) ;
    QTextStream in(&f) ;
    QString html = in.readAll() ;
    f.close() ;
    setHtml(html, url) ;
}

// Initialise Called Following Page Load (first on load finished, and second on webchannel setup)
void MapsWidget::initialise2(bool ok)
{
    if (cachedWorkingCollectionUuid.isEmpty()) return ;

    // In QT 5.95, this function got called twice, so the registration is checked
    // so that it is only performed once - the javascript call will fail on the first
    // pass.
    // In 5.15, this function gets called once, and both the registration and the
    // initial javascript run are performed in the same pass.

    if (ok) {

        if (!this->page()->webChannel()) {
            // On First Load-Finished, set up the webchannel
            this->page()->setWebChannel(&channel) ;
            channel.registerObject("MapsWidget", this) ;
            QHash<QString, QObject *> o = channel.registeredObjects() ;
            qDebug() << "MapsWidget: Registering Object" ;
            qDebug() << "MapsWidget::loadFinished(true) - Initial Load" ;

        }

        // initialise the javascript (connect back to channel & register workingcollectionuuid)
        runJavaScript("initialise3();") ;
        qDebug() << "MapsWidget::loadFinished(true) - Channel Configuration Complete" ;

    } else {
        QMessageBox::information(NULL, "Error", "Error, unable to load Maps") ;
        qDebug() << "MapsWidget::loadFinished(false)" ;
    }
}


// Initialise4 Callback When Channel setup in javascript is complete
void MapsWidget::jsinitialise4(bool ok)
{
  Q_UNUSED(ok);
  QString cmd = "initialiseMap(\"" + cachedBingKey + "\", " +
                QString::number(cachedAerialTileZoom) + ", \"" + cachedAerialTileUrl + "\", " +
                QString::number(cachedSatelliteOverlayTileZoom) + ", \"" + cachedSatelliteOverlayTileUrl + "\", " +
                QString::number(cachedMapTileZoom) + ", \"" + cachedMapTileUrl + "\", " +
                QString::number(cachedContourTileZoom) + ", \"" + cachedContourTileUrl + "\", " +
                QString::number(cachedTrailTileZoom) + ", \"" + cachedTrailTileUrl + "\");" ;
  runJavaScript(cmd);

  cmd = "initialiseGeocoder(\"" + cachedGeocodeType + "\", \"" + cachedHereId + "\", \"" + cachedHereCode + "\");" ;
  runJavaScript(cmd);

  registerUuids(cachedWorkingCollectionUuid, cachedFileCollectionUuid, cachedTrackCollectionUuid);

}


void MapsWidget::clearCookies()
{
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

bool MapsWidget::isVisible(double west, double north)
{
    double spanwe = dWest - dEast ;   // TODO: will have problems at -180 degrees
    double spanns = dNorth - dSouth ; // TODO: will be negative for southern hemisphere
    return ( west > dWest + spanwe * 0.1 ) &&
            (west < dWest + spanwe * 0.9 ) &&
            (north > dSouth + spanns * 0.1 ) &&
            (north < dSouth + spanns * 0.9) ;
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


//
// Javascript navigaion functions
//
void MapsWidget::showMaps(bool base, bool aerial, bool topo, bool tracks, bool satellite)
{
    runJavaScript(QString("showMaps(%1,%2,%3,%4,%5)")
                  .arg(base?"true":"false")
                  .arg(aerial?"true":"false")
                  .arg(topo?"true":"false")
                  .arg(tracks?"true":"false")
                  .arg(satellite?"true":"false")) ;
}

void MapsWidget::showTracks(bool enabled)
{
    runJavaScript(QString("drawLines(%1);").arg(enabled?"true":"false")) ;
}

void MapsWidget::gotoCoordinates(double west, double lon, int zoom)
{
    dLat = west ; dLon = lon ; iZoom = zoom ;
    runJavaScript(QString("gotoCoordinates(%1, %2, %3);").dbl(west).dbl(lon).arg(zoom)) ;
}

//
// Javascript marker set / management functions
//

void MapsWidget::setMarker(QString uuid, QString collectionuuid, double east, double north, QString address, int sequence, bool drop)
{
    removeMarker(uuid) ;
    runJavaScript(QString("setMarker(\"%1\", \"%2\", %3, %4, \"%5\", %6, %7);").
                  arg(uuid).arg(collectionuuid).dbl(east).dbl(north).
                  arg(address).arg(sequence).arg(drop?"true":"false")) ;
}

void MapsWidget::setMarkerCollection(QString uuid, QString collectionuuid)
{
    runJavaScript(QString("setMarkerCollection(\"%1\", \"%2\");").
                  arg(uuid).
                  arg(collectionuuid)) ;
}

void MapsWidget::selectMarker(QString uuid)
{
    runJavaScript(QString("selectMarker(\"%1\");").arg(uuid)) ;
}


void MapsWidget::seekToMarker(QString uuid, int zoom)
{
    runJavaScript(QString("seekToMarker(\"%1\",%2);").arg(uuid).arg(zoom)) ;
}

void MapsWidget::removeMarker(QString uuid)
{
    runJavaScript(QString("removeMarker(\"%1\");").arg(uuid)) ;
}

void MapsWidget::removeAllMarkers()
{
    runJavaScript(QString("removeAllMarkers();")) ;
}

//
// Javascript signal handlers
//
void MapsWidget::jsmapMoved(double lat, double lon, int zoom, double tllat, double tllon, double brlat, double brlon)
{
    QString str = QString("jsMapMoved( %1, %2, %3, %4, %5, %6, %7)").arg(lat).arg(lon).arg(zoom).arg(tllat).arg(tllon).arg(brlat).arg(brlon) ;
    qDebug() << "MapsWidget::" << str ;
    dLat = lat ; dLon = lon ; iZoom = zoom ;
    dNorth = tllat ; dWest = tllon ; dSouth = brlat ; dEast = brlon ;
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

void MapsWidget::jsGeocodeFailed(QString error)
{
    QString str = QString("jsGeocodeFailed(\"%1\")").arg(error) ;
    qDebug() << "MapsWidget::" << str ;
    emit geocodeFailed(error) ;
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
