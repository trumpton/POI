#include "poicollection.h"
#include <QUuid>
#include <QString>
#include <QRegExp>
#include <QDateTime>
#include <QDebug>

// ///////////////////////////////////////////////////////
//
// PoiEntry
//

PoiEntry::PoiEntry() {
    QUuid quuid = QUuid::createUuid() ;
    sUuid = quuid.toString() ;
    valid=false ; dirty=false ;
    dLat=0 ; dLon=0 ;
    sDescription="" ; sAddress="" ;
    sPhone="" ; sDoor="" ;
    sPlaceId="" ;
}

bool PoiEntry::isValid() { return valid ; }
bool PoiEntry::isDirty() { return dirty ; }
void PoiEntry::setUuid(QString uuid) { sUuid = uuid ; }
void PoiEntry::setAddress(QString address) { sAddress = address ; }
void PoiEntry::setPhone(QString phone) { sPhone = phone ; valid=true ; dirty=true ; }
void PoiEntry::setDoor(QString door) { sDoor = door ; valid=true ; dirty=true ; }
void PoiEntry::setLatLon(double lat, double lon) { dLat = lat ; dLon = lon ; valid=true ; dirty=true ; }
void PoiEntry::setPlaceId(QString placeId) { sPlaceId = placeId ; }

const QString& PoiEntry::uuid() { return sUuid ; }
const QString& PoiEntry::description() { return sDescription ; }
const QString& PoiEntry::address() { return sAddress ; }
const QString& PoiEntry::phone() { return sPhone ; }
const QString& PoiEntry::door() { return sDoor ; }
double PoiEntry::lat() { return dLat ; }
double PoiEntry::lon() { return dLon ; }
const QString& PoiEntry::placeId() { return sPlaceId ; }

void PoiEntry::setDescription(QString description)
{
    // Parse / translate description [number]>phone

    QString door ;
    QString phone ;

    QRegExp rxp("^(.*)>(.+)") ;
    if (rxp.indexIn(description)>=0) {
        description=rxp.cap(1).trimmed() ;
        phone=rxp.cap(2).trimmed() ;
    }
    QRegExp rxn("^(.*)\\[(.+)\\]") ;
    if (rxn.indexIn(description)>=0) {
        description=rxn.cap(1).trimmed() ;
        door=rxn.cap(2).trimmed() ;
    }

    if (!door.isEmpty()) setDoor(door) ;
    if (!phone.isEmpty()) setPhone(phone) ;

    sDescription = description ; valid=true ; dirty=true ;

}

// ///////////////////////////////////////////////////////
//
// PoiCollection
//

PoiCollection::PoiCollection()
{
    clear() ;
    updateLastEdited() ;
}

PoiCollection::~PoiCollection()
{
}

QString& PoiCollection::getSequenceText()
{
    return sLastSavedSequence ;
}

void PoiCollection::updateLastEdited()
{
    QDateTime now = QDateTime::currentDateTimeUtc() ;
    sLastEdited = now.toString("yyyy-MM-dd-hh:mm:ss") ;
}

bool PoiCollection::clear()
{
    sFilename = "" ;
    QUuid quuid = QUuid::createUuid() ;
    sUuid = quuid.toString() ;
    poiList.clear() ;
    bListDirty=false ;
    sLastEdited = "" ;
    sLastSavedSequence = "" ;
    return true ;
}

const QString& PoiCollection::uuid() { return sUuid ; }

void PoiCollection::setFilename(QString filename) { sFilename = filename ; }
const QString& PoiCollection::filename() { return sFilename ; }

bool PoiCollection::isDirty()
{
    bool dirty=false ;
    for (int i=poiList.size()-1; i>=0; i--) {
        dirty |= poiList[i].isDirty() ;
    }
    return dirty || bListDirty;
}

bool PoiCollection::load()
{
    if (sFilename.isEmpty()) return false ;
    poiList.clear() ;

    QFile inputstream(sFilename) ;
    if (!inputstream.open(QIODevice::ReadOnly))
        return false ;

    bool success ;
    sLastSavedSequence = "0" ;
    sLastEdited = QString("unknown") ;

    do {
        PoiEntry record ;
        success=record.readOv2(inputstream) ;
        if (record.isValid()) {

            if (record.lat()==-180.0 && record.lon()==0.0) {

                // Last Edited Date stored as a record at Latitude: -180, Longitude: 0
                sLastEdited = record.description() ;
                sLastEdited = sLastEdited.replace(QRegExp("[^0-9\\- :]"), "").trimmed() ;
                sLastSavedSequence = record.door() ;

            } else {

                // Normal Record
                poiList.append(record) ;
            }

        }

    } while (success && !inputstream.atEnd()) ;

    inputstream.close() ;
    qDebug() << "PoiCollection::load() => " << sFilename << " (version: " << sLastEdited << ", sequence: " << sLastSavedSequence << ")" ;
    bListDirty=false ;
    return success ;
}

bool PoiCollection::save()
{
    if (sFilename.isEmpty()) return false ;
    if (!isDirty()) return true ;

    updateLastEdited() ;

    QFile outputstream(sFilename) ;
    if (!outputstream.open(QIODevice::ReadWrite))
        return false ;

    bool success=true ;

    // Add Source Record (deleted record, lat=-180, lon=1)
    PoiEntry source ;
    source.setLatLon(-180.0, 1.0) ;
    source.setDescription(QString("TomTom / Google Points of Interest Editor - www.trumpton.uk")) ;
    success = source.writeOv2(outputstream, 0) ;

    // Add Date Stamp (valid type 2 record, lat=-180, lon=0)
    if (success) {
        PoiEntry lastEdited ;
        lastEdited.setLatLon(-180.0, 0.0) ;
        lastEdited.setDescription(QString("( Version ") + sLastEdited + QString(")"))  ;
        sLastSavedSequence = QString(QString::number(sLastSavedSequence.toInt()+1) ) ;
        lastEdited.setDoor(sLastSavedSequence) ;
        success = lastEdited.writeOv2(outputstream, 2) ;
    }

    for (int i=poiList.size()-1; i>=0 && success; i--) {
        PoiEntry& record = poiList[i] ;
        success = record.writeOv2(outputstream, 2) ;
    }

    outputstream.close() ;
    qDebug() << "PoiCollection::save() =>" << sFilename << " (version: " << sLastEdited << ", sequence: " << sLastSavedSequence << ")" ;
    bListDirty=false ;
    return success ;
}

int PoiCollection::size()
{
   return poiList.size() ;
}

PoiEntry& PoiCollection::at(int i)
{
    return poiList[i] ;
}

bool PoiCollection::add(PoiEntry& newEntry)
{
    remove(newEntry.uuid()) ;
    poiList.insert(0, newEntry) ;
    bListDirty=true ;
    return true ;
}


bool PoiCollection::remove(QString uuid)
{
    int i=poiList.size()-1 ;
    while (i>=0 && poiList[i].uuid() != uuid) i-- ;
    if (i>=0) {
        bListDirty=true ;
        poiList.removeAt(i) ;
        return true ;
    } else {
        return false ;
    }
}

PoiEntry& PoiCollection::find(QString uuid)
{
    int i=poiList.size()-1 ;
    while (i>=0 && poiList[i].uuid() != uuid) i-- ;
    if (i>=0) {
        return poiList[i] ;
    } else {
        return nullPoiEntry ;
    }
}

////////////////////////////////////////////////////////
//
// File I/O
//
//

bool PoiEntry::readOv2(QFile& inputstream)
{
    dirty=false ;
    valid=false ;

    if (inputstream.atEnd()) return false ;

    QByteArray b = inputstream.read(1) ;

    if (b.at(0)==0) {
        // Deleted Record
        long int total = arrayToLong(inputstream.read(4)) ;
        if (total>=13) {
            // Assume deleted type 2 record
            long int llon = arrayToLong(inputstream.read(4)) ;
            long int llat = arrayToLong(inputstream.read(4)) ;
            double lon = (double)llon / 100000.0;
            double lat = (double)llat / 100000.0;
            setLatLon(lat, lon) ;
            QString description = inputstream.read((int)total-13) ;
            setDescription(description) ;
        } else {
            // Assume deleted something else
            inputstream.read((int)total-5) ;
            setDescription("") ;
            setLatLon(0,0) ;
        }
        valid=false ;
        dirty=false ;

    } else if (b.at(0)==1) {
        // Skipper Record
        inputstream.read(20) ;

    } else if (b.at(0)==2) {

        // POI Record
        long int total = arrayToLong(inputstream.read(4)) ;

        // Lat/Lon
        long int llon = arrayToLong(inputstream.read(4)) ;
        long int llat = arrayToLong(inputstream.read(4)) ;
        double lon = (double)llon / 100000.0;
        double lat = (double)llat / 100000.0;

        // Read and parse

        QString description = inputstream.read((int)total-13) ;

        // And store (setDescription parses [door]>phone as required
        setDescription(description) ;
        setLatLon(lat, lon) ;
        dirty=false ;

    } else {
        // Corrupt file
        return false ;

    }

    return true ;
}

bool PoiEntry::writeOv2(QFile& outputstream, int type)
{
    dirty=false ;
    QByteArray buf = QByteArray(4, '\0') ;

    QString desc = description() ;
    QString dr = door() ;
    QString ph = phone() ;

    desc.replace('[',"(") ;
    desc.replace(']',")") ;
    desc.replace('<',"(") ;
    desc.replace('>',")") ;

    if (!dr.isEmpty()) { desc = desc + " [" + dr + "]" ; }
    if (!ph.isEmpty()) { desc = desc + ">" + ph ; }

    buf[0]=type ;
    outputstream.write(buf, 1) ;

    buf = longToArray(desc.length() + 1 + 13) ;
    outputstream.write(buf) ;

    buf = longToArray( (long int)(lon()*100000.0)) ;
    outputstream.write(buf) ;

    buf = longToArray( (long int)(lat()*100000.0)) ;
    outputstream.write(buf) ;

    outputstream.write(desc.toLatin1()) ;
    outputstream.write("\0", 1) ;

    return true ;
}

long int PoiEntry::arrayToLong(QByteArray data)
{
    long int res ;
    res = (unsigned char)data.at(0) ;
    res += ((unsigned char)data.at(1)) << 8 ;
    res += ((unsigned char)data.at(2)) << 16 ;
    res += ((unsigned char)data.at(3)) << 24 ;
    return res ;
}

QByteArray PoiEntry::longToArray(long int data)
{
    QByteArray res ;
    res.append( (unsigned char)(data&0xFF)) ;
    res.append( (unsigned char)((data>>8)&0xFF)) ;
    res.append( (unsigned char)((data>>16)&0xFF)) ;
    res.append( (unsigned char)((data>>24)&0xFF)) ;
    return res ;
}

