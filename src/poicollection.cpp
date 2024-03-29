/*
 * Application: POI Manager
 * File: poicollection.cpp
 *
 * Class to manage/load/store/sort etc. a collection of Points of Interest
 *
 */

/*
 *
 * POI Manager
 * Copyright (C) 2021  "Steve Clarke www.vizier.uk/poi"
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Any modification to the code must be contributed back to the community.
 *
 * Redistribution and use in binary or source form with or without modification
 * is permitted provided that the following conditions are met:
 *
 * Clause 7b - attribution for the original author shall be provided with the
 * inclusion of the above Copyright statement in its entirity, which must be
 * clearly visible in each application source file, in any documentation and also
 * in a pop-up window in the application itself. It is requested that the charity
 * donation link to Guide Dogs for the Blind remain within the program, and any
 * derivitive thereof.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "poicollection.h"
#include <QUuid>
#include <QString>
#include <QRegularExpression>
#include <QDateTime>
#include <QDebug>
#include <math.h>
#include <QDir>
#include <QFileInfo>
#include <QTimeZone>
#include <QList>
#include <QMessageBox>



#include "segmentchooser.h"

#define STAR  QChar(0x2605)

// Local function to calculate Open Location Code
bool calculateOlc(double lat, double lon, int resolution, QString *olc) ;

//
// Duration Calculations:
//
// Estimate used if real time not presented
//
// Walking time = 3 miles per hour (4.82kph)
// Climbing Time = 1 minute for every 10 metres ascent
// Descent Time = 1 minute for every 20 metre descent
//

// Returns distance between 2 coordinates in metres
double _distanceFrom(double lat1, double lon1, double lat2, double lon2)
{
    double dlat, dlon, a, c ;

    double R = 6371e3;
    double d2r = 3.141592654 / 180 ;
    double rlat1 = lat1 * d2r ;
    double rlat2 = lat2 * d2r ;
    double rlon1 = lon1 * d2r ;
    double rlon2 = lon2 * d2r ;

    dlat = rlat2-rlat1 ;
    dlon = rlon2-rlon1 ;
    a = sin(dlat/2) * sin(dlat/2) +
            cos(rlat1) * cos(rlat2) * sin(dlon/2) * sin(dlon/2) ;
    c = 2 * atan2(sqrt(a), sqrt(1-a)) ;

    return R * c ;
}

// ///////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////

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
    dLat=0 ; dLon=0 ;
    updateOlc() ;
    iSequence = 0 ;
    for (int i=0; i<NUMFIELDTYPES; i++) {
        sFields[i].clear() ;
    }
    valid=false ; dirty=false ; flagvalue=false ;
}

bool PoiEntry::isValid() { return valid ; }
bool PoiEntry::isDirty() { return dirty ; }
void PoiEntry::markAsClean() { dirty = false ; }

bool PoiEntry::flag() { return flagvalue ; }
void PoiEntry::setFlag(bool state) { flagvalue = state ; }


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

double PoiEntry::distanceFrom(PoiEntry &other)
{
    return _distanceFrom(dLat, dLon, other.lat(), other.lon()) ;
}

const QString& PoiEntry::get(PoiEntry::FieldType type) { return sFields[(int)type] ; }

const QString& PoiEntry::get(FieldType type1, FieldType type2) {
    if (sFields[(int)type1].isEmpty()) {
        return sFields[(int)type2] ;
    } else {
        return sFields[(int)type1] ;
    }
}


void PoiEntry::setDate(QString date, int timezoneoffset) {

    QRegularExpression re("(\\d\\d\\d\\d):(\\d\\d):(\\d\\d) (\\d\\d):(\\d\\d):(\\d\\d)") ;
    QRegularExpressionMatch match ;
    match = re.match(date) ;
    if (match.hasMatch()) {
        // Handle '2018:08:30 15:52:14' Format
        QDate qdate(match.captured(1).toInt(), match.captured(2).toInt(), match.captured(3).toInt()) ;
        QTime qtime(match.captured(4).toInt(), match.captured(5).toInt(), match.captured(6).toInt()) ;
        tDate = QDateTime(qdate, qtime, Qt::UTC) ;
    } else {
        // Handle ISO Date Format
        tDate = tDate.fromString(date, Qt::ISODate) ;
    }
    tDate = tDate.addSecs(timezoneoffset*3600) ;
    tDate.setTimeSpec(Qt::UTC);
}


QDateTime PoiEntry::date() { return tDate ; }

void PoiEntry::setLatLon(double lat, double lon) { dLat = lat ; dLon = lon ; updateOlc() ; valid=true ; dirty=true ; }

double PoiEntry::lat() { return dLat ; }

double PoiEntry::lon() { return dLon ; }

const QString &PoiEntry::olc() {
    if (sFields[OLC].isEmpty()) { updateOlc() ; }
    return sFields[OLC] ;
}

void PoiEntry::setSequence(int seq) { iSequence = seq ; }
int PoiEntry::sequence() { return iSequence ; }

bool PoiEntry::setImage(QImage img) {
    bool status = pPixmap.convertFromImage(img.scaled(180,130)) ;
    return status ;
}

QPixmap& PoiEntry::pixmap() { return pPixmap ; }

bool PoiEntry::writeOv2(QFile& outputstream, int type)
{
    dirty=false ;
    QByteArray buf = QByteArray(4, '\0') ;

    QString door = get(PoiEntry::EDITEDDOOR) ;
    QString title = get(PoiEntry::EDITEDTITLE) ;
    QString phone = get(PoiEntry::EDITEDPHONE1) ;
    QString typesrc = get(PoiEntry::EDITEDTYPE) ;

    // Extract abbreviation for type
    QString enttype ;
    bool lastchar=false ;
    for (int i=0; i<typesrc.length(); i++) {
         if (!lastchar && typesrc.at(i).isLetter()) enttype = enttype + typesrc.at(i).toUpper() ;
         lastchar = typesrc.at(i).isLetter() ;
    }
    title = title.replace(STAR, '*') ;
    if (!enttype.isEmpty()) { title = enttype + ": " + title ; }
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
            qDebug() << "OV2 1: deleted record" ;
            return true ;
        }

    } else if (b.at(0)==1) {
        // Skipper Record
        inputstream.read(20) ;
        qDebug() << "OV2 1: skipper recors" ;
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

            QRegularExpressionMatch match ;

            QRegularExpression rxq("^(.*)>(.+)>(.+)") ;
            if (description.indexOf(rxq, 0, &match)>=0) {
                description=match.captured(1).trimmed() ;
                phone=match.captured(2).trimmed() ;
            }
            QRegularExpression rxp("^(.*)>(.+)") ;
            if (description.indexOf(rxp, 0, &match)>=0) {
                description=match.captured(1).trimmed() ;
                phone=match.captured(2).trimmed() ;
            }

            QRegularExpression rxn("^(.*)\\[(.+)\\]") ;
            if (description.indexOf(rxn, 0, &match)>=0) {
                door=match.captured(2).trimmed() ;
            }

            set(PoiEntry::EDITEDTITLE, description) ;
            set(PoiEntry::EDITEDDOOR, door) ;
            set(PoiEntry::EDITEDPHONE1, phone) ;

            setLatLon(lat, lon) ;

            qDebug() << "OV2 2: " << description ;

            return true ;
        }

    } else {
        // Corrupt file

        qDebug() << "OV2 " << (int)b.at(0) << ": ???" ;

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
    QString thisId = sUuid ;
    *this = source ;
    sUuid = thisId ;
    dirty = false ;
}


bool PoiEntry::updateOlc()
{
    // resolution = 0,1,2,3,4 => 13.9m, 2.8m, 56x87cm, 11x22cm, 2x5cm, 4x14mm
    int resolution=2 ;
    QString olc="" ;
    calculateOlc(dLat, dLon, resolution, &olc) ;
    // Store Open Location Code
    set(OLC, olc) ;
    return true ;
}


// ///////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////

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


int PoiCollection::rating()
{
    return iRating ;
}

void PoiCollection::clearFlags()
{
    int len = poiList.count() ;
    for (int i=0; i<len; i++) {
        poiList[i].setFlag(false) ;
    }
}

int PoiCollection::setRating(int rating)
{
    iRating = rating ;
    return iRating ;
}

void PoiCollection::updateLastEdited()
{
    QDateTime now = QDateTime::currentDateTimeUtc() ;
    sLastEdited = now.toString("yyyy-MM-dd-hh:mm:ss") ;
}

bool PoiCollection::clear()
{
    QUuid quuid = QUuid::createUuid() ;
    sUuid = quuid.toString() ;
    QUuid qtrackuuid = QUuid::createUuid() ;
    sTrackUuid = qtrackuuid.toString() ;
    sFilename = "" ;
    poiList.clear() ;
    trackList.clear() ;
    bListDirty=false ;
    sLastEdited = "" ;
    sLastSavedSequence = "" ;
    dtracklength = 0 ;
    dheightgain = 0 ;
    dheightloss = 0 ;
    dtracktime = 0 ;
    dtracktimeest = 0 ;
    sName = "" ;
    iRating = 0 ;
    return true ;
}

const QString& PoiCollection::uuid() { return sUuid ; }
const QString& PoiCollection::trackUuid() { return sTrackUuid ; }

void PoiCollection::setUuid(QString uuid)
{
    sUuid = uuid ;
}

void PoiCollection::setFilename(QString filename) { sFilename = filename ; }
const QString& PoiCollection::filename() { return sFilename ; }

bool PoiCollection::isDirty()
{
    bool dirty=bListDirty ;
    for (int i=poiList.size()-1; i>=0; i--) {
        dirty |= poiList[i].isDirty() ;
    }
    for (int i=trackList.size()-1; i>=0; i--) {
        dirty |= trackList[i].isDirty() ;
    }
    return dirty ;
}

void PoiCollection::markAsDirty() { bListDirty = true ; }

bool poiLessThan (PoiEntry &v1, PoiEntry &v2)
{
        return v1.sequence() < v2.sequence() ;
}

bool poiDateLessThan (PoiEntry &v1, PoiEntry &v2)
{
        return v1.date() < v2.date() ;
}

bool trackLessThan (TrackEntry &v1, TrackEntry &v2)
{
        return v1.sequence() < v2.sequence() ;
}

void PoiCollection::sortBySequence()
{
    std::sort(poiList.begin(), poiList.end(), poiLessThan) ;

    int seq = 0 ;

    for (int i=0; i<poiList.size(); i++) {
        poiList[i].setSequence(seq) ;
        seq+=2 ;
    }

    std::sort(trackList.begin(), trackList.end(), trackLessThan) ;

    for (int i=0; i<trackList.size(); i++) {
        trackList[i].setSequence(seq) ;
        seq+=2 ;
    }

    calculateTrack() ;
}

void PoiCollection::reorderWaypointByDate()
{
    std::sort(poiList.begin(), poiList.end(), poiDateLessThan) ;

    int seq = 0 ;

    for (int i=0; i<poiList.size(); i++) {
        poiList[i].setSequence(seq) ;
        seq+=2 ;
    }
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

QString& PoiCollection::name() {
    return sName ;
}

void PoiCollection::setName(QString name) {
    if (sName.compare(name)!=0) {
        sName = name ;
        bListDirty = true ;
    }
}


QString& PoiCollection::formattedName(bool includerating, bool includeduration, bool includedistance, bool includeheight, bool asfilename, bool starasasterisk)
{

    sFormattedName.clear() ;

    // Set base name

    sFormattedName = sName ;

    // Attach Rating

    if (includerating && iRating>0) {
        sFormattedName = sFormattedName + " " ;
        for (int i=0; i<iRating; i++) {
            sFormattedName = sFormattedName + STAR ;
        }
        sFormattedName = sFormattedName + QString(" ") ;
    }

    // Attach Duration

    if (includeduration) {
        long int duration = trackTimeEst() ;
        if (!sFormattedName.isEmpty() && duration>0) {
            sFormattedName = sFormattedName + QString(" ") + QString::number((long int)(duration/60)) + QString("h") + (duration%60<10?QString("0"):QString("")) + QString::number((long int)(duration%60)) ;
        }
    }

    // Attach Distance

    if (includedistance) {
        double distance = trackLength() ;
        if (!sFormattedName.isEmpty() && distance>0) {
            if (includeduration) { sFormattedName = sFormattedName + "," ; }
            sFormattedName = sFormattedName + QString(" ") + QString::number(distance/1000,'f',1) + QString("km") ;
        }
    }

    // Attach Height

    if (includeheight) {
        double climb = heightGain() ;
        if (!sFormattedName.isEmpty() && climb>0) {
            if (includeduration || includedistance) { sFormattedName = sFormattedName + "," ; }
            sFormattedName = sFormattedName + QString(" ") + QString::number(climb,'f',0) + QString("m") ;
        }
    }

    // Force Stars to be asterisks

    if (starasasterisk) sFormattedName = sFormattedName.replace(STAR, '*') ;

    // Format as a filename
    if (asfilename) {
        sFormattedName = sFormattedName.replace(" ","") ;
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
        // Replace 'reserved' Unix filename characters
        sFormattedName = sFormattedName.replace("/","-") ;
#else
        // Replace 'reserved' Windows filename characters
        sFormattedName = sFormattedName.replace("\\","-") ;
        sFormattedName = sFormattedName.replace(":",";") ;
        sFormattedName = sFormattedName.replace(STAR,"x") ;
        sFormattedName = sFormattedName.replace("*","x") ;
        sFormattedName = sFormattedName.replace("?",".") ;
        sFormattedName = sFormattedName.replace("|",".") ;
        sFormattedName = sFormattedName.replace(">",")") ;
        sFormattedName = sFormattedName.replace("<","(") ;
#endif
    }
    return sFormattedName ;
}


QString PoiCollection::loadGpx(QString filename)
{
    static QString msg ;
    int seq = 0 ;

    if (filename.isEmpty()) filename = sFilename ;
    if (filename.isEmpty()) return QString("") ;
    clear() ;
    sFilename = filename ;

    QFileInfo fileinfo(sFilename) ;
    QDir gpxDir(fileinfo.absoluteDir());

    bool success=true ;

    QDomDocument doc;
     QFile file(filename);
     if (!file.open(QIODevice::ReadOnly)) {
         msg = QString("Enable to open file") ;
         return msg ;
     }

     QString errorStr;
     int errorLine;
     int errorColumn;

     if (!doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn)) {
         msg = QString("Error loading: ") + filename + QString(": ") + errorStr +
                 QString(" - Line: ") + QString::number(errorLine) + QString(", Column: ") +
                 QString::number(errorColumn) ;
         return msg ;
     }

     QDomElement gpx = doc.firstChildElement("gpx") ;
     QDomNodeList wpt = gpx.elementsByTagName("wpt") ;

     // Load File Details

     QDomElement metadataelement = gpx.firstChildElement("metadata") ;

     if (metadataelement.isElement()) {

         QDomElement name = doc.createElement("name") ;
         QDomText text = doc.createTextNode(sName) ;
         name.appendChild(text) ;

         QDomElement uuidelement = metadataelement.firstChildElement("poix:uuid") ;
         if (uuidelement.isElement()) sUuid = uuidelement.text() ;

         QDomElement ratingelement = metadataelement.firstChildElement("poix:rating") ;
         if (ratingelement.isElement()) iRating = ratingelement.text().toInt() ;

         QDomElement nameuserelement = metadataelement.firstChildElement("poix:name") ;
         if (nameuserelement.isElement()) sName = nameuserelement.text() ;

         if (sName.isEmpty()) {
             QDomElement nameelement = metadataelement.firstChildElement("name") ;
             if (nameelement.isElement()) sName = nameelement.text() ;
         }

     }

     // Load Waypoints

     for (int i=0; i<wpt.size(); i++) {

         PoiEntry ent ;

         QDomNode n = wpt.item(i) ;

         QDomNode latnode = n.attributes().namedItem("lat") ;
         QDomNode lonnode = n.attributes().namedItem("lon") ;

         double lat=-360, lon=-360 ;

         if (latnode.isAttr()) lat = latnode.nodeValue().toDouble() ;
         if (lonnode.isAttr()) lon = lonnode.nodeValue().toDouble() ;
         if (lat>-360.0 && lon>-360.0) {
             ent.setLatLon(lat, lon) ;
         }

         extractXmlData(n, "ele", PoiEntry::GEOELEVATION, ent) ;
         extractXmlData(n, "time", PoiEntry::AUTOTIME, ent) ;
         extractXmlData(n, "name", PoiEntry::EDITEDTITLE, ent) ;
         extractXmlData(n, "cmt", PoiEntry::AUTOCOMMENT, ent) ;
         extractXmlData(n, "desc", PoiEntry::EDITEDDESCR, ent) ;
         extractXmlData(n, "sym", PoiEntry::EDITEDSYMBOL, ent) ;
         extractXmlData(n, "type", PoiEntry::EDITEDTYPE, ent) ;

         QDomNode ext = n.firstChildElement("extension") ;
         if (!ext.isNull()) {

            QDomNode poiext = ext.firstChildElement("poix:WaypointExtension") ;

            if (!poiext.isNull()) {

                // Set waypoint UUID
                QDomElement extensionuuid = poiext.firstChildElement("poix:Uuid") ;
                if (extensionuuid.isElement()) {
                    ent.setUuid(extensionuuid.text()) ;
                }

                // Set waypoint field entries
                extractXmlData(poiext, "poix:EditedStreet", PoiEntry::EDITEDSTREET, ent) ;
                extractXmlData(poiext, "poix:EditedDoorNumber", PoiEntry::EDITEDDOOR, ent) ;
                extractXmlData(poiext, "poix:EditedStreet", PoiEntry::EDITEDSTREET, ent) ;
                extractXmlData(poiext, "poix:EditedCity", PoiEntry::EDITEDCITY, ent) ;
                extractXmlData(poiext, "poix:EditedState", PoiEntry::EDITEDSTATE, ent) ;
                extractXmlData(poiext, "poix:EditedPostcode", PoiEntry::EDITEDPOSTCODE, ent) ;
                extractXmlData(poiext, "poix:EditedCountry", PoiEntry::EDITEDCOUNTRY, ent) ;
                extractXmlData(poiext, "poix:GeoAddress", PoiEntry::GEOSTREET, ent) ;
                extractXmlData(poiext, "poix:GeoDoorNumber", PoiEntry::GEODOOR, ent) ;
                extractXmlData(poiext, "poix:GeoStreet", PoiEntry::GEOSTREET, ent) ;
                extractXmlData(poiext, "poix:GeoCity", PoiEntry::GEOCITY, ent) ;
                extractXmlData(poiext, "poix:GeoState", PoiEntry::GEOSTATE, ent) ;
                extractXmlData(poiext, "poix:GeoPostcode", PoiEntry::GEOPOSTCODE, ent) ;
                extractXmlData(poiext, "poix:GeoCountry", PoiEntry::GEOCOUNTRY, ent) ;
                extractXmlData(poiext, "poix:GeoCountryCode", PoiEntry::GEOCOUNTRYCODE, ent) ;
                extractXmlData(poiext, "poix:PhoneNumber1", PoiEntry::EDITEDPHONE1, ent) ;
                extractXmlData(poiext, "poix:PhoneNumber2", PoiEntry::EDITEDPHONE2, ent) ;
                extractXmlData(poiext, "poix:Email", PoiEntry::EDITEDEMAIL, ent) ;
                extractXmlData(poiext, "poix:URL", PoiEntry::EDITEDURL, ent) ;
                extractXmlData(poiext, "poix:Geocoded", PoiEntry::GEOCODED, ent) ;
                extractXmlData(poiext, "poix:Date", PoiEntry::DATETIME, ent) ;

                // Extract date/time
                QString date = ent.get(PoiEntry::DATETIME) ;
                ent.setDate(date) ;

                // Ensure photo path is absolute
                extractXmlData(poiext, "poix:PhotoFilename", PoiEntry::PHOTOFILENAME, ent) ;
                QString photopath = gpxDir.cleanPath(gpxDir.absoluteFilePath(ent.get(PoiEntry::PHOTOFILENAME)));
                ent.set(PoiEntry::PHOTOFILENAME, photopath) ;

                extractXmlData(poiext, "poix:PhotoLat", PoiEntry::PHOTOLAT, ent) ;
                extractXmlData(poiext, "poix:PhotoLon", PoiEntry::PHOTOLON, ent) ;
                extractXmlData(poiext, "poix:PhotoElevation", PoiEntry::PHOTOELEVATION, ent) ;
                extractXmlData(poiext, "poix:PhotoDate", PoiEntry::PHOTODATE, ent) ;
                QString photo = ent.get(PoiEntry::PHOTOFILENAME) ;
                if (!photo.isEmpty()) {
                    QImage img ;
                    img.load(photo) ;
                    ent.setImage(img) ;
                }

            } else {

                // POI Extensions not present - try GPXX Extensions
                // Note import of phone numbers / email addresses and urls not supported
                QDomNode wptext = ext.firstChildElement("gpxx:WaypointExtension") ;
                if (!wptext.isNull()) {
                    QDomNode address = wptext.firstChildElement("gpxx:Address") ;
                    if (!address.isNull()) {
                        extractXmlData(address, "gpxx:StreetAddress", PoiEntry::EDITEDSTREET, ent) ;
                        extractXmlData(address, "gpxx:City", PoiEntry::EDITEDCITY, ent) ;
                        extractXmlData(address, "gpxx:State", PoiEntry::EDITEDSTATE, ent) ;
                        extractXmlData(address, "gpxx:Country", PoiEntry::EDITEDCOUNTRY, ent) ;
                        extractXmlData(address, "gpxx:PostalCode", PoiEntry::EDITEDPOSTCODE, ent) ;
                    }
                }
            }

         }

         // add Waypoint Sequence
         ent.setSequence(seq) ; seq = seq + 2 ;

         // add to list
         ent.markAsClean() ;
         poiList.append(ent) ;
     }

     // Load Track Segments
     QDomNodeList trk = gpx.elementsByTagName("trk") ;

     for (int i=0, in = trk.size(); i<in; i++) {

        QDomNode tn = trk.item(i) ;

        QDomElement trknameelement = tn.firstChildElement("name") ;
        if (sName.isEmpty() && trknameelement.isElement()) {
            sName = trknameelement.text() ;
        }
        QDomNodeList trkseg = tn.toElement().elementsByTagName("trkseg") ;


        // Query which segment to load if more than one exist
        int firstseg=0, lastseg=trkseg.size() ;
        if (lastseg>1) {
            // Several Tracks, so prompt for which one to use
            QStringList dates ;
            for (int j=firstseg;j<=lastseg;j++) {
                QDomNode sn = trkseg.item(j) ;
                QDomNode pn = sn.toElement().firstChildElement("trkpt") ;
                QDomNode ndate = pn.firstChildElement("time") ;
                if (!ndate.isNull()) {
                    QDomElement e = ndate.toElement() ;
                    if (e.isElement()) {
                        QString t = e.text() ;
                        QDateTime date = QDateTime::fromString(t, Qt::ISODate);
                        date.setTimeSpec(Qt::UTC);
                        dates.append(date.toString());
                    } else {
                        dates.append("Unknown") ;
                    }
                }
            }
            SegmentChooser seg ;
            seg.setChoices(dates);
            seg.exec() ;
            if (seg.choice()>0) {
                // User selected single segment so use it
                firstseg = seg.choice()-1 ;
                lastseg = firstseg ;
            }
        }


        for (int j=firstseg; j<=lastseg; j++) {

            QDomNode sn = trkseg.item(j) ;
            QDomNodeList trkpt = sn.toElement().elementsByTagName("trkpt") ;

            for (int k=0, kn=trkpt.size(); k<kn; k++) {

                TrackEntry ent ;
                QDomNode pn = trkpt.item(k) ;
                QDomNode latnode = pn.attributes().namedItem("lat") ;
                QDomNode lonnode = pn.attributes().namedItem("lon") ;

                double lat=0.0, lon=0.0, elev=0.0 ;
                QDateTime date = QDateTime() ;

                if (latnode.isAttr()) lat = latnode.nodeValue().toDouble() ;
                if (lonnode.isAttr()) lon = lonnode.nodeValue().toDouble() ;

                QDomNode nelev = pn.firstChildElement("ele") ;
                if (!nelev.isNull()) {
                    QDomElement e = nelev.toElement() ;
                    if (e.isElement()) {
                        QString t = e.text() ;
                        elev = t.toDouble() ;
                    }
                }

                QDomNode ndate = pn.firstChildElement("time") ;
                if (!ndate.isNull()) {
                    QDomElement e = ndate.toElement() ;
                    if (e.isElement()) {
                        QString t = e.text() ;
                        date = QDateTime::fromString(t, Qt::ISODate);
                        date.setTimeSpec(Qt::UTC);
                    }
                }

                date = date.toUTC() ;
                ent.set(lat, lon, elev, date) ;

                // add Track Sequence
                ent.setSequence(seq) ; seq = seq + 2 ;

                trackList.append(ent) ;

            }
        }
    }

    calculateTrack() ;
    bListDirty=false ;
    return QString("") ;
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

bool PoiCollection::saveGpx(QString filename)
{
    if (!filename.isEmpty()) sFilename = filename ;
    if (sFilename.isEmpty()) return false ;

    QFileInfo filenameinfo(sFilename) ;
    QDir savedir(filenameinfo.absoluteDir()) ;

    bool success=true ;

    QDomDocument doc;

     QFile file(sFilename);
     if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) )
         return false;

     QDomElement gpx = doc.createElement("gpx") ;

     gpx.setAttribute("xmlns", "http://www.topografix.com/GPX/1/1") ;
     gpx.setAttribute("xmlns:gpxx", "http://www.garmin.com/xmlschemas/GpxExtensions/v3") ;
     gpx.setAttribute("xmlns:poix", "http://www.trumpton.uk/xmlschemas/GpxExtensions/v3") ;
     gpx.setAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance") ;
     gpx.setAttribute("xsi:schemaLocation", "http://www.topografix.com/GPX/1/1 http://www.topografix.com/GPX/1/1/gpx.xsd http://www.garmin.com/xmlschemas/GpxExtensions/v3 http://www8.garmin.com/xmlschemas/GpxExtensions/v3/GpxExtensionsv3.xsd") ;
     gpx.setAttribute("version", "1.1") ;
     gpx.setAttribute("creator", "trumpton.org") ;

     // Set the metadata

     QDomElement metadata = doc.createElement("metadata") ;

     // Store the formatted name
     QDomElement name = doc.createElement("name") ;
     QString formattedname = formattedName(true, true, true, true, false, false) ;
     QDomText text = doc.createTextNode(formattedname) ;
     name.appendChild(text) ;
     metadata.appendChild(name) ;

     // Store the user-entered name
     QDomElement poixname = doc.createElement("poix:name") ;
     QDomText poixtext = doc.createTextNode(sName) ;
     poixname.appendChild(poixtext) ;
     metadata.appendChild(poixname) ;

     // Store the collection uuid
     QDomElement extensionuuid = doc.createElement("poix:uuid") ;
     extensionuuid.appendChild(doc.createTextNode(sUuid)) ;
     metadata.appendChild(extensionuuid) ;

     // Store the rating
     QDomElement extensionrating = doc.createElement("poix:rating") ;
     extensionrating.appendChild(doc.createTextNode(QString::number(iRating))) ;
     metadata.appendChild(extensionrating) ;

     gpx.appendChild(metadata) ;

     for (int i=0; i<poiList.size(); i++) {

        PoiEntry& ent = poiList[i] ;

        QDomElement wpt = doc.createElement("wpt") ;
        QDomElement extension = doc.createElement("extension") ;
        QDomElement wptext = doc.createElement("gpxx:WaypointExtension") ;
        QDomElement addr = doc.createElement("gpxx:Address") ;
        QDomElement poiext = doc.createElement("poix:WaypointExtension") ;

        // Attach nodes in hierarchy
        gpx.appendChild(wpt) ;
        wpt.appendChild(extension) ;
        extension.appendChild(poiext) ;
        extension.appendChild(wptext) ;
        wptext.appendChild(addr) ;

        // Set Lat/Lon
        wpt.setAttribute("lat", QString::number(ent.lat(), 'f', 16)) ;
        wpt.setAttribute("lon", QString::number(ent.lon(), 'f', 16)) ;

        storeXmlData(doc, PoiEntry::EDITEDPHONE1, ent, wptext, "gpxx:PhoneNumber", "Category", "Phone") ;
        storeXmlData(doc, PoiEntry::EDITEDPHONE2, ent, wptext, "gpxx:PhoneNumber", "Category", "Phone2") ;
        storeXmlData(doc, PoiEntry::EDITEDEMAIL, ent, wptext, "gpxx:PhoneNumber", "Category", "Email") ;
        storeXmlData(doc, PoiEntry::EDITEDURL, ent, wptext, "gpxx:PhoneNumber", "Category", "URL") ;

        // Populate nodes
        storeXmlData(doc, PoiEntry::GEOELEVATION, ent, wpt, "ele") ;
        storeXmlData(doc, PoiEntry::AUTOTIME, ent, wpt, "time") ;
        storeXmlData(doc, PoiEntry::EDITEDTITLE, ent, wpt, "name") ;
        storeXmlData(doc, PoiEntry::AUTOCOMMENT, ent, wpt, "cmt") ;
        storeXmlData(doc, PoiEntry::EDITEDDESCR, ent, wpt, "desc") ;
        storeXmlData(doc, PoiEntry::EDITEDSYMBOL, ent, wpt, "sym") ;
        storeXmlData(doc, PoiEntry::EDITEDTYPE, ent, wpt, "type") ;

        // Append UUID
        QDomElement waypointuuid = doc.createElement("poix:Uuid") ;
        waypointuuid.appendChild(doc.createTextNode(ent.uuid())) ;
        poiext.appendChild(waypointuuid) ;

        storeXmlData(doc, PoiEntry::GEODOOR, ent, poiext, "poix:GeoDoorNumber") ;
        storeXmlData(doc, PoiEntry::GEOSTREET, ent, poiext, "poix:GeoStreet") ;
        storeXmlData(doc, PoiEntry::GEOCITY, ent, poiext, "poix:GeoCity") ;
        storeXmlData(doc, PoiEntry::GEOSTATE, ent, poiext, "poix:GeoState") ;
        storeXmlData(doc, PoiEntry::GEOPOSTCODE, ent, poiext, "poix:GeoPostcode") ;
        storeXmlData(doc, PoiEntry::GEOCOUNTRY, ent, poiext, "poix:GeoCountry") ;
        storeXmlData(doc, PoiEntry::GEOCOUNTRYCODE, ent, poiext, "poix:GeoCountryCode") ;
        storeXmlData(doc, PoiEntry::OLC, ent, poiext, "poix:OpenLocationCode") ;
        storeXmlData(doc, PoiEntry::EDITEDDOOR, ent, poiext, "poix:EditedDoorNumber") ;
        storeXmlData(doc, PoiEntry::EDITEDSTREET, ent, poiext, "poix:EditedStreet") ;
        storeXmlData(doc, PoiEntry::EDITEDCITY, ent, poiext, "poix:EditedCity") ;
        storeXmlData(doc, PoiEntry::EDITEDSTATE, ent, poiext, "poix:EditedState") ;
        storeXmlData(doc, PoiEntry::EDITEDPOSTCODE, ent, poiext, "poix:EditedPostcode") ;
        storeXmlData(doc, PoiEntry::EDITEDCOUNTRY, ent, poiext, "poix:EditedCountry") ;
        storeXmlData(doc, PoiEntry::EDITEDPHONE1, ent, poiext, "poix:PhoneNumber1") ;
        storeXmlData(doc, PoiEntry::EDITEDPHONE2, ent, poiext, "poix:PhoneNumber2") ;
        storeXmlData(doc, PoiEntry::EDITEDEMAIL, ent, poiext, "poix:Email") ;
        storeXmlData(doc, PoiEntry::EDITEDURL, ent, poiext, "poix:URL") ;
        storeXmlData(doc, PoiEntry::GEOCODED, ent, poiext, "poix:Geocoded") ;
        storeXmlData(doc, PoiEntry::DATETIME, ent, poiext, "poix:Date") ;

        if (!ent.get(PoiEntry::PHOTOFILENAME).isEmpty()) {

            // Ensure photo filename is relative to gpx save file
            QString photofilename = ent.get(PoiEntry::PHOTOFILENAME) ;
            photofilename = savedir.relativeFilePath(photofilename) ;
            storeXmlData(doc, photofilename, poiext, "poix:PhotoFilename") ;

            storeXmlData(doc, PoiEntry::PHOTOLAT, ent, poiext, "poix:PhotoLat") ;
            storeXmlData(doc, PoiEntry::PHOTOLON, ent, poiext, "poix:PhotoLon") ;
            storeXmlData(doc, PoiEntry::PHOTOELEVATION, ent, poiext, "poix:PhotoElevation") ;
            storeXmlData(doc, PoiEntry::PHOTODATE, ent, poiext, "poix:PhotoDate") ;
        }

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
        storeXmlData(doc, fullstreet, addr, "gpxx:StreetAddress") ;
        storeXmlData(doc, city, addr, "gpxx:City") ;
        storeXmlData(doc, state, addr, "gpxx:State") ;
        storeXmlData(doc, country, addr, "gpxx:Country") ;
        storeXmlData(doc, postcode, addr, "gpxx:PostalCode") ;

        ent.markAsClean();

     }

     if (trackList.size()>0) {

         QDomElement trk = doc.createElement("trk") ;
         gpx.appendChild(trk) ;

         QDomElement trkname = doc.createElement("name");
         QDomText trktext = doc.createTextNode(formattedName(true, true, true, true, false, true)) ;
         trkname.appendChild(trktext) ;
         trk.appendChild(trkname) ;

         QDomElement trkseg = doc.createElement("trkseg") ;
        trk.appendChild(trkseg) ;

        for (int i=0; i<trackList.size(); i++) {
            TrackEntry &ent = trackList[i] ;
            QDomElement trkpt = doc.createElement("trkpt") ;
            trkseg.appendChild(trkpt) ;

            // Set Lat/Lon
            trkpt.setAttribute("lat", QString::number(ent.lat(), 'f', 16)) ;
            trkpt.setAttribute("lon", QString::number(ent.lon(), 'f', 16)) ;

            QDomElement eelev = doc.createElement("ele") ;
            QDomText delev = doc.createTextNode(QString::number(ent.elev(), 'f', 4)) ;
            eelev.appendChild(delev) ;
            trkpt.appendChild(eelev) ;

            // TODO: DATE NOT BEING SAVED !!!
            if (ent.date().isValid()) {
                QDomElement edate = doc.createElement("time") ;
                QDomText ddate = doc.createTextNode(QString("%1").arg(ent.date().toString(Qt::ISODate))) ;
                edate.appendChild(ddate) ;
                trkpt.appendChild(edate) ;
            }

            ent.markAsClean() ;
        }
     }


     doc.appendChild(gpx) ;

     QDomNode xmlnode( doc.createProcessingInstruction( "xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\""));
     doc.insertBefore( xmlnode, doc.firstChild() );

     QTextStream stream( &file );
     QString str = doc.toString() ;
     stream << str;
     file.close();

     updateLastEdited() ;
     bListDirty=false ;

     qDebug() << "PoiCollection::saveGpx() =>" << filename << " (version: " << sLastEdited << ", sequence: " << sLastSavedSequence << ")" ;

     return success ;
}

//
// Export to OV2
//
bool PoiCollection::saveOv2(QString filename)
{
    if (filename.isEmpty()) filename = sFilename ;
    if (sFilename.isEmpty()) return false ;

    filename = filename.replace(".gpx", ".ov2") ;

    QFile outputstream(filename) ;
    if (!outputstream.open(QIODevice::WriteOnly))
        return false ;

    bool success=true ;

    // Add Source Record (deleted record, lat=-180, lon=1)
    PoiEntry source ;
    source.setLatLon(-180.0, 1.0) ;
    source.set(PoiEntry::EDITEDDESCR, QString("POI Editor - www.trumpton.uk")) ;
    success = source.writeOv2(outputstream, 0) ;

    for (int i=poiList.size()-1; i>=0 && success; i--) {
        PoiEntry& record = poiList[i] ;
        success = record.writeOv2(outputstream, 2) ;
    }

    outputstream.close() ;
    qDebug() << "PoiCollection::saveOv2() =>" << filename << " (version: " << sLastEdited << ", sequence: " << sLastSavedSequence << ")" ;
    return success ;
}

int PoiCollection::size()
{
   return poiList.size() ;
}

int PoiCollection::trackSize()
{
   return trackList.size() ;
}

PoiEntry& PoiCollection::at(int i)
{
    return poiList[i] ;
}

TrackEntry &PoiCollection::trackAt(int i)
{
    return trackList[i] ;
}

bool PoiCollection::add(PoiEntry& newEntry)
{
    remove(newEntry.uuid()) ;
    poiList.insert(0, newEntry) ;
    bListDirty=true ;
    return true ;
}

bool PoiCollection::add(TrackEntry& newEntry)
{
    removeTrack(newEntry.uuid()) ;
    trackList.insert(0, newEntry) ;
    bListDirty=true ;
    calculateTrack() ;
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

bool PoiCollection::removeTrack(QString uuid)
{
    int i=trackList.size()-1 ;
    while (i>=0 && trackList[i].uuid() != uuid) i-- ;
    if (i>=0) {
        bListDirty=true ;
        trackList.removeAt(i) ;
        calculateTrack() ;
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

PoiEntry& PoiCollection::findPrev(QString uuid)
{
    int i=poiList.size()-1 ;
    while (i>=0 && poiList[i].uuid() != uuid) i-- ;
    if (i>0) {
        return poiList[i-1] ;
    } else {
        return nullPoiEntry ;
    }
}

PoiEntry& PoiCollection::findNext(QString uuid)
{
    int i=poiList.size()-1 ;
    while (i>=0 && poiList[i].uuid() != uuid) i-- ;
    if (i>=0 && i<poiList.size()-1) {
        return poiList[i+1] ;
    } else {
        return nullPoiEntry ;
    }
}

// TODO: Need to take into consideration if camera time is GMT or not
TrackEntry& PoiCollection::findTrack(QDateTime when, int withinSeconds)
{
    int bestmatch=-1 ;
    int bestwithin=withinSeconds ;
    for (int i=trackList.size()-1; i>=0; i--) {
        QDateTime trackdate = trackList[i].date() ;
        qint64 diff = when.secsTo(trackdate) ;
        if (diff>=0 && diff <= bestwithin) {
            bestmatch = i ;
            bestwithin = diff ;
        } else if (diff <0 && (-diff) <= bestwithin) {
            bestmatch = i ;
            bestwithin = -diff ;
        }
    }
    if (bestmatch>=0) {
        return trackList[bestmatch] ;
    } else {
        return nullTrackEntry ;
    }
}


TrackEntry& PoiCollection::findTrack(QString uuid)
{
    int i = trackList.size()-1 ;
    while (i>=0 && trackList[i].uuid() != uuid) i-- ;
    if (i>=0) {
        return trackList[i] ;
    } else {
        return nullTrackEntry ;
    }
}

TrackEntry& PoiCollection::findNextTrack(QString uuid)
{
    int i = trackList.size()-2 ;
    while (i>=0 && trackList[i].uuid() != uuid) i-- ;
    if (i>=0) {
        return trackList[i+1] ;
    } else {
        return nullTrackEntry ;
    }
}


TrackEntry& PoiCollection::findPrevTrack(QString uuid)
{
    int i = trackList.size()-1 ;
    while (i>0 && trackList[i].uuid() != uuid) i-- ;
    if (i>=1) {
        return trackList[i-1] ;
    } else {
        return nullTrackEntry ;
    }
}

// Import OV2 (See https://www.tomtom.com/lib/doc/ttnavsdk3_manual.pdf for format)
bool PoiCollection::importOv2(QString filename)
{
    QFile inputstream(filename) ;
    if (!inputstream.open(QIODevice::ReadOnly))
        return false ;

    bool success ;
    int i=0 ;

    do {
        PoiEntry record ;
        success=record.importOv2(inputstream) ;
        if (record.isValid()) {
            record.setSequence(i*2) ;
            poiList.append(record) ;
            i++ ;
        }
    } while (success && !inputstream.atEnd()) ;

    inputstream.close() ;
    if (i>=0) {
        bListDirty=true ;
    }
    return success ;
}


double PoiCollection::trackLength() { return dtracklength ; }
double PoiCollection::heightGain() { return dheightgain ; }
double PoiCollection::heightLoss() { return dheightloss ; }
double PoiCollection::trackTime() { return dtracktime ; }
double PoiCollection::trackTimeEst() { return dtracktimeest ; }

bool PoiCollection::calculateTrack()
{
    dtracklength = 0 ;
    dheightgain = 0 ;
    dheightloss = 0 ;
    dtracktime = 0 ;
    dtracktimeest = 0 ;

    if (trackList.size()<=1) {

        return false ;

    } else {

        double lastheight = trackList[0].elev() ; ;

        for (int i=1; i<trackList.size(); i++) {

            TrackEntry& lastpoint = trackList[i-1] ;
            TrackEntry& thispoint = trackList[i] ;

            double distance = thispoint.distanceFrom(lastpoint) ;

            double thisheight = thispoint.elev() ;
            if (thisheight <= 0) thisheight = lastheight ;
            double heightdelta = thisheight - lastheight ;

            if (heightdelta>0) dheightgain += heightdelta ;
            else dheightloss -= heightdelta ;
            dtracklength += distance ;

            lastheight = thisheight ;

        }

        // Naismith's Rule (minutes)
        // Walking time = 3 miles per hour (5 km/h, 83.333333333 m/min)
        dtracktimeest += dtracklength / 83.333333333 ;
        // Climbing Time = 1 minute for every 10 metres ascent
        dtracktimeest += dheightgain / 10 ;
        // Descent Time = 1 minute for every 20 metre descent
        dtracktimeest += dheightloss / 20 ;

        // Actual Time (minutes)
        QDateTime firsttime = trackList[0].date() ;
        QDateTime lasttime = trackList[trackList.size()-1].date() ;
        if (firsttime.isValid() && lasttime.isValid()) {
            dtracktime = firsttime.msecsTo(lasttime) / 60000.0 ;
        }

        return true ;
    }
}


// ///////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////

// ///////////////////////////////////////////////////////
//
// TrackEntry
//


TrackEntry::TrackEntry()
{
    QUuid quuid = QUuid::createUuid() ;
    sUuid = quuid.toString() ;
    set(0.0, 0.0, 0.0, QDateTime()) ;
    bValid = false ;
    bDirty = false ;
}

TrackEntry::~TrackEntry()
{
}

void TrackEntry::markAsClean() { bDirty = false ; }

void TrackEntry::set(double lat, double lon, double elev, QDateTime date)
{
    setLatLon(lat, lon) ;
    dElev = elev ;
    tDate = date ;
    bValid = true ;
    bDirty = true  ;
}

void TrackEntry::setLatLon(double lat, double lon)
{
    // resolution = 0,1,2,3,4 => 13.9m, 2.8m, 56x87cm, 11x22cm, 2x5cm, 4x14mm
    int resolution=2 ;
    dLat = lat ;
    dLon = lon ;
    sOlc = "" ;
    calculateOlc(lat, lon, resolution, &sOlc) ;
    bValid = true ;
    bDirty = true  ;
}

double TrackEntry::distanceFrom(TrackEntry& other)
{
    return _distanceFrom(dLat, dLon, other.lat(), other.lon()) ;
}

double TrackEntry::distanceFrom(PoiEntry& other)
{
    return _distanceFrom(dLat, dLon, other.lat(), other.lon()) ;
}

void TrackEntry::setSequence(int sequence)
{
    iSequence = sequence ;
}

double TrackEntry::lat()
{
    return dLat ;
}

double TrackEntry::lon()
{
    return dLon ;
}

double TrackEntry::elev()
{
    return dElev ;
}

QString TrackEntry::olc()
{
    return sOlc ;
}

int TrackEntry::sequence()
{
    return iSequence ;
}

QString TrackEntry::uuid()
{
    return sUuid ;
}

QDateTime TrackEntry::date()
{
    return tDate ;
}

bool TrackEntry::isValid()
{
    return bValid ;
}

bool TrackEntry::isDirty()
{
    return bDirty ;
}


//
//
//

bool calculateOlc(double lat, double lon, int resolution, QString *olc)
{
    static char b20[] = "23456789CFGHJMPQRVWX" ;

    if (!olc) return false ;

    // resolution (diagonal length of 'box')
    //   0 ~ 20m
    //   1 ~ 5m
    //   2 ~ 1m
    //   3 ~ 250mm
    //   4 ~ 50mm
    //   5 ~ 15mm

    if (resolution<0 || resolution>5) {
        resolution=2 ;
    }

    // Clip to ensure legal

    if (lat < -90.0) { lat= -90.0 ; }
    if (lat > +90.0) { lat= +90.0 ; }
    if (lon < -180.0) { lon = -180.0 ; }
    if (lon > +180.0) { lon = +180.0 ; }


    // Calculating the most significant 10 digits

    // Add 90 to the latitude
    // Add 180 to the longitude

    lat = lat + 90.0 ;
    lon = lon + 180.0 ;

    // Divide latitude by 20
    // Divide longitude by 20

    lat = lat / 20 ;
    lon = lon / 20 ;

    // Lookup Integer of Latitude in table for first output character
    // Lookup Integer of Longitude in table next output character

    (*olc) = (*olc) + b20[int(lat)] ;
    (*olc) = (*olc) + b20[int(lon)] ;

    // Loop 4 times:

    for (int l=1; l<=4; l++) {

        // If on 4th loop
        if (l==4) {
            // Add plus as next output character
            (*olc) = (*olc) + "+" ;
        }

        // Multiply remainder of Latitude by 20
        // Multiply remainder of Longitude by 20

        lat = 20 * (lat-int(lat)) ;
        lon = 20 * (lon-int(lon)) ;

        // Lookup Integer of Latitude for next output character
        // Lookup Integer of Longitude for next output character

        (*olc) = (*olc) + b20[int(lat)] ;
        (*olc) = (*olc) + b20[int(lon)] ;

    }

    // Calculating the least significant 5 digits

    // Loop up to 5 times:

    for (int l=1; l<=resolution; l++) {

        // Multiply remainder of Latitude by 5
        // Multiply remainder of Longitude by 4

        lat = 5 * (lat-int(lat)) ;
        lon = 4 * (lon-int(lon)) ;

        // Lookup Integer of Lat*4 + Integer of Lon for next output character

        (*olc) = (*olc) + b20[4*int(lat)+int(lon)] ;

    }

    return true ;
}


