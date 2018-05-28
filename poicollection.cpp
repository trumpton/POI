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
    clear() ;
}

void PoiEntry::clear() {
    QUuid quuid = QUuid::createUuid() ;
    sUuid = quuid.toString() ;
    valid=false ; dirty=false ;
    dLat=0 ; dLon=0 ;
    for (int i=0; i<NUMFIELDTYPES; i++) {
        sFields[i].clear() ;
    }
}

bool PoiEntry::isValid() { return valid ; }
bool PoiEntry::isDirty() { return dirty ; }
void PoiEntry::markAsClean() { dirty = false ; }

void PoiEntry::setUuid(QString uuid) { sUuid = uuid ; }
const QString& PoiEntry::uuid() { return sUuid ; }

void PoiEntry::set(PoiEntry::FieldType type, QString data)
{
    if (sFields[(int)type].compare(data)!=0) {
        sFields[(int)type] = data ;
        valid=true ;
        dirty=true ;
    }
}

const QString& PoiEntry::get(PoiEntry::FieldType type) { return sFields[(int)type] ; }

void PoiEntry::setLatLon(double lat, double lon) { dLat = lat ; dLon = lon ; valid=true ; dirty=true ; }
double PoiEntry::lat() { return dLat ; }
double PoiEntry::lon() { return dLon ; }


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

/***************************************************************************

 ** Example GPX File

<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<gpx
xmlns="http://www.topografix.com/GPX/1/1"
xmlns:gpxx = "http://www.garmin.com/xmlschemas/GpxExtensions/v3"
xmlns:xsi = "http://www.w3.org/2001/XMLSchema-instance"
xsi:schemaLocation="http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd
http://www.garmin.com/xmlschemas/GpxExtensions/v3
http://www8.garmin.com/xmlschemas/GpxExtensions/v3/GpxExtensionsv3.xsd"
version="1.1"
creator="creator.com">
   <wpt lat="43.362623" lon="-1.401430">
      <ele>1</ele>
      <time>2018-05-12T08:55:40Z</time>
      <name>Hotel Bellvue</name>
      <cmt>Comment</cmt>
      <desc>Restaurant &amp; Hotel</desc>
      <sym>hotel</sym>
      <type>Hotel</type>
      <extensions>
         <gpxx:WaypointExtension>
            <gpxx:Proximity>10</gpxx:Proximity>
            <gpxx:Categories>
               <gpxx:Category>Hotel</gpxx:Category>
               <gpxx:Category>Restaurant</gpxx:Category>
            </gpxx:Categories>
            <gpxx:Address>
               <gpxx:StreetAddress>Boulevard des Terrasses</gpxx:StreetAddress>
               <gpxx:City>Cambo-les-Bains</gpxx:City>
               <gpxx:State>Nouvelle-Aquitaine</gpxx:State>
               <gpxx:Country>FR</gpxx:Country>
               <gpxx:PostalCode>64250</gpxx:PostalCode>
            </gpxx:Address>
            <gpxx:PhoneNumber Category="Phone">01234567891</gpxx:PhoneNumber>
            <gpxx:PhoneNumber Category="Phone2">01234567892</gpxx:PhoneNumber>
            <gpxx:PhoneNumber Category="Fax">01234567893</gpxx:PhoneNumber>
            <gpxx:PhoneNumber Category="Email">email@email.com</gpxx:PhoneNumber>
            <gpxx:PhoneNumber Category="URL">http://url.com/</gpxx:PhoneNumber>
         </gpxx:WaypointExtension>
      </extensions>
   </wpt>
</gpx>

************************************************************************************************/


//
// Load a GPX File
//
bool PoiCollection::extractXmlData(QDomNode n, const char *tag, PoiEntry::FieldType type, PoiEntry& entry)
{
    QDomNode node = n.firstChildElement(tag) ;
    if (!node.isNull()) {
        QDomElement element = node.toElement() ;
        if (element.isElement()) {
            QString text = element.text() ;
            entry.set(type, text) ;
            return true ;
        }
    }
    return false ;
}

bool PoiCollection::loadGpx(bool importing, QString filename)
{
    if (filename.isEmpty()) filename = sFilename ;
    if (importing) filename = filename + ".gpx" ;
    else filename = filename + ".poi" ;

    if (filename.isEmpty()) return false ;
    poiList.clear() ;

    bool success=true ;

    QDomDocument doc;
     QFile file(filename);
     if (!file.open(QIODevice::ReadOnly) || !doc.setContent(&file))
         return false;

     QDomElement gpx = doc.firstChildElement("gpx") ;
     QDomNodeList wpt = gpx.elementsByTagName("wpt") ;

     for (int i=0; i<wpt.size(); i++) {

         PoiEntry ent ;

         QDomNode n = wpt.item(i) ;

         QDomNamedNodeMap attribs = n.attributes() ;
         QDomNode latnode = n.attributes().namedItem("lat") ;
         QDomNode lonnode = n.attributes().namedItem("lon") ;

         double lat, lon ;

         if (latnode.isAttr()) lat = latnode.nodeValue().toDouble() ;
         if (lonnode.isAttr()) lon = lonnode.nodeValue().toDouble() ;

         ent.setLatLon(lat, lon) ;

         extractXmlData(n, "ele", PoiEntry::GEOELEVATION, ent) ;
         extractXmlData(n, "time", PoiEntry::AUTOTIME, ent) ;
         extractXmlData(n, "name", PoiEntry::EDITEDTITLE, ent) ;
         extractXmlData(n, "cmt", PoiEntry::AUTOCOMMENT, ent) ;
         extractXmlData(n, "desc", PoiEntry::EDITEDDESCR, ent) ;
         extractXmlData(n, "sym", PoiEntry::EDITEDSYMBOL, ent) ;
         extractXmlData(n, "type", PoiEntry::EDITEDTYPE, ent) ;

         QDomNode ext = n.firstChildElement("extension") ;
         if (!ext.isNull()) {

            QDomNode poiext = ext.firstChildElement("poix::WaypointExtension") ;

            if (!poiext.isNull()) {

                extractXmlData(poiext, "poix::EditedStreet", PoiEntry::EDITEDSTREET, ent) ;
                extractXmlData(poiext, "poix::EditedDoorNumber", PoiEntry::EDITEDDOOR, ent) ;
                extractXmlData(poiext, "poix::EditedStreet", PoiEntry::EDITEDSTREET, ent) ;
                extractXmlData(poiext, "poix::EditedCity", PoiEntry::EDITEDCITY, ent) ;
                extractXmlData(poiext, "poix::EditedState", PoiEntry::EDITEDSTATE, ent) ;
                extractXmlData(poiext, "poix::EditedPostcode", PoiEntry::EDITEDPOSTCODE, ent) ;
                extractXmlData(poiext, "poix::EditedCountry", PoiEntry::EDITEDCOUNTRY, ent) ;
                extractXmlData(poiext, "poix::GeoAddress", PoiEntry::GEOSTREET, ent) ;
                extractXmlData(poiext, "poix::GeoDoorNumber", PoiEntry::GEODOOR, ent) ;
                extractXmlData(poiext, "poix::GeoStreet", PoiEntry::GEOSTREET, ent) ;
                extractXmlData(poiext, "poix::GeoCity", PoiEntry::GEOCITY, ent) ;
                extractXmlData(poiext, "poix::GeoState", PoiEntry::GEOSTATE, ent) ;
                extractXmlData(poiext, "poix::GeoPostcode", PoiEntry::GEOPOSTCODE, ent) ;
                extractXmlData(poiext, "poix::GeoCountry", PoiEntry::GEOCOUNTRY, ent) ;
                extractXmlData(poiext, "poix::PhoneNumber1", PoiEntry::EDITEDPHONE1, ent) ;
                extractXmlData(poiext, "poix::PhoneNumber2", PoiEntry::EDITEDPHONE2, ent) ;
                extractXmlData(poiext, "poix::Email", PoiEntry::EDITEDEMAIL, ent) ;
                extractXmlData(poiext, "poix::URL", PoiEntry::EDITEDURL, ent) ;
                extractXmlData(poiext, "poix::Geocoded", PoiEntry::GEOCODED, ent) ;


            } else {

                // POI Extensions not present - try GPXX Extensions
                // Note import of phone numbers / email addresses and urls not supported
                QDomNode wptext = ext.firstChildElement("gpxx:WaypointExtension") ;
                if (!wptext.isNull()) {
                    QDomNode address = wptext.firstChildElement("gpxx::Address") ;
                    if (!address.isNull()) {
                        extractXmlData(address, "gpxx::StreetAddress", PoiEntry::EDITEDSTREET, ent) ;
                        extractXmlData(address, "gpxx::City", PoiEntry::EDITEDCITY, ent) ;
                        extractXmlData(address, "gpxx::State", PoiEntry::EDITEDSTATE, ent) ;
                        extractXmlData(address, "gpxx::Country", PoiEntry::EDITEDCOUNTRY, ent) ;
                        extractXmlData(address, "gpxx::PostalCode", PoiEntry::EDITEDPOSTCODE, ent) ;
                    }
                }
            }

         }
         // add to list
         ent.markAsClean() ;
         poiList.append(ent) ;
     }
    bListDirty=false ;
    return success ;
}

//
// Save a GPX File
//


bool PoiCollection::storeXmlData(QDomDocument& doc, PoiEntry::FieldType type, PoiEntry& entry, QDomElement element, const char *tag, QString attribute, QString attrval)
{
    return storeXmlData(doc, entry.get(type), element, tag, attribute, attrval) ;
}


bool PoiCollection::storeXmlData(QDomDocument& doc, QString text, QDomElement element, const char *tag, QString attribute, QString attrval)
{
    QDomElement newelement = doc.createElement(QString(tag)) ;
    QDomText newtext = doc.createTextNode(text) ;
    if (!attribute.isEmpty()) newelement.setAttribute(attribute, attrval);
    newelement.appendChild(newtext) ;
    element.appendChild(newelement) ;
    return true ;
}

bool PoiCollection::saveGpx(bool exporting, QString filename)
{

    if (filename.isEmpty()) filename = sFilename ;
    if (filename.isEmpty()) return false ;

    if (exporting) filename = filename + QString(".gpx") ;
    else filename = filename + QString(".poi") ;

    bool success=true ;

    QDomDocument doc;
     QFile file(filename);
     if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
         return false;

     QDomElement gpx = doc.createElement("gpx") ;

     if (exporting) {
         gpx.setAttribute("xmlns", "http://www.topografix.com/GPX/1/1") ;
         gpx.setAttribute("xmlns:gpxx", "http://www.garmin.com/xmlschemas/GpxExtensions/v3") ;
         gpx.setAttribute("xmlns:poix", "http://www.trumpton.uk/xmlschemas/GpxExtensions/v3") ;
         gpx.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance") ;
         gpx.setAttribute("xsi:schemaLocation", "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd http://www.garmin.com/xmlschemas/GpxExtensions/v3 http://www8.garmin.com/xmlschemas/GpxExtensions/v3/GpxExtensionsv3.xsd") ;
         gpx.setAttribute("version", "1.1") ;
         gpx.setAttribute("creator", "trumpton.org") ;
     }

     for (int i=0; i<poiList.size(); i++) {

         PoiEntry& ent = poiList[i] ;

        QDomElement wpt = doc.createElement("wpt") ;
        QDomElement extension = doc.createElement("extension") ;
        QDomElement wptext = doc.createElement("gpxx::WaypointExtension") ;
        QDomElement addr = doc.createElement("gpxx::Address") ;
        QDomElement poiext = doc.createElement("poix::WaypointExtension") ;

        // Attach nodes in hierarchy
        gpx.appendChild(wpt) ;
          wpt.appendChild(extension) ;
            if (!exporting) extension.appendChild(poiext) ;
            if (exporting) extension.appendChild(wptext) ;
                wptext.appendChild(addr) ;

        // Set Lat/Lon
        wpt.setAttribute("lat", QString("%1").arg(ent.lat())) ;
        wpt.setAttribute("lon", QString("%1").arg(ent.lon())) ;

        storeXmlData(doc, PoiEntry::EDITEDPHONE1, ent, wptext, "gpxx::PhoneNumber", "Category", "Phone") ;
        storeXmlData(doc, PoiEntry::EDITEDPHONE2, ent, wptext, "gpxx::PhoneNumber", "Category", "Phone2") ;
        storeXmlData(doc, PoiEntry::EDITEDEMAIL, ent, wptext, "gpxx::PhoneNumber", "Category", "Email") ;
        storeXmlData(doc, PoiEntry::EDITEDURL, ent, wptext, "gpxx::PhoneNumber", "Category", "URL") ;

        // Populate nodes
        storeXmlData(doc, PoiEntry::GEOELEVATION, ent, wpt, "ele") ;
        storeXmlData(doc, PoiEntry::AUTOTIME, ent, wpt, "time") ;
        storeXmlData(doc, PoiEntry::EDITEDTITLE, ent, wpt, "name") ;
        storeXmlData(doc, PoiEntry::AUTOCOMMENT, ent, wpt, "cmt") ;
        storeXmlData(doc, PoiEntry::EDITEDDESCR, ent, wpt, "desc") ;
        storeXmlData(doc, PoiEntry::EDITEDSYMBOL, ent, wpt, "sym") ;
        storeXmlData(doc, PoiEntry::EDITEDTYPE, ent, wpt, "type") ;

        storeXmlData(doc, PoiEntry::GEODOOR, ent, poiext, "poix::GeoDoorNumber") ;
        storeXmlData(doc, PoiEntry::GEOSTREET, ent, poiext, "poix::GeoStreet") ;
        storeXmlData(doc, PoiEntry::GEOCITY, ent, poiext, "poix::GeoCity") ;
        storeXmlData(doc, PoiEntry::GEOSTATE, ent, poiext, "poix::GeoState") ;
        storeXmlData(doc, PoiEntry::GEOPOSTCODE, ent, poiext, "poix::GeoPostcode") ;
        storeXmlData(doc, PoiEntry::GEOCOUNTRY, ent, poiext, "poix::GeoCountry") ;
        storeXmlData(doc, PoiEntry::EDITEDDOOR, ent, poiext, "poix::EditedDoorNumber") ;
        storeXmlData(doc, PoiEntry::EDITEDSTREET, ent, poiext, "poix::EditedStreet") ;
        storeXmlData(doc, PoiEntry::EDITEDCITY, ent, poiext, "poix::EditedCity") ;
        storeXmlData(doc, PoiEntry::EDITEDSTATE, ent, poiext, "poix::EditedState") ;
        storeXmlData(doc, PoiEntry::EDITEDPOSTCODE, ent, poiext, "poix::EditedPostcode") ;
        storeXmlData(doc, PoiEntry::EDITEDCOUNTRY, ent, poiext, "poix::EditedCountry") ;
        storeXmlData(doc, PoiEntry::EDITEDPHONE1, ent, poiext, "poix::PhoneNumber1") ;
        storeXmlData(doc, PoiEntry::EDITEDPHONE2, ent, poiext, "poix::PhoneNumber2") ;
        storeXmlData(doc, PoiEntry::EDITEDEMAIL, ent, poiext, "poix::Email") ;
        storeXmlData(doc, PoiEntry::EDITEDURL, ent, poiext, "poix::URL") ;
        storeXmlData(doc, PoiEntry::GEOCODED, ent, poiext, "poix::Geocoded") ;

        QString door, street, fullstreet, city, state, country, postcode ;

        if (ent.get(PoiEntry::EDITEDDOOR).isEmpty()) {
            door = ent.get(PoiEntry::GEODOOR) ;
        } else {
            door = ent.get(PoiEntry::EDITEDDOOR) ;
        }

        if (ent.get(PoiEntry::EDITEDSTREET).isEmpty()) {
            street = ent.get(PoiEntry::GEOSTREET) ;
        } else {
            street = ent.get(PoiEntry::EDITEDSTREET) ;
        }

        if (ent.get(PoiEntry::EDITEDCITY).isEmpty()) {
            city = ent.get(PoiEntry::GEOCITY) ;
        } else {
            city = ent.get(PoiEntry::EDITEDCITY) ;
        }

        if (ent.get(PoiEntry::EDITEDSTATE).isEmpty()) {
            state = ent.get(PoiEntry::GEOSTATE) ;
        } else {
            state = ent.get(PoiEntry::EDITEDSTATE) ;
        }

        if (ent.get(PoiEntry::EDITEDCOUNTRY).isEmpty()) {
            country = ent.get(PoiEntry::GEOCOUNTRY) ;
        } else {
            country = ent.get(PoiEntry::EDITEDCOUNTRY) ;
        }

        if (ent.get(PoiEntry::EDITEDPOSTCODE).isEmpty()) {
            postcode = ent.get(PoiEntry::GEOPOSTCODE) ;
        } else {
            postcode = ent.get(PoiEntry::EDITEDPOSTCODE) ;
        }

        if (door.isEmpty()) {
            fullstreet = street ;
        } else {
            fullstreet = door + " " + street ;
        }
        storeXmlData(doc, fullstreet, addr, "gpxx::StreetAddress") ;
        storeXmlData(doc, city, addr, "gpxx::City") ;
        storeXmlData(doc, state, addr, "gpxx::State") ;
        storeXmlData(doc, country, addr, "gpxx::Country") ;
        storeXmlData(doc, postcode, addr, "gpxx::PostalCode") ;

        if (!exporting) ent.markAsClean();

     }
     doc.appendChild(gpx) ;

     QTextStream stream( &file );
     stream << doc.toString();
     file.close();

     if (!exporting) {
         updateLastEdited() ;
        bListDirty=false ;
     }

    return success ;
}

//
// Export to OV2
//
bool PoiCollection::saveOv2()
{
// TODO: Untangle save function as this is now an export function
    if (sFilename.isEmpty()) return false ;

    QFile outputstream(sFilename + QString(".ov2")) ;
    if (!outputstream.open(QIODevice::ReadWrite))
        return false ;

    bool success=true ;

    // Add Source Record (deleted record, lat=-180, lon=1)
    PoiEntry source ;
    source.setLatLon(-180.0, 1.0) ;
    source.set(PoiEntry::EDITEDDESCR, QString("TomTom / Google Points of Interest Editor - www.trumpton.uk")) ;
    success = source.writeOv2(outputstream, 0) ;

    for (int i=poiList.size()-1; i>=0 && success; i--) {
        PoiEntry& record = poiList[i] ;
        success = record.writeOv2(outputstream, 2) ;
    }

    outputstream.close() ;
    qDebug() << "PoiCollection::save() =>" << sFilename << " (version: " << sLastEdited << ", sequence: " << sLastSavedSequence << ")" ;
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

bool PoiEntry::writeOv2(QFile& outputstream, int type)
{
    dirty=false ;
    QByteArray buf = QByteArray(4, '\0') ;

    QString door = get(PoiEntry::EDITEDDOOR) ;
    QString title = get(PoiEntry::EDITEDTITLE) ;
    QString phone = get(PoiEntry::EDITEDPHONE1) ;
    if (door.isEmpty()) door=get(PoiEntry::GEODOOR) ;

    if (!door.isEmpty()) { title = title + " [" + door + "]" ; }
    if (!phone.isEmpty()) { title = title  + ">" + phone ; }

    QByteArray titlebuf = title.toLatin1() ;

    buf[0]=type ;
    outputstream.write(buf, 1) ;

    buf = longToArray(titlebuf.length() + 1 + 13) ;
    outputstream.write(buf) ;

    buf = longToArray( (long int)(lon()*100000.0)) ;
    outputstream.write(buf) ;

    buf = longToArray( (long int)(lat()*100000.0)) ;
    outputstream.write(buf) ;

    outputstream.write(titlebuf) ;
    outputstream.write("\0", 1) ;

    return true ;
}


// Import OV2 (See https://www.tomtom.com/lib/doc/ttnavsdk3_manual.pdf for format)
bool PoiCollection::importOv2(QString filename)
{
    QFile inputstream(filename) ;
    if (!inputstream.open(QIODevice::ReadOnly))
        return false ;

    bool success ;
    do {
        PoiEntry record ;
        success=record.importOv2(inputstream) ;
        if (record.isValid()) {
            poiList.append(record) ;
        }
    } while (success && !inputstream.atEnd()) ;

    inputstream.close() ;
    bListDirty=true ;
    return success ;
}


// Assume that OV2 is encoded as ISO-8859-1
bool PoiEntry::importOv2(QFile& inputstream)
{
    if (inputstream.atEnd()) return false ;

    QByteArray b = inputstream.read(1) ;

    if (b.at(0)==0) {
        // Deleted Record
        long int total = arrayToLong(inputstream.read(4)) ;
        if (total<6) {
            return false ;
        } else {
            inputstream.read((int)total-5) ;
            return true ;
        }

    } else if (b.at(0)==1) {
        // Skipper Record
        inputstream.read(20) ;
        return true;

    } else if (b.at(0)==2) {

        // POI Record
        long int total = arrayToLong(inputstream.read(4)) ;
        if (total<14) {
            return false ;
        } else {
            // Lat/Lon
            long int llon = arrayToLong(inputstream.read(4)) ;
            long int llat = arrayToLong(inputstream.read(4)) ;
            double lon = (double)llon / 100000.0;
            double lat = (double)llat / 100000.0;
    
            // Read and parse
            QByteArray ba = inputstream.read((int)total-13) ;
            QString description = toUtf8(ba) ;
            QString door ;
            QString phone ;

            QRegExp rxq("^(.*)>(.+)>(.+)") ;
            if (rxq.indexIn(description)>=0) {
                description=rxq.cap(1).trimmed() ;
                phone=rxq.cap(2).trimmed() ;
            }
            QRegExp rxp("^(.*)>(.+)") ;
            if (rxp.indexIn(description)>=0) {
                description=rxp.cap(1).trimmed() ;
                phone=rxp.cap(2).trimmed() ;
            }
            QRegExp rxn("^(.*)\\[(.+)\\]") ;
            if (rxn.indexIn(description)>=0) {
                door=rxn.cap(2).trimmed() ;
            }

            set(PoiEntry::EDITEDTITLE, description) ;
            set(PoiEntry::EDITEDDOOR, door) ;
            set(PoiEntry::EDITEDPHONE1, phone) ;

            setLatLon(lat, lon) ;
            return true ;
        }

    } else {
        // Corrupt file
        return false ;

    }
}




// Convert string to UTF-8
QString PoiEntry::toUtf8(QByteArray ba)
{
    QString s = QString::fromUtf8(ba);
    if (s.toUtf8() != ba) {
      s = QString::fromLatin1(ba);
    }
    return s ;
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

void PoiEntry::copyFrom(PoiEntry& source)
{
    for (int i=0; i<PoiEntry::NUMFIELDTYPES; i++) {
        set( (PoiEntry::FieldType)i, source.get( (PoiEntry::FieldType)i ) ) ;
    }
    setLatLon(source.lat(), source.lon()) ;
    valid=true ;
    dirty=false ;
}
