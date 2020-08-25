#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QDialog>
#include <QSettings>
#include <QDataStream>

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

    QString& poiFolder() ;
    QString& tracksFolder() ;
    QString& importFolder() ;
    QString& garminFolder() ;
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

private:
    QString sFileName, sGarmin, sTracks ;
    QString sImportFolder, sOpenFolder, sImageFolder, sIniFolder ;
    QString sImport, sFilter;
    bool bMove ;
    QSettings *settings ;
    QSettings *filesettings ;
    Ui::Configuration *ui;

    // Expand Keys for src url
    QString expandkeys(QString src);

};

#endif // CONFIGURATION_H
