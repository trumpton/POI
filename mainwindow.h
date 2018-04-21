#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "googlemapswidget.h"
#include "tomtomwidget.h"
#include "poicollection.h"
#include "configuration.h"

#include <QMainWindow>
#include <QListWidget>

#define NOTHINGSELECTED 1
#define WORKINGSELECTED 2
#define FILESELECTED 3

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
    void mapCallbackMarkerGeocoded(QString uuid, QString collectionUuid, QString address) ;

    // Called when search completes / fails
    void mapCallbackSearchResultsReady(QString placeId, double lat, double lon, QString formattedAddress, QString phoneNumber) ;
    void mapCallbackSearchFailed(QString error) ;

    // Called when new POI is provided from TomTom Page
    void importPoi(QString description, double lat, double lon) ;

    // Search
    void on_btnFind_pressed();
    void on_editSearch_returnPressed();

    // Form field updates
    void on_editDescription_editingFinished();
    void on_editPhone_editingFinished();
    void on_editDoorNumber_editingFinished() ;

    // List management
    void on_listWorking_itemClicked(QListWidgetItem *item);
    void on_listFile_itemClicked(QListWidgetItem *item);
    void on_listWorking_itemDoubleClicked(QListWidgetItem *item);
    void on_listFile_itemDoubleClicked(QListWidgetItem *item);

    // Main menu handlers
    void on_action_Setup_triggered();
    void on_action_Exit_triggered();
    void on_actionClear_Cookies_triggered();
    void on_actionRefresh_Google_Map_triggered();
    void on_actionRefresh_Tom_Tom_triggered();

    // Button handlers
    void on_btnNew_clicked();
    void on_btnRemove_clicked();
    void on_btnStore_clicked();
    void on_btnDuplicate_clicked();
    void on_btnDuplicateFile_clicked();
    void on_btnDelete_clicked();

    // File List Handler
    void on_cbPOIFiles_currentIndexChanged(int index);

    void on_action_Save_triggered();

private:

    // Refresh edit filds on the form
    bool updateForm() ;

    // Update the edit fields from the given Poi Entry
    void updateFormFields(PoiEntry* en) ;

    // Load / re-load ov2 files
    void loadFiles() ;

    // Save curent File
    void saveCollection(bool autoyes = false) ;

    // Refresh the map selected icons
    bool updateMapSelection(int zoom=0) ;

    // Set up the enables
    void setEnables(int status) ;

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
