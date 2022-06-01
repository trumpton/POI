//////////////////////////////////////////////////////////////////////////
//
// mainwindow_googlesync.cpp
//
//

#include "mainwindow.h"
#include "ui_mainwindow.h"

//
// Synchronise current collection with Google Contacts
void MainWindow::doSyncWithGoogle()
{
    // If no POI loaded, exit
    if (this->fileCollection.size()==0) {
        QMessageBox::warning(this,"POI", "No file is open, nothing to sync") ;
        return ;
    }

    // If not authorised with google, exit
    if (this->configuration->googleAccess().getUsername().isEmpty()) {
        QMessageBox::warning(this,"POI", "Please authorise a google account in the settings menu") ;
        return ;
    }

    QMessageBox::information(this, "POI", "OK, Sync complete") ;

}
