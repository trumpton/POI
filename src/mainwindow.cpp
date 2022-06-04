#include "mainwindow.h"
#include "ui_mainwindow.h"

// FEATURES:

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
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringList>
#include <QFileDialog>
#include <QClipboard>

#include "prompt.h"

#define STAR  QChar(0x2605)
#define GOOGLEMAPS

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

    // Set up nullpixmap
    QImage nullimage(":/icons/nullimage.png") ;
    nullpixmap.convertFromImage(nullimage.scaled(180,130)) ;

    // Configuration and Configuration form
    configuration = new Configuration(this) ;

    // Check ini loaded
    if (!configuration->iniFileLoadedOK()) {
        QMessageBox::warning(this,
            "POI", QString("POI.ini could not be found.  Please use File/Settings to search for or create one."));
    }

    // Configure and Load Web Pages
    ui->mapWebView->initialise(QString("qrc:///openlayers/openlayers.html"),
                               configuration->bingKey(),
                               configuration->geocodeType(),
                               configuration->hereId(), configuration->hereCode(),
                               configuration->hereApiKey(),
                               configuration->aerialTileZoom(), configuration->aerialTileUrl(),
                               configuration->satelliteOverlayZoom(), configuration->satelliteOverlayUrl(),
                               configuration->mapTileZoom(), configuration->mapTileUrl(),
                               configuration->contourTileZoom(), configuration->contourTileUrl(),
                               configuration->trailTileZoom(), configuration->trailTileUrl(),
                               workingCollection.uuid(), fileCollection.uuid(), fileCollection.trackUuid()) ;

    // TODO: Shouldn't need these
    ui->mapWebView->clearCookies() ;

    // Update markers when selected or moved on the map
    connect(ui->mapWebView, &MapsWidget::markerSelected, this, &MainWindow::mapCallbackMarkerSelected);
    connect(ui->mapWebView, &MapsWidget::markerMoved, this, &MainWindow::mapCallbackMarkerMoved);

    // Map Moved
    connect(ui->mapWebView, &MapsWidget::mapMoved, this, &MainWindow::mapCallbackMapMoved);

    // Handle Geocoding Update
    connect(ui->mapWebView, &MapsWidget::markerGeocoded, this, &MainWindow::mapCallbackMarkerGeocoded);

    // Handle search results / failure
    connect(ui->mapWebView, &MapsWidget::searchResultsReady, this, &MainWindow::mapCallbackSearchResultsReady);
    connect(ui->mapWebView, &MapsWidget::searchFailed, this, &MainWindow::mapCallbackSearchFailed);

    // Refresh menu before opening
    connect(ui->menu_Export, &QMenu::aboutToShow, this, &MainWindow::on_menuExport_aboutToShow) ;

    // Initialise
    on_action_ShowStandard_triggered() ;
    ui->action_ShowTrack->setChecked(false) ;
    ui->action_ShowActualDuration->setChecked(false) ;

    // Set flag so first tick can initialise
    juststarted=true ;

    // Setup Timer
    timer.setSingleShot(false) ;
    timer.setInterval(500) ;
    timer.callOnTimeout(this, &MainWindow::tick) ;
    timer.start(500) ;

}

MainWindow::~MainWindow()
{
    saveCollection() ;
    delete configuration ;
    delete ui;
}

//////////////////////////////////////////////////////////////////////////
//
// Tick Functions
//
void MainWindow::tick()
{
    if (ui->menu_Export->isVisible()) {
        on_menuExport_aboutToShow() ;
    }

    if (juststarted) {
      refresh() ;
      juststarted=false ;
    }

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
                QString formattedName = fileCollection.formattedName(false, false, false, false, true, false) ;

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
                    refreshTracksPoi() ;
                }
            }
        }
    }
}

bool MainWindow::refreshTracksPoi()
{
    // Calculate tracks filename
    QFileInfo fi(fileCollection.filename()) ;
    QString trackfilename = configuration->poiFolder() ;
    QString trackfolder = configuration->tracksFolder() ;

    if (!trackfilename.isEmpty() && !trackfolder.isEmpty()) {

        // Fix the track filename and the UUID
        trackfilename = trackfilename +  "/" + "Tracks.gpx" ;
        PoiCollection tracks ;
        tracks.setUuid("{6ca95da8-1171-466f-9926-d34bb6e29e63}") ;

        // Parse all gpx files in the tracks folder
        QDir gpxfolder(trackfolder) ;
        QStringList gpxfiles = gpxfolder.entryList(QStringList("*.gpx")) ;
        for (int i=0; i<gpxfiles.length(); i++) {

            PoiCollection track ;
            track.loadGpx(trackfolder + QString("/") + gpxfiles.at(i)) ;

            // Only process non-empty gpx files

            if (track.size()>0 || track.trackSize()>0) {

              // Create a New Waypoint entry, using the first waypoint, or the first track point
              // (pre-requisit is that the fileCollection() is already sorted)

              PoiEntry ent ;

              if (track.size()>0) {
                ent.copyFrom(track.at(0)) ;
              } else {
                ent.setLatLon(track.trackAt(0).lat(), track.trackAt(0).lon()) ;
              }
              ent.setUuid(track.uuid()) ;

              QString name = track.formattedName(true, true, true, true, false, false) ;
              ent.set(PoiEntry::EDITEDTITLE, name) ;
              ent.setSequence(0) ;

              QString typetxt ;
              for (int i=track.rating(); i>0; i--) {
                  typetxt = typetxt + QString("★") ;
              }
              ent.set(PoiEntry::EDITEDTYPE, typetxt) ;

              tracks.add(ent) ;

            }

        }

        tracks.saveGpx(trackfilename) ;
        tracks.saveOv2() ;
    }

    return true ;

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
        if (refreshMarkers) {
            refreshMap() ;
            refreshTrackDetails() ;
        }

        // Centre and zoom
        if (centreOnMarker) updateMapSelection(zoom) ;

        // Update Lists and Edit Box

        // Edit Details
        ui->groupBox_Details->setEnabled(false) ;
        ui->groupBox_Details->setVisible(false) ;

        // Photo
        ui->groupBox_PhotoOlc->setEnabled(false) ;
        ui->groupBox_PhotoOlc->setVisible(false) ;

        // Track Details
        ui->groupBox_TrackDetails->setEnabled(false) ;
        ui->groupBox_TrackDetails->setVisible(false) ;

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
            ui->groupBox_PhotoOlc->setVisible(true) ;
            ui->groupBox_PhotoOlc->setEnabled(true) ;

        } else if (updateListSelection(&fileCollection, ui->listFile)) {

            updateForm() ;
            ui->groupBox_Details->setVisible(true) ;
            ui->btnEditFile->setEnabled(true) ;
            ui->btnCopyToClipboard->setEnabled(true) ;
            ui->groupBox_PhotoOlc->setVisible(true) ;
            ui->groupBox_PhotoOlc->setEnabled(true) ;
            if (ui->action_ShowTrack->isChecked()) {
                ui->btnDown->setEnabled(true) ;
                ui->btnUp->setEnabled(true) ;
            }
        } else if (fileCollection.findTrack(thisUuid).isValid()) {
            TrackEntry& te = fileCollection.findTrack(thisUuid) ;
            ui->groupBox_TrackDetails->setEnabled(true) ;
            ui->groupBox_TrackDetails->setVisible(true) ;
            ui->label_TrackPointDateTIme->setText(te.date().toString()) ;
            ui->label_TrackPointLat->setText(QString::number(te.lon(),'f',8)) ;
            ui->label_TrackPointLon->setText(QString::number(te.lat(),'f',8)) ;
            ui->label_TrackPointAlt->setText(QString::number(te.elev(),'f',2)) ;
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
        ui->actionUndo->setEnabled(undo.undoable()) ;

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
    QString garminfromfolder = configuration->garminFromFolder() ;
    bool garminfromexists = false ;
    if (!garminfromfolder.isEmpty()) {
        QDir garminfrom(garminfromfolder) ;
        garminfromexists = garminfrom.isReadable() ;
    }
    ui->actionTransfer_to_Garmin->setEnabled(garminexists) ;
    ui->actionTransfer_from_Garmin->setEnabled(garminexists||garminfromexists) ;
    ui->action_UnmountGarminDevice->setEnabled(garminexists||garminfromexists) ;
}

bool MainWindow::updateMapSelection(int zoom)
{
    ui->mapWebView->selectMarker(thisUuid) ;
    if (!thisUuid.isEmpty()) ui->mapWebView->seekToMarker(thisUuid, zoom) ;
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
        ui->labelImage->setPixmap(nullpixmap);
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

        ui->label_olc->setText(pe.olc()) ;

        if (pe.get(PoiEntry::GEOCODED).compare("yes")==0) {
            ui->label_MarkerPin->setVisible(true) ;
            ui->label_MarkerPinError->setVisible(false) ;
        } else {
            ui->label_MarkerPin->setVisible(false) ;
            ui->label_MarkerPinError->setVisible(true) ;
        }

        if (!pe.pixmap().isNull()) {
            ui->labelImage->setPixmap(pe.pixmap());
            ui->groupBox_PhotoOlc->setEnabled(true);
        } else {
            ui->labelImage->setPixmap(nullpixmap);
            ui->groupBox_PhotoOlc->setEnabled(false);
        }

        return true ;

    }
}

bool MainWindow::updateSearchFilter()
{
        QStringList filterEntries ;

        for (int i=0; i<fileCollection.size(); i++) {
            QRegularExpression re("[^a-zA-Z0-9★]") ;
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

        // Get current text
        QString currentText = ui->comboBox_Filter->currentText() ;

        ui->comboBox_Filter->clear() ;
        ui->comboBox_Filter->addItem(" -- Show All -- ") ;
        ui->comboBox_Filter->addItem(" -- Undefined -- ") ;
/*
        ui->comboBox_Filter->addItem(QString(STAR)) ;
        ui->comboBox_Filter->addItem(QString(STAR)+QString(STAR)) ;
        ui->comboBox_Filter->addItem(QString(STAR)+QString(STAR)+QString(STAR)) ;
*/
        for (int i=0; i<filterEntries.size(); i++) {
            QString ent = filterEntries.at(i) ;
            ui->comboBox_Filter->addItem(ent);
        }

        // Search for current text and re-set index
        int idx = ui->comboBox_Filter->findText(currentText) ;
        if (idx>=0) ui->comboBox_Filter->setCurrentIndex(idx) ;

        ui->comboBox_Filter->blockSignals(false) ;

        return true ;
}

bool MainWindow::updateList(PoiCollection *collection, QListWidget *widget, QString filterText)
{
    bool isstar = (filterText.size()>0 && filterText.at(0) == STAR) ;
    bool selectall = filterText == QString(" -- Show All -- ") ;
    bool selectempty = filterText == QString(" -- Undefined -- ") ;

    if (collection==NULL || widget==NULL) return false ;

    widget->blockSignals(true) ;
    widget->clear() ;

    filterText = filterText.toLower() ;

    // Populate List
    int size = collection->size() ;
    for (int i=0; i<size; i++) {

        QString type = collection->at(i).get(PoiEntry::EDITEDTYPE).toLower() ;

        if (selectall || (selectempty && type.isEmpty()) ||
                (isstar && type.compare(filterText)==0) || (!isstar && type.contains(filterText))) {

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

bool MainWindow::refreshTrackDetails()
{
  fileCollection.calculateTrack() ;
  ui->label_distance->setText(QString::number(fileCollection.trackLength()/1000,'f',1)+QString(" km")) ;
  ui->label_climb->setText(QString::number(fileCollection.heightGain(),'f',0)+QString(" m / ")+QString::number(fileCollection.heightLoss(),'f',0)+QString(" m")) ;
  return true ;
}

bool MainWindow::refreshMap()
{

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

    ui->mapWebView->removeAllMarkers() ;
    for (int i=0, ni=workingCollection.size(); i<ni; i++) {
        PoiEntry& ent = workingCollection.at(i) ;
        ui->mapWebView->setMarker(ent.uuid(),
          workingCollection.uuid(),
          ent.lat(),
          ent.lon(),
          ent.get(PoiEntry::EDITEDTITLE),
          ent.sequence()) ;
    }
    for (int i=0, ni=fileCollection.size(); i<ni; i++) {
        PoiEntry& ent = fileCollection.at(i) ;
        ui->mapWebView->setMarker(ent.uuid(),
          fileCollection.uuid(),
          ent.lat(),
          ent.lon(),
          ent.get(PoiEntry::EDITEDTITLE),
          ent.sequence()) ;
    }
    for (int i=0, ni=fileCollection.trackSize(); i<ni; i++) {
        TrackEntry& ent = fileCollection.trackAt(i) ;
        ui->mapWebView->setMarker(ent.uuid(),
          fileCollection.trackUuid(),
          ent.lat(),
          ent.lon(),
          QString(""),
          ent.sequence()) ;
    }
    ui->mapWebView->selectMarker(thisUuid);
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



void MainWindow::on_action_ShowTrack_triggered()
{
    refresh(false) ;
}


void MainWindow::on_button_copyOlc_clicked()
{
    QGuiApplication::clipboard()->setText(ui->label_olc->text()) ;
}


void MainWindow::on_actionSync_with_Google_triggered()
{
    doSyncWithGoogle() ;
}



void MainWindow::on_action_Purge_All_Google_Entries_triggered()
{
    doPurgeAllGoogleEntries() ;
}

