#ifndef POICOLLECTION_H
#define POICOLLECTION_H

#include <QString>
#include <QList>
#include <QFile>
#include <QDomDocument>
#include <QDateTime>

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
    int iSequence ;

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

    // Route Information
    void setSequence(int seq) ;
    const int sequence() ;

    // OV2 Stream read & write
    bool importOv2(QFile& inputstream) ;
    bool writeOv2(QFile& outputstream, int type=2) ;

    // PoiEntry Copy
    void copyFrom(PoiEntry& source) ;

};

class TrackEntry
{
private:
    double dLat ;
    double dLon ;
    double dElev ;
    QDateTime tDate ;
    QString sUuid ;
    int iSequence ;
    bool bValid ;
    bool bDirty ;

public:
    TrackEntry() ;
    ~TrackEntry() ;
    void set(double lat, double lon, double elev, QDateTime date) ;
    void setLatLon(double lat, double lon) ;
    void setSequence(int sequence) ;
    double lat() ;
    double lon() ;
    double elev() ;
    bool isValid() ;
    bool isDirty() ;
    void markAsClean() ;
    int sequence() ;
    QString uuid() ;
    QDateTime date() ;
    double distanceFrom(TrackEntry& other) ;
    double distanceFrom(PoiEntry& other) ;
} ;


class PoiCollection
{
private:
    QString sFilename ;
    QString sUuid ;
    QString sTrackUuid ;

    QList<PoiEntry> poiList ;
    QList<TrackEntry> trackList ;
    PoiEntry nullPoiEntry ;
    TrackEntry nullTrackEntry ;

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
    bool clear() ;
    bool isDirty() ;
    QString& getSequenceText() ;

    const QString& uuid() ;
    const QString& trackUuid() ;

    bool add(PoiEntry& newEntry) ;
    bool add(TrackEntry& newEntry) ;

    bool remove(QString uuid) ;
    bool removeTrack(QString uuid) ;

    int size() ;
    int trackSize() ;

    PoiEntry& find(QString uuid) ;
    PoiEntry& findPrev(QString uuid) ;
    PoiEntry& findNext(QString uuid) ;

    TrackEntry& findTrack(QString uuid) ;
    TrackEntry& findNextTrack(QString uuid) ;

    PoiEntry& at(int i) ;
    TrackEntry& trackAt(int i) ;


    // Sort the list by sequence number
    void sort() ;

    // Import Files into current poiList
    bool importOv2(QString filename) ;

    // Load and Save current PoiList
    bool loadGpx(QString filename = QString("")) ;
    bool saveGpx(QString filename = QString("")) ;
    bool saveOv2(QString filename = QString("")) ;

private:

    // Extract XML Data
    bool extractXmlData(QDomNode n, const char *tag, PoiEntry::FieldType type, PoiEntry& entry) ;

    // Store XML Data
    bool storeXmlData(QDomDocument& doc, PoiEntry::FieldType type, PoiEntry& entry, QDomElement n, const char *tag, QString attribute = QString(""), QString attrval = QString("")) ;
    bool storeXmlData(QDomDocument& doc, QString text, QDomElement element, const char *tag, QString attribute = QString(""), QString attrval = QString("")) ;

    void updateLastEdited() ;
};


#endif // POICOLLECTION_H
