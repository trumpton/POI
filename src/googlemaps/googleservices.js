var map; // Setup by initialise
var GoogleMapsWidget; // Setup by initialise
var searchservice; // Setup by initialise
var detailsservice; // Setup by initialise
var geocoderservice; // Setup by initialise
var collectionworking; // Setup by registerUuids
var collectionfile; // Setup by registerUuids
var collectiontrack; // Setup by registerUuids

var showlines ;
var polyline ;
var markers = [];

var initialLat = 48.13;
var initialLon = 11.57;
var initialZoom = 3;

// https://sites.google.com/site/gmapsdevelopment/
var iconworking = { url: "qrc:///googlemaps/marker-red.png", size: new google.maps.Size(29, 48), anchor: new google.maps.Point(15, 48) };
var iconworkingselected = { url: "qrc:///googlemaps/marker-red-dot.png", size: new google.maps.Size(29, 48), anchor: new google.maps.Point(15, 48) };
var iconfile = { url: "qrc:///googlemaps/marker-green.png", size: new google.maps.Size(29, 48), anchor: new google.maps.Point(15, 48) };
var iconfileselected = { url: "qrc:///googlemaps/marker-green-dot.png", size: new google.maps.Size(29, 48), anchor: new google.maps.Point(15, 48) };
var icontrack = { url: "qrc:///googlemaps/trackmarker-blue.png", size: new google.maps.Size(32, 32), anchor: new google.maps.Point(16, 16) };
var icontrackselected = { url: "qrc:///googlemaps/trackmarker-blue-dot.png", size: new google.maps.Size(32, 32), anchor: new google.maps.Point(16, 16) };

// zindex
var zIndexSelectedMarker = 30;
var zIndexMarker = 20;
var zIndexTrackPlot = 10;
var zIndexMap = 0;

// Updated from map moves
var centre = null;
var bounds = null;
var zoom = 0;

// Register UUIDs
function registerUuids(workinguuid, fileuuid, trackuuid) {
    collectionworking = workinguuid;
    collectionfile = fileuuid;
    collectiontrack = trackuuid ;
}

// Create Map
function initialise() {

    var myOptions = {
        center: new google.maps.LatLng(initialLat, initialLon),
        zoom: initialZoom,
        mapTypeId: google.maps.MapTypeId.ROADMAP,
        panControl: true,
    };

    showlines = false ;

    // HACK
    showlines = true ;

    map = new google.maps.Map(document.getElementById("map_canvas"), myOptions);

    polyline = null ;
    searchservice = new google.maps.places.PlacesService(map);
    detailsservice = new google.maps.places.PlacesService(map);
    geocoderservice = new google.maps.Geocoder;

    new QWebChannel(qt.webChannelTransport, function(channel) {

        // WebEngineView (set via channel)
        GoogleMapsWidget = channel.objects.MapsWidget;

        map.addListener('center_changed', function() {
            handleMapMoved(map);
        });
        map.addListener('zoom_changed', function() {
            handleMapMoved(map);
        });
        map.addListener('bounds_changed', function() {
            handleMapMoved(map);
        });
        GoogleMapsWidget.jsmapMoved(initialLat, initialLon, initialZoom);

    });

}

// Enable / disable lines for route
function showTracks(enabled) {
    showlines = enabled ;
    drawLines() ;
}

// Search and response handler
function searchLocation(searchstring) {
    searchservice.textSearch({
        query: searchstring
    }, function(results, status) {
        handlerSearchLocationResults(results, status);
    });
}

function handlerSearchLocationResults(results, status) {

    if (typeof GoogleMapsWidget === 'undefined') return;
    if (status === google.maps.places.PlacesServiceStatus.OK) {
        var placeid = results[0].place_id;
        var lat = results[0].geometry.location.lat();
        var lon = results[0].geometry.location.lng();
        var addr = results[0].formatted_address;
        var phone = results[0].international_phone_number;
        if (typeof addr === 'undefined') addr = "unknown";
        if (typeof phone === 'undefined') {
            phone = results[0].formatted_phone_number;
            if (typeof phone === 'undefined') phone = "";
        }
        GoogleMapsWidget.jsSearchResultsReady(placeid, lat, lon, addr, phone);
    } else if (status === google.maps.places.PlacesServiceStatus.ZERO_RESULTS) {
        GoogleMapsWidget.jsSearchFailed("Nothing Found");
    } else {
        GoogleMapsWidget.jsSearchFailed(status);
    }

}

// Navigation
function gotoCoordinates(pos, requiredzoom) {
    if (requiredzoom > 0) map.setZoom(requiredzoom);
    map.setCenter(pos);
    handleMapMoved();
}

function seekToMarker(uuid, requiredzoom) {
    var i = markers.length - 1;
    while (i >= 0 && markers[i].uuid !== uuid) i--;
    if (i >= 0) {

        var update = false ;

        if (zoom < requiredzoom-2 || zoom > requiredzoom+2) {
            zoom = requiredzoom ;
            update=true ;
        }

        var pos = markers[i].position;
        if (!bounds || !bounds.contains(pos) ) {
            update=true ;
        }

        if (update) {
            gotoCoordinates(pos, zoom);
        }
    }
}

// Marker Management
function setMarker(uuid, collectionuuid, lat, lon, address, sequence, drop) {

    var draggable = false;
    if (collectionuuid === collectionworking) draggable = true;

    var marker = new google.maps.Marker({
        map: map,
        uuid: uuid,
        collectionuuid: collectionuuid,
        zIndex: zIndexMarker,
        position: new google.maps.LatLng(lat, lon),
        draggable: draggable,
        address: address,
        sequence: sequence
    });

    if (drop) {
        marker.setAnimation(google.maps.Animation.DROP) ;
    }

    if (collectionuuid === collectionworking) {
        marker.setIcon(iconworking);
        marker.draggable = true;
    } else if (collectionuuid === collectiontrack) {
        marker.setIcon(icontrack);
        marker.draggable = true;
    } else {
        marker.setIcon(iconfile);
        marker.draggable = false;
    }

    removeMarker(uuid);
    markers.push(marker);
    sortMarkers() ;

    google.maps.event.addListener(marker, 'click', function() {
        handleMarkerSelected(marker);
    });
    google.maps.event.addListener(marker, 'dragstart', function() {
        handleMarkerSelected(marker);
    });
    google.maps.event.addListener(marker, 'dragend', function() {
        handleMarkerMoved(marker);
    });

}

function sortMarkers()
{
    markers.sort( function(a,b) { if (a.sequence < b.sequence) return -1 ; if (a.sequence > b.sequence) return 1 ; return 0 ; } ) ;
}


function setMarkerCollection(uuid, collectionuuid) {
    var i = markers.length - 1;
    while (i >= 0 && markers[i].uuid !== uuid) i--;
    if (i >= 0) {
        markers[i].collectionuuid = collectionuuid;
        if (collectionuuid === collectionworking || collectionuuid === collectiontrack) {
            markers[i].draggable = true;
        } else {
            markers[i].draggable = false;
        }
    }
    selectMarker(uuid);
}

function selectMarker(uuid) {

    var selectedmarker = -1;
    var selectedisworking = false;
    var selectedistrack = false;

    // Redraw non-selected marker, and identify selected
    for (var i = 0; i < markers.length; i++) {

        var selected = (markers[i].uuid === uuid);
        var working = (markers[i].collectionuuid === collectionworking);
        var track = (markers[i].collectionuuid === collectiontrack);

        if (selected) {
            // Set icon and zindex for selected marker
            if (working) {
                markers[i].setIcon(iconworkingselected);
            } else if (track) {
                markers[i].setIcon(icontrackselected)
            } else {
                markers[i].setIcon(iconfileselected);
            }
            markers[i].setZIndex(zIndexSelectedMarker);
        } else {
            // Set marker to non-selected version
            if (working) {
                markers[i].setIcon(iconworking);
            } else if (track) {
                markers[i].setIcon(icontrack)
            } else {
                markers[i].setIcon(iconfile);
            }
            markers[i].setZIndex(zIndexMarker);
        }
    }
}


//
// Perform Geocode
//
//  Resuts:
//    DOOR:     street_number
//    STREET:   route
//    TOWN:     sublocality, locality
//    STATE:    administrative_area_level_3, administrative_area_level_2, administrative_area_level_1
//    COUNTRY:  country
//    POSTCODE: postal_code
//
function geocodeMarker(uuid, collectionuuid, forcegeocode) {
    for (var i = 0; i < markers.length ; i++) {
        if (markers[i].uuid === uuid && (markers[i].address === "" || forcegeocode)) {
            var pos = markers[i].getPosition();

            geocoderservice.geocode({
                    latLng: pos
                },
                function(results, status) {
                    handleGeocoderResults(pos, uuid, collectionuuid, results, status);
                });
        }
    }
}


function handleGeocoderResults(pos, uuid, collectionuuid, results, status) {

    var formattedaddress = "Unknown Location";
    var street_number = "",
        route = "Unknown Location",
        sublocality = "",
        locality = "",
        administrative_area_level_3 = "";
    var administrative_area_level_2 = "",
        administrative_area_level_1 = "";
    var door = "",
        street = "",
        town = "",
        state = "",
        country = "",
        postcode = "";
    var lat = pos.lat();
    var lon = pos.lng();

    if (status === google.maps.GeocoderStatus.OK) {

        formattedaddress = results[0].formatted_address;
        for (var j = 0; j < results[0].address_components.length; j++) {
            var long_name = results[0].address_components[j].long_name;
            var type = "";
            for (var k = 0; k < results[0].address_components[j].types.length; k++) {
                var type = results[0].address_components[j].types[k];
                if (type == "street_number") street_number = long_name;
                else if (type === "route") route = long_name;
                else if (type === "sublocality") sublocality = long_name;
                else if (type === "locality") locality = long_name;
                else if (type === "administrative_area_level_3") administrative_area_level_3 = long_name;
                else if (type === "administrative_area_level_2") administrative_area_level_2 = long_name;
                else if (type === "administrative_area_level_1") administrative_area_level_1 = long_name;
                else if (type === "country") country = long_name;
                else if (type === "postal_code") postcode = long_name;
            }
        }

        door = street_number;
        street = route;

        if (sublocality != "" && locality != "") town = sublocality + ", " + locality;
        else if (sublocality != "") town = sublocality;
        else town = locality;

        state = administrative_area_level_1;
        if (administrative_area_level_2 != "") state = administrative_area_level_2 + ", " + state;
        if (administrative_area_level_3 != "") state = administrative_area_level_3 + ", " + state;

        if (typeof GoogleMapsWidget !== 'undefined') {
            GoogleMapsWidget.jsmarkerGeocoded(uuid, collectionuuid, formattedaddress, door, street, town, state, country, postcode);
        }
    }
}

function removeMarker(uuid) {
    var j = -1;
    for (var i = 0; i < markers.length; i++) {
        if (markers[i].uuid === uuid) {
            j = i;
        }
    }
    if (j >= 0) {
        markers[j].setMap(null);
        if (markers[j].collectionuuid === collectiontrack) drawLines() ;
        markers.splice(j, 1);
    }
}


function removeAllMarkers() {
    for (var i = 0; i < markers.length; i++) {
        markers[i].setMap(null);
    }
    markers.length = 0;
    drawLines() ;
}


// BUG: doesnt draw lines on start
// BUG: seems to ignore order
// BUG: leaves old lines there
// BUG:

// Line Drawing
function drawLines() {

    if (polyline) {
        polyline.setMap(null) ;
        delete polyline ;
        polyline = null ;
    }

    GoogleMapsWidget.jsDebug("drawLines start") ;

    if (showlines) {

        var lineset = [] ;
        for (var i=0; i<markers.length; i++) {
            if (markers[i].collectionuuid === collectiontrack) {
                var lat = markers[i].position.lat() ;
                var lng = markers[i].position.lng() ;
                lineset.push( { lat: lat, lng: lng } ) ;
            }
        }

        if (lineset.length>1) {
            polyline = new google.maps.Polyline({
              path: lineset,
              geodesic: true,
              strokeColor: '#000080',
              strokeOpacity: 1.0,
              strokeWeight: 2
            });

            polyline.setOptions({ zIndex: zIndexTrackPlot });
            polyline.setMap(map);
        }

    }
    GoogleMapsWidget.jsDebug("drawLines ok") ;
}


// Event Handlers / Dispatchers

function handleMapMoved() {
    centre = map.getCenter();
    zoom = map.getZoom();
    bounds =  map.getBounds();
    if (typeof GoogleMapsWidget !== 'undefined')
        GoogleMapsWidget.jsmapMoved(centre.lat(), centre.lng(), zoom);
}

function handleMarkerMoved(marker) {
    var pos = marker.getPosition();
    GoogleMapsWidget.jsmarkerMoved(marker.uuid, marker.collectionuuid, pos.lat(), pos.lng());
    if (marker.collectionuuid === collectiontrack) drawLines() ;
}

function handleMarkerSelected(marker) {
    selectMarker(marker.uuid);
    GoogleMapsWidget.jsmarkerSelected(marker.uuid, marker.collectionuuid);
}
