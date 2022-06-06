//////////////////////////////////////////////////////////////////////////
//
// mainwindow_googlesync.cpp
//
//

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonDocument>

//
// Purge all PoiManager entries on Google
//

void MainWindow::doPurgeAllGoogleEntries()
{

    int removed=0 ;

    // If not authorised with google, exit
    if (this->configuration->googleAccess().getUsername().isEmpty()) {
        QMessageBox::warning(this,"POI", "Please authorise a google account in the settings menu") ;
        return ;
    }

    // Loop round until all contacts have been processed
    do {

       // Get chunk of current contacts list (do with several calls if more than records available)
        QString contacts = configuration->googleAccess()
            .googleGet(  QString("https://people.googleapis.com/v1/people/me/connections") +
                         QString("?pageSize=1000&personFields=addresses,externalIds,metadata,names,locations,userDefined")
                      );

       // Process the chunk of contacts
       googlesync_removeallpoicontacts(contacts,&removed) ;

    } while (0) ;

    QMessageBox::information(this, "POI", QString("Update complete: ") +
                             QString::number(removed) + QString(" deleted"));


}


//
// Synchronise current collection with Google Contacts
//
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

    // Prevent sync from running too quickly (prevents duplicate creations)
    QDateTime now = QDateTime::currentDateTime() ;
    if ( !googleLastSyncFileName.isEmpty() &&
         this->fileCollection.filename() == googleLastSyncFileName &&
         googleLastSyncTime.addSecs(30) > now &&
         googleLastAdded > 0)  {
        QMessageBox::warning(this, "POI", "Please wait 30 seconds and try again") ;
        return ;
    }

    googleLastSyncFileName = this->fileCollection.filename() ;
    googleLastSyncTime = now ;

    // Clear 'processed' flags
    fileCollection.clearFlags() ;

    int added=0, removed=0, modified=0 ;

    // Loop round until all contacts have been processed
    do {

       // Get chunk of current contacts list (do with several calls if more than records available)
        QString contacts = configuration->googleAccess()
            .googleGet(  QString("https://people.googleapis.com/v1/people/me/connections") +
                         QString("?pageSize=1000&personFields=addresses,externalIds,metadata,names,locations,userDefined")
                      );

       if (configuration->googleAccess().getNetworkErrorCode()) {
         QMessageBox::warning(this, "POI", QString("Sync download failed: ") +
                              configuration->googleAccess().getNetworkError()) ;
         return ;
       }

       // Process the chunk of contacts
       googlesync_processcontacts(contacts,&modified,&removed) ;

    } while (0) ;

    if (configuration->googleAccess().getNetworkErrorCode()) {
      QMessageBox::warning(this, "POI", QString("Sync modify failed: ") +
                           configuration->googleAccess().getNetworkError()) ;
      return ;
    }

    googlesync_uploadcontacts(&added) ;

    if (configuration->googleAccess().getNetworkErrorCode()) {
      QMessageBox::warning(this, "POI", QString("Sync upload failed: ") +
                           configuration->googleAccess().getNetworkError()) ;
      return ;
    }

    QMessageBox::information(this, "POI", QString("Update complete: ") + QString::number(added) +
                             QString(" added, ") + QString::number(modified) + QString(" modified, ") +
                             QString::number(removed) + QString(" deleted"));
    googleLastAdded = added ;
}



//
// Upload all new contacts that have not been seen/visited
//
void MainWindow::googlesync_uploadcontacts(int *added)
{
    int q=0 ;

    QJsonArray contacts ;

    for (int i=0; i<fileCollection.size(); i++) {

        if (!fileCollection.at(i).flag()) {

            // Add entry to list
            QJsonObject contactperson ;
            contactperson.insert("contactPerson", googlesync_buildcontact(
                                     fileCollection.name(),
                                     fileCollection.uuid(),
                                     &(fileCollection.at(i))
                                 )) ;
            contacts.append( contactperson ) ;

            (*added)++ ;
            q++ ;

        }

        // Do upload in chunks of 150
        if (q==150 || (q>0 && i==(fileCollection.size()-1))) {

            // Build and upload contacts
            QJsonDocument doc ;
            QJsonObject obj ;
            // obj.insert("readMask","blah") ;
            obj.insert("contacts",contacts) ;
            doc.setObject(obj) ;

            QByteArray json = doc.toJson() ;
            QString tmp = json.data() ; // Debug
            QString resp = configuration->googleAccess().googlePutPostDelete(
                        "https://people.googleapis.com/v1/people:batchCreateContacts",
                        GoogleAccess::Post, json) ;

            // Check response
            QJsonDocument respdoc ;
            respdoc.fromJson(resp.toLatin1()) ;
            QJsonObject respobj = respdoc.object() ;
            QString errormessage = respobj["error"].toObject()["message"].toString("") ;
            if (!errormessage.isEmpty()) {
                // Error occurred
            }

            // Clear structure to go round for more
            contacts = QJsonArray() ;
            q=0 ;
        }

    }
}


//
// Process contacts within supplied JSON, and modify / delete as
// required.
//
void MainWindow::googlesync_processcontacts(QString contacts, int *modified, int *removed)
{

    int mq=0, dq=0 ;

    // Response structures
    QJsonArray respdeletelist ;
    QJsonObject respmodifylist ;
    QJsonDocument respmodify ;

    // Get source
    QJsonDocument srcdoc = QJsonDocument::fromJson(contacts.toUtf8()) ;
    QJsonObject srcobj = srcdoc.object() ;
    QJsonArray srcconnections = srcobj["connections"].toArray() ;

    // Step through the json file, and capture all relevant entries
    for (int i=0; i<srcconnections.size(); i++) {

        QJsonObject connection;
        QString setUuid, thisUuid ;

        connection = srcconnections.at(i).toObject() ;

        foreach(const QJsonValue & userdefined, connection["userDefined"].toArray()) {
            if (userdefined.toObject()["key"].toString("")=="poiManagerSetUuid") {
                setUuid=userdefined.toObject()["value"].toString("") ;
            }
            if (userdefined.toObject()["key"].toString("")=="poiManagerRecordUuid") {
                thisUuid=userdefined.toObject()["value"].toString("") ;
            }
        }

        // Process entries that are/were part of this file collection
        if (setUuid == thisCollectionUuid) {

            // Search for, and process entry in local file collection
            PoiEntry& pe = fileCollection.find(thisUuid);
            QString resourceName = connection["resourceName"].toString() ;

            if (pe.isValid()) {

                // Entry found, check details
                QString familyName = connection["names"].toArray().at(0)["familyName"].toString("") ;
                QString givenName = connection["names"].toArray().at(0)["givenName"].toString("") ;
                QString postalCode = connection["addresses"].toArray().at(0)["postalCode"].toString("") ;

                // If entry has changed, add it to the modified list

                if (postalCode != pe.get(PoiEntry::OLC) ||
                    givenName != pe.get(PoiEntry::EDITEDTITLE) ||
                    familyName != fileCollection.name()) {

                    QString etag = connection["etag"].toString() ;

                    // TODO: add entry to the modify list
                    (*modified)++ ;
                    respmodifylist.insert(resourceName, googlesync_buildcontact(
                                     fileCollection.name(),
                                     fileCollection.uuid(),
                                     &pe, etag)) ;
                    mq++ ;
                }


            // If entry has been removed, add it to the remove list

            } else {

                // Add entry to the remove list
                respdeletelist.append(QJsonValue(resourceName)) ;
                (*removed)++ ;
                dq++ ;

            }

            // Mark as processed
            pe.setFlag(true) ;

        }

        if (mq==150 || (mq>0 && i==(srcconnections.size()-1))) {

            // Upload block of modified entries in chunks of 150
            QJsonDocument doc ;
            QJsonObject obj ;
            obj.insert("updateMask", "addresses,names,userDefined") ;
            obj.insert("contacts", respmodifylist) ;
            doc.setObject(obj) ;
            QByteArray json = doc.toJson() ;
            QString tmp = json.data() ; // Debug
            QString resp = configuration->googleAccess().googlePutPostDelete(
                        "https://people.googleapis.com/v1/people:batchUpdateContacts",
                        GoogleAccess::Post, json) ;

            // Check response
            QJsonDocument respdoc ;
            respdoc.fromJson(resp.toLatin1()) ;
            QJsonObject respobj = respdoc.object() ;
            QString errormessage = respobj["error"].toObject()["message"].toString("") ;
            if (!errormessage.isEmpty()) {
                // Error occurred
            }

            // Clear list of entries uploaded and reset counter
            respmodifylist = QJsonObject() ;
            mq=0 ;
        }


        if (dq==150 || (dq>0 && i==(srcconnections.size()-1))) {

            // Upload block of deleted entries in chunks of 150
            QJsonDocument doc ;
            QJsonObject obj ;
            obj.insert("resourceNames", respdeletelist) ;
            doc.setObject(obj) ;
            QByteArray json = doc.toJson() ; // Debug
            QString tmp = json.data() ;
            QString resp = configuration->googleAccess().googlePutPostDelete(
                        "https://people.googleapis.com/v1/people:batchDeleteContacts",
                        GoogleAccess::Post, json) ;

            // Check response
            QJsonDocument respdoc ;
            respdoc.fromJson(resp.toLatin1()) ;
            QJsonObject respobj = respdoc.object() ;
            QString errormessage = respobj["error"].toObject()["message"].toString("") ;
            if (!errormessage.isEmpty()) {
                // Error occurred
            }

            // Clear list of entries deleted and reset counter
            respdeletelist = QJsonArray() ;
            dq=0 ;
        }

    }
}


//
// Builds a JSON object for the contact, lifting data from the given Poi Entry.
//
QJsonObject MainWindow::googlesync_buildcontact(QString fcname, QString fcuuid, PoiEntry *pe, QString etag)
{
    QJsonObject contact ;

    if (!etag.isEmpty()) {
        contact.insert("etag", etag) ;
    }

    QJsonObject address ;
    address.insert("type","POI") ;
    // We don't add the street address in the proper field as this is used
    // by google in preference to the OLC postcode
    //address.insert("streetAddress",pe->get(PoiEntry::EDITEDSTREET, PoiEntry::GEOSTREET)) ;
    address.insert("city", pe->get(PoiEntry::EDITEDCITY, PoiEntry::GEOCITY)) ;
    address.insert("country", pe->get(PoiEntry::EDITEDCOUNTRY, PoiEntry::GEOCOUNTRY)) ;
    address.insert("postalCode", pe->get(PoiEntry::OLC)) ;

    QJsonArray addresses ;
    addresses.append(address) ;

    contact.insert("addresses", addresses) ;

    QJsonObject name ;
    name.insert("givenName", pe->get(PoiEntry::EDITEDTITLE)) ;
    name.insert("familyName", fcname) ;
    QJsonArray names ;
    names.append(name) ;
    contact.insert("names", names) ;

    QJsonArray userdefined ;
    QJsonObject setuuid ;
    setuuid.insert("key", "poiManagerSetUuid") ;
    setuuid.insert("value", fcuuid) ;
    userdefined.append(setuuid) ;
    QJsonObject recorduuid ;
    recorduuid.insert("key", "poiManagerRecordUuid") ;
    recorduuid.insert("value", pe->uuid()) ;
    userdefined.append(recorduuid) ;

    contact.insert("userDefined", userdefined) ;

    return contact ;
}

//
// Purge all POI contacts from google
//
void MainWindow::googlesync_removeallpoicontacts(QString contacts,int *removed)
{

    int dq=0 ;

    // Response structures
    QJsonArray respdeletelist ;

    // Get source
    QJsonDocument srcdoc = QJsonDocument::fromJson(contacts.toUtf8()) ;
    QJsonObject srcobj = srcdoc.object() ;
    QJsonArray srcconnections = srcobj["connections"].toArray() ;

    // Step through the json file, and capture all relevant entries
    for (int i=0; i<srcconnections.size(); i++) {

        QJsonObject connection;

        connection = srcconnections.at(i).toObject() ;

        foreach(const QJsonValue & userdefined, connection["userDefined"].toArray()) {
          if (userdefined.toObject()["key"].toString("")=="poiManagerSetUuid") {
            // Add entry to the remove list
            QString resourceName = connection["resourceName"].toString() ;
            respdeletelist.append(QJsonValue(resourceName)) ;
            (*removed)++ ;
            dq++ ;
          }
        }

        if (dq==150 || (dq>0 && i==(srcconnections.size()-1))) {

          // Upload block of deleted entries in chunks of 150
          QJsonDocument doc ;
          QJsonObject obj ;
          obj.insert("resourceNames", respdeletelist) ;
          doc.setObject(obj) ;
          QByteArray json = doc.toJson() ; // Debug
          QString tmp = json.data() ;

          QString resp = configuration->googleAccess().googlePutPostDelete(
                      "https://people.googleapis.com/v1/people:batchDeleteContacts",
                      GoogleAccess::Post, json) ;

          // Check response
          QJsonDocument respdoc ;
          respdoc.fromJson(resp.toLatin1()) ;
          QJsonObject respobj = respdoc.object() ;
          QString errormessage = respobj["error"].toObject()["message"].toString("") ;
          if (!errormessage.isEmpty()) {
              // Error occurred
          }

          // Clear list of entries deleted and reset counter
          respdeletelist = QJsonArray() ;
          dq=0 ;
        }
    }
}

