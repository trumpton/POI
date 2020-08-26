#ifndef POICOLLECTION_H
#define POICOLLECTION_H

#include <QString>
#include <QList>
#include <QFile>
#include <QDomDocument>
#include <QDateTime>
#include <QImage>
#include <QPixmap>

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

        // Photo Input
        PHOTOFILENAME,
        PHOTOLAT,
        PHOTOLON,
        PHOTOELEVATION,
        PHOTODATE,

        // Geocoded Results
        GEOELEVATION,
        GEODOOR,
        GEOSTREET,
        GEOCITY,
        GEOSTATE,
        GEOPOSTCODE,
        GEOCOUNTRY,

        // Date / TIme
        DATETIME,

        // Flags
        GEOCODED,

        NUMFIELDTYPES
    } FieldType ;

private:
    QString sUuid ;
    QString sFields[NUMFIELDTYPES] ;
    QPixmap pPixmap ;
    QDateTime tDate ;

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

    bool setImage(QImage img) ;
    QPixmap& pixmap() ;

    // Date
    void setDate(QString date, int timezoneoffset=0) ;
    QDateTime date() ;

    // Route Information
    void setSequence(int seq) ;
    const int sequence() ;

    // OV2 Stream read & write
    bool importOv2(QFile& inputstream) ;
    bool writeOv2(QFile& outputstream, int type=2) ;

    // PoiEntry Copy All except for UUID
    void copyFrom(PoiEntry& source) ;

    // Operator Assignment
    void operator = (const PoiEntry &source ) {
        sUuid = source.sUuid ;
        for (int i=0; i<PoiEntry::NUMFIELDTYPES; i++) { sFields[i] = source.sFields[i] ; }
        pPixmap = source.pPixmap ;
        tDate = source.tDate ;
        dLat = source.dLat ;
        dLon = source.dLon ;
        valid = source.valid ;
        dirty = source.dirty ;
        iSequence = source.iSequence ;
        if (valid) dirty = true ;
    }

};

class TrackEntry
{
private:
    QString sUuid ;
    double dLat ;
    double dLon ;
    double dElev ;
    QDateTime tDate ;
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

    // Operator Assignment
    void operator = (const TrackEntry &source ) {
        sUuid = source.sUuid ;
        dLat = source.dLat ;
        dLon = source.dLon ;
        dElev = source.dElev ;
        tDate = source.tDate ;
        iSequence = source.iSequence ;
        bValid = source.bValid ;
        bDirty = source.bDirty ;
        if (bValid) bDirty = true ;
    }

} ;


class PoiCollection
{
private:
    QString sFilename ;
    QString sUuid ;
    QString sTrackUuid ;
    QString sName ;
    QString sFormattedName ;
    int iRating ;

    QList<PoiEntry> poiList ;
    QList<TrackEntry> trackList ;
    PoiEntry nullPoiEntry ;
    TrackEntry nullTrackEntry ;

    bool bListDirty ;
    QString sLastEdited ;
    QString sLastSavedSequence ;

    double dtracklength, dheightgain, dheightloss ;
    double dtracktime, dtracktimeest ;

public:
    PoiCollection();
    ~PoiCollection() ;
    void setFilename(QString filename) ;
    const QString& filename() ;
    void setVersion(QString version) ;
    const QString& getVersion() ;
    bool clear() ;

    bool isDirty() ;
    void markAsDirty() ;

    QString& getSequenceText() ;

    int setRating(int rating) ;
    int rating() ;

    double trackLength() ;
    double heightGain() ;
    double heightLoss() ;
    double trackTime() ;
    double trackTimeEst() ;


    const QString& uuid() ;
    void setUuid(QString uuid) ;

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

    TrackEntry& findTrack(QDateTime when, int withinSeconds = 30) ;
    TrackEntry& findTrack(QString uuid) ;
    TrackEntry& findNextTrack(QString uuid) ;
    TrackEntry& findPrevTrack(QString uuid) ;

    PoiEntry& at(int i) ;
    TrackEntry& trackAt(int i) ;

    // Sort the list by sequence number
    void sortBySequence() ;

    // Reorder the list by waypoint date
    void reorderWaypointByDate() ;

    // Import Files into current poiList
    bool importOv2(QString filename) ;

    // Load and Save current PoiList
    bool loadGpx(QString filename = QString("")) ;
    bool saveGpx(QString filename = QString("")) ;
    bool saveOv2(QString filename = QString("")) ;

    // Name access
    QString& name() ;
    void setName(QString name) ;

    // Formatted Name (inc rating, time etc)
    QString& formattedName(bool includerating, bool includeduration, bool includedistance, bool includeheight, bool asfilename, bool starasasterisk=false) ;


private:

    // Extract XML Data
    bool extractXmlData(QDomNode n, const char *tag, PoiEntry::FieldType type, PoiEntry& entry) ;

    // Store XML Data
    bool storeXmlData(QDomDocument& doc, PoiEntry::FieldType type, PoiEntry& entry, QDomElement n, const char *tag, QString attribute = QString(""), QString attrval = QString("")) ;
    bool storeXmlData(QDomDocument& doc, QString text, QDomElement element, const char *tag, QString attribute = QString(""), QString attrval = QString("")) ;

    void updateLastEdited() ;

    bool calculateTrack() ;
};


#endif // POICOLLECTION_H
