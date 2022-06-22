/*
 * Application: POI Manager
 * File: photoimportdialog.h
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

#ifndef PHOTOIMPORTDIALOG_H
#define PHOTOIMPORTDIALOG_H

#include <QDialog>
#include <QDateTime>

namespace Ui {
class PhotoImportDialog;
}

class PhotoImportDialog : public QDialog
{
    Q_OBJECT

private:
    void refresh() ;

public:
    explicit PhotoImportDialog(QWidget *parent = 0);
    ~PhotoImportDialog();

    void setTrackTimes(QDateTime from, QDateTime to) ;
    void setPhoto(QDateTime photo, QImage img) ;
    int offset() ;
    bool repeat() ;


private slots:
    void on_spinBox_adjustHours_valueChanged(int arg1);

private:
    Ui::PhotoImportDialog *ui;
    QDateTime phototime, trackfrom, trackto ;
};

#endif // PHOTOIMPORTDIALOG_H
