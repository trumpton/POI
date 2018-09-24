#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QDialog>
#include <QSettings>
#include <QDataStream>

//-----------------------------------------------------------------------------
//
// Default Tile Server / URLs
//

// Base Map
#define TILE_MAP "OSM"
#define ZOOM_MAP 19

// Aerial View
#define TILE_AERIAL "Bing"
#define ZOOM_AERIAL 17

// Satellite Overlay
#define TILE_OVERLAY "Bing"
#define ZOOM_OVERLAY 17

// Contoured Map
#define TILE_CONTOUR "https://{a-c}.tile.opentopomap.org/{z}/{x}/{y}.png"
//#define TILE_CONTOUR "https://server.arcgisonline.com/ArcGIS/rest/services/World_Topo_Map/MapServer/tile/{z}/{y}/{x}"
#define ZOOM_CONTOUR 19

// Trails Overlay
#define TILE_TRAIL "https://tile.waymarkedtrails.org/hiking/{z}/{x}/{y}.png"
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

    QString& poiFolder() ;
    QString& tracksFolder() ;
    QString& importFolder() ;
    QString& garminFolder() ;
    QString& importFilter() ;
    QString& imageFolder() ;
    bool importMove() ;

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

private:
    QString sFileName, sGarmin, sTracks, sImportFolder, sOpenFolder, sImageFolder, sImport, sFilter;
    bool bMove ;
    QSettings *settings ;
    QSettings *filesettings ;
    Ui::Configuration *ui;

};

#endif // CONFIGURATION_H
