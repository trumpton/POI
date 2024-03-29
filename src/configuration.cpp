/*
 * Application: POI Manager
 * File: configuration.cpp
 *
 * Supports the management of the POI.ini file and the configuration
 * parameters stored in the user's profile.  Manages the configuration
 * pop-up dialog.
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

#include "configuration.h"
#include "ui_configuration.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

#define AUTHSCOPE "https://www.googleapis.com/auth/contacts"

Configuration::Configuration(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Configuration)
{
    ui->setupUi(this);

    // Configuration (non-ini) Settings
    settings = new QSettings("trumpton.uk", "Poi");

    // Load settings from ini file
    filesettings=NULL ;
    reloadIni() ;
    QDir::setCurrent(poiFolder()) ;

    // Configure Google Access
    googleaccess = new GoogleAccess(googleOauth2Id(),AUTHSCOPE,googleOauth2Secret()) ;

}

void Configuration::reloadIni() {

    // Load keys ini file from designated folder

    if (filesettings) delete filesettings ;

    if (QDir(iniFolder()).exists(INIFILE)) {
        // INI file exists, so load it
        filesettings = new QSettings(iniFolder() + QString("/") + QString(INIFILE),
                                     QSettings::IniFormat) ;
    } else {
        // Not found, so prompt TODO
        filesettings = new QSettings() ;
    }
}

Configuration::~Configuration()
{
    if (filesettings) delete filesettings ;
    if (settings) delete settings ;
    delete ui ;
}

QString& Configuration::poiFolder()
{
    sFileName = settings->value("folder").toString() ;
    return sFileName ;
}

QString& Configuration::garminFolder()
{
    sGarmin = settings->value("garmin").toString() ;
    return sGarmin ;
}

QString& Configuration::garminFromFolder()
{
    sGarminFrom = settings->value("garminfrom").toString() ;
    return sGarminFrom ;
}

QString& Configuration::tracksFolder()
{
    sTracks = settings->value("tracks").toString() ;
    return sTracks ;
}

QString& Configuration::importFolder()
{
    sImport = settings->value("import").toString() ;
    return sImport ;
}

QString& Configuration::imageFolder()
{
    sImageFolder = settings->value("image").toString() ;
    return sImageFolder ;
}

QString& Configuration::iniFolder()
{
    sIniFolder = settings->value("ini").toString() ;
    return sIniFolder ;
}


QString& Configuration::importFilter()
{
    sFilter = settings->value("filter").toString() ;
    if (sFilter.isEmpty()) { sFilter = QString("Track_*.gpx") ; }
    return sFilter ;
}

bool Configuration::importMove()
{
    bMove = settings->value("move").toBool() ;
    return bMove ;
}

QString& Configuration::openFolder()
{
    sOpenFolder = settings->value("openfolder").toString() ;
    if (sOpenFolder.isEmpty()) sOpenFolder = poiFolder() ;
    return sOpenFolder ;
}

void Configuration::setOpenFolder(QString folder)
{
    settings->setValue("openfolder", folder) ;
}

int Configuration::exec()
{
    ui->confPoiFolder->setText(poiFolder()) ;
    ui->confTracksFolder->setText(tracksFolder());
    ui->confImportFolder->setText(importFolder()) ;
    ui->confImageFolder->setText(imageFolder()) ;
    ui->confGarminFolder->setText(garminFolder()) ;
    ui->confGarminFromFolder->setText(garminFromFolder()) ;
    if (importMove()) {
        ui->radioButton_moveData->setChecked(true) ;
    } else {
        ui->radioButton_copyData->setChecked(true) ;
    }
    ui->confIniFolder->setText(iniFolder()) ;

    // Refresh google account name
    ui->label_google_account->setText(googleaccess->getUsername()) ;

    int i=QDialog::exec() ;
    if (i==QDialog::Accepted) {
        settings->setValue("folder", ui->confPoiFolder->text()) ;
        settings->setValue("garmin", ui->confGarminFolder->text()) ;
        settings->setValue("garminfrom", ui->confGarminFromFolder->text()) ;
        settings->setValue("tracks", ui->confTracksFolder->text()) ;
        settings->setValue("import", ui->confImportFolder->text()) ;
        settings->setValue("image", ui->confImageFolder->text()) ;
        settings->setValue("filter", ui->importFilter->text()) ;
        settings->setValue("move", ui->radioButton_moveData->isChecked()) ;
        settings->setValue("ini", ui->confIniFolder->text()) ;

        QDir::setCurrent(poiFolder()) ;
    }
    return i ;
}

//
//
//

int Configuration::aerialTileZoom()
{
    return filesettings->value("aerial/zoom",ZOOM_AERIAL).toInt() ;
}

int Configuration::satelliteOverlayZoom()
{
    return filesettings->value("aerialoverlay/zoom",ZOOM_OVERLAY).toInt() ;
}

int Configuration::mapTileZoom()
{
    return filesettings->value("map/zoom",ZOOM_MAP).toInt() ;
}

int Configuration::contourTileZoom()
{
    return filesettings->value("contour/zoom",ZOOM_CONTOUR).toInt() ;
}

int Configuration::trailTileZoom()
{
    return filesettings->value("trail/zoom",ZOOM_TRAIL).toInt() ;
}

QString Configuration::aerialTileUrl()
{
    return expandkeys(filesettings->value("aerial/url",
        QString("")).toString()) ;
}

QString Configuration::satelliteOverlayUrl()
{
    return expandkeys(filesettings->value("aerialoverlay/url",
        QString("")).toString()) ;
}

QString Configuration::geocodeType()
{
    return filesettings->value("geocode/type",
        QString("OSM")).toString().toLower() ;
}

QString Configuration::bingKey()
{
    return filesettings->value("keys/bing",
        QString("")).toString() ;
}

QString Configuration::hereId()
{
    return filesettings->value("keys/hereid",
        QString("")).toString() ;
}

QString Configuration::hereCode()
{
    return filesettings->value("keys/herecode",
        QString("")).toString() ;
}

QString Configuration::hereApiKey()
{
    return filesettings->value("keys/hereapikey",
        QString("")).toString() ;
}

QString Configuration::googleOauth2Id()
{
    return filesettings->value("keys/googleoauth2id", QString("")).toString() ;
}

QString Configuration::googleOauth2Secret()
{
    return filesettings->value("keys/googleoauth2secret", QString("")).toString() ;
}

QString Configuration::mapTileUrl()
{
    return expandkeys(filesettings->value("map/url",
        QString("OSM")).toString()) ;
}

QString Configuration::contourTileUrl()
{
    return expandkeys(filesettings->value("contour/url",
        QString("")).toString()) ;
}

QString Configuration::trailTileUrl()
{
    return expandkeys(filesettings->value("trailsoverlay/url",
       QString("")).toString()) ;
}

// TODO: Key Expansion
QString Configuration::expandkeys(QString src)
{
    // for step through keys section
    filesettings->beginGroup("keys");
    foreach (const QString &key, filesettings->childKeys()) {
        QString s = QString("{") + key + QString("}") ;
        QString d = filesettings->value(key).toString();
        src.replace(s,d) ;
    }
    filesettings->endGroup();
    return src ;
}

//
// Form Controls
//

void Configuration::on_pushButton_SearchPoi_clicked()
{
    QString results = QFileDialog::getExistingDirectory(0,
                QString("POI Directory"),
                poiFolder(),
                QFileDialog::ShowDirsOnly) ;
    ui->confPoiFolder->setText(results) ;
    sFileName = results ;
}

void Configuration::on_pushButton_SearchGarmin_clicked()
{
    QString results = QFileDialog::getExistingDirectory(0,
                QString("Garmin Device (Read/Write Folder)"),
                garminFolder(),
                QFileDialog::ShowDirsOnly) ;
    ui->confGarminFolder->setText(results) ;
    sGarmin = results ;
}

void Configuration::on_pushButton_SearchGarminFrom_clicked()
{
    QString results = QFileDialog::getExistingDirectory(0,
                QString("Garmin Device (Read Folder)"),
                garminFromFolder(),
                QFileDialog::ShowDirsOnly) ;
    ui->confGarminFromFolder->setText(results) ;
    sGarminFrom = results ;
}

void Configuration::on_pushButton_SearchTracks_clicked()
{
    QString results = QFileDialog::getExistingDirectory(0,
                QString("Tracks Directory"),
                tracksFolder(),
                QFileDialog::ShowDirsOnly) ;
    ui->confTracksFolder->setText(results) ;
    sTracks = results ;
}

void Configuration::on_pushButton_SearchImport_clicked()
{
    QString results = QFileDialog::getExistingDirectory(0,
                QString("Import Directory"),
                importFolder(),
                QFileDialog::ShowDirsOnly) ;
    ui->confImportFolder->setText(results) ;
    sImport = results ;
}

void Configuration::on_pushButton_ImageFolder_clicked()
{
    QString results = QFileDialog::getExistingDirectory(0,
                QString("Image Directory"),
                imageFolder(),
                QFileDialog::ShowDirsOnly) ;
    ui->confImageFolder->setText(results) ;
    sImageFolder = results ;
}

void Configuration::on_pushButton_IniFolder_clicked()
{

    QString results = QFileDialog::getExistingDirectory(0,
                QString("Folder containing ") + INIFILE,
                iniFolder(),QFileDialog::ReadOnly) ;

    if (!results.isEmpty() && !QDir(results).exists(INIFILE)) {

        // File does not exist - attempt to copy one from the application folder

        QMessageBox::warning(this,
            "POI", QString("A ") + QString(INIFILE) +
                   QString(" configuration file does not exist, attempting to create one..."));

        QString src1=QCoreApplication::applicationDirPath() + QString("/example-") + QString(INIFILE) ;
        QString src2=QCoreApplication::applicationDirPath() + QString("/../share/POI/example-") + QString(INIFILE) ;
        QString dst=QString(results) + QString("/") + QString(INIFILE) ;

        if (QFileInfo::exists(src1) && QFileInfo(src1).isFile()) {
            QFile::copy(src1, dst) ;
        } else if (QFileInfo::exists(src2) && QFileInfo(src2).isFile()) {
            QFile::copy(src2, dst) ;
        } else {
            QMessageBox::warning(this, "POI",
                src1 + QString(" or ") + src2 + QString(" not found, unable to copy file")) ;
            return ;
        }

        if (!QDir(results).exists(INIFILE)) {
            QMessageBox::warning(this,
                "POI", QString("Unable to create file.  Is the folder writeable?"));
        }
    }

    if (QDir(results).exists(INIFILE)) {
        // If file is valid, update config
        ui->confIniFolder->setText(results) ;
        sIniFolder = results ;
    }

    reloadIni() ;

}

bool Configuration::iniFileLoadedOK()
{
    if (filesettings==NULL) return false ;
    if (filesettings->allKeys().count()<=0) return false ;
    return true ;
}


GoogleAccess& Configuration::googleAccess()
{
    return *googleaccess ;
}

void Configuration::on_pushButton_googleAuthorise_clicked()
{
   // Check ini file parameters are present
   if (googleOauth2Id().isEmpty() || googleOauth2Secret().isEmpty()) {
       QMessageBox::warning(this, "POI", QString("Google Access requires the googleoauth2id and googleoauth2secret to be defined in the POI.ini file")) ;
       return ;
   }

   // Re-authorise google account
   if (!googleaccess->Authorise()) {
     QMessageBox::warning(this, "POI", QString("Google Access: ") + googleaccess->getNetworkError()) ;
   }

   // Refresh google account name
   ui->label_google_account->setText(googleaccess->getUsername()) ;
}

