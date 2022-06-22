/*
 * Application: POI Manager
 * File: geocode.js
 *
 * Geocoding javascript functions
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

//
// Open Streetmap
//


class OSM {
  /**
   * @constructor
   */

    constructor() {
       this.settings = {
          searchurl: 'https://nominatim.openstreetmap.org/search?format=json&addressdetails=1&limit=1&q=QUERY',
          geocodeurl: 'https://nominatim.openstreetmap.org/reverse?format=json&adressdetails=1&zoom=18&lon=LON&lat=LAT',
          searchoptions: {},
          geocodeoptions: {}
       };
    }

    setkey(k1, k2, k3) {
        // Keys are not required
    }

    search(query, callback) {
        var url = this.settings.searchurl.replace("QUERY", query) ;
        fetchit(url, this.settings.searchoptions, callback, this.handleSearchResponse)
    }

    geocode(lat, lon, callback) {
        var url = this.settings.geocodeurl.replace("LON",lon).replace("LAT",lat) ;
        fetchit(url, this.settings.geocodeoptions, callback, this.handleGeocodeResponse) ;
    }

    handleSearchResponse(results, callback) {
        if (results && results[0]) {
            var state
            callback( {
                     error: false,
                     status: 'OK',
                     errormessage: 'OK',
                     placeid: results[0].place_id || '',
                     lat: results[0].lat || '0',
                     lon: results[0].lon || '0',
                     name: results[0].display_name || 'Unknown',
                     road: results[0].address.road || '',
                     number: results[0].address.house_number || '',
                     postcode: results[0].address.postcode || '',
                     city: results[0].address.village || results[0].address.town || results[0].address.city || '',
                     state: (results[0].address.country==="UK") ?
                                results[0].address.county || results[0].address.state || '' :
                                results[0].address.state || results[0].address.county || '',
                     country: results[0].address.country || '',
                     countrycode: results[0].address.country_code || ''
            }) ;
        } else {
            callback( {
                         error: true,
                         status: 'FAILED',
                         errormessage: 'error'
            }) ;
        }
    }

    handleGeocodeResponse(results, callback) {
        if (results && results.address) {
            callback( {
                         error: false,
                         status: 'OK',
                         errormessage: 'OK',
                         placeid: results.place_id || '',
                         lat: results.lat || 0,
                         lon: results.lon || 0,
                         name: results.address.display_name || 'Unknown',
                         road: results.address.road || '',
                         number: results.address.house_number || '',
                         postcode: results.address.postcode || '',
                         city: results.address.village || results.address.town || results.address.city || '',
                         state: (results.address.country==="UK") ?
                                    results.address.county || results.address.state || '' :
                                    results.address.state || results.address.county || '',
                         country: results.address.country || '',
                         contrycode: results.address.country_code || ''
            }) ;
        } else {
            callback( {
                        error: true,
                         status: 'FAILED',
                         errormessage: 'error'
            }) ;
        }

    }
}


//
// HERE
//

// https://developer.here.com/documentation/geocoder/topics/example-geocoding-free-form.html
// https://developer.here.com/documentation/geocoder/topics/example-reverse-geocoding.html

class HERE {
  /**
   * @constructor
   */

    constructor(id, code, apikey) {
       this.settings = {
          searchurl: 'https://geocode.search.hereapi.com/v1/geocode?apiKey=APIKEY&q=QUERY',
          geocodeurl: 'https://revgeocode.search.hereapi.com/v1/revgeocode?apiKey=APIKEY&at=LAT,LON',
          searchoptions: {},
          geocodeoptions: {},
          id: id,
          code: code,
          apikey: apikey
       };
    }

    search(query, callback) {
        var url = this.settings.searchurl.replace("QUERY", query).replace("APPID", this.settings.id).replace("APPCODE", this.settings.code).replace("APIKEY", this.settings.apikey) ;
        fetchit(url, this.settings.searchoptions, callback, this.handleSearchResponse)
    }

    geocode(lat, lon, callback) {
        var url = this.settings.geocodeurl.replace("LON",lon).replace("LAT",lat).replace("APPID", this.settings.id).replace("APPCODE", this.settings.code).replace("APIKEY", this.settings.apikey) ;
        fetchit(url, this.settings.geocodeoptions, callback, this.handleGeocodeResponse) ;
    }

    handleSearchResponse(results, callback) {

        if (results.items && results.items[0] &&
            results.items[0].address && results.items[0].position) {

            var location = results.items[0] ;
            callback( {
                         error: false,
                         status: 'OK',
                         errormessage: 'OK',
                         placeid: location.id || '',
                         lat: location.position.lat || '0',
                         lon: location.position.lon || '0',
                         name: location.address.Label || 'Unknown',
                         road: location.address.street || '',
                         number: location.address.houseNumber || '',
                         postcode: location.address.postalCode || '',
                         city: location.address.city || '',
                         state: location.address.county || location.address.district || location.address.state || '',
                         country: location.address.countryName || '',
                         countrycode: location.address.countryCode || ''
            }) ;
        } else {
            callback( {
                         error: true,
                         status: 'FAILED',
                         errormessage: (results.error || 'error') + " - " + (results.error_description || 'unknown')
            }) ;
        }
    }

    handleGeocodeResponse(results, callback) {

        if (results.items && results.items[0] &&
            results.items[0].position && results.items[0].address) {

            var location = results.items[0] ;
            callback( {
                         error: false,
                         status: 'OK',
                         errormessage: 'OK',
                         placeid: location.id || '',
                         lat: location.position.lat || '0',
                         lon: location.position.lon || '0',
                         name: location.address.Label || 'Unknown',
                         road: location.address.street || '',
                         number: location.address.houseNumber || '',
                         postcode: location.address.postalCode || '',
                         city: location.address.city || '',
                         state: location.address.county || location.address.district || location.address.state || '',
                         country: location.address.countryName || '',
                         countrycode: location.address.countryCode || ''
            }) ;
        } else {
            callback( {
                        error: true,
                        status: 'FAILED',
                        errormessage: (results.error || 'error') + " - " + (results.error_description || 'unknown')
            }) ;
        }

    }
}

//
// Support Functions
//

function fetchit(url, options, callback, decoderesponse) {
    console.log("Fetching: " + url) ;
    fetch(url, options).then(function(response) {
        console.log("Received promise") ;
        var promise = response.json() ;
        return promise;
    }).then(function(results) {
        console.log("Received response") ;
        decoderesponse(results, callback) ;
        console.log(results);
    }).catch(function(error) {
        console.log(error);
    });
}

//
// Geocode
//

class Geocode {

    constructor(type, hereid, herecode, hereapikey) {
      this.osm = new OSM() ;
      this.here = new HERE(hereid, herecode, hereapikey) ;
      this.optsearch = type ;
      this.optgeocode = type ;
    }

    setkey(hereid, herecode, hereapikey) {
        this.here.setkey(hereid, herecode, hereapikey) ;
    }

    search(query, callback) {
      if (this.optsearch === 'osm') {
        this.osm.search(query, function(results) {
          callback(results) ;
        }) ;
      } else if (this.optsearch === 'here') {
        this.here.search(query, function(results) {
          callback(results) ;
        }) ;
      }
    }

    geocode(lat, lon, callback) {
      if (this.optgeocode === 'osm') {
        this.osm.geocode(lat, lon, function(results) {
          callback(results) ;
        }) ;
      } else if (this.optgeocode === 'here') {
        this.here.geocode(lat, lon, function(results) {
          callback(results) ;
        }) ;
      }
    }

}

