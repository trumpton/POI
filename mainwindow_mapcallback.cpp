//////////////////////////////////////////////////////////////////////////
//
// mainwindow_mapcallback.cpp
//
//

#include "mainwindow.h"
#include "ui_mainwindow.h"

// Format for QString::arg(double)
#define dbl(n) arg(n,20,'g',15)


//////////////////////////////////////////////////////////////////////////
//
// Callbacks from Map
//

//
// callback - Address search results found
//
void MainWindow::mapCallbackSearchResultsReady(double lat, double lon, QString formattedAddress, QString phoneNumber)
{
    QString str = QString("mapCallbackSearchResultsReady(%1,%2,\"%3\",\"%4\")").dbl(lat).dbl(lon).arg(formattedAddress).arg(phoneNumber) ;
    qDebug() << "MainWindow::" << str ;

    PoiEntry newEntry ;

    newEntry.set(PoiEntry::EDITEDTITLE, searchtext) ;
    newEntry.set(PoiEntry::EDITEDDESCR, formattedAddress) ;
    newEntry.set(PoiEntry::EDITEDPHONE1, phoneNumber) ;
    newEntry.set(PoiEntry::GEOCODED, "no") ;
    newEntry.setLatLon(lat, lon) ;
    newEntry.setSequence(9999) ;

    thisUuid = newEntry.uuid() ;
    thisCollectionUuid = workingCollection.uuid() ;
    workingCollection.add(newEntry) ;
    ui->googlemapsWebView->setMarker(thisUuid, thisCollectionUuid, lat, lon, formattedAddress, 9999, true);
    ui->googlemapsWebView->geocodeMarker(thisUuid, thisCollectionUuid, true);

    refresh(false, true, PREFZOOM) ;

}

//
// callback - No place could be found
//
void MainWindow::mapCallbackSearchFailed(QString error)
{
    QString str = QString("mapCallbackSearchFailed(\"%1\")").arg(error) ;
    qDebug() << "MainWindow::" << str ;

    QMessageBox::warning(this, tr("Search Error"), error) ;
    ui->googlemapsWebView->removeAllMarkers() ;
    thisUuid = "" ;
    thisCollectionUuid = "" ;
    refresh() ;
}

//
// callback - selected and updated marker from map
//
void MainWindow::mapCallbackMarkerMoved(QString uuid, QString collectionUuid, double lat, double lon)
{
    QString str = QString("mapCallbackMarkerMoved(\"%1\",\"%2\",%3,%4)").arg(uuid).arg(collectionUuid).dbl(lat).dbl(lon) ;
    qDebug() << "MainWindow::" << str ;

    PoiEntry& pe = findEntryByUuid(uuid, collectionUuid) ;
    if (pe.isValid()) {
        pe.setLatLon(lat, lon) ;
        pe.set(PoiEntry::GEOCODED, "no") ;
        ui->googlemapsWebView->geocodeMarker(uuid, collectionUuid, true);
        refresh(true) ;
    }

    TrackEntry &te = findTrackEntryByUuid(uuid, collectionUuid) ;
    if (te.isValid()) {

        // Move Track Point
        te.setLatLon(lat, lon) ;

        if (ui->actionSnap->isChecked()) {
            // Snap to POI
            double smallestdistance = 100 ;
            for (int i=fileCollection.size()-1; i>=0; i--) {
                PoiEntry npe = fileCollection.at(i) ;
                double distance = te.distanceFrom(npe) ;
                if (distance<5 && distance<smallestdistance) {
                    smallestdistance = distance ;
                    lat=npe.lat() ;
                    lon=npe.lon() ;
                }
            }
            if (smallestdistance<100) {
                ui->googlemapsWebView->setMarker(uuid, collectionUuid, lat, lon, QString(""), te.sequence()) ;
                te.setLatLon(lat, lon) ;
            }
        }
        refresh(true) ;
    }
}

void MainWindow::mapCallbackMarkerGeocoded(QString uuid, QString collectionUuid, QString formattedaddress, QString door, QString street, QString town, QString state, QString country, QString postcode)
{
    QString str = QString("mapCallbackMarkerGeocoded(\"%1\",\"%2\",\"%3\",...)").arg(uuid).arg(collectionUuid).arg(formattedaddress) ;
    qDebug() << "MainWindow::" << str ;

    PoiEntry& pe = findEntryByUuid(uuid, collectionUuid) ;
    if (pe.isValid()) {
        // Update record
        pe.set(PoiEntry::AUTOCOMMENT, formattedaddress) ;
        pe.set(PoiEntry::GEODOOR, door) ;
        pe.set(PoiEntry::GEOSTREET, street) ;
        pe.set(PoiEntry::GEOCITY, town) ;
        pe.set(PoiEntry::GEOSTATE, state) ;
        pe.set(PoiEntry::GEOCOUNTRY, country) ;
        pe.set(PoiEntry::GEOPOSTCODE, postcode) ;
        pe.set(PoiEntry::GEOCODED, "yes") ;
        refresh() ;
    }
}


//
// callback - marker on map has been seelcted
//
void MainWindow::mapCallbackMarkerSelected(QString uuid, QString collectionUuid)
{
    QString str = QString("mapCallbackMarkerSelected(\"%1\",\"%2\")").arg(uuid).arg(collectionUuid) ;
    qDebug() << "MainWindow::" << str ;

    PoiEntry& pe = findEntryByUuid(uuid, collectionUuid) ;
    if (pe.isValid()) {
        thisUuid = uuid ;
        thisCollectionUuid = collectionUuid ;
        if (pe.get(PoiEntry::GEOCODED).compare("yes")!=0) {
            ui->googlemapsWebView->geocodeMarker(uuid, collectionUuid, true);
        }
        // Refresh but don't update markers.  Centre on selected item.  Don't zoom.
        refresh(false, true) ;
    }

    TrackEntry& te = findTrackEntryByUuid(uuid, collectionUuid) ;
    if (te.isValid()) {
        thisUuid = uuid ;
        thisCollectionUuid = collectionUuid ;
        // Refresh, don't update markers, don't centre, don't zoom
        refresh() ;
    }

}

