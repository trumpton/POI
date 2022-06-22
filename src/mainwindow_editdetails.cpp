/*
 * Application: POI Manager
 * File: mainwindow_editdetails.cpp
 *
 * Main window / application GUI
 * Slots for managing editing of details
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




