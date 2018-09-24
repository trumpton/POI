#include "configuration.h"
#include "ui_configuration.h"

#include <QFileDialog>
#include <QDir>
#include "apikeys.h"


Configuration::Configuration(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Configuration)
{
    ui->setupUi(this);
    settings = new QSettings("trumpton.uk", "Poi");
#ifdef _WINDOWS_
    QString inifilename = qApp->applicationFilePath().replace(".EXE",".ini").replace(".exe",".ini") ;
#else
    QString inifilename = qApp->applicationFilePath() + QString(".ini") ;
#endif
    filesettings = new QSettings(inifilename, QSettings::IniFormat) ;

    QString bing = filesettings->value("keys/bing", "Unknown").toString();

    QDir::setCurrent(poiFolder()) ;
}

Configuration::~Configuration()
{
    delete settings ;
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
    if (importMove()) {
        ui->radioButton_moveData->setChecked(true) ;
    } else {
        ui->radioButton_copyData->setChecked(true) ;
    }

    int i=QDialog::exec() ;
    if (i==QDialog::Accepted) {
        settings->setValue("folder", ui->confPoiFolder->text()) ;
        settings->setValue("garmin", ui->confGarminFolder->text()) ;
        settings->setValue("tracks", ui->confTracksFolder->text()) ;
        settings->setValue("import", ui->confImportFolder->text()) ;
        settings->setValue("image", ui->confImageFolder->text()) ;
        settings->setValue("filter", ui->importFilter->text()) ;
        settings->setValue("move", ui->radioButton_moveData->isChecked()) ;

        QDir::setCurrent(poiFolder()) ;
    }
    return i ;
}

//
//
//

int Configuration::aerialTileZoom()
{
    return filesettings->value("satellite/zoom",ZOOM_AERIAL).toInt() ;
}

int Configuration::satelliteOverlayZoom()
{
    return filesettings->value("satelliteoverlay/zoom",ZOOM_OVERLAY).toInt() ;
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
    return filesettings->value("satellite/url",
        QString(TILE_AERIAL)).toString() ;
}

QString Configuration::satelliteOverlayUrl()
{
    return filesettings->value("satelliteoverlay/url",
        QString(TILE_OVERLAY)).toString() ;
}

QString Configuration::geocodeType()
{
    return filesettings->value("geocode/type",
        QString("OSM")).toString().toLower() ;
}

QString Configuration::bingKey()
{
    return filesettings->value("keys/bing",
        QString(BINGKEY)).toString() ;
}

QString Configuration::googleKey()
{
    return filesettings->value("keys/google",
        QString(GOOGLEKEY)).toString();
}

QString Configuration::hereId()
{
    return filesettings->value("keys/hereid",
        QString(HEREID)).toString() ;
}

QString Configuration::hereCode()
{
    return filesettings->value("keys/herecode",
        QString(HERECODE)).toString() ;
}

QString Configuration::mapTileUrl()
{
    return filesettings->value("map/url",
        QString(TILE_MAP)).toString() ;
}

QString Configuration::contourTileUrl()
{
    return filesettings->value("contour/url",
        QString(TILE_CONTOUR)).toString() ;
}

QString Configuration::trailTileUrl()
{
    return filesettings->value("trail/url",
       QString(TILE_TRAIL)).toString() ;
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
                QString("Garmin Device"),
                garminFolder(),
                QFileDialog::ShowDirsOnly) ;
    ui->confGarminFolder->setText(results) ;
    sGarmin = results ;
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
