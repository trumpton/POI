/*
 * Application: POI Manager
 * File: photoimportdialog.cpp
 *
 * Manages dialog for the import of data from ohotographs
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

#include "photoimportdialog.h"
#include "ui_photoimportdialog.h"

#include <QPixmap>

PhotoImportDialog::PhotoImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PhotoImportDialog)
{
    ui->setupUi(this);
}

PhotoImportDialog::~PhotoImportDialog()
{
    delete ui;
}

void PhotoImportDialog::refresh()
{
    QDateTime displayphototime = phototime.addSecs(3600*ui->spinBox_adjustHours->value()) ;

    // Set Palette
    QPalette palette = ui->label_photo->palette();
    ui->label_photo->setAutoFillBackground(true);
    if (displayphototime>=trackfrom && displayphototime<=trackto) {
        // Set Colour Green
        palette.setColor(QPalette::WindowText, Qt::green);
        ui->label_photo->setPalette(palette);
    } else {
        // Set Colour Red
        palette.setColor(QPalette::WindowText, Qt::red);
        ui->label_photo->setPalette(palette);
        ui->label_photoTime->setPalette(palette);
    }
    // Set Text
    ui->label_photoTime->setText(displayphototime.toString()) ;
}

void PhotoImportDialog::setTrackTimes(QDateTime from, QDateTime to)
{
    trackfrom = from ;
    trackto = to ;
    ui->label_trackStartTime->setText(from.toString()) ;
    ui->label_trackEndTime->setText(to.toString()) ;
}

void PhotoImportDialog::setPhoto(QDateTime photo, QImage img)
{
    qint64 duration = ((trackfrom.secsTo(trackto)+3599)/3600) - 1 ;
    qint64 offset = (photo.secsTo(trackfrom))/3600 ;

    phototime = photo ;
    phototime.setTimeSpec(Qt::UTC);

    if (offset>-12 && offset<12) {
        ui->spinBox_adjustHours->setMinimum(offset) ;
        ui->spinBox_adjustHours->setMaximum(offset+duration);
        ui->spinBox_adjustHours->setValue(offset);
    } else {
        ui->spinBox_adjustHours->setMinimum(-2);
        ui->spinBox_adjustHours->setMaximum(2);
        ui->spinBox_adjustHours->setValue(0) ;
    }

    QPixmap pm ;
    pm.convertFromImage(img.scaled(ui->label_photo->width(),ui->label_photo->height())) ;
    ui->label_photo->setPixmap(pm);

    refresh() ;
}

int PhotoImportDialog::offset()
{
    return ui->spinBox_adjustHours->value() ;
}

bool PhotoImportDialog::repeat()
{
    return ui->checkBox_repeat->isChecked() ;
}

void PhotoImportDialog::on_spinBox_adjustHours_valueChanged(int arg1)
{
    Q_UNUSED(arg1) ;
    refresh() ;
}
