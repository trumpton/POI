//////////////////////////////////////////////////////////////////////////
//
// mainwindow_menu.cpp
//
//

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "urls.h"
#include "version.h"



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


void MainWindow::on_action_LaunchTomTom_triggered()
{

    QDesktopServices::openUrl(QString(TOMTOMURL)) ;
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
    QString filename = QFileDialog::getOpenFileName(this, QString("Load File"), QString(""), QString("GPX Files (*.gpx)")) ;

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
    refresh(true) ;
}


void MainWindow::on_btnUp_clicked()
{
    fileCollection.sort() ;
    PoiEntry& thisEnt = fileCollection.find(thisUuid) ;
    PoiEntry& lastEnt = fileCollection.findPrev(thisUuid) ;
    if (thisEnt.isValid() && lastEnt.isValid()) {
        int seq = thisEnt.sequence() ;
        thisEnt.setSequence(lastEnt.sequence());
        lastEnt.setSequence(seq) ;
        fileCollection.sort() ;
        refresh(true) ;
    }
}

void MainWindow::on_btnDown_clicked()
{
    fileCollection.sort() ;
    PoiEntry& thisEnt = fileCollection.find(thisUuid) ;
    PoiEntry& nextEnt = fileCollection.findNext(thisUuid) ;
    if (thisEnt.isValid() && nextEnt.isValid()) {
        int seq = thisEnt.sequence() ;
        thisEnt.setSequence(nextEnt.sequence());
        nextEnt.setSequence(seq) ;
        fileCollection.sort() ;
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
    fileCollection.sort() ;

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
        fileCollection.sort() ;
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
