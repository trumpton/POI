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
    QDir::setCurrent(configFolder()) ;
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

QString& Configuration::configFolder()
{
    sFileName = settings->value("folder").toString() ;
    return sFileName ;
}

int Configuration::exec()
{
    ui->confKey->setText(key()) ;
    ui->confWorkingFolder->setText(configFolder()) ;
    int i=QDialog::exec() ;
    if (i==QDialog::Accepted) {
        settings->setValue("key", ui->confKey->text()) ;
        settings->setValue("folder", ui->confWorkingFolder->text()) ;
        QDir::setCurrent(configFolder()) ;
    }
    return i ;
}

//
// Form Controls
//

void Configuration::on_pushButton_clicked()
{
    QString results = QFileDialog::getExistingDirectory(0,
                QString("Working Directory"),
                configFolder(),
                QFileDialog::ShowDirsOnly) ;
    ui->confWorkingFolder->setText(results) ;
    QDir::setCurrent(results) ;
}

