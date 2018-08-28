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
        ui->googlemapsWebView->geocodeMarker(thisUuid, thisCollectionUuid, true);
    }
    ui->googlemapsWebView->selectMarker(thisUuid) ;
    refresh(false, true, PREFZOOM) ;
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
        ui->googlemapsWebView->setMarker(thisUuid, thisCollectionUuid, newEntry.lat(), newEntry.lon(), newEntry.get(PoiEntry::EDITEDTITLE), newEntry.sequence());

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
        ui->googlemapsWebView->geocodeMarker(thisUuid, thisCollectionUuid, true);
    }

    refresh(false, true, PREFZOOM) ;
    ui->googlemapsWebView->selectMarker(thisUuid) ;
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
        ui->googlemapsWebView->setMarkerCollection(thisUuid, thisCollectionUuid) ;

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
        ui->googlemapsWebView->setMarker(thisUuid, thisCollectionUuid, newEntry.lat(), newEntry.lon(), newEntry.get(PoiEntry::EDITEDTITLE), newEntry.sequence(), true);

        refresh(true) ;

    }

}

// Handle clipboard delete button pressed
void MainWindow::on_btnDelete_clicked()
{
    QString selection = currentSelectionUuid(ui->listWorking) ;

    if (!selection.isEmpty()) {

        workingCollection.remove(selection) ;
        ui->googlemapsWebView->removeMarker(selection) ;

        refresh(true) ;

    }
}

// Handle clipboard new button pressed
void MainWindow::on_btnNew_clicked()
{
    double lat=ui->googlemapsWebView->getLat() ;
    double lon=ui->googlemapsWebView->getLon() ;
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
    ui->googlemapsWebView->setMarker(thisUuid, thisCollectionUuid, lat, lon, "New Entry", 9999, true);
    ui->googlemapsWebView->geocodeMarker(thisUuid, thisCollectionUuid, true);

    refresh(true, true, PREFZOOM) ;
}

