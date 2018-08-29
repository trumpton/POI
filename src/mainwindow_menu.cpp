//////////////////////////////////////////////////////////////////////////
//
// mainwindow_menu.cpp
//
//

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "urls.h"
#include "version.h"
#include "easyexif/exif.h"


//////////////////////////////////////////////////////////////////////////
//
// File List Functions
//

void MainWindow::on_action_New_triggered()
{
    saveCollection(false) ;
    fileCollection.clear() ;
    ui->action_ShowTrack->setChecked(false) ;
    ui->action_ShowActualDuration->setChecked(false) ;
    ui->lineEdit_fileTitle->setText(fileCollection.name()) ;

    // Register new UUIDs
    ui->googlemapsWebView->registerUuids(workingCollection.uuid(), fileCollection.uuid(), fileCollection.trackUuid()) ;
    refresh(true) ;
}


void MainWindow::on_action_Save_triggered()
{
    if (fileCollection.filename().isEmpty()) {
        on_action_SaveAs_triggered();
    } else {
        saveCollection(true) ;
        refresh() ;
    }
}

void MainWindow::on_action_SaveAs_triggered()
{
    fileCollection.setFilename(QString(""));
    fileCollection.markAsDirty() ;
    saveCollection(true) ;
}


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


void MainWindow::on_action_EditAll_triggered()
{
    while (fileCollection.size()>0) {
        workingCollection.add(fileCollection.at(0)) ;
        fileCollection.remove(fileCollection.at(0).uuid()) ;
    }
    refresh(true) ;
}


void MainWindow::on_action_EmptyClipboard_triggered()
{
    while (workingCollection.size()>0) {
        workingCollection.remove(workingCollection.at(0).uuid()) ;
    }
    refresh(true) ;
}

// Undo currently handles moves only
void MainWindow::on_actionUndo_triggered()
{
    switch (undo.nextType()) {
        case Undo::PoiRecord: {
                PoiEntry& extract = undo.popPoiEntry() ;
                PoiEntry& searchFileCollection = fileCollection.find(extract.uuid()) ;
                if (searchFileCollection.isValid()) {
                    searchFileCollection.setLatLon(extract.lat(), extract.lon()) ;
                }
                PoiEntry& searchWorkingCollection = workingCollection.find(extract.uuid()) ;
                if (searchWorkingCollection.isValid()) {
                    searchWorkingCollection.setLatLon(extract.lat(), extract.lon());
                }
            }
            refresh(true) ;
            break ;
        case Undo::TrackRecord: {
                TrackEntry& extract = undo.popTrackEntry() ;
                TrackEntry& searchFileCollection = fileCollection.findTrack(extract.uuid()) ;
                if (searchFileCollection.isValid()) {
                    searchFileCollection.setLatLon(extract.lat(), extract.lon());
                }
            }
            refresh(true) ;
            break ;
    }
}


//////////////////////////////////////////////////////////////////////////
//
// Import Menu
//

#include <QIODevice>

easyexif::EXIFInfo getExif(QString fileName)
{
    easyexif::EXIFInfo info;
    QFile CurrentFile(fileName);
    if(!CurrentFile.open(QIODevice::ReadOnly)) return info;
    QByteArray DataFile = CurrentFile.readAll();
    info.parseFrom(DataFile.toStdString()) ;
    return info ;
}

// Import Photo
void MainWindow::on_action_Photo_triggered()
{
    bool success=true ;

    // prompt for photo
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
        tr("Select Photo"), configuration->imageFolder(), tr("JPG Files (*.jpg *.JPG *.png *.PNG)"));
    if (fileNames.isEmpty()) return ;

    for (int i=0; i<fileNames.count(); i++) {

        // create new poientry
        PoiEntry ent ;

        QImage img ;

        if (!img.load(fileNames.at(i))) {

            success = false ;

        } else {

            ent.setImage(img) ;
            ent.setSequence(9999) ;

            QFileInfo fileinfo(fileNames.at(i)) ;
            ent.set(PoiEntry::EDITEDTITLE, fileinfo.baseName());

            // parse exif / tracks and store information
            easyexif::EXIFInfo inf = getExif(fileNames.at(i)) ;
            QString date = QString(inf.DateTime.data()) ;
            double lat = inf.GeoLocation.Latitude ;
            double lon = inf.GeoLocation.Longitude ;
            double alt = inf.GeoLocation.Altitude ;
            ent.set(PoiEntry::DATETIME, date) ;
            ent.set(PoiEntry::PHOTODATE, date) ;
            ent.set(PoiEntry::PHOTOFILENAME, fileNames.at(i)) ;
            ent.set(PoiEntry::PHOTOLAT, QString("%1").arg(lat)) ;
            ent.set(PoiEntry::PHOTOLON, QString("%1").arg(lon)) ;
            ent.set(PoiEntry::PHOTOELEVATION, QString("%1").arg(alt)) ;
            ent.setDate(date) ;

            // If lat/lon not set, search for lat/lon using date in tracks
            // TODO

            // If lat/lon still not set, place it in screen centre
            if (lat==0.0 && lon==0.0) {
                lat = ui->googlemapsWebView->getLat() ;
                lon = ui->googlemapsWebView->getLon() ;
            }

            ent.setLatLon(lat, lon) ;

            // add waypoint to the list with title of the photo
            // and re-sort so newest files end up at the end of the list
            fileCollection.add(ent) ;
            fileCollection.sortBySequence();
        }
    }

    refresh(true) ;

    if (!success) {
        // Report, some files failed to load
    }

}


void MainWindow::on_actionResort_Waypoints_By_Date_triggered()
{
    fileCollection.reorderWaypointByDate();
    refresh(true) ;
}


// Import GPX
void MainWindow::on_action_ImportGpx_triggered()
{
    // Ask for the name of the input file
    QString importName = QFileDialog::getOpenFileName(this,
        tr("Import Garmin"), "", tr("GPX Files (*.gpx)"));
    if (importName.isEmpty()) return ;

    // Import the file
    PoiCollection gpx ;
    bool cleanload = gpx.loadGpx(importName) ;
    if (gpx.size()==0) {

        QMessageBox::information(this, "POI", "The import failed - no points of interest found", QMessageBox::Ok);
        return ;

    } else {

        if (!cleanload) {
            QMessageBox::information(this, "POI", "The import only managed to partially load", QMessageBox::Ok);
        }

        int idx = fileCollection.size() ;
        int seq = 0 ;
        if (idx>0) seq=fileCollection.at(idx-1).sequence()+2 ;

        for (int i=0; i<gpx.size(); i++) {
            PoiEntry& ent = gpx.at(i) ;
            ent.setSequence(seq+i*2) ;
            fileCollection.add(ent) ;
        }

        refresh(true) ;
    }
}

//
// Import OV2
//

void MainWindow::on_action_ImportOv2_triggered()
{
    // Ask for the name of the input file
    QString importName = QFileDialog::getOpenFileName(this,
        tr("Import Tom Tom OV2 File"), "", tr("OV2 Files (*.ov2)"));
    if (importName.isEmpty()) return ;

    // Import the file
    bool cleanload = fileCollection.importOv2(importName) ;
    if (fileCollection.size()==0) {

        QMessageBox::information(this, "POI", "The import failed - no points of interest found") ;
        return ;

    } else {

        if (!cleanload) {
            // Prompt for load, else exit
            if (QMessageBox::question(this, "POI", "The file is corrupt, and can only be partially loaded.  Do you wish to continue?", QMessageBox::Yes|QMessageBox::No)!=QMessageBox::Yes) {
                return ;
            }
        }

        // Re-load (so that the input is marked as dirty)
        fileCollection.clear() ;
        fileCollection.importOv2(importName) ;

        refresh(true) ;

    }

}

//
// Import GPX Route to Clipboard
//
void MainWindow::on_action_ImportGpxToClipboard_triggered()
{
    // Ask for the name of the input file
    QString importName = QFileDialog::getOpenFileName(this,
        tr("Import Garmin"), "", tr("GPX Files (*.gpx)"));
    if (importName.isEmpty()) return ;

    // Import the file
    PoiCollection gpx ;
    bool cleanload = gpx.loadGpx(importName) ;
    if (gpx.size()==0) {

        QMessageBox::information(this, "POI", "The import failed - no points of interest found", QMessageBox::Ok);
        return ;

    } else {

        if (!cleanload) {
            QMessageBox::information(this, "POI", "The import only managed to partially load", QMessageBox::Ok);
        }

        for (int i=0; i<gpx.size(); i++) {
            PoiEntry& ent = gpx.at(i) ;
            workingCollection.add(ent) ;
        }

        refresh(true) ;

    }
}

void MainWindow::on_action_Open_triggered()
{
    QString folder = configuration->openFolder() ;
    QString filename = QFileDialog::getOpenFileName(this, QString("Load File"), folder, QString("GPX Files (*.gpx)")) ;

    saveCollection() ;
    fileCollection.clear() ;
    ui->action_ShowActualDuration->setChecked(false) ;

    if (!filename.isEmpty()) {
        if (!fileCollection.loadGpx(filename)) {
            thisCollectionUuid.clear() ;
        } else {

            thisCollectionUuid = fileCollection.uuid() ;
            bool hastracks = fileCollection.trackSize()>0 ;
            ui->action_ShowTrack->setChecked(hastracks) ;

            QFileInfo fileinfo(filename) ;
            configuration->setOpenFolder(fileinfo.absolutePath());

            if (fileCollection.name().isEmpty()) fileCollection.setName(fileinfo.baseName().replace(".gpx","")) ;
            ui->lineEdit_fileTitle->setText(fileCollection.name()) ;

            // Add an entry if the file has no waypoints
            if (fileCollection.size()==0 && fileCollection.trackSize()>0) {
                PoiEntry anchor ;
                anchor.setLatLon(fileCollection.trackAt(0).lat(), fileCollection.trackAt(0).lon()) ;
                anchor.setSequence(0) ;
                anchor.set(PoiEntry::EDITEDTITLE, fileCollection.name()) ;
                anchor.set(PoiEntry::EDITEDDESCR, fileCollection.name()) ;
                fileCollection.add(anchor) ;
            }
        }
    }
    // Register New Collection UUIDs
    ui->googlemapsWebView->registerUuids(workingCollection.uuid(), fileCollection.uuid(), fileCollection.trackUuid()) ;
    refresh(true) ;
}


void MainWindow::on_btnUp_clicked()
{
    fileCollection.sortBySequence(); ;
    PoiEntry& thisEnt = fileCollection.find(thisUuid) ;
    PoiEntry& lastEnt = fileCollection.findPrev(thisUuid) ;
    if (thisEnt.isValid() && lastEnt.isValid()) {
        int seq = thisEnt.sequence() ;
        thisEnt.setSequence(lastEnt.sequence());
        lastEnt.setSequence(seq) ;
        fileCollection.sortBySequence(); ;
        refresh(true) ;
    }
}

void MainWindow::on_btnDown_clicked()
{
    fileCollection.sortBySequence(); ;
    PoiEntry& thisEnt = fileCollection.find(thisUuid) ;
    PoiEntry& nextEnt = fileCollection.findNext(thisUuid) ;
    if (thisEnt.isValid() && nextEnt.isValid()) {
        int seq = thisEnt.sequence() ;
        thisEnt.setSequence(nextEnt.sequence());
        nextEnt.setSequence(seq) ;
        fileCollection.sortBySequence(); ;
        refresh(true) ;
    }
}

// Track Management


void MainWindow::on_action_ShowTrack_toggled(bool arg1)
{
    refresh(true) ;
}


void MainWindow::on_NewTrackPoint_clicked()
{
    if (thisCollectionUuid.compare(fileCollection.trackUuid())!=0) {
        QMessageBox::information(this, QString("Create Track Point"), QString("A track point must be selected."), QMessageBox::Ok);
        return ;
    }

    // TODO: The file collection should already be sorted
    fileCollection.sortBySequence(); ;

    TrackEntry& thisent = fileCollection.findTrack(thisUuid) ;
    TrackEntry& nextent = fileCollection.findNextTrack(thisUuid) ;

    if (!nextent.isValid()) {
        QMessageBox::information(this, QString("POI"), QString("The track point must not be the last one."), QMessageBox::Ok);
    } else {
        TrackEntry newpoint ;
        double lat = thisent.lat() + (nextent.lat() - thisent.lat()) / 2 ;
        double lon = thisent.lon() + (nextent.lon() - thisent.lon()) / 2 ;
        int seq = thisent.sequence() ;
        newpoint.set(lat, lon, 0, QDateTime()) ;
        newpoint.setSequence(seq+1);
        fileCollection.add(newpoint) ;
        thisUuid = newpoint.uuid() ;
        fileCollection.sortBySequence(); ;
        refresh(true) ;
    }
}

void MainWindow::on_DeleteTrackPoint_clicked()
{
    if (thisCollectionUuid.compare(fileCollection.trackUuid())!=0) return ;

    TrackEntry& nextentry = fileCollection.findNextTrack(thisUuid) ;
    fileCollection.removeTrack(thisUuid) ;
    if (fileCollection.trackSize()==1) {
        QMessageBox::information(this, QString("POI"), QString("Removing all remaining points"), QMessageBox::Ok);
        fileCollection.removeTrack(fileCollection.trackAt(0).uuid()) ;
        // No more track points available
        thisCollectionUuid="" ;
        thisUuid="" ;
    } else {
        if (nextentry.isValid()) {
            // Select next track point
            thisUuid = nextentry.uuid() ;
        } else {
            // Track point was last, can't select next
            thisCollectionUuid="" ;
            thisUuid="" ;
        }
    }

    refresh(true) ;
}

void MainWindow::on_action_CreateTrackFromWaypoints_triggered()
{
    if (fileCollection.trackSize()>0 || fileCollection.size()<2) {
        QMessageBox::information(this, QString("POI"), QString("At least two waypoints, and no existing track points must exist."), QMessageBox::Ok);
        return ;
    }
    for (int i=0; i<fileCollection.size(); i++) {
        TrackEntry newent ;
        PoiEntry& ent = fileCollection.at(i) ;
        newent.setLatLon(ent.lat(), ent.lon());
        newent.setSequence((fileCollection.size()+i)*2) ;
        fileCollection.add(newent) ;
    }
    refresh(true) ;
}

void MainWindow::on_action_ReduceTrackPoints_triggered()
{
    if (fileCollection.trackSize()<10) {
        QMessageBox::information(this, QString("POI"), QString("Reduction aborted.  There are fewer than 10 track points."), QMessageBox::Ok) ;
        return ;
    }
    int i=1 ;
    do {
        TrackEntry& ent = fileCollection.trackAt(i) ;
        bool neartowaypoint=false ;
        for (int j=fileCollection.size()-1; j>=0; j--) {
            if (ent.distanceFrom(fileCollection.at(j))<5) neartowaypoint=true ;
        }
        if (!neartowaypoint) {
            fileCollection.removeTrack(ent.uuid()) ;
        }
        i++ ;
    } while (i<fileCollection.trackSize()-1) ;
    refresh(true) ;
}


//////////////////////////////////////////////////////////////////////////
//
// Search Menu
//

void MainWindow::on_action_FindLocation_triggered()
{
    bool ok;
    searchtext = QInputDialog::getText(0, "POI","Search For", QLineEdit::Normal,"", &ok);
    if (!searchtext.isEmpty()) ui->googlemapsWebView->searchLocation(searchtext) ;
}



//////////////////////////////////////////////////////////////////////////
//
// About Menu
//

void MainWindow::on_action_About_POI_triggered()
{
    QMessageBox::information(this, QString("POI"), QString("Version ") + QString(POIVERSION) + QString(". Build ") + QString(POIBUILD), QMessageBox::Ok) ;
}





void MainWindow::on_action_ShowActualDuration_triggered()
{
    refresh(true) ;
}


//////////////////////////////////////////////////////////////////////////
//
// Transfer Menu
//

void MainWindow::on_action_LaunchTomTom_triggered()
{

    QDesktopServices::openUrl(QString(TOMTOMURL)) ;
}


void MainWindow::on_actionTransfer_to_Garmin_triggered()
{
    QDir garmin(configuration->garminFolder()) ;
    QDir tracks(configuration->tracksFolder()) ;

    if (!garmin.isReadable()) {
        QMessageBox::information(this, QString("POI"), QString("Unable to access Garmin device.  Is it connected, and is the folder configured in setup?"), QMessageBox::Ok) ;
        return ;
    }

    if (!tracks.isReadable()) {
        QMessageBox::information(this, QString("POI"), QString("Unable access the tracks folder.  Check it is correctly setup"), QMessageBox::Ok) ;
        return ;
    }

    tracks.setNameFilters(QStringList()<<"*.gpx");
    QStringList fileList = tracks.entryList();

    QProgressDialog dlg(QString("POI"), QString("Abort"), 0, fileList.size(), this) ;
    dlg.setWindowModality(Qt::WindowModal);
    dlg.show() ;

    bool success = true ;

    for (int i=0; success && i<fileList.size() ; i++) {

        dlg.setValue(i) ;
        QApplication::processEvents() ;

        QString src = configuration->tracksFolder() + QDir::separator() + fileList.at(i) ;
        QString dst = configuration->garminFolder() + QDir::separator() + fileList.at(i) ;

        if (QFile::exists(dst)) { QFile::remove(dst); }

        success &= QFile::copy(src, dst) ;

        if (!success) {
            QMessageBox::information(this, QString("POI"), QString("Error Transferring ") + fileList.at(i), QMessageBox::Ok) ;
        }

        if (dlg.wasCanceled()) {
            QMessageBox::information(this, QString("POI"), QString("Transfer Aborted"), QMessageBox::Ok) ;
            success=false ;
        }

    }

#ifndef MSWINDOWS
    system("/bin/sync") ;
#endif

    dlg.setValue(fileList.size());
    dlg.hide() ;

    if (success) {
        QMessageBox::information(this, QString("POI"), QString("Transfer Complete. Ensure that the GPS is ejected before unplugging the cable."), QMessageBox::Ok) ;
    }

}


void MainWindow::on_actionTransfer_from_Garmin_triggered()
{
    QDir garmin(configuration->garminFolder()) ;
    QDir import(configuration->importFolder()) ;

    if (!garmin.isReadable()) {
        QMessageBox::information(this, QString("POI"), QString("Unable to access Garmin device.  Is it connected, and is the folder configured in setup?"), QMessageBox::Ok) ;
        return ;
    }

    if (!import.isReadable()) {
        QMessageBox::information(this, QString("POI"), QString("Unable access the import folder.  Check it is correctly setup"), QMessageBox::Ok) ;
        return ;
    }

    garmin.setNameFilters(QStringList()<< configuration->importFilter());
    QStringList fileList = garmin.entryList();

    QProgressDialog dlg(QString("POI"), QString("Abort"), 0, fileList.size(), this) ;
    dlg.setWindowModality(Qt::WindowModal);
    dlg.show() ;
    QApplication::processEvents() ;

    bool success = true ;

    for (int i=0; success && i<fileList.size() ; i++) {

        dlg.setValue(i) ;
        QApplication::processEvents() ;

        QString src = configuration->garminFolder() + QDir::separator() + fileList.at(i) ;
        QString dst = configuration->importFolder() + QDir::separator() + fileList.at(i) ;

        if (QFile::exists(dst)) { QFile::remove(dst); }

        success &= QFile::copy(src, dst) ;

        if (success) {
            if (configuration->importMove()) {
                garmin.remove(src) ;
            }
        } else {
            QMessageBox::information(this, QString("POI"), QString("Error Transferring ") + fileList.at(i), QMessageBox::Ok) ;
        }

        if (dlg.wasCanceled()) {
            QMessageBox::information(this, QString("POI"), QString("Transfer Aborted"), QMessageBox::Ok) ;
            success=false ;
        }
    }

#ifndef MSWINDOWS
    system("/bin/sync") ;
#endif

    QApplication::processEvents() ;
    dlg.setValue(fileList.size());
    dlg.hide() ;

    if (success) {
        QMessageBox::information(this, QString("POI"), QString("Transfer Complete."), QMessageBox::Ok) ;
    }

}
