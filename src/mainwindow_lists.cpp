/*
 * Application: POI Manager
 * File: mainwindow_lists.cpp
 *
 * Main window / application GUI
 * Slots for managing the points of interest lists
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
// mainwindow_lists.cpp
//
//

#include "mainwindow.h"
#include "ui_mainwindow.h"



//////////////////////////////////////////////////////////////////////////
//
// Identify the UUID currentl selected in the specified list
//

QString MainWindow::currentSelectionUuid(QListWidget *widget)
{
    QList<QListWidgetItem *>items = widget->selectedItems() ;
    if (items.size()>0) {
        return items[0]->data(Qt::UserRole).toString() ;
    } else {
        return QString("") ;
    }
}


//////////////////////////////////////////////////////////////////////////
//
// File List Functions
//

// Handle single click on file item
void MainWindow::on_listFile_itemClicked(QListWidgetItem *item)
{
    thisUuid = item->data(Qt::UserRole).toString() ;
    thisCollectionUuid = fileCollection.uuid() ;

    PoiEntry& currentEntry = fileCollection.find(thisUuid) ;
    if (currentEntry.get(PoiEntry::GEOCODED).compare("yes")!=0) {
        ui->mapWebView->geocodeMarker(thisUuid, thisCollectionUuid, true);
    }

    ui->mapWebView->selectMarker(thisUuid) ;

    int zoom = ui->mapWebView->getZoom() ;

    if (ui->mapWebView->isVisible(currentEntry.lat(), currentEntry.lon()) && zoom>=PREFZOOM-3) {
        // Refresh, don't update markers, don't centre
        refresh(false, false) ;
    } else {
        // Refresh but don't update markers.  Centre on selected item & zoom if necessary
        if (zoom < PREFZOOM-3) zoom = PREFZOOM ;
        refresh(false, true, zoom) ;
    }

}


// Handle file duplicate
void MainWindow::on_btnCopyToClipboard_clicked()
{
    QString selection = currentSelectionUuid(ui->listFile) ;

    if (!selection.isEmpty()) {

        PoiEntry newEntry ;
        PoiEntry& currentEntry = fileCollection.find(selection) ;

        for (int i=0; i<PoiEntry::NUMFIELDTYPES; i++) {
            newEntry.set( (PoiEntry::FieldType)i, currentEntry.get( (PoiEntry::FieldType)i ) ) ;
        }
        newEntry.setLatLon(currentEntry.lat(), currentEntry.lon()) ;
        newEntry.setSequence(currentEntry.sequence()) ;

        thisUuid = newEntry.uuid() ;
        thisCollectionUuid = workingCollection.uuid() ;
        workingCollection.add(newEntry) ;
        ui->mapWebView->setMarker(thisUuid, thisCollectionUuid, newEntry.lat(), newEntry.lon(), newEntry.get(PoiEntry::EDITEDTITLE), newEntry.sequence());

        refresh() ;

    }
}


// Handle file edit
void MainWindow::on_btnEditFile_clicked()
{
    QString selection = currentSelectionUuid(ui->listFile) ;

    if (!selection.isEmpty()) {

        // Get the item details
        thisUuid = selection ;
        thisCollectionUuid = workingCollection.uuid() ;

        // Move the entry
        workingCollection.add(fileCollection.find(thisUuid)) ;
        fileCollection.remove(thisUuid) ;

        // Update the lists
        refresh(true) ;
    }
}





//////////////////////////////////////////////////////////////////////////
//
// Clipboard List Functions
//

// Handle single click on clipboard item
void MainWindow::on_listWorking_itemClicked(QListWidgetItem *item)
{
    thisUuid = item->data(Qt::UserRole).toString() ;
    thisCollectionUuid = workingCollection.uuid() ;

    PoiEntry& currentEntry = workingCollection.find(thisUuid) ;
    if (currentEntry.get(PoiEntry::GEOCODED).compare("yes")!=0) {
        ui->mapWebView->geocodeMarker(thisUuid, thisCollectionUuid, true);
    }

    ui->mapWebView->selectMarker(thisUuid) ;

    int zoom = ui->mapWebView->getZoom() ;

    if (ui->mapWebView->isVisible(currentEntry.lat(), currentEntry.lon()) && zoom>=PREFZOOM-3) {
        // Refresh, don't update markers, don't centre
        refresh(false, false) ;
    } else {
        // Refresh but don't update markers.  Centre on selected item & zoom if necessary
        if (zoom < PREFZOOM-3) zoom = PREFZOOM ;
        refresh(false, true, zoom) ;
    }

}


// Handle clipboard store button pressed
void MainWindow::on_btnStore_clicked()
{
    QString selection = currentSelectionUuid(ui->listWorking) ;
    if (!selection.isEmpty()) {

        // Get the item details
        thisUuid = selection ;
        thisCollectionUuid = fileCollection.uuid() ;

        // Move the entry
        fileCollection.add(workingCollection.find(thisUuid)) ;
        workingCollection.remove(thisUuid) ;
        fileCollection.sortBySequence(); ;

        // Modify the google map record
        ui->mapWebView->setMarkerCollection(thisUuid, thisCollectionUuid) ;

        // TODO: Set MarkerCollection Sequence

        refresh(true) ;

    }
}

// Handle clipboard duplicate button pressed
void MainWindow::on_btnDuplicate_clicked()
{
    QString selection = currentSelectionUuid(ui->listWorking) ;

    if (!selection.isEmpty()) {

        PoiEntry newEntry ;
        PoiEntry& currentEntry = workingCollection.find(selection) ;

        newEntry.copyFrom(currentEntry) ;

        thisUuid = newEntry.uuid() ;
        thisCollectionUuid = workingCollection.uuid() ;
        workingCollection.add(newEntry) ;
        ui->mapWebView->setMarker(thisUuid, thisCollectionUuid, newEntry.lat(), newEntry.lon(), newEntry.get(PoiEntry::EDITEDTITLE), newEntry.sequence(), true);

        refresh(true) ;

    }

}

// Handle clipboard delete button pressed
void MainWindow::on_btnDelete_clicked()
{
    QString selection = currentSelectionUuid(ui->listWorking) ;

    if (!selection.isEmpty()) {

        workingCollection.remove(selection) ;
        ui->mapWebView->removeMarker(selection) ;

        refresh(true) ;

    }
}

// Handle clipboard new button pressed
void MainWindow::on_btnNew_clicked()
{
    double lat=ui->mapWebView->getLat() ;
    double lon=ui->mapWebView->getLon() ;
    PoiEntry newEntry ;

    qDebug() << "" ;
    qDebug() << "on_btnNew_clicked() ==>" ;
    qDebug() << "lat: " << lat <<", lon:" << lon  ;

    newEntry.set(PoiEntry::EDITEDTITLE, "New Entry") ;
    newEntry.setLatLon(lat, lon) ;
    newEntry.setSequence(9999) ;

    thisUuid = newEntry.uuid() ;
    thisCollectionUuid = workingCollection.uuid() ;
    workingCollection.add(newEntry) ;
    ui->mapWebView->setMarker(thisUuid, thisCollectionUuid, lat, lon, "New Entry", 9999, true);
    ui->mapWebView->geocodeMarker(thisUuid, thisCollectionUuid, true);

    refresh(true, true, PREFZOOM) ;
}

