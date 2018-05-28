#ifndef POICOLLECTION_H
#define POICOLLECTION_H

#include <QString>
#include <QList>
#include <QFile>
#include <QDomDocument>

//            //  OV2  GPXv1  GPXv3
// TIME       //  no    yes    yes
// ELEVATION  //  no    yes    yes
// TITLE      //  yes   yes    yes
// COMMENT    //  no    yes    yes
// DESCR      //  no    yes    yes
// DOOR       //  no    no     yes
// STREET     //  no    no     yes
// CITY       //  no    no     yes
// STATE      //  no    no     yes
// POSTCODE   //  no    no     yes
// COUNTRY    //  no    no     yes
// TYPE       //  no    yes    yes
// SYMBOL     //  no    yes    yes
// URL        //  no    yes    yes
// PHONE1     //  yes   no     yes
// PHONE2     //  no    no     yes
// EMAIL      //  no    no     yes

class PoiEntry {

public:
    typedef enum {

        // Automatically Generated
        AUTOTIME,
        AUTOSTREET,
        AUTOCOMMENT,

        // Manually Edited
        EDITEDTITLE,
        EDITEDDESCR,
        EDITEDDOOR,
        EDITEDSTREET,
        EDITEDCITY,
        EDITEDSTATE,
        EDITEDPOSTCODE,
        EDITEDCOUNTRY,
        EDITEDTYPE,
        EDITEDSYMBOL,
        EDITEDURL,
        EDITEDPHONE1,
        EDITEDPHONE2,
        EDITEDEMAIL,

        // Geocoded Results
        GEOELEVATION,
        GEODOOR,
        GEOSTREET,
        GEOCITY,
        GEOSTATE,
        GEOPOSTCODE,
        GEOCOUNTRY,

        // Flags
        GEOCODED,

        NUMFIELDTYPES
    } FieldType ;

private:
    QString sUuid ;
    QString sFields[NUMFIELDTYPES] ;

    double dLat ;
    double dLon ;
    bool valid ;
    bool dirty ;

    // Data Conversion
    long int arrayToLong(QByteArray data) ;
    QByteArray longToArray(long int data) ;
    QString toUtf8(QByteArray ba) ;

public:
    PoiEntry() ;
    bool isValid() ;
    bool isDirty() ;
    void markAsClean() ;

    void clear() ;

    // UUID, used to uniquely identify the POI
    void setUuid(QString uuid) ;
    const QString& uuid() ;

    // POI Information
    void set(FieldType type, QString data) ;
    const QString& get(FieldType type) ;

    void setLatLon(double lat, double lon) ;
    double lat() ;
    double lon() ;

    // OV2 Stream read & write
    bool importOv2(QFile& inputstream) ;
    bool writeOv2(QFile& outputstream, int type=2) ;

    // PoiEntry Copy
    void copyFrom(PoiEntry& source) ;
};

class PoiCollection
{
private:
    QString sFilename ;
    QString sUuid ;
    QList<PoiEntry> poiList ;
    PoiEntry nullPoiEntry ;
    bool bListDirty ;
    QString sLastEdited ;
    QString sLastSavedSequence ;

public:
    PoiCollection();
    ~PoiCollection() ;
    void setFilename(QString filename) ;
    const QString& filename() ;
    void setVersion(QString version) ;
    const QString& getVersion() ;
    int size() ;
    const QString& uuid() ;
    bool clear() ;
    bool isDirty() ;
    bool add(PoiEntry& newEntry) ;
    bool remove(QString uuid) ;
    QString& getSequenceText() ;
    PoiEntry& find(QString uuid) ;
    PoiEntry& at(int i) ;

    // Import Files into current poiList
    bool importOv2(QString filename) ;

    // Load and Save current PoiList
    bool loadGpx(bool importing = false, QString filename = QString("")) ;
    bool saveGpx(bool exporting = false, QString filename = QString("")) ;
    bool saveOv2() ;

private:

    // Extract XML Data
    bool extractXmlData(QDomNode n, const char *tag, PoiEntry::FieldType type, PoiEntry& entry) ;

    // Store XML Data
    bool storeXmlData(QDomDocument& doc, PoiEntry::FieldType type, PoiEntry& entry, QDomElement n, const char *tag, QString attribute = QString(""), QString attrval = QString("")) ;
    bool storeXmlData(QDomDocument& doc, QString text, QDomElement element, const char *tag, QString attribute = QString(""), QString attrval = QString("")) ;

    void updateLastEdited() ;
};


#endif // POICOLLECTION_H
