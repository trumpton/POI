/*
 * Application: POI Manager
 * File: mapswidget.h
 *
 * Handle interface to html/javqascript maps from C++ side
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

#ifndef GOOGLEMAPSWIDGET_H
#define GOOGLEMAPSWIDGET_H

#include "configuration.h"

#include <QtWebEngineWidgets>
#include <QtWebChannel/QtWebChannel>

#include <QObject>
#include <QString>


class MapsWidget: public QWebEngineView
{
    // Enable slots and signals
    Q_OBJECT

private:
    QString cachedGeocodeType ;
    QString cachedBingKey, cachedHereId, cachedHereCode, cachedHereApiKey ;
    QString cachedWorkingCollectionUuid, cachedFileCollectionUuid, cachedTrackCollectionUuid ;
    int cachedAerialTileZoom, cachedSatelliteOverlayTileZoom, cachedMapTileZoom, cachedContourTileZoom, cachedTrailTileZoom ;
    QString cachedAerialTileUrl, cachedSatelliteOverlayTileUrl, cachedMapTileUrl, cachedContourTileUrl, cachedTrailTileUrl ;


signals:
    void mapMoved(double lat, double lon, int zoom) ;
    void markerMoved(const QString& uuid, const QString& collectionuuid, const double lat, const double lon) ;
    void markerGeocoded(const QString& uuid, const QString& collectionuuid, const QString& formattedaddress, QString &door, QString &street, QString& town, QString& state, QString& country, QString& countrycode, QString& postcode) ;
    void geocodeFailed(const QString& error) ;
    void markerSelected(const QString& uuid, const QString& collectionuuid) ;
    void searchResultsReady(const double lat, const double lon, const QString& address, const QString& phone) ;
    void searchFailed(const QString& error) ;

public slots:
    void initialise2(bool ok) ;
    void jsinitialise4(bool ok) ;
    void jsmarkerSelected(QString uuid, QString collectionuuid) ;
    void jsmarkerMoved(QString uuid, QString collectionuuid, double lat, double lon) ;
    void jsmarkerGeocoded(QString uuid, QString collectionuuid, QString address, QString door, QString street, QString town, QString state, QString country, QString countrycode, QString postcode) ;
    void jsGeocodeFailed(QString error) ;
    void jsSearchResultsReady(QString placeid, double lat, double lon, QString address, QString phone) ;
    void jsmapMoved(double lat, double lon, int zoom, double tllat, double tllon, double brlat, double brlon) ;
    void jsSearchFailed(QString error) ;
    void jsDebug(QString message) ;

public:
    explicit MapsWidget(QWidget *parent = 0);
    ~MapsWidget() ;

    // Initialise key, and load web page
    void initialise(QString url,
                    QString bingKeyData, QString geocodeType, QString hereId, QString hereCode,
                    QString hereApiKey,
                    int aerialTileZoom, QString aerialTileUrl,
                    int satelliteOverlayTileZoom, QString satelliteOverlayTileUrl,
                    int mapTileZoom, QString mapTileUrl,
                    int contourTileZoom, QString contourTileUrl,
                    int trailTileZoom, QString trailTileUrl,
                    QString workingUuid, QString fileUuid, QString trackUuid) ;

    void registerUuids(QString workingUuid, QString fileUuid, QString trackUuid) ;
    void clearCookies() ;

    // Show Map Layers
    void showMaps(bool base, bool aerial, bool topo, bool tracks, bool satellite) ;

    // Show Route Lines
    void showTracks(bool enabled) ;

    // Search Functions
    void searchLocation(QString address) ;

    // Geocode Functions
    void geocodeMarker(QString uuid, QString collectionuuid, bool forcegeocode=true) ;

    // Navigation Management
    void gotoCoordinates(double west, double north, int zoom=16) ;
    double getLat() ;
    double getLon() ;
    int getZoom() ;

    bool isVisible(double west, double north) ;

    // Marker Management
    void setMarker(QString uuid, QString collectionuuid, double lat, double lon, QString address, int sequence, bool drop=false) ;
    void removeMarker(QString uuid) ;
    void removeAllMarkers() ;
    void setMarkerCollection(QString uuid, QString collectionuuid) ;
    bool queryMarker(QString uuid, QString& collectionuuid, double *lat, double *lon, QString& addresstxt) ;
    void selectMarker(QString uuid) ;
    void seekToMarker(QString uuid, int zoom) ;

private:
    // Initialise Javascript and set working collection uuid
    void initialiseJavascript() ;
    void runJavaScript(QString command) ;

    QWebChannel channel ;
    double dNorth, dWest ; // Top Left of viewport
    double dSouth, dEast ; // Bottom Right of viewport
    double dLat, dLon ;    // Centre of viewport
    int iZoom ;            // Current zoom
};

#endif // GOOGLEMAPSWIDGET_H
