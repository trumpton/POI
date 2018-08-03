//////////////////////////////////////////////////////////////////////////
//
// mainwindow_editdetails.cpp
//
//

#include "mainwindow.h"
#include "ui_mainwindow.h"


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
        pe.set(PoiEntry::EDITEDDOOR, ui->lineEdit_Door->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_Street_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        pe.set(PoiEntry::EDITEDSTREET, ui->lineEdit_Street->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_City_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        pe.set(PoiEntry::EDITEDCITY, ui->lineEdit_City->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_State_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        pe.set(PoiEntry::EDITEDSTATE, ui->lineEdit_State->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_Postcode_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        pe.set(PoiEntry::EDITEDPOSTCODE, ui->lineEdit_Postcode->text()) ;
        refresh() ;
    }
}


void MainWindow::on_lineEdit_Country_editingFinished()
{
    PoiEntry& pe = findEntryByUuid(thisUuid, thisCollectionUuid) ;
    if (pe.isValid()) {
        pe.set(PoiEntry::EDITEDCOUNTRY, ui->lineEdit_Country->text()) ;
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



void MainWindow::on_lineEdit_Door_returnPressed()
{
    ui->lineEdit_Door->editingFinished() ;
    ui->listWorking->setFocus() ;
}

//
//
//

void MainWindow::setLineEditText(QLineEdit *control, PoiEntry& data, PoiEntry::FieldType edited, PoiEntry::FieldType geocoded, QLabel *qledited, QLabel *qlgeo)
{
    QPalette red, black ;

    if (ui->groupBox_Details->isEnabled()) {
        red.setColor(QPalette::Text,Qt::red);
        black.setColor(QPalette::Text,Qt::black);
    }

    if (!data.get(edited).isEmpty()) {
        control->setPalette(red);
        control->setText(data.get(edited)) ;
        qledited->setVisible(true) ;
        qlgeo->setVisible(false) ;
    } else {
        control->setPalette(black);
        control->setText(data.get(geocoded)) ;
        qledited->setVisible(false) ;
        qlgeo->setVisible(true) ;
    }

    refresh() ;
}




