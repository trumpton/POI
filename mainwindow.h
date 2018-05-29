#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "googlemapswidget.h"
#include "poicollection.h"
#include "configuration.h"
#include "merge.h"

#include <QMainWindow>
#include <QListWidget>

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

    // Called when search completes / fails
    void mapCallbackSearchResultsReady(double lat, double lon, QString formattedAddress, QString phoneNumber) ;
    void mapCallbackSearchFailed(QString error) ;

    // Search
    void on_btnFind_pressed();
    void on_lineEdit_Search_returnPressed();

    // Main menu handlers
    void on_action_Save_triggered();
    void on_action_Setup_triggered();
    void on_action_Exit_triggered();
    void on_actionClear_Cookies_triggered();
    void on_actionRefresh_Google_Map_triggered();

    // Clipboard handlers
    void on_listWorking_itemClicked(QListWidgetItem *item);
    void on_listWorking_itemDoubleClicked(QListWidgetItem *item);
    void on_btnNew_clicked();
    void on_btnStore_clicked();
    void on_btnDuplicate_clicked();
    void on_btnDelete_clicked();

    // File List Handler
    void on_listFile_itemClicked(QListWidgetItem *item);
    void on_listFile_itemDoubleClicked(QListWidgetItem *item);
    void on_cbPOIFiles_currentIndexChanged(int index);
    void on_action_LaunchTomTom_triggered();
    void on_btnEditFile_clicked();
    void on_btnCopyToClipboard_clicked();

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


    void on_action_New_triggered();

    void on_action_Delete_triggered();

    void on_action_ImportTomTom_triggered();

    void on_actionAuto_Geocode_triggered();

    void on_action_About_POI_triggered();

    void on_action_MergeWith_triggered();

    void on_action_EditAll_triggered();

    void on_action_EmptyClipboard_triggered();

    void on_lineEdit_Door_returnPressed();

private:

    // Merge Dialog
    Merge merge ;

    // Update the lists and entry form
    bool refresh(bool refreshMarkers = false, int zoom = 0) ;


    // Sets the line entry text data and colour
    void setLineEditText(QLineEdit *control, PoiEntry& data, PoiEntry::FieldType edited, PoiEntry::FieldType geocoded, QLabel *qledited, QLabel *qlgeo) ;

    // Refresh edit filds on the form
    bool updateForm() ;

    // Update the edit fields from the given Poi Entry
    void updateFormFields(PoiEntry* en) ;

    // Load / re-load ov2 files
    bool loadFiles() ;

    // Save curent File
    void saveCollection(bool autoyes = false) ;

    // Refresh the map selected icons
    bool updateMapSelection(int zoom=0) ;

    // Refresh the Lists
    bool updateLists() ;
    bool updateList(PoiCollection *collection, QListWidget *widget) ;

    // Refresh the Lists Selection
    bool updateListsSelection(bool refreshMarkers=false) ;
    bool updateListSelection(PoiCollection *collection, QListWidget *widget, bool refreshMarkers=false) ;

    // Return currently selected UUID of the given list widget
    QString currentSelectionUuid(QListWidget *widget) ;

    // Search the Working and Current POI Lists and return match
    PoiEntry& findEntryByUuid(QString uuid, QString collectionUuid) ;

    Ui::MainWindow *ui;
    Configuration *configuration ;

    // Currently selected entry
    QString thisUuid, thisCollectionUuid ;

    // Current working list and POI File
    PoiCollection workingCollection, fileCollection ;

};

#endif // MAINWINDOW_H
