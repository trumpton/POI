#ifndef POICOLLECTION_H
#define POICOLLECTION_H

#include <QString>
#include <QList>
#include <QFile>

class PoiEntry {
private:
    QString sPlaceId ;
    QString sUuid ;
    QString sDescription ;
    QString sAddress ;
    QString sDoorNumber ;
    QString sPhone ;
    QString sDoor ;

    double dLat ;
    double dLon ;
    bool valid ;
    bool dirty ;

    // Data Conversion
    long int arrayToLong(QByteArray data) ;
    QByteArray longToArray(long int data) ;


public:
    PoiEntry() ;
    bool isValid() ;
    bool isDirty() ;

    // UUID, used to uniquely identify the POI
    void setUuid(QString uuid) ;
    const QString& uuid() ;

    // POI Information (stored in OV2 files)
    void setDescription(QString description) ;
    void setPhone(QString phone) ;
    void setDoor(QString door) ;
    void setLatLon(double lat, double lon) ;
    void setPlaceId(QString placeId) ;

    const QString& description() ;
    const QString& door() ;
    const QString& phone() ;
    double lat() ;
    double lon() ;
    const QString& placeId() ;

    // Google Geo lookup results (for reference)
    void setAddress(QString address) ;
    const QString& address() ;

    // Stream read & write
    bool readOv2(QFile& inputstream) ;
    bool writeOv2(QFile& outputstream, int type=2) ;
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
    bool load() ;
    bool save() ;
    bool isDirty() ;
    bool add(PoiEntry& newEntry) ;
    bool remove(QString uuid) ;
    QString& getSequenceText() ;
    PoiEntry& find(QString uuid) ;
    PoiEntry& at(int i) ;

private:
    void updateLastEdited() ;
};


#endif // POICOLLECTION_H
