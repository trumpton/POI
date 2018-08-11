#include "mainwindow.h"
#include "ui_mainwindow.h"

// FEATURES:

// BUG: ov2 import adds 2 of each entry

// TODO: Export to TomTOm should default in the Poi folder

// TODO: Check OV2 handles stars OK

// BUG: Select blue then green, and form fields are grey
//      Select red then green, and some form fields are black when they should be disabled (grey)
//

// BUG: Delete waypoint from clipboard not working

// BUG: Save as doesn't mark as saved



#include <QDesktopServices>
#include <QString>
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QRegExp>
#include <QStringList>
#include "apikeys.h"
#include "prompt.h"

#define STAR  QChar(0x2605)

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
    ui->googlemapsWebView->initialise(key, workingCollection.uuid(), fileCollection.uuid(), fileCollection.trackUuid()) ;

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

    // Handle Export Menu Refresh
    connect(ui->menu_Export, &QMenu::aboutToShow, this, &MainWindow::on_menuExport_aboutToShow) ;

    refresh() ;
}

MainWindow::~MainWindow()
{
    saveCollection() ;
    delete configuration ;
    delete ui;
}

//////////////////////////////////////////////////////////////////////////
//
// Find Functions
//

PoiEntry& MainWindow::findEntryByUuid(QString uuid, QString collectionUuid)
{
    if (collectionUuid == fileCollection.uuid()) {
        PoiEntry& pl=fileCollection.find(uuid) ;
        if (pl.isValid()) return pl ;
    }
    PoiEntry& wl=workingCollection.find(uuid) ;
    return wl ;
}

TrackEntry& MainWindow::findTrackEntryByUuid(QString uuid, QString collectionUuid)
{
    return fileCollection.findTrack(uuid) ;
}


//////////////////////////////////////////////////////////////////////////
//
// Save Function
//


void MainWindow::saveCollection(bool autoyes)
{
    if (fileCollection.isDirty()) {
        QMessageBox::StandardButton reply ;
        if (autoyes) {
            reply = QMessageBox::Yes ;
        } else {
            reply = QMessageBox::question(this,
                "POI", "File as been edited, do you want to save?",
                 QMessageBox::Yes|QMessageBox::No);
        }
        if (reply == QMessageBox::Yes) {

            if (fileCollection.filename().isEmpty()) {

                // Get filename
                QString formattedName = fileCollection.formattedName(true, false, false) ;

                // Get Folder
                QString folder ;
                if (fileCollection.trackLength()==0) {
                    folder = configuration->poiFolder() ;
                } else {
                    folder = configuration->tracksFolder() ;
                }
                if (!folder.isEmpty()) folder = folder + QString("/") ;

                QString filename = QFileDialog::getSaveFileName(this, QString("Save File"), folder + formattedName, QString("GPX Files (*.gpx)")) ;

                // Add extension if missing
                QRegularExpression re(".*\\.gpx", QRegularExpression::CaseInsensitiveOption) ;
                if (!re.match(filename).hasMatch()) {
                    filename = filename + QString(".gpx") ;
                }

                QFile f(filename) ;

                if (!f.exists() ||
                    QMessageBox::question(this, "POI", "Do you wish to overwrite this file?",
                                          QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes) {

                    fileCollection.setFilename(filename) ;
                }

            }

            if (!fileCollection.filename().isEmpty()) {

                // Save the collection
                fileCollection.saveGpx() ;

                if (fileCollection.trackSize()==0) {

                    // There are no tracks, so save the OV2 file
                    fileCollection.saveOv2() ;

                } else {

                    // There are tracks, so also update the "Tracks.gpx" and "Tracks.ov2" files

                    // Create a New Waypoint entry, using the first waypoint, or the first track point
                    // (pre-requisit is that the fileCollection() is already sorted)
                    PoiEntry ent ;

                    if (fileCollection.size()>0) {
                        ent.copyFrom(fileCollection.at(0)) ;
                    } else {
                        ent.setLatLon(fileCollection.trackAt(0).lat(), fileCollection.at(0).lon()) ;
                    }

                    ent.setUuid(fileCollection.uuid()) ;

                    QString name = fileCollection.formattedName(false) ;
                    ent.set(PoiEntry::EDITEDTITLE, name) ;
                    ent.setSequence(0) ;

                    QString typetxt ;
                    for (int i=fileCollection.rating(); i>0; i--) {
                        typetxt = typetxt + QString("★") ;
                    }
                    ent.set(PoiEntry::EDITEDTYPE, typetxt) ;

                    // Calculate tracks filename
                    QFileInfo fi(fileCollection.filename()) ;
                    QString trackfilename = configuration->poiFolder() ;

                    if (!trackfilename.isEmpty()) {

                        trackfilename = trackfilename +  "/" + "Tracks.gpx" ;
                        PoiCollection tracks ;

                        // Load existing data
                        tracks.loadGpx(trackfilename) ;

                        // Update the entry and save
                        tracks.remove(ent.uuid()) ;
                        tracks.add(ent) ;
                        tracks.saveGpx() ;
                        tracks.saveOv2() ;

                    }
                }

            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//
// Refresh the screen
//

bool MainWindow::refresh(bool refreshMarkers, bool centreOnMarker, int zoom)
{
    static int mutex=0 ;

    if (++mutex>1) {

        mutex--;
        return false ;

    } else {

        // Re-populate lists
        updateSearchFilter() ;
        updateList(&workingCollection, ui->listWorking) ;
        updateList(&fileCollection, ui->listFile, ui->comboBox_Filter->currentText()) ;

        // Set Rating
        ui->comboBox_Rating->setCurrentIndex(fileCollection.rating());

        // Redraw the Map
        if (refreshMarkers) refreshMap() ;

        // Centre and zoom
        if (centreOnMarker) updateMapSelection(zoom) ;

        // Update Lists and Edit Box

        // Edit Details
        ui->groupBox_Details->setEnabled(false) ;
        ui->groupBox_Details->setVisible(false) ;

        // File Collection
        ui->btnEditFile->setEnabled(false) ;
        ui->btnCopyToClipboard->setEnabled(false) ;
        ui->btnDown->setEnabled(false) ;
        ui->btnUp->setEnabled(false) ;

        // Working Collection
        ui->btnNew->setEnabled(true) ;
        ui->btnStore->setEnabled(false) ;
        ui->btnDuplicate->setEnabled(false) ;
        ui->btnDelete->setEnabled(false) ;

        if (updateListSelection(&workingCollection, ui->listWorking)) {

            updateForm() ;
            ui->groupBox_Details->setEnabled(true) ;
            ui->groupBox_Details->setVisible(true) ;
            ui->btnStore->setEnabled(true) ;
            ui->btnDuplicate->setEnabled(true) ;
            ui->btnDelete->setEnabled(true) ;

        } else if (updateListSelection(&fileCollection, ui->listFile)) {

            updateForm() ;
            ui->groupBox_Details->setVisible(true) ;
            ui->btnEditFile->setEnabled(true) ;
            ui->btnCopyToClipboard->setEnabled(true) ;
            if (ui->action_ShowTrack->isChecked()) {
                ui->btnDown->setEnabled(true) ;
                ui->btnUp->setEnabled(true) ;
            }
        }

        // Track Edit Buttons
        if (thisCollectionUuid.compare(fileCollection.trackUuid())==0) {
            ui->NewTrackPoint->setEnabled(true) ;
            ui->DeleteTrackPoint->setEnabled(true) ;
        } else {
            ui->NewTrackPoint->setEnabled(false) ;
            ui->DeleteTrackPoint->setEnabled(false) ;
        }

        // Menu Enables
        ui->action_CreateTrackFromWaypoints->setEnabled(fileCollection.size()>=2) ;
        ui->action_ReduceTrackPoints->setEnabled(fileCollection.trackSize()>0) ;
        ui->action_ShowTrack->setEnabled(fileCollection.size()>0) ;
        ui->action_EmptyClipboard->setEnabled(workingCollection.size()>0) ;

        mutex-- ;
        return true ;
    }
}

void MainWindow::on_menuExport_aboutToShow()
{
    QString garminfolder = configuration->garminFolder() ;
    bool garminexists = false ;
    if (!garminfolder.isEmpty()) {
        QDir garmin(garminfolder) ;
        garminexists = garmin.isReadable() ;
    }
    ui->actionTransfer_to_Garmin->setEnabled(garminexists) ;
    ui->actionTransfer_from_Garmin->setEnabled(garminexists) ;
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

        setLineEditText(ui->lineEdit_Door, pe, PoiEntry::EDITEDDOOR, PoiEntry::GEODOOR, ui->label_DoorEdited, ui->label_DoorGeo) ;
        setLineEditText(ui->lineEdit_Street, pe, PoiEntry::EDITEDSTREET, PoiEntry::GEOSTREET, ui->label_StreetEdited, ui->label_StreetGeo) ;
        setLineEditText(ui->lineEdit_City, pe, PoiEntry::EDITEDCITY, PoiEntry::GEOCITY, ui->label_CityEdited, ui->label_CityGeo) ;
        setLineEditText(ui->lineEdit_State, pe, PoiEntry::EDITEDSTATE, PoiEntry::GEOSTATE, ui->label_StateEdited, ui->label_StateGeo) ;
        setLineEditText(ui->lineEdit_Country, pe, PoiEntry::EDITEDCOUNTRY, PoiEntry::GEOCOUNTRY, ui->label_CountryEdited, ui->label_CountryGeo) ;
        setLineEditText(ui->lineEdit_Postcode, pe, PoiEntry::EDITEDPOSTCODE, PoiEntry::GEOPOSTCODE, ui->label_PostcodeEdited, ui->label_PostcodeGeo) ;

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

bool MainWindow::updateSearchFilter()
{

        QStringList filterEntries ;
        for (int i=0; i<fileCollection.size(); i++) {
            QRegExp re("[^a-zA-Z0-9★]") ;
            PoiEntry& file = fileCollection.at(i) ;
            QString type = file.get(PoiEntry::EDITEDTYPE) ;
            QStringList ents = type.replace(re, " ").toLower().split(" ") ;
            for (int j=0; j<ents.size(); j++) {
                QString ent = ents.at(j) ;
                if (!ent.isEmpty()) {

                    // Capitalise
                    ent.replace(0, 1, ent.at(0).toUpper()) ;

                    // Search if already exists
                    bool found = false ;
                    for (int k=0; k<filterEntries.size(); k++) {
                        if (filterEntries.at(k).compare(ent)==0) found=true ;
                    }

                    // Add entry to list
                    if (!found) {
                        filterEntries.append(ent) ;
                    }

                }
            }
        }

        filterEntries.sort() ;

        ui->comboBox_Filter->blockSignals(true) ;
        QString currentText = ui->comboBox_Filter->currentText() ;
        ui->comboBox_Filter->clear() ;
        ui->comboBox_Filter->addItem("") ;

        for (int i=0; i<filterEntries.size(); i++) {
            QString ent = filterEntries.at(i) ;
            ui->comboBox_Filter->addItem(ent);
            if (filterEntries.at(i).compare(currentText)==0) {
                ui->comboBox_Filter->setCurrentIndex(i+1);
            }
        }

        ui->comboBox_Filter->blockSignals(false) ;
}

bool MainWindow::updateList(PoiCollection *collection, QListWidget *widget, QString filterText)
{
    bool isstar = (filterText.size()>0 && filterText.at(0) == STAR) ;
    bool selectall = filterText.isEmpty() ;

    if (collection==NULL || widget==NULL) return false ;

    widget->blockSignals(true) ;
    widget->clear() ;

    filterText = filterText.toLower() ;

    // Populate List
    int size = collection->size() ;
    for (int i=0; i<size; i++) {
        QString type = collection->at(i).get(PoiEntry::EDITEDTYPE).toLower() ;

        if (selectall || (isstar && type.compare(filterText)==0) || (!isstar && type.contains(filterText))) {

            QString title = collection->at(i).get(PoiEntry::EDITEDTITLE) ;
            QString sequence ;
            if (ui->action_ShowTrack->isChecked()) {
                sequence = QString("%1").arg(collection->at(i).sequence()/2, 2) + QString(": ");
            }
            QListWidgetItem *item = new QListWidgetItem(sequence + title) ;
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
    }

    widget->blockSignals(false) ;
    return (size>0) ;
}


bool MainWindow::updateListSelection(PoiCollection *collection, QListWidget *widget)
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

    }

    return foundUuid ;
}


bool MainWindow::refreshMap()
{
    ui->label_distance->setText(QString::number(fileCollection.trackLength()/1000,'f',1)+QString(" km")) ;
    ui->label_climb->setText(QString::number(fileCollection.heightGain(),'f',0)+QString(" m / ")+QString::number(fileCollection.heightLoss(),'f',0)+QString(" m")) ;

    QString time ;

    if (ui->action_ShowActualDuration->isChecked()) {
        long int duration ;
        duration = (long int) fileCollection.trackTime() ;
        time = QString::number(duration/60)+QString("h")+((duration%60<10)?QString("0"):QString(""))+QString::number(duration%60) + QString(" (act)");
    } else {
        long int duration ;
        duration = (long int) fileCollection.trackTimeEst() ;
        time = QString::number(duration/60)+QString("h")+((duration%60<10)?QString("0"):QString(""))+QString::number(duration%60);
    }
    ui->label_duration->setText(time) ;

    ui->googlemapsWebView->removeAllMarkers() ;
    for (int i=0, ni=workingCollection.size(); i<ni; i++) {
        PoiEntry& ent = workingCollection.at(i) ;
        ui->googlemapsWebView->setMarker(ent.uuid(),
          workingCollection.uuid(),
          ent.lat(),
          ent.lon(),
          ent.get(PoiEntry::EDITEDTITLE),
          ent.sequence()) ;
    }
    for (int i=0, ni=fileCollection.size(); i<ni; i++) {
        PoiEntry& ent = fileCollection.at(i) ;
        ui->googlemapsWebView->setMarker(ent.uuid(),
          fileCollection.uuid(),
          ent.lat(),
          ent.lon(),
          ent.get(PoiEntry::EDITEDTITLE),
          ent.sequence()) ;
    }
    for (int i=0, ni=fileCollection.trackSize(); i<ni; i++) {
        TrackEntry& ent = fileCollection.trackAt(i) ;
        ui->googlemapsWebView->setMarker(ent.uuid(),
          fileCollection.trackUuid(),
          ent.lat(),
          ent.lon(),
          QString(""),
          ent.sequence()) ;
    }
    ui->googlemapsWebView->showTracks(ui->action_ShowTrack->isChecked());
    ui->googlemapsWebView->selectMarker(thisUuid);
    return true ;
}


void MainWindow::on_comboBox_Filter_currentIndexChanged(int index)
{
    refresh(false) ;
}


void MainWindow::on_lineEdit_fileTitle_editingFinished()
{
    fileCollection.setName(ui->lineEdit_fileTitle->text()) ;
}


void MainWindow::on_comboBox_Rating_currentIndexChanged(int index)
{
    fileCollection.setRating(index) ;
}

