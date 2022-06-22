/*
 * Application: POI Manager
 * File: mainwindow_mapcallback.cpp
 *
 * Main window / application GUI
 * Slots for responding to changes / updates on the map
 * (javascript).
 *
 */

/*
 *
 * POI Manager
 * Copyright (C) 2021  "Steve Clarke www.vizier.uk/poi"
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Any modification to the code must be contributed back to the community.
 *
 * Redistribution and use in binary or source form with or without modification
 * is permitted provided that the following conditions are met:
 *
 * Clause 7b - attribution for the original author shall be provided with the
 * inclusion of the above Copyright statement in its entirity, which must be
 * clearly visible in each application source file, in any documentation and also
 * in a pop-up window in the application itself. It is requested that the charity
 * donation link to Guide Dogs for the Blind remain within the program, and any
 * derivitive thereof.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */



//////////////////////////////////////////////////////////////////////////
//
// mainwindow_mapcallback.cpp
//
//

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

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
    ui->mapWebView->setMarker(thisUuid, thisCollectionUuid, lat, lon, formattedAddress, 9999, true);
    ui->mapWebView->geocodeMarker(thisUuid, thisCollectionUuid, true);

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
    ui->mapWebView->removeAllMarkers() ;
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
        PoiEntry newpos ;
        newpos.setLatLon(lat, lon) ;
        double distance = newpos.distanceFrom(pe) ;
        undo.pushPoiEntry(pe) ;
        pe.setLatLon(lat, lon) ;
        pe.set(PoiEntry::GEOCODED, "no") ;
        refresh(true) ;
        if (distance>1) {
          // Geocode if waypoint marker moved by more than 1m
          ui->mapWebView->geocodeMarker(uuid, collectionUuid, true);
        }
      }

    TrackEntry &te = findTrackEntryByUuid(uuid, collectionUuid) ;
    if (te.isValid()) {
        undo.pushTrackEntry(te);
        te.setLatLon(lat, lon) ;
        refreshTrackDetails() ;
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
                ui->mapWebView->setMarker(uuid, collectionUuid, lat, lon, QString(""), te.sequence()) ;
                te.setLatLon(lat, lon) ;
            }
        }
        refresh(true) ;
    }
}

void MainWindow::mapCallbackMarkerGeocoded(QString uuid, QString collectionUuid, QString formattedaddress, QString door, QString street, QString town, QString state, QString country, QString countrycode, QString postcode)
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
        pe.set(PoiEntry::GEOCOUNTRYCODE, countrycode) ;
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

        ui->mapWebView->selectMarker(thisUuid) ;
        // Refresh, don't update markers, don't centre, don't zoom
        refresh() ;

        if (pe.get(PoiEntry::GEOCODED).compare("yes")!=0) {
            ui->mapWebView->geocodeMarker(uuid, collectionUuid, true);
        }

    }

    TrackEntry& te = findTrackEntryByUuid(uuid, collectionUuid) ;
    if (te.isValid()) {
        thisUuid = uuid ;
        thisCollectionUuid = collectionUuid ;
        // Refresh, don't update markers, don't centre, don't zoom
        refresh() ;
    }

}


//
// callback - map has moved
//
void MainWindow::mapCallbackMapMoved(double lat, double lon, int zoom)
{
    ui->label_zoom->setText(QString::number(zoom)) ;
}
