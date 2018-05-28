#include "mainwindow.h"
#include "ui_mainwindow.h"

//
// TODO: Importing geocoding trips up over itself, and the different responses are not
//       necessarily assigned to the correct query.  Need a query identifier which
//       is returned with the response - i.e. the uuid.

//
// TODO: When using the duplicate on the file list, the red working copy can be behind.
//       Selected item shouldalways be on the top.  Fix this in javascript 'selectmarker'
//       Hopefully there's a bring to front, otherwise, it's remember where you were, and
//       draw the selected one last.
//
//


#include <QDesktopServices>
#include <QString>
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QRegExp>
#include "apikeys.h"
#include "urls.h"
#include "prompt.h"
#include "version.h"

#define PREFZOOM 17

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
    if (key.isEmpty()) key=QString(GOOGLEAPIKEY) ;
    ui->googlemapsWebView->initialise(key, workingCollection.uuid()) ;

    // TODO: Shouldn't need these
    ui->googlemapsWebView->clearCookies() ;

    // Update markers when selected or moved on the map
    connect(ui->googlemapsWebView, &GoogleMapsWidget::markerSelected, this, &MainWindow::mapCallbackMarkerSelected);
    connect(ui->googlemapsWebView, &GoogleMapsWidget::markerMoved, this, &MainWindow::mapCallbackMarkerMoved);

    // Handle Geocoding Update
    connect(ui->googlemapsWebView, &GoogleMapsWidget::markerGeocoded, this, &MainWindow::mapCallbackMarkerGeocoded);

    // Handle search results / failure
    connect(ui->googlemapsWebView, &GoogleMapsWidget::searchResultsReady, this, &MainWindow::mapCallbackSearchResultsReady);
    connect(ui->googlemapsWebView, &GoogleMapsWidget::searchFailed, this, &MainWindow::mapCallbackSearchFailed);

    // POI Files
    while (!loadFiles()) {
        // TODO: Prompt for setup
        on_action_Setup_triggered() ;
    }

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
        if (reply == QMessageBox::Yes) {
            fileCollection.saveGpx(false) ;
            fileCollection.saveGpx(true) ;
            fileCollection.saveOv2() ;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//
// Refresh the screen
//


bool MainWindow::refresh(bool refreshMarkers, int zoom)
{
    static int mutex=0 ;
    if (++mutex>1) {
        mutex--;
        return false ;
    } else {
        updateLists() ;
        updateListsSelection(refreshMarkers) ;
        updateForm() ;
        updateMapSelection(zoom) ;
        mutex-- ;
        return true ;
    }
}


bool MainWindow::updateMapSelection(int zoom)
{
    ui->googlemapsWebView->selectMarker(thisUuid) ;
    if (!thisUuid.isEmpty()) ui->googlemapsWebView->seekToMarker(thisUuid, zoom) ;
    return true ;
}


bool MainWindow::updateForm()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;

    if (!pe.isValid()) {

        ui->plainTextEdit_Description->clear() ;
        ui->lineEdit_title->clear() ;
        ui->lineEdit_Door->clear() ;
        ui->lineEdit_Street->clear() ;
        ui->lineEdit_City->clear() ;
        ui->lineEdit_State->clear() ;
        ui->lineEdit_Country->clear() ;
        ui->lineEdit_Postcode->clear() ;
        ui->lineEdit_Phone1->clear() ;
        ui->lineEdit_Phone2->clear() ;
        ui->lineEdit_Type->clear() ;
        return true ;

    } else {

        ui->plainTextEdit_Description->setPlainText(pe.get(PoiEntry::EDITEDDESCR)) ;
        ui->lineEdit_title->setText(pe.get(PoiEntry::EDITEDTITLE)) ;
        setLineEditText(ui->lineEdit_Door, pe, PoiEntry::EDITEDDOOR, PoiEntry::GEODOOR) ;
        setLineEditText(ui->lineEdit_Street, pe, PoiEntry::EDITEDSTREET, PoiEntry::GEOSTREET) ;
        setLineEditText(ui->lineEdit_City, pe, PoiEntry::EDITEDCITY, PoiEntry::GEOCITY) ;
        setLineEditText(ui->lineEdit_State, pe, PoiEntry::EDITEDSTATE, PoiEntry::GEOSTATE) ;
        setLineEditText(ui->lineEdit_Country, pe, PoiEntry::EDITEDCOUNTRY, PoiEntry::GEOCOUNTRY) ;
        setLineEditText(ui->lineEdit_Postcode, pe, PoiEntry::EDITEDPOSTCODE, PoiEntry::GEOPOSTCODE) ;
        ui->lineEdit_Phone1->setText(pe.get(PoiEntry::EDITEDPHONE1)) ;
        ui->lineEdit_Phone2->setText(pe.get(PoiEntry::EDITEDPHONE2)) ;
        ui->lineEdit_Type->setText(pe.get(PoiEntry::EDITEDTYPE)) ;
        if (pe.get(PoiEntry::GEOCODED).compare("yes")==0) {
            ui->label_MarkerPin->setVisible(true) ;
            ui->label_MarkerPinError->setVisible(false) ;
        } else {
            ui->label_MarkerPin->setVisible(false) ;
            ui->label_MarkerPinError->setVisible(true) ;
        }
        return true ;

    }
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

    widget->blockSignals(true) ;
    widget->clear() ;

    // Populate List
    int size = collection->size() ;
    for (int i=0; i<size; i++) {
        QListWidgetItem *item = new QListWidgetItem(collection->at(i).get(PoiEntry::EDITEDTITLE)) ;
        QString uuid = collection->at(i).uuid() ;
        QVariant variant(uuid) ;
        item->setData(Qt::UserRole, variant) ;

        if (collection->at(i).isDirty()) {
            item->setForeground(Qt::red);
        }

        widget->addItem(item) ;

        if (uuid.compare(thisUuid)==0) {
            item->setSelected(true) ;
            widget->setCurrentItem(item) ;
        }

    }

    widget->blockSignals(false) ;
    return (size>0) ;
}


bool MainWindow::updateListsSelection(bool refreshMarkers)
{
    bool resp=false ;
    if (refreshMarkers) ui->googlemapsWebView->removeAllMarkers() ;

    if (updateListSelection(&workingCollection, ui->listWorking, refreshMarkers)) {
        ui->groupBox_Details->setEnabled(true) ;
        ui->groupBox_Details->setVisible(true) ;
        resp=true ;
    }
    if (updateListSelection(&fileCollection, ui->listFile, refreshMarkers)) {
        ui->groupBox_Details->setEnabled(false) ;
        ui->groupBox_Details->setVisible(true) ;
        resp=true ;
    }
    if (!resp) {
        ui->groupBox_Details->setEnabled(false) ;
        ui->groupBox_Details->setVisible(false) ;
    }
    return resp ;
}

bool MainWindow::updateListSelection(PoiCollection *collection, QListWidget *widget, bool refreshMarkers)
{
    bool foundUuid=false ;
    if (collection==NULL || widget==NULL) return false ;

    int widgetcount = widget->count() ;

    for (int i=0; i<widgetcount; i++) {

        QListWidgetItem *item = widget->item(i) ;
        QString uuid = item->data(Qt::UserRole).toString() ;

        if (uuid == thisUuid) {
            item->setSelected(true) ;
            widget->scrollToItem(item);
            foundUuid=true ;
        } else {
            item->setSelected(false) ;
        }

        PoiEntry& ent = findEntryByUuid(uuid, collection->uuid()) ;

        if (!ent.isValid()) {
            return false ;
        }

        if (refreshMarkers)
            ui->googlemapsWebView->setMarker(ent.uuid(),
              collection->uuid(),
              ent.lat(),
              ent.lon(),
              ent.get(PoiEntry::EDITEDTITLE)) ;
    }

    return foundUuid ;
}


//////////////////////////////////////////////////////////////////////////
//
// Search for New Locations on Map
//

// Search for a new location
void MainWindow::on_btnFind_pressed()
{
    ui->googlemapsWebView->searchLocation(ui->lineEdit_Search->text()) ;
}

void MainWindow::on_lineEdit_Search_returnPressed()
{
    on_btnFind_pressed() ;
}

//////////////////////////////////////////////////////////////////////////
//
// Manage / sync data between edit fields and lists
//


void MainWindow::setLineEditText(QLineEdit *control, PoiEntry& data, PoiEntry::FieldType edited, PoiEntry::FieldType geocoded)
{
    QPalette red, black ;

    if (ui->groupBox_Details->isEnabled()) {
        red.setColor(QPalette::Text,Qt::red);
        black.setColor(QPalette::Text,Qt::black);
    }

    if (!data.get(edited).isEmpty()) {
        control->setPalette(red);
        control->setText(data.get(edited)) ;
    } else {
        control->setPalette(black);
        control->setText(data.get(geocoded)) ;
    }

    refresh() ;
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
// Clipboard Functions
//

void MainWindow::on_listWorking_itemClicked(QListWidgetItem *item)
{
    thisUuid = item->data(Qt::UserRole).toString() ;
    thisCollectionUuid = workingCollection.uuid() ;

    PoiEntry& currentEntry = workingCollection.find(thisUuid) ;
    if (currentEntry.get(PoiEntry::GEOCODED).compare("yes")!=0) {
        ui->googlemapsWebView->geocodeMarker(thisUuid, thisCollectionUuid, true);
    }

    refresh() ;
    ui->googlemapsWebView->selectMarker(thisUuid) ;
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

        refresh() ;

    }
}

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
        ui->googlemapsWebView->setMarker(thisUuid, thisCollectionUuid, newEntry.lat(), newEntry.lon(), newEntry.get(PoiEntry::EDITEDTITLE));

        refresh() ;

    }

}

void MainWindow::on_btnDelete_clicked()
{
    QString selection = currentSelectionUuid(ui->listWorking) ;

    if (!selection.isEmpty()) {

        workingCollection.remove(selection) ;
        ui->googlemapsWebView->removeMarker(selection) ;

        refresh() ;

    }
}

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

    thisUuid = newEntry.uuid() ;
    thisCollectionUuid = workingCollection.uuid() ;
    workingCollection.add(newEntry) ;
    ui->googlemapsWebView->setMarker(thisUuid, thisCollectionUuid, lat, lon, "New Entry");
    ui->googlemapsWebView->geocodeMarker(thisUuid, thisCollectionUuid, true);

    refresh(false, PREFZOOM) ;

}


//////////////////////////////////////////////////////////////////////////
//
// File List Functions
//

void MainWindow::on_action_Save_triggered()
{
    saveCollection(true) ;
    refresh() ;
}

void MainWindow::on_action_New_triggered()
{
    QString& poiFiles = configuration->configFolder() ;
    if (poiFiles.isEmpty()) {
        // Open Configuration
    } else {
        Prompt p ;
        p.setup("New Group", "Group Name", "Enter the name of the new group") ;
        p.exec() ;

        // Create Empty File
        QString name = p.text().replace("/","").replace("\\","").replace("*","").replace("?","").replace(":","") ;
        QString filebase = poiFiles + "/" + name ;
        QFile f(filebase + ".poi") ;
        f.open(QIODevice::WriteOnly) ;
        f.close() ;

        // Refresh Files
        loadFiles() ;
        refresh() ;

        // Find the newly created file
        for (int i=0; i<ui->cbPOIFiles->count(); i++) {
            QString filename = ui->cbPOIFiles->itemData(i, Qt::UserRole).toString() ;
            if (filename.compare(filebase)==0) {
                ui->cbPOIFiles->setCurrentIndex(i) ;
            }

        }
    }
}

void MainWindow::on_action_Delete_triggered()
{
    QString& poiFiles = configuration->configFolder() ;
    if (!poiFiles.isEmpty()) {
        if (QMessageBox::question(this, "Delete Group", "Are you sure you want to remove this group", QMessageBox::Yes|QMessageBox::No)!=QMessageBox::Yes)
            return ;
    }
    int index = ui->cbPOIFiles->currentIndex() ;
    QString filename = ui->cbPOIFiles->itemData(index, Qt::UserRole).toString() ;
    QString deletedFilename = filename + ".bak" ;
    QString ov2Filename = filename + ".ov2" ;
    QString gpxFilename = filename + ".gpx" ;
    QFile deletedfile(deletedFilename) ;
    QFile ov2file(ov2Filename) ;
    QFile gpxfile(gpxFilename) ;
    deletedfile.remove() ;
    ov2file.remove() ;
    gpxfile.remove() ;
    QFile::rename(filename + ".poi", deletedFilename) ;
    loadFiles() ;
}

bool MainWindow::loadFiles()
{
    // Clear all data
    fileCollection.clear() ;
    ui->cbPOIFiles->clear() ;

    // TODO: Search directory and iterate
    QString& poiFiles = configuration->configFolder() ;

    if (poiFiles.isEmpty()) return false ;

    QStringList fileMasks ;
    fileMasks.append("*.poi") ;

    QDir path = QDir(poiFiles);
    path.setSorting(QDir::Name);
    QStringList files = path.entryList(QStringList(fileMasks), QDir::Files | QDir::NoSymLinks);

    ui->cbPOIFiles->addItem(" -- Select File -- ", "") ;

    for (int i=0; i<files.size(); i++) {
        QString& filename = files[i] ;
        QRegExp rxd("^(.*)\\.poi");
        QString basename = filename ;
        if (rxd.indexIn(basename)>=0) basename=rxd.cap(1) ;
        ui->cbPOIFiles->addItem(basename, poiFiles + "/" + basename) ;
    }

    refresh() ;
    return true ;
}

void MainWindow::on_cbPOIFiles_currentIndexChanged(int index)
{
    QString filename = ui->cbPOIFiles->itemData(index, Qt::UserRole).toString() ;

    saveCollection() ;
    fileCollection.clear() ;

    if (!filename.isEmpty()) {
        fileCollection.setFilename(filename) ;
        if (!fileCollection.loadGpx()) {
            // TODO: error handle
            // set index of list
            thisCollectionUuid.clear() ;
        } else {
            thisCollectionUuid = fileCollection.uuid() ;
        }
    }
    refresh(true) ;
}

void MainWindow::on_listFile_itemClicked(QListWidgetItem *item)
{
    thisUuid = item->data(Qt::UserRole).toString() ;
    thisCollectionUuid = fileCollection.uuid() ;

    PoiEntry& currentEntry = fileCollection.find(thisUuid) ;
    if (currentEntry.get(PoiEntry::GEOCODED).compare("yes")!=0) {
        ui->googlemapsWebView->geocodeMarker(thisUuid, thisCollectionUuid, true);
    }
    refresh() ;
    ui->googlemapsWebView->selectMarker(thisUuid) ;

}

void MainWindow::on_listFile_itemDoubleClicked(QListWidgetItem *item)
{
    on_listFile_itemClicked(item) ;
    updateMapSelection(PREFZOOM) ;
}

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

        thisUuid = newEntry.uuid() ;
        thisCollectionUuid = workingCollection.uuid() ;
        workingCollection.add(newEntry) ;
        ui->googlemapsWebView->setMarker(thisUuid, thisCollectionUuid, newEntry.lat(), newEntry.lon(), newEntry.get(PoiEntry::EDITEDTITLE));

        refresh() ;

    }
}



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
// Callbacks from Map
//

//
// callback - Address search results found
//
void MainWindow::mapCallbackSearchResultsReady(double lat, double lon, QString formattedAddress, QString phoneNumber)
{
    PoiEntry newEntry ;
    QString searchtext ;

    qDebug() << "" ;
    qDebug() << "MainWindow::mapCallbackSearchResultsReady() ==>" ;
    qDebug() << "lat: " << lat <<", lon:" << lon  ;
    qDebug() << "phoneNumber" << phoneNumber ;

    searchtext=ui->lineEdit_Search->text() ;

    newEntry.set(PoiEntry::EDITEDTITLE, searchtext) ;
    newEntry.set(PoiEntry::EDITEDDESCR, formattedAddress) ;
    newEntry.set(PoiEntry::EDITEDPHONE1, phoneNumber) ;
    newEntry.set(PoiEntry::GEOCODED, "no") ;
    newEntry.setLatLon(lat, lon) ;

    thisUuid = newEntry.uuid() ;
    thisCollectionUuid = workingCollection.uuid() ;
    workingCollection.add(newEntry) ;
    ui->googlemapsWebView->setMarker(thisUuid, thisCollectionUuid, lat, lon, formattedAddress);
    ui->googlemapsWebView->geocodeMarker(thisUuid, thisCollectionUuid, true);

    refresh(false, PREFZOOM) ;

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

    refresh() ;
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
        pe.set(PoiEntry::GEOCODED, "no") ;
        ui->googlemapsWebView->geocodeMarker(uuid, collectionUuid, true);
        refresh() ;
    }
}

void MainWindow::mapCallbackMarkerGeocoded(QString uuid, QString collectionUuid, QString formattedaddress, QString door, QString street, QString town, QString state, QString country, QString postcode)
{
    qDebug() << "MainWindow::mapCallbackMarkerGeocoded(\"" << uuid << ", \"" << formattedaddress << "\")" ;
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
    qDebug() << "MainWindow::mapCallbackMarkerSelected(\"" << uuid << ", " << collectionUuid << "\")" ;
    PoiEntry& pe = findEntryByUuid(uuid, collectionUuid) ;
    if (pe.isValid()) {
        thisUuid = uuid ;
        thisCollectionUuid = collectionUuid ;
        if (pe.get(PoiEntry::GEOCODED).compare("yes")!=0) {
            ui->googlemapsWebView->geocodeMarker(uuid, collectionUuid, true);
        }
        refresh() ;
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


void MainWindow::on_actionClear_Cookies_triggered()
{
    ui->googlemapsWebView->clearCookies() ;
}


void MainWindow::on_action_LaunchTomTom_triggered()
{

    QDesktopServices::openUrl(QString(TOMTOMURL)) ;
}



//////////////////////////////////////////////////////////////////////////
//
// POI editing text field management
//

void MainWindow::on_plainTextEdit_Description_textChanged()
{
   PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
   if (pe.isValid()) {
       pe.set(PoiEntry::EDITEDDESCR, ui->plainTextEdit_Description->toPlainText()) ;
       refresh() ;
   }

}

void MainWindow::on_lineEdit_title_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        pe.set(PoiEntry::EDITEDTITLE, ui->lineEdit_title->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_Door_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        if (ui->lineEdit_Door->text().compare(pe.get(PoiEntry::GEODOOR))!=0) pe.set(PoiEntry::EDITEDDOOR, ui->lineEdit_Door->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_Street_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        if (ui->lineEdit_Street->text().compare(pe.get(PoiEntry::GEOSTREET))!=0) pe.set(PoiEntry::EDITEDSTREET, ui->lineEdit_Street->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_City_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        if (ui->lineEdit_City->text().compare(pe.get(PoiEntry::GEOCITY))!=0) pe.set(PoiEntry::EDITEDCITY, ui->lineEdit_City->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_State_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        if (ui->lineEdit_State->text().compare(pe.get(PoiEntry::GEOSTATE))!=0) pe.set(PoiEntry::EDITEDSTATE, ui->lineEdit_State->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_Postcode_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        if (ui->lineEdit_Postcode->text().compare(pe.get(PoiEntry::GEOPOSTCODE))!=0) pe.set(PoiEntry::EDITEDPOSTCODE, ui->lineEdit_Postcode->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_Country_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        if (ui->lineEdit_Country->text().compare(pe.get(PoiEntry::GEOCOUNTRY))!=0) pe.set(PoiEntry::EDITEDCOUNTRY, ui->lineEdit_Country->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_Type_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        pe.set(PoiEntry::EDITEDTYPE, ui->lineEdit_Type->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_Url_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        pe.set(PoiEntry::EDITEDURL, ui->lineEdit_Url->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_Phone1_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        pe.set(PoiEntry::EDITEDPHONE1, ui->lineEdit_Phone1->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_Phone2_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        pe.set(PoiEntry::EDITEDPHONE2, ui->lineEdit_Phone2->text()) ;
        refresh() ;
    }
}




void MainWindow::on_action_ImportTomTom_triggered()
{
    // Offer to save current collection
    saveCollection(false) ;

    // Clear the current list
    fileCollection.clear() ;

    // Ask for the name of the input file
    QString importName = QFileDialog::getOpenFileName(this,
        tr("Import Tom Tom OV2 File"), "", tr("OV2 Files (*.ov2)"));
    if (importName.isEmpty()) return ;

    // Import the file
    bool cleanload = fileCollection.importOv2(importName) ;
    if (fileCollection.size()==0) {

        QMessageBox::information(this, "Import Failed", "The import failed - no points of interest found") ;
        return ;

    } else {

        if (!cleanload) {
            // Prompt for load, else exit
            if (QMessageBox::question(this, "Partial Import", "The file is corrupt, and can only be partially loaded.  Do you wish to continue?", QMessageBox::Yes|QMessageBox::No)!=QMessageBox::Yes) {
                return ;
            }
        }

        // Create a New group
        on_action_New_triggered() ;

        // Re-load (so that the input is marked as dirty)
        fileCollection.importOv2(importName) ;

        refresh(true) ;

    }

}

void MainWindow::on_actionAuto_Geocode_triggered()
{
    int collectionsize = fileCollection.size() ;
    int geocodecount=0, geocodefailed=0 ;

    QProgressDialog progress("Geocoding", "Abort", 0, collectionsize, this) ;
    progress.setWindowModality(Qt::WindowModal);
    progress.show() ;

    for (int i=0; i<collectionsize && !progress.wasCanceled(); i++) {
        int retries=0 ;
        PoiEntry& ent = fileCollection.at(i) ;
        while (ent.isValid() && ent.get(PoiEntry::GEOCODED).compare("yes")!=0 && retries++<5) {
            if (retries==1) geocodecount++ ;
            QString uuid = fileCollection.at(i).uuid() ;
            QString collectionUuid = fileCollection.uuid() ;
            ui->googlemapsWebView->geocodeMarker(uuid, collectionUuid, true) ;
            for (int j=0; j<(13+retries*2); j++) {
                QThread::msleep(50) ;
                QApplication::processEvents() ;
            }
        }
        if (retries>=5) geocodefailed++ ;
        progress.setValue(i) ;
    }
    progress.hide() ;

    if (geocodecount==0) {
        QMessageBox::information(this, QString("POI"), QString("No Geocoding was Required"), QMessageBox::Ok) ;
    } else if (geocodefailed==0) {
        QMessageBox::information(this, QString("POI"), QString("Geocoding Complete. ") + QString::number(geocodecount) + QString("entries geocoded"), QMessageBox::Ok) ;
    } else {
        QMessageBox::information(this, QString("POI"), QString("Geocoding Complete. ") + QString::number(geocodefailed) + QString("entries of ") + QString::number(geocodecount) + QString(" failed to geocode."), QMessageBox::Ok) ;
    }
}

void MainWindow::on_action_About_POI_triggered()
{
    QMessageBox::information(this, QString("POI"), QString("Version ") + QString(POIVERSION) + QString(". Build ") + QString(POIBUILD), QMessageBox::Ok) ;
}
