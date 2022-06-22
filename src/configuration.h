/*
 * Application: POI Manager
 * File: configuration.h
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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QDialog>
#include <QSettings>
#include <QDataStream>
#include "googleaccess/googleaccess.h"

// INI File Name
#define INIFILE "POI.ini"

//-----------------------------------------------------------------------------
//
// Default Tile Server / URLs
//

// Base Map
//#define TILE_MAP "OSM"

// Default Zooms
#define ZOOM_MAP 19
#define ZOOM_AERIAL 17
#define ZOOM_OVERLAY 17
#define ZOOM_CONTOUR 19
#define ZOOM_TRAIL 19


//-----------------------------------------------------------------------------
//
// Configuration Class
//

namespace Ui {
class Configuration;
}

class Configuration : public QDialog
{
    Q_OBJECT

public:
    explicit Configuration(QWidget *parent = 0);
    ~Configuration();

    void reloadIni() ;

    // Get GoogleAccess
    GoogleAccess& googleAccess() ;

    QString& poiFolder() ;
    QString& tracksFolder() ;
    QString& importFolder() ;
    QString& garminFolder() ;
    QString& garminFromFolder() ;
    QString& importFilter() ;
    QString& imageFolder() ;
    QString& iniFolder() ;

    bool importMove() ;
    bool iniFileLoadedOK() ;

    QString& openFolder() ;
    void setOpenFolder(QString folder) ;

    // Type of Geocode to Use
    QString geocodeType() ;

    // Keys used for Built-In Functions / Geocoding
    QString bingKey() ;
    QString googleKey() ;
    QString hereId() ;
    QString hereCode() ;
    QString hereApiKey() ;
    QString googleOauth2Id() ;
    QString googleOauth2Secret() ;


    int aerialTileZoom() ;
    int satelliteOverlayZoom() ;
    int mapTileZoom() ;
    int contourTileZoom() ;
    int trailTileZoom() ;
    QString aerialTileUrl() ;
    QString satelliteOverlayUrl() ;
    QString mapTileUrl() ;
    QString contourTileUrl() ;
    QString trailTileUrl() ;

    int exec() ;

private slots:
    void on_pushButton_SearchPoi_clicked();
    void on_pushButton_SearchGarmin_clicked();
    void on_pushButton_SearchTracks_clicked();
    void on_pushButton_SearchImport_clicked();


    void on_pushButton_ImageFolder_clicked();

    void on_pushButton_IniFolder_clicked();

    void on_pushButton_SearchGarminFrom_clicked();

    void on_pushButton_googleAuthorise_clicked();

private:
    QString sFileName, sGarmin, sGarminFrom, sTracks ;
    QString sImportFolder, sOpenFolder, sImageFolder, sIniFolder ;
    QString sImport, sFilter;
    bool bMove ;
    QSettings *settings ;
    QSettings *filesettings ;
    Ui::Configuration *ui;
    GoogleAccess *googleaccess ;

    // Expand Keys for src url
    QString expandkeys(QString src);

};

#endif // CONFIGURATION_H
