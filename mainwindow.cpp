#include "mainwindow.h"
#include "ui_mainwindow.h"

//
// TODO: When using the duplicate on the file list, the red working copy can be behind.
//       Selected item shouldalways be on the top.  Fix this in javascript 'selectmarker'
//       Hopefully there's a bring to front, otherwise, it's remember where you were, and
//       draw the selected one last.
//

//
// TODO: Search
//
// GoogleMapsWidget::searchLocation() =>
// "searchLocation(\"cambo+les+bains\");"
// Could not convert argument QJsonValue(null) to target type "double" .
// Could not convert argument QJsonValue(null) to target type "double" .
//

//
// TODO: +
//
// GoogleMapsWidget::searchLatLon() =>
// "searchLatLon(48.13,11.57);"
// Could not convert argument QJsonValue(null) to target type "double" .
// Could not convert argument QJsonValue(null) to target type "double" .
//


#include <QString>
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QRegExp>

#define PREFZOOM 17
#define FALLBACKAPIKEY "AIzaSyDCfLGkiprk5pZUEUSyEv2K8mCGqrw5wd0"

// TODO:
//
//  Have a single function to sync map / ui and list selections
//  call it when list box entries selected, or map icon clicked
//

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Current Marker
    thisUuid="" ;

    // Configuration and Configuration form
    configuration = new Configuration(this) ;

    // Configure and Load Web Pages
    QString key = configuration->key() ;
    if (key.isEmpty()) key=QString(FALLBACKAPIKEY) ;
    ui->googlemapsWebView->initialise(key, workingCollection.uuid()) ;
    ui->tomtomWebView->initialise() ;

    // TODO: Shouldn't need these
    ui->googlemapsWebView->clearCookies() ;
    ui->tomtomWebView->clearCookies() ;

    // Update markers when selected or moved on the map
    connect(ui->googlemapsWebView, SIGNAL(markerSelected(QString, QString)), this, SLOT(mapCallbackMarkerSelected(QString, QString))) ;
    connect(ui->googlemapsWebView, SIGNAL(markerMoved(QString, QString, double, double)), this, SLOT(mapCallbackMarkerMoved(QString, QString, double, double))) ;
    connect(ui->googlemapsWebView, SIGNAL(markerGeocoded(QString, QString, QString)), this, SLOT(mapCallbackMarkerGeocoded(QString, QString, QString))) ;

    // Handle search results / failure
    connect(ui->googlemapsWebView, SIGNAL(searchResultsReady(QString, double, double, QString, QString)), this, SLOT(mapCallbackSearchResultsReady(QString, double, double, QString, QString))) ;
    connect(ui->googlemapsWebView, SIGNAL(searchFailed(QString)), this, SLOT(mapCallbackSearchFailed(QString))) ;

    // Handle imports from TomTom
    connect(ui->tomtomWebView, SIGNAL(exportPoi(QString, double, double)), this, SLOT(importPoi(QString, double, double))) ;

    // POI Files
    loadFiles() ;
}

MainWindow::~MainWindow()
{
    saveCollection() ;
    delete configuration ;
    delete ui;
}

void MainWindow::saveCollection(bool autoyes)
{
    if (fileCollection.isDirty()) {
        QMessageBox::StandardButton reply ;
        if (autoyes) {
            reply = QMessageBox::Yes ;
        } else {
            reply = QMessageBox::question(this,
                "POI Save", "File as been edited, do you want to save?",
                 QMessageBox::Yes|QMessageBox::No);
        }
        if (reply == QMessageBox::Yes) fileCollection.save() ;
    }
}

void MainWindow::setEnables(int status)
{
    switch (status) {
    case NOTHINGSELECTED:
        ui->btnDelete->setEnabled(false) ;
        ui->btnDuplicate->setEnabled(false) ;
        ui->btnStore->setEnabled(false) ;
        ui->btnRemove->setEnabled(false) ;
        ui->btnDuplicateFile->setEnabled(false) ;
        ui->editDescription->setEnabled(false) ;
        ui->editDoorNumber->setEnabled(false) ;
        ui->editPhone->setEnabled(false) ;
        break ;
    case WORKINGSELECTED:
        ui->btnDelete->setEnabled(true) ;
        ui->btnDuplicate->setEnabled(true) ;
        ui->btnStore->setEnabled(true) ;
        ui->btnRemove->setEnabled(false) ;
        ui->btnDuplicateFile->setEnabled(false) ;
        ui->editDescription->setEnabled(true) ;
        ui->editDoorNumber->setEnabled(true) ;
        ui->editPhone->setEnabled(true) ;
        break ;
    case FILESELECTED:
        ui->btnDelete->setEnabled(false) ;
        ui->btnDuplicate->setEnabled(false) ;
        ui->btnStore->setEnabled(false) ;
        ui->btnRemove->setEnabled(true) ;
        ui->btnDuplicateFile->setEnabled(true) ;
        ui->editDescription->setEnabled(false) ;
        ui->editDoorNumber->setEnabled(false) ;
        ui->editPhone->setEnabled(false) ;
        break ;
    }
}

//////////////////////////////////////////////////////////////////////////
//
// Search for New Locations on Map
//

// Search for a new location
void MainWindow::on_btnFind_pressed()
{
    ui->googlemapsWebView->searchLocation(ui->editSearch->text()) ;
}

void MainWindow::on_editSearch_returnPressed()
{
    on_btnFind_pressed() ;
}

//////////////////////////////////////////////////////////////////////////
//
// Manage / sync data between edit fields and lists
//

bool MainWindow::updateMapSelection(int zoom)
{
    ui->googlemapsWebView->selectMarker(thisUuid) ;
    if (!thisUuid.isEmpty()) ui->googlemapsWebView->seekToMarker(thisUuid, zoom) ;
    return true ;
}

bool MainWindow::updateForm()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    ui->editDescription->setText(pe.description()) ;
    ui->viewAddress->setText(pe.address()) ;
    ui->editPhone->setText(pe.phone()) ;
    ui->viewLatLon->setText(QString::number(pe.lat()) + QString(", ") + QString::number(pe.lon())) ;
    ui->editDoorNumber->setText(pe.door()) ;
    if (fileCollection.getSequenceText().isEmpty()) {
        ui->labelVersion->setText("") ;
    } else {
        ui->labelVersion->setText(QString("v") + fileCollection.getSequenceText()) ;
    }
    return true ;
}

bool MainWindow::updateLists()
{
    bool resp=false ;
    if (updateList(&workingCollection, ui->listWorking)) { resp=true ; }
    if (updateList(&fileCollection, ui->listFile)) { resp=true ; }
    return resp ;
}

bool MainWindow::updateList(PoiCollection *collection, QListWidget *widget)
{
    if (collection==NULL || widget==NULL) return false ;
    widget->clear() ;

    // Populate List
    int size = collection->size() ;
    for (int i=0; i<size; i++) {
        QListWidgetItem *item = new QListWidgetItem(collection->at(i).description()) ;
        QVariant variant(collection->at(i).uuid()) ;
        item->setData(Qt::UserRole, variant) ;
        if (collection->at(i).isDirty()) {
            item->setForeground(Qt::red);
        }
        widget->addItem(item) ;
    }
    return (size>0) ;
}

bool MainWindow::updateListsSelection(bool refreshMarkers)
{
    bool resp=false ;
    if (refreshMarkers) ui->googlemapsWebView->removeAllMarkers() ;

    if (updateListSelection(&workingCollection, ui->listWorking, refreshMarkers)) {
        setEnables(WORKINGSELECTED) ;
        resp=true ;
    }
    if (updateListSelection(&fileCollection, ui->listFile, refreshMarkers)) {
        setEnables(FILESELECTED) ;
        resp=true ;
    }
    if (!resp) {
        setEnables(NOTHINGSELECTED) ;
    }
    return resp ;
}

bool MainWindow::updateListSelection(PoiCollection *collection, QListWidget *widget, bool refreshMarkers)
{
    bool foundUuid=false ;
    if (collection==NULL || widget==NULL) return false ;

    for (int i=0; i<widget->count(); i++) {

        QListWidgetItem *item = widget->item(i) ;
        QString uuid = item->data(Qt::UserRole).toString() ;

        if (uuid == thisUuid) {
            item->setSelected(true) ;
            widget->scrollToItem(item);
            foundUuid=true ;
        } else {
            item->setSelected(false) ;
        }

        if (refreshMarkers) ui->googlemapsWebView->setMarker(collection->at(i).uuid(),
              collection->uuid(),
              collection->at(i).lat(),
              collection->at(i).lon(),
              collection->at(i).address()) ;
    }

    return foundUuid ;
}


QString MainWindow::currentSelectionUuid(QListWidget *widget)
{
    QList<QListWidgetItem *>items = widget->selectedItems() ;
    if (items.size()>0) {
        return items[0]->data(Qt::UserRole).toString() ;
    } else {
        return QString("") ;
    }
}

PoiEntry& MainWindow::findEntryByUuid(QString uuid, QString collectionUuid)
{
    if (collectionUuid == fileCollection.uuid()) {
        PoiEntry& pl=fileCollection.find(uuid) ;
        if (pl.isValid()) return pl ;
    }
    PoiEntry& wl=workingCollection.find(uuid) ;
    return wl ;
}


//////////////////////////////////////////////////////////////////////////
//
// POI editing text field management
//

void MainWindow::on_editDescription_editingFinished()
{
   PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
   pe.setDescription(ui->editDescription->text()) ;
   // Refresh door and phone in case user typed [door]>phone in description
   ui->editDescription->setText(pe.description()) ;
   ui->editDoorNumber->setText(pe.door()) ;
   ui->editPhone->setText(pe.phone()) ;
   updateLists() ;
   updateListsSelection(false) ;
}

void MainWindow::on_editPhone_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) pe.setPhone(ui->editPhone->text()) ;
}

void MainWindow::on_editDoorNumber_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) pe.setDoor(ui->editDoorNumber->text()) ;
}

//////////////////////////////////////////////////////////////////////////
//
// Working List Functions
//

void MainWindow::on_listWorking_itemClicked(QListWidgetItem *item)
{
    thisUuid = item->data(Qt::UserRole).toString() ;
    thisCollectionUuid = workingCollection.uuid() ;

    PoiEntry& currentEntry = workingCollection.find(thisUuid) ;
    if (currentEntry.address().isEmpty()) {
        ui->googlemapsWebView->geocodeMarker(thisUuid) ;
    }

    updateForm() ;
    updateLists() ;
    updateListsSelection(false) ;
    updateMapSelection(0) ;
}


void MainWindow::on_listWorking_itemDoubleClicked(QListWidgetItem *item)
{
    on_listWorking_itemClicked(item) ;
    updateMapSelection(PREFZOOM) ;
}

void MainWindow::on_btnStore_clicked()
{
    if (fileCollection.filename().isEmpty()) return ;

    QString selection = currentSelectionUuid(ui->listWorking) ;
    if (!selection.isEmpty()) {

        // Get the item details
        thisUuid = selection ;
        thisCollectionUuid = fileCollection.uuid() ;

        // Move the entry
        fileCollection.add(workingCollection.find(thisUuid)) ;
        workingCollection.remove(thisUuid) ;

        // Modify the google map record
        ui->googlemapsWebView->setMarkerCollection(thisUuid, thisCollectionUuid) ;

        // Update the lists
        updateLists() ;
    }
}

void MainWindow::on_btnDuplicate_clicked()
{
    QString selection = currentSelectionUuid(ui->listWorking) ;

    if (!selection.isEmpty()) {

        PoiEntry newEntry ;
        PoiEntry& currentEntry = workingCollection.find(selection) ;

        newEntry.setDescription(currentEntry.description()) ;
        newEntry.setAddress(currentEntry.address()) ;
        newEntry.setDoor(currentEntry.door()) ;
        newEntry.setPhone(currentEntry.phone()) ;
        newEntry.setLatLon(currentEntry.lat(), currentEntry.lon()) ;

        thisUuid = newEntry.uuid() ;
        thisCollectionUuid = workingCollection.uuid() ;
        workingCollection.add(newEntry) ;
        ui->googlemapsWebView->setMarker(thisUuid, thisCollectionUuid, newEntry.lat(), newEntry.lon(), newEntry.address());

        updateLists() ;
        updateForm() ;
        updateMapSelection(0) ;

    }

}

void MainWindow::on_btnDelete_clicked()
{
    QString selection = currentSelectionUuid(ui->listWorking) ;

    if (!selection.isEmpty()) {

        workingCollection.remove(selection) ;
        ui->googlemapsWebView->removeMarker(selection) ;

        updateForm() ;
        updateLists() ;
        updateListsSelection(false) ;
        updateMapSelection(0) ;

    }
}

//////////////////////////////////////////////////////////////////////////
//
// File List Functions
//

void MainWindow::on_action_Save_triggered()
{
    saveCollection(true) ;
    updateForm() ;
    updateLists() ;
    updateListsSelection(true) ;
    updateMapSelection(0) ;
}

void MainWindow::loadFiles()
{
    // Clear all data
    fileCollection.clear() ;
    ui->cbPOIFiles->clear() ;

    ui->cbPOIFiles->addItem("", QString("")) ;

    // TODO: Search directory and iterate
    QString& poiFiles = configuration->configFolder() ;

    QStringList ov2files ;
    ov2files.append("*.ov2") ;
    ov2files.append("*.OV2") ;
    ov2files.append("*.oV2") ;
    ov2files.append("*.Ov2") ;

    QDir path = QDir(poiFiles);
    path.setSorting(QDir::Name);
    QStringList files = path.entryList(QStringList(ov2files), QDir::Files | QDir::NoSymLinks);

    for (int i=0; i<files.size(); i++) {
        QString& filename = files[i] ;
        QString name = filename ;
        QRegExp rxd("^(.*)\\.[Oo][Vv]2");
        if (rxd.indexIn(name)>=0) name=rxd.cap(1).trimmed() ;
        ui->cbPOIFiles->addItem(name, poiFiles + "/" + filename) ;
    }

}

void MainWindow::on_cbPOIFiles_currentIndexChanged(int index)
{
    QString filename = ui->cbPOIFiles->itemData(index, Qt::UserRole).toString() ;

    saveCollection() ;
    fileCollection.clear() ;
    if (!filename.isEmpty()) {
        fileCollection.setFilename(filename) ;
        if (!fileCollection.load()) {
            // TODO: error handle
            // set index of list
        }
        thisCollectionUuid = fileCollection.uuid() ;
    }

    updateForm() ;
    updateLists() ;
    updateListsSelection(true) ;
    updateMapSelection(0) ;
}

void MainWindow::on_listFile_itemClicked(QListWidgetItem *item)
{
    thisUuid = item->data(Qt::UserRole).toString() ;
    thisCollectionUuid = fileCollection.uuid() ;

    PoiEntry& currentEntry = fileCollection.find(thisUuid) ;
    if (currentEntry.address().isEmpty()) {
        ui->googlemapsWebView->geocodeMarker(thisUuid) ;
    }

    updateForm() ;
    updateLists() ;
    updateListsSelection(false) ;
    updateMapSelection(0) ;
}

void MainWindow::on_listFile_itemDoubleClicked(QListWidgetItem *item)
{
    on_listFile_itemClicked(item) ;
    updateMapSelection(PREFZOOM) ;
}

void MainWindow::on_btnRemove_clicked()
{
    QString selection = currentSelectionUuid(ui->listFile) ;

    if (!selection.isEmpty()) {

        // Get the item details
        thisUuid = selection ;
        thisCollectionUuid = workingCollection.uuid() ;

        // Move the entry
        workingCollection.add(fileCollection.find(thisUuid)) ;
        fileCollection.remove(thisUuid) ;

        // Modify the google map record
        ui->googlemapsWebView->removeMarker(thisUuid) ;
        thisUuid=QString("") ;

        // Update the lists
        updateLists() ;
        updateListsSelection(true) ;
        updateMapSelection(0) ;
    }
}

void MainWindow::on_btnDuplicateFile_clicked()
{
    QString selection = currentSelectionUuid(ui->listFile) ;

    if (!selection.isEmpty()) {

        PoiEntry newEntry ;
        PoiEntry& currentEntry = fileCollection.find(selection) ;

        newEntry.setDescription(currentEntry.description()) ;
        newEntry.setAddress(currentEntry.address()) ;
        newEntry.setPhone(currentEntry.phone()) ;
        newEntry.setDoor(currentEntry.door()) ;
        newEntry.setLatLon(currentEntry.lat(), currentEntry.lon()) ;

        thisUuid = newEntry.uuid() ;
        thisCollectionUuid = workingCollection.uuid() ;
        workingCollection.add(newEntry) ;
        ui->googlemapsWebView->setMarker(thisUuid, thisCollectionUuid, newEntry.lat(), newEntry.lon(), newEntry.address());

        updateLists() ;
        updateListsSelection(false) ;
        updateMapSelection(0) ;
        updateForm() ;

    }
}

//////////////////////////////////////////////////////////////////////////
//
// Callbacks from TomTom
//
void MainWindow::importPoi(QString description, double lat, double lon)
{
    PoiEntry newEntry ;

    newEntry.setDescription(description) ;
    newEntry.setAddress(description) ;
    newEntry.setLatLon(lat, lon) ;

    thisUuid = newEntry.uuid() ;
    thisCollectionUuid = workingCollection.uuid() ;
    workingCollection.add(newEntry) ;
    ui->googlemapsWebView->setMarker(thisUuid, thisCollectionUuid, newEntry.lat(), newEntry.lon(), newEntry.address());

    updateLists() ;
    updateListsSelection(false) ;
    updateMapSelection(0) ;
    updateForm() ;
}

//////////////////////////////////////////////////////////////////////////
//
// Callbacks from Map
//

//
// callback - Address search results found
//
void MainWindow::mapCallbackSearchResultsReady(QString placeId, double lat, double lon, QString formattedAddress, QString phoneNumber)
{
    PoiEntry newEntry ;
    QString description ;

    qDebug() << "" ;
    qDebug() << "MainWindow::mapCallbackSearchResultsReady() ==>" ;
    qDebug() << "placeid: " << placeId ;
    qDebug() << "lat: " << lat <<", lon:" << lon  ;
    qDebug() << "formattedaddress" << formattedAddress ;
    qDebug() << "phoneNumber" << phoneNumber ;

    description=ui->editSearch->text() ;
    if (description.isEmpty()) { description = formattedAddress ; }
    newEntry.setDescription(description) ;

    newEntry.setAddress(formattedAddress) ;
    newEntry.setPhone(phoneNumber) ;
    newEntry.setLatLon(lat, lon) ;
    newEntry.setPlaceId(placeId) ;

    thisUuid = newEntry.uuid() ;
    thisCollectionUuid = workingCollection.uuid() ;
    workingCollection.add(newEntry) ;
    ui->googlemapsWebView->setMarker(thisUuid, thisCollectionUuid, lat, lon, formattedAddress);

    updateLists() ;
    updateListsSelection(false) ;
    updateMapSelection(PREFZOOM) ;
    updateForm() ;

}

//
// callback - No place could be found
//
void MainWindow::mapCallbackSearchFailed(QString error)
{
    qDebug() << "MainWindow::mapCallbackSearchResultsFailed(\"" << error << "\")" ;
    QMessageBox::warning(this, tr("Search Error"), error) ;
    ui->googlemapsWebView->removeAllMarkers() ;

    thisUuid = "" ;
    thisCollectionUuid = "" ;

    updateForm() ;
    updateMapSelection(0) ;
}

//
// callback - selected and updated marker from map
//
void MainWindow::mapCallbackMarkerMoved(QString uuid, QString collectionUuid, double lat, double lon)
{
    qDebug() << "MainWindow::mapCallbackMarkerMoved(\"" << uuid << ", " << collectionUuid << ", " << lat << ", " << lon << "\")" ;
    PoiEntry& pe = findEntryByUuid(uuid, collectionUuid) ;
    if (pe.isValid()) {
        pe.setLatLon(lat, lon) ;
        updateForm() ;
    }
}

void MainWindow::mapCallbackMarkerGeocoded(QString uuid, QString collectionUuid, QString address)
{
    qDebug() << "MainWindow::mapCallbackMarkerGeocoded(\"" << uuid << ", \"" << address << "\")" ;
    PoiEntry& pe = findEntryByUuid(uuid, collectionUuid) ;
    if (pe.isValid()) {
        // Update record
        pe.setAddress(address) ;
        updateForm() ; 
    }
}

//
// callback - marker on map has been seelcted
//
void MainWindow::mapCallbackMarkerSelected(QString uuid, QString collectionUuid)
{
    qDebug() << "MainWindow::mapCallbackMarkerSelected(\"" << uuid << ", " << collectionUuid << "\")" ;
    PoiEntry& pe = findEntryByUuid(uuid, collectionUuid) ;
    if (pe.isValid()) {
        thisUuid = uuid ;
        thisCollectionUuid = collectionUuid ;

        updateForm() ;
        updateLists() ;
        updateListsSelection(false) ;
    }
}

//
// MENU HANDLERS
//

void MainWindow::on_action_Setup_triggered()
{
    configuration->exec() ;
}

void MainWindow::on_action_Exit_triggered()
{
    close() ;
}

void MainWindow::on_actionRefresh_Google_Map_triggered()
{
    ui->googlemapsWebView->reload() ;
}

void MainWindow::on_actionRefresh_Tom_Tom_triggered()
{
    ui->tomtomWebView->reload() ;
}



void MainWindow::on_actionClear_Cookies_triggered()
{
    ui->googlemapsWebView->clearCookies() ;
    ui->tomtomWebView->clearCookies() ;
}

void MainWindow::on_btnNew_clicked()
{
    double lat=ui->googlemapsWebView->getLat() ;
    double lon=ui->googlemapsWebView->getLon() ;
    ui->googlemapsWebView->searchLatLon(lat,lon) ;
}

