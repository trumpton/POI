#include "configuration.h"
#include "ui_configuration.h"

#include <QFileDialog>
#include <QDir>

Configuration::Configuration(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Configuration)
{
    ui->setupUi(this);
    settings = new QSettings("trumpton.uk", "Poi");
    QDir::setCurrent(poiFolder()) ;
}

Configuration::~Configuration()
{
    delete settings ;
    delete ui ;
}

QString& Configuration::key()
{
    sKey = settings->value("key").toString() ;
    return sKey ;
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
    ui->confKey->setText(key()) ;
    ui->confPoiFolder->setText(poiFolder()) ;
    ui->confTracksFolder->setText(tracksFolder());
    ui->confImportFolder->setText(importFolder()) ;
    ui->confGarminFolder->setText(garminFolder()) ;
    if (importMove()) {
        ui->radioButton_moveData->setChecked(true) ;
    } else {
        ui->radioButton_copyData->setChecked(true) ;
    }

    int i=QDialog::exec() ;
    if (i==QDialog::Accepted) {
        settings->setValue("key", ui->confKey->text()) ;
        settings->setValue("folder", ui->confPoiFolder->text()) ;
        settings->setValue("garmin", ui->confGarminFolder->text()) ;
        settings->setValue("tracks", ui->confTracksFolder->text()) ;
        settings->setValue("import", ui->confImportFolder->text()) ;
        settings->setValue("filter", ui->importFilter->text()) ;
        settings->setValue("move", ui->radioButton_moveData->isChecked()) ;

        QDir::setCurrent(poiFolder()) ;
    }
    return i ;
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
