// BING MAPS EXAMPLE: https://openlayers.org/en/latest/examples/feature-move-animation.html?q=move

// TODO:
//
//  1) TrackLines are not shown
//  2) Track Markers and Working Markers cannot be dragged
//  3) Searching not supported
// OK) Double click on map doesn't center
// OK) SelectMarker currently does nothing
// BUG) switch marker from file to working via list, and colour doesnt change.  Then click on it, and colour changes correctly.
//

// Search and geocode
var geocoder; // Setup by initialise

// Interraction & Callback to C++ handle
var OpenStreetMapsWidget; // Setup by initialise
var webChannel; // Setub by Initialise

var map; // Setup by initialise
var view; // Setup by initialise

// Marker Collections
var collectionworking; // Setup by registerUuids
var collectionfile; // Setup by registerUuids
var collectiontrack; // Setup by registerUuids

// Current selection
var selecteduuid;

// Markers / tracks Setup by initialise
var markerlayer; // Markers
var linelayer; // Blue line for track

// Maps setup by initialise
var maplayer; // 0
var satroutelayer; // 1
var topolayer; // 2
var satlayer;
var trailslayer;

// Set with setMarker
var markers = [];
var drawrouteenabled = false;

// Interactions setup by initialise
var selectInteraction;

var initialLat = 48.13;
var initialLon = 11.57;
var initialZoom = 3;

//
// Set style for markers
//
function markerStyleFunction(feature) {

    var uuid = feature.uuid;
    var collectionuuid = feature.collectionuuid;
    var selected = "";
    var thisstyle ;
    var zindex = 2000 ;

    if (uuid === selecteduuid) {
        selected = "-selected";
        zindex = 3000 ;
    }

    if (collectionuuid === collectiontrack) {

        var radius ;
        var stroke ;
        var fill ;
        var red ;

        // Colour of markers goes from blue to purple along the route
        var sequence = feature.sequence ;
        var numsequences = markers.length ;
        red= 8 + (sequence * 64 / numsequences) ;

        if (selected==="") {
            radius=8 ;
            fill = new ol.style.Fill({ color: [red, 0, 255, 0.9] }) ;
            stroke = new ol.style.Stroke({ color: [255, 255, 255, 0.9],
                                           width: 2 }) ;
        } else {
            // Selected marker is 'hollow'
            radius=10 ;
            fill = null ;
            stroke = new ol.style.Stroke({ color: [red, 0, 255, 0.9],
                                           width: 4 }) ;
        }


        thisstyle = new ol.style.Style({
              image: new ol.style.Circle({
                          radius: radius,
                          fill: fill,
                          stroke: stroke
                  }),
              zIndex: zindex
        }) ;

    } else  {

        var file ;

        if (collectionuuid === collectionworking) {
            file = "working" + selected ;
            zindex = zindex + 10 ;
        } else {
            file = "file" + selected ;
            zindex = zindex + 20 ;
        }

        thisstyle = new ol.style.Style({
              image: new ol.style.Icon({
                  anchor: [0.5, 1],
                  anchorXUnits: 'fraction',
                  anchorYUnits: 'fraction',
                  opacity: 0.9,
                  src: 'qrc:///mapicons/marker-' + file + '.png'
              }),
              zIndex: zindex
        }) ;

    }

    return [thisstyle] ;
}



// Initialise3 Channel and callback when complete
function initialise3() {
    webChannel = new QWebChannel(qt.webChannelTransport, function(channel) {
        // WebEngineView (set via channel)
        OpenStreetMapsWidget = channel.objects.MapsWidget;
        OpenStreetMapsWidget.jsinitialise4(true);
    });
}

// Register UUIDs
function registerUuids(workinguuid, fileuuid, trackuuid) {
    collectionworking = workinguuid;
    collectionfile = fileuuid;
    collectiontrack = trackuuid;
}

// Initialise Map
function initialiseMap(bingKey, aerialZoom, aerialUrl, overlayZoom, overlayUrl, mapZoom, mapUrl, contourZoom, contourUrl, trailZoom, trailUrl) {

    // Vanilla Open Streetmap
    if (mapUrl==="OSM") {
        maplayer = new ol.layer.Tile({
            title: "Map",
            type: "base",
            visible: true,
            source: new ol.source.OSM()
        });
    } else {
        maplayer = new ol.layer.Tile({
            title: "Map",
            type: "base",
            visible: true,
            source: new ol.source.XYZ({
                url: mapUrl,
                maxZoom: mapZoom
            })
        });
    }
    maplayer.setZIndex(1000);

    // Satellite Image Layer from Bing with Labels / Names
    if (aerialUrl==="Bing") {
        satroutelayer = new ol.layer.Tile({
            title: "Satellite With Routes",
            type: "base",
            visible: false,
            source: new ol.source.BingMaps({
                maxZoom: 19,
                imagerySet: 'AerialWithLabels',
                key: bingKey // http://www.bingmapsportal.com/
            })
        });
    } else {
        satroutelayer = new ol.layer.Tile({
            title: "Satellite With Routes",
            type: "base",
             visible: false,
            source: new ol.source.XYZ({
                maxZoom: aerialZoom,
                url: aerialUrl
            })
        });
    }
    satroutelayer.setZIndex(1010);

    // Topographical Layer
    topolayer = new ol.layer.Tile({
        title: "Contour",
        type: "base",
        visible: false,
        source: new ol.source.XYZ({
            maxZoom: contourZoom,
            url: contourUrl
        })
    });
    topolayer.setZIndex(1020);


    // Satellite Image Layer from Bing
    if (overlayUrl==="Bing") {
        satlayer = new ol.layer.Tile({
            title: "Satellite",
            visible: false,
            opacity: 0.5,
            source: new ol.source.BingMaps({
                maxZoom: 19,
                imagerySet: 'Aerial',
                key: bingKey
            })
        });
    } else {
        satlayer = new ol.layer.Tile({
            title: "Satellite",
            visible: false,
            opacity: 0.5,
            source: new ol.source.XYZ({
                maxZoom: overlayZoom,
                url: overlayUrl
            })
        });

    }
    satlayer.setZIndex(1030);

    // Trails Layer
    trailslayer = new ol.layer.Tile({
        title: "Trails",
        type: "base",
        visible: false,
        opacity: 0.8,
        source: new ol.source.XYZ({
            maxZoom: trailZoom,
            url: trailUrl
        })
    });
    trailslayer.setZIndex(1040);

    // Route / Track Layers

    markerlayer = new ol.layer.Vector({
        title: "Markers",
        visible: true,
        source: new ol.source.Vector({
            features: []
        }),
        style: markerStyleFunction
    });
    markerlayer.setZIndex(2010);

    linelayer = new ol.layer.Vector({
        title: "Track Lines",
        visible: true,
        source: new ol.source.Vector({
            features: []
        })
    });
    linelayer.setZIndex(2000);

    map = new ol.Map({
        target: 'map_canvas',
        layers: [maplayer, satroutelayer, topolayer, satlayer, trailslayer, markerlayer, linelayer],
        view: new ol.View({
            center: ol.proj.fromLonLat([initialLon, initialLat]),
            zoom: initialZoom
        })
    });

    map.on('moveend', function(evt) {
        handleMapMoved(evt);
    });

    map.on('dblclick', function(evt) {
        handleDoubleClick(evt);
    });


    // Enable feature dragging
    var dragInteraction = new ol.interaction.Translate({
        layers: function(layer) {
             return layer === markerlayer ;
        },
/*        features: function(feature) {
            return true ;
        },
*/
        style: markerStyleFunction
    });
    map.addInteraction(dragInteraction);
    dragInteraction.on("translateend", function(evt) {
        if (evt && evt.features && evt.features.item(0)) {
            handleMarkerSelected(evt.features.item(0)) ;
            handleMarkerMoved(evt.features.item(0));
        }
    });


    // Enable feature selection
    selectInteraction = new ol.interaction.Select({
        layers: function(layer) {
             return layer === markerlayer ;
        },
        condition: ol.events.condition.click,
        style: markerStyleFunction
    });
    map.addInteraction(selectInteraction);
    selectInteraction.on("select", function(evt) {
        handleMarkerSelected(evt.selected[0]);
    });


}

// Enable / disable satellite
function showMaps(osm, aerial, topo, trails, satellite) {

    maplayer.setVisible(false);
    satroutelayer.setVisible(false);
    topolayer.setVisible(false);
    satlayer.setVisible(false);
    trailslayer.setVisible(false);

    if (osm) {
        maplayer.setVisible(true);
    }
    if (aerial) {
        satroutelayer.setVisible(true);
        satellite = false;
    }
    if (topo) {
        topolayer.setVisible(true);
    }
    if (satellite) {
        satlayer.setVisible(true);
    }
    if (trails) {
        trailslayer.setVisible(true);
    }
}

// Enable / Disable Track Marker Drawing
function drawLines(enabled) {
    drawrouteenabled = enabled;
    redrawMarkers(collectiontrack);
}

//===========================================================
//
// Marker Adding / Removing and Re-Drawing
//

function findmarkerByUuid(uuid) {
    for (var i = 0; i < markers.length; i++) {
        if (markers[i].uuid === uuid) return markers[i];
    }
    return null;
}

// Change Marker icon to selected
function selectMarker(uuid) {
    selectFeature(findmarkerByUuid(uuid));
}

function selectFeature(feature) {
    selectInteraction.getFeatures().clear() ;
    if (feature !== null) {
        selecteduuid = feature.uuid;
        selectInteraction.getFeatures().push(feature) ;
    }
}


// Change Marker Collection
function setMarkerCollection(uuid, collectionuuid) {
    var feature = findmarkerByUuid(uuid);
    if (feature.collectionuuid === collectionuuid) return;
    feature.collectionuuid = collectionuuid;
    redrawMarkers(collectionuuid);
}

// Add / Change Marker
function setMarker(uuid, collectionuuid, lat, lon, address, sequence, drop) {

    removeMarker(uuid);

    //create marker... with coordinates
    var marker = new ol.Feature({
        geometry: new ol.geom.Point(ol.proj.transform([lon, lat], 'EPSG:4326', 'EPSG:3857'))
    });
    marker.uuid = uuid;
    marker.collectionuuid = collectionuuid;
    marker.sequence = sequence;
    marker.coords = [] ;
    marker.coords[0] = lon ;
    marker.coords[1] = lat ;

    // Add and redraw
    markers.push(marker);
    redrawMarkers(collectionuuid);
}

// Remove marker by UUID
function removeMarker(uuid) {

    var j = -1;
    for (var i = 0; i < markers.length; i++) {
        if (markers[i].uuid === uuid) {
            j = i;
        }
    }

    if (j >= 0) {
        collectionuuid = markers[j].collectionuuid;
        markers.splice(j, 1);
        redrawMarkers(collectionuuid);
    }
}


// Remove ALl Markers
function removeAllMarkers() {
    markers.length = 0;
    markerlayer.getSource().clear(true);
    linelayer.getSource().clear(true);
}


// Redraw / Refresh Markers
function redrawMarkers(uuidcollection) {

    markerlayer.getSource().clear(true);
    markerlayer.getSource().addFeatures(markers);

    if (uuidcollection === collectiontrack) {

        linelayer.getSource().clear(true);

        if (drawrouteenabled) {

            markers.sort(function(a, b) {
                if (a.sequence < b.sequence) return -1;
                else if (a.sequence > b.sequence) return 1;
                else return 0;
            });

            var points = [] ;
            for (var i=0; i < markers.length; i++) {
                if (markers[i].collectionuuid === collectiontrack) {
                    var coords = markers[i].getGeometry().getCoordinates();
                    points.push(coords);
                }
            }

            var featureLine = new ol.Feature({
                geometry: new ol.geom.LineString(points),
             });

            var styleLine = new ol.style.Style({
                stroke: new ol.style.Stroke({
                    color: [0, 0, 255, 0.1],
                    width: 8
                })
            }) ;
            featureLine.setStyle(styleLine) ;

            linelayer.getSource().addFeatures([featureLine]);

        }

    }

}

//===========================================================
//
// Event Handlers / Dispatchers
//

function handleDoubleClick(evt) {
    var coordinates = evt.coordinate;
    var zoom = map.getView().getZoom() + 1;
    map.getView().animate({
        center: coordinates,
        zoom: zoom,
        duration: 400
    });
}

function handleMapMoved(evt) {

    // centre
    // 0 -> lon
    // 1 -> lat
    var centre = map.getView().getCenter();
    centre = ol.proj.transform(centre, 'EPSG:3857', 'EPSG:4326');

    // extent
    // 0 -> left
    // 1 -> bottom
    // 2 -> right
    // 3 -> top
    var extent = map.getView().calculateExtent(map.getSize());
    extent = ol.proj.transformExtent(extent, 'EPSG:3857', 'EPSG:4326');

    zoom = map.getView().getZoom();

    if (typeof OpenStreetMapsWidget !== 'undefined')
        OpenStreetMapsWidget.jsmapMoved(centre[1], centre[0], zoom, extent[3], extent[0], extent[1], extent[2]);
}

function handleMarkerMoved(marker) {
    if (typeof OpenStreetMapsWidget !== 'undefined') {
        var coords = marker.getGeometry().getFirstCoordinate();
        coords = ol.proj.transform(coords, 'EPSG:3857', 'EPSG:4326');
        var lon = coords[0];
        var lat = coords[1];
        // Callback iff marker has actually moved
        var distance = ol.sphere.getDistance(coords, marker.coords) ;
        if (distance>0.000001) {
            // Marker moved more than 1um
            console.log("Marker " + marker.uuid + " moved");
            marker.lat = lat ;
            marker.lon = lon ;
            OpenStreetMapsWidget.jsmarkerMoved(marker.uuid, marker.collectionuuid, lat, lon);
        } else {
            console.log("Marker " + marker.uuid + " 'moved' < 2m");
            selectFeature(marker); // Select with miniscule drag does not show as select
            OpenStreetMapsWidget.jsmarkerSelected(marker.uuid, marker.collectionuuid);
        }
    }
}

function handleTrackMarkerMoved(marker) {}

function handleMarkerSelected(marker) {
    if (typeof OpenStreetMapsWidget !== 'undefined') {
        selectFeature(marker);
        OpenStreetMapsWidget.jsmarkerSelected(marker.uuid, marker.collectionuuid);
    }
}

//===========================================================
//
// Navigation
//

function gotoCoordinates(pos, requiredzoom) {
    map.getView().setCenter(ol.proj.transform([pos[1], pos[0]], 'EPSG:4326', 'EPSG:3857'));
    map.getView().setZoom(requiredzoom);
}

function seekToMarker(uuid, requiredzoom) {
    for (var i = 0; i < markers.length; i++) {
        if (markers[i].uuid === uuid) {
            map.getView().setCenter(markers[i].getGeometry().getFirstCoordinate());
            if (requiredzoom > 0) map.getView().setZoom(requiredzoom);
            return;
        }
    }
}



//===========================================================
//
// Geocode, Search and response handlers
//


function initialiseGeocoder(type, hereid, herecode, hereapikey)
{
    geocoder = new Geocode(type, hereid, herecode, hereapikey);
}


// Perform reverse geocode
function geocodeMarker(uuid, collectionuuid, forcegeocode) {

    if (!geocoder) return ;

    var marker = findmarkerByUuid(uuid);

    if (marker === null) {
        // uuid invalid / marker not found
    } else {
        var coords = marker.getGeometry().getCoordinates();
        coords = ol.proj.transform(coords, 'EPSG:3857', 'EPSG:4326');
        var lon = coords[0];
        var lat = coords[1];

        geocoder.geocode(lat, lon, function(results) {
            handleGeocoderResults(marker, results);
        })
    }
}

function handleGeocoderResults(marker, results) {
    //    results.error, results.status, results.lat, results.lon, results.name, results,road, results.number, results.postcode, results.city, results.state, results.country
    if (results.error) {
        OpenStreetMapsWidget.jsGeocodeFailed(results.status);
    } else {
        var name = results.name;
        var door = results.number;
        var street = results.road;
        var city = results.city;
        var state = results.state;
        var country = results.country;
        var countrycode = results.countrycode ;
        var postcode = results.postcode;
        OpenStreetMapsWidget.jsmarkerGeocoded(marker.uuid, marker.collectionuuid, name, door, street, city, state, country, countrycode, postcode);
    }
}


// Perform Search
function searchLocation(searchstring) {

    if (!geocoder) return ;

    geocoder.search(searchstring, function(results) {
        handlerSearchLocationResults(results)
    });
}

function handlerSearchLocationResults(results) {
    //    results.error, results.status, results.lat, results.lon, results.name, results,road, results.number, results.postcode, results.city, results.state, results.country
    if (results.error) {
        OpenStreetMapsWidget.jsSearchFailed(results.status);
    } else {
        var placeid = results.placeid;
        var lat = results.lat;
        var lon = results.lon;
        var address = results.name;
        var phone = "";
        OpenStreetMapsWidget.jsSearchResultsReady(placeid, lat, lon, address, phone);
    }
}
