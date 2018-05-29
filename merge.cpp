#include "merge.h"
#include "ui_merge.h"

Merge::Merge(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Merge)
{
    ui->setupUi(this);
}

Merge::~Merge()
{
    delete ui;
}

void Merge::clear()
{
    ui->comboBox_Files->clear() ;
    selected.clear() ;
}

void Merge::addItem(QString filename, QString filepath)
{
    ui->comboBox_Files->addItem(filename, filepath) ;
}

QString& Merge::currentData()
{
    return selected ;
}

void Merge::on_comboBox_Files_currentIndexChanged(int index)
{
    if (index<0) selected = QString("") ;
    else selected = ui->comboBox_Files->itemData(index, Qt::UserRole).toString() ;
}
