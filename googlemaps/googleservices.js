var map; // Setup by initialise
var GoogleMapsWidget; // Setup by initialise
var searchservice; // Setup by initialise
var detailsservice; // Setup by initialise
var geocoderservice; // Setup by initialise
var collectionworking; // Setup by initialise

var markers = [];

var initialLat = 48.13;
var initialLon = 11.57;
var initialZoom = 3;

// https://sites.google.com/site/gmapsdevelopment/
var iconworking = "qrc:///googlemaps/marker-red.png";
var iconworkingselected = "qrc:///googlemaps/marker-red-dot.png";
var iconfile = "qrc:///googlemaps/marker-green.png";
var iconfileselected = "qrc:///googlemaps/marker-green-dot.png";

// Create Map
function initialise(workinguuid) {

    var myOptions = {
        center: new google.maps.LatLng(initialLat, initialLon),
        zoom: initialZoom,
        mapTypeId: google.maps.MapTypeId.ROADMAP,
        panControl: true
    };

    map = new google.maps.Map(document.getElementById("map_canvas"), myOptions);

    searchservice = new google.maps.places.PlacesService(map);
    detailsservice = new google.maps.places.PlacesService(map);
    geocoderservice = new google.maps.Geocoder;
    collectionworking = workinguuid;

    if ("GoogleMapsWebViewWidget" in window) {

        // WebView (set with addToJavaScriptWindowObject)
        GoogleMapsWidget = GoogleMapsWebViewWidget;

        map.addListener('center_changed', function() {
            handleMapMoved(map);
        });
        map.addListener('zoom_changed', function() {
            handleMapMoved(map);
        });
        GoogleMapsWidget.jsmapMoved(initialLat, initialLon, initialZoom);

    } else {

        new QWebChannel(qt.webChannelTransport, function(channel) {

            // WebEngineView (set via channel)
            GoogleMapsWidget = channel.objects.GoogleMapsWidget;

            map.addListener('center_changed', function() {
                handleMapMoved(map);
            });
            map.addListener('zoom_changed', function() {
                handleMapMoved(map);
            });
            GoogleMapsWidget.jsmapMoved(initialLat, initialLon, initialZoom);

        });
    }

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
function gotoCoordinates(pos, zoom) {
    if (zoom > 0) map.setZoom(zoom);
    map.setCenter(pos);
    handleMapMoved();
}



// Marker Management
function setMarker(uuid, collectionuuid, lat, lon, address) {
    var draggable = false;
    if (collectionuuid == collectionworking) draggable = true;
    var marker = new google.maps.Marker({
        map: map,
        uuid: uuid,
        collectionuuid: collectionuuid,
        zIndex: 1,
        position: new google.maps.LatLng(lat, lon),
        draggable: draggable,
        address: address,
        animation: google.maps.Animation.DROP
    });
    if (collectionuuid === collectionworking) {
        marker.setIcon(iconworking);
    } else {
        marker.setIcon(iconfile);
    }
    removeMarker(uuid);
    markers.push(marker);
    google.maps.event.addListener(marker, 'click', function() {
        handleMarkerSelected(marker);
    });
    google.maps.event.addListener(marker, 'dragstart', function() {
        handleMarkerSelected(marker);
    });
    google.maps.event.addListener(marker, 'dragend', function() {
        handleMarkerMoved(marker);
    });

    // TODO: Do we need to select and geocode new markers
    //        selectMarker(uuid) ;
    //        geocodeMarker(uuid) ;
}



function setMarkerCollection(uuid, collectionuuid) {
    var i = markers.length - 1;
    while (i >= 0 && markers[i].uuid !== uuid) i--;
    if (i >= 0) {
        markers[i].collectionuuid = collectionuuid;
        if (collectionuuid == collectionworking) {
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

    // Redraw non-selected marker, and identify selected
    for (var i = 0; i < markers.length; i++) {
        var selected = (markers[i].uuid === uuid);
        var working = (markers[i].collectionuuid === collectionworking);
        if (selected) {
            selectedmarker = i;
            selectedisworking = working;
        } else {
            if (working) {
                markers[i].setIcon(iconworking);
            } else {
                markers[i].setIcon(iconfile);
            }
            markers[i].setZIndex(1);
        }
    }

    // TODO: This marker must be brought to the front
    // Re-draw selected marker
    if (selectedmarker >= 0) {
        if (selectedisworking) {
            markers[selectedmarker].setIcon(iconworkingselected);
        } else {
            markers[selectedmarker].setIcon(iconfileselected);
        }
        markers[selectedmarker].setZIndex(google.maps.Marker.MAX_ZINDEX + 1);
    }

}



function seekToMarker(uuid, zoom) {
    var i = markers.length - 1;
    while (i >= 0 && markers[i].uuid !== uuid) i--;
    if (i >= 0) {
        var pos = markers[i].position;
        gotoCoordinates(pos, zoom);
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
                else if (type == "route") route = long_name;
                else if (type == "sublocality") sublocality = long_name;
                else if (type == "locality") locality = long_name;
                else if (type == "administrative_area_level_3") administrative_area_level_3 = long_name;
                else if (type == "administrative_area_level_2") administrative_area_level_2 = long_name;
                else if (type == "administrative_area_level_1") administrative_area_level_1 = long_name;
                else if (type == "country") country = long_name;
                else if (type == "postal_code") postcode = long_name;
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

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! !! !! !!
        // TODO: Check where Emily Carrs House is listed - there appears to be another line in the format
        // address =  "Emily Carr's House, Bear Ln, Oxford OX1 4EJ, UK" , door =  "" , street =  "Bear Lane" , town=  "Oxford" , state =  "Oxfordshire, England" , country =  "United Kingdom" , postcode =  "OX1 4EJ"

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
        markers.splice(j, 1);
    }
}




function removeAllMarkers() {
    for (var i = 0; i < markers.length; i++) {
        markers[i].setMap(null);
    }
    markers.length = 0;
}




// Event Handlers / Dispatchers

function handleMapMoved() {
    var Loc = map.getCenter();
    var zoom = map.getZoom();
    if (typeof GoogleMapsWidget !== 'undefined')
        GoogleMapsWidget.jsmapMoved(Loc.lat(), Loc.lng(), zoom);
}



function handleMarkerMoved(marker) {
    var pos = marker.getPosition();
    GoogleMapsWidget.jsmarkerMoved(marker.uuid, marker.collectionuuid, pos.lat(), pos.lng());
}



function handleMarkerSelected(marker) {
    selectMarker(marker.uuid);
    GoogleMapsWidget.jsmarkerSelected(marker.uuid, marker.collectionuuid);
}
