#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "poicollection.h"
#include "configuration.h"

#include <QMainWindow>
#include <QListWidget>
#include <QPixmap>
#include <QLabel>

#include "undo.h"

#define PREFZOOM 17


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    // Called whenever a marker has been modified or selected on the map
    void mapCallbackMarkerMoved(QString uuid, QString collectionUuid, double lat, double lon) ;
    void mapCallbackMarkerSelected(QString uuid, QString collectionUuid) ;
    void mapCallbackMarkerGeocoded(QString uuid, QString collectionUuid, QString formattedaddress, QString door, QString street, QString town, QString state, QString country, QString postcode) ;

    // Called when the map has been moved
    void mapCallbackMapMoved(double lat, double lon, int zoom) ;

    // Called when search completes / fails
    void mapCallbackSearchResultsReady(double lat, double lon, QString formattedAddress, QString phoneNumber) ;
    void mapCallbackSearchFailed(QString error) ;

    // Main menu handlers
    void on_action_New_triggered();
    void on_action_Open_triggered();
    void on_action_Save_triggered();
    void on_action_SaveAs_triggered();
    void on_action_Setup_triggered();
    void on_action_Exit_triggered();
    void on_actionClear_Cookies_triggered();

    // Edit
    void on_action_CreateTrackFromWaypoints_triggered();
    void on_action_ReduceTrackPoints_triggered();

    // Import
    void on_action_ImportGpx_triggered();
    void on_action_ImportOv2_triggered();
    void on_action_ImportGpxToClipboard_triggered();
    void on_actionAuto_Geocode_triggered();
    void on_action_About_POI_triggered();
    void on_action_EditAll_triggered();
    void on_action_EmptyClipboard_triggered();

    // Export
    void on_action_LaunchTomTom_triggered();
    void on_menuExport_aboutToShow() ;

    // Search
    void on_action_FindLocation_triggered();

    // About

    // File List Handler
    void on_comboBox_Filter_currentIndexChanged(int index);
    void on_listFile_itemClicked(QListWidgetItem *item);
    void on_btnEditFile_clicked();
    void on_btnCopyToClipboard_clicked();
    void on_btnUp_clicked();
    void on_btnDown_clicked();

    // Track Handlers
    void on_NewTrackPoint_clicked();
    void on_DeleteTrackPoint_clicked();

    // Clipboard handlers
    void on_listWorking_itemClicked(QListWidgetItem *item);
    void on_btnNew_clicked();
    void on_btnStore_clicked();
    void on_btnDuplicate_clicked();
    void on_btnDelete_clicked();

    // Form Field Updates
    void on_lineEdit_title_editingFinished();
    void on_plainTextEdit_Description_textChanged();
    void on_lineEdit_Door_editingFinished();
    void on_lineEdit_Street_editingFinished();
    void on_lineEdit_City_editingFinished();
    void on_lineEdit_State_editingFinished();
    void on_lineEdit_Postcode_editingFinished();
    void on_lineEdit_Country_editingFinished();
    void on_lineEdit_Type_editingFinished();
    void on_lineEdit_Url_editingFinished();
    void on_lineEdit_Phone1_editingFinished();
    void on_lineEdit_Phone2_editingFinished();
    void on_lineEdit_Door_returnPressed();
    void on_lineEdit_fileTitle_editingFinished();
    void on_comboBox_Rating_currentIndexChanged(int index);
    void on_actionTransfer_to_Garmin_triggered();
    void on_actionTransfer_from_Garmin_triggered();
    void on_action_Photo_triggered();
    void on_actionResort_Waypoints_By_Date_triggered();
    void on_actionUndo_triggered();

    void on_action_ShowStandard_triggered();
    void on_action_ShowAerial_triggered();
    void on_action_ShowContour_triggered();

    void on_action_ShowTrack_toggled(bool checked);
    void on_action_ShowActualDuration_toggled(bool checked);

    void on_action_ShowTrailsOverlay_toggled(bool arg1);

    void on_action_ShowSatelliteOverlay_toggled(bool arg1);

private:

    // Update the lists and entry form
    bool refresh(bool refreshMarkers = false, bool centreOnMarker = false, int zoom = 0) ;

    // Sets the line entry text data and colour
    void setLineEditText(QLineEdit *control, PoiEntry& data, PoiEntry::FieldType edited, PoiEntry::FieldType geocoded, QLabel *qledited, QLabel *qlgeo) ;

    // Refresh edit filds on the form
    bool updateForm() ;

    // Update the edit fields from the given Poi Entry
    void updateFormFields(PoiEntry* en) ;

    // Load / re-load ov2 files
    bool loadFiles() ;

    // Save curent File
    QString buildFilename() ;
    void saveCollection(bool autoyes = false) ;

    // Refresh the map selected icons
    bool updateMapSelection(int zoom=0) ;

    // Refresh the Lists
    bool updateLists() ;
    bool updateList(PoiCollection *collection, QListWidget *widget, QString filterText = "") ;

    // Refresh the Lists Selection
    bool updateListsSelection() ;
    bool updateListSelection(PoiCollection *collection, QListWidget *widget) ;
    bool updateSearchFilter() ;

    // Update tracks.ov2
    bool refreshTracksPoi() ;

    // Refresh the Map
    bool refreshMap() ;


    // Return currently selected UUID of the given list widget
    QString currentSelectionUuid(QListWidget *widget) ;

    // Search the Working and Current POI Lists and return match
    PoiEntry& findEntryByUuid(QString uuid, QString collectionUuid) ;
    TrackEntry& findTrackEntryByUuid(QString uuid, QString collectionUuid) ;

    Undo undo ;

    QPixmap nullpixmap ;

    Ui::MainWindow *ui;
    Configuration *configuration ;

    // Currently selected entry
    QString thisUuid, thisCollectionUuid ;

    // Current working list and POI File
    PoiCollection workingCollection, fileCollection ;

    // Search QUery
    QString searchtext ;

};

#endif // MAINWINDOW_H
