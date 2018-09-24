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

    setkey(k1, k2) {
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
                     country: results[0].address.country || ''
            }) ;
        } else {
            callback( {
                         error: true,
                         status: 'FAILED'
            }) ;
        }
    }

    handleGeocodeResponse(results, callback) {
        if (results && results.address) {
            callback( {
                         error: false,
                         status: 'OK',
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
                         country: results.address.country || ''
            }) ;
        } else {
            callback( {
                        error: true,
                        status: 'FAILED'
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

    constructor(id, code) {
       this.settings = {
          searchurl: 'https://geocoder.api.here.com/6.2/geocode.json?app_id=APPID&app_code=APPCODE&searchtext=QUERY',
          geocodeurl: 'https://reverse.geocoder.api.here.com/6.2/reversegeocode.json?app_id=APPID&app_code=APPCODE&&mode=retrieveAddresses&prox=LAT,LON,25',
          searchoptions: {},
          geocodeoptions: {},
          id: id,
          code: code
       };
    }

    search(query, callback) {
        var url = this.settings.searchurl.replace("QUERY", query).replace("APPID", this.settings.id).replace("APPCODE", this.settings.code) ;
        fetchit(url, this.settings.searchoptions, callback, this.handleSearchResponse)
    }

    geocode(lat, lon, callback) {
        var url = this.settings.geocodeurl.replace("LON",lon).replace("LAT",lat).replace("APPID", this.settings.id).replace("APPCODE", this.settings.code) ;
        fetchit(url, this.settings.geocodeoptions, callback, this.handleGeocodeResponse) ;
    }

    handleSearchResponse(results, callback) {

        if (results.Response && results.Response.View && results.Response.View[0] &&
            results.Response.View[0].Result && results.Response.View[0].Result[0] &&
            results.Response.View[0].Result[0].Location && results.Response.View[0].Result[0].Location.Address) {

            var location = results.Response.View[0].Result[0].Location ;
            callback( {
                         error: false,
                         status: 'OK',
                         placeid: location.LocationId || '',
                         lat: location.DisplayPosition.Latitude || '0',
                         lon: location.DisplayPosition.Longitude || '0',
                         name: location.Address.Label || 'Unknown',
                         road: location.Address.Street || '',
                         number: location.Address.HouseNumber || '',
                         postcode: location.Address.PostalCode || '',
                         city: location.Address.City || '',
                         state: location.Address.County || location.Address.District || location.Address.State || '',
                         country: location.Address.Country || ''
            }) ;
        } else {
            callback( {
                         error: true,
                         status: 'FAILED'
            }) ;
        }
    }

    handleGeocodeResponse(results, callback) {

        if (results.Response && results.Response.View && results.Response.View[0] &&
            results.Response.View[0].Result && results.Response.View[0].Result[0] &&
            results.Response.View[0].Result[0].Location && results.Response.View[0].Result[0].Location.Address) {

            var location = results.Response.View[0].Result[0].Location ;
            callback( {
                         error: false,
                         status: 'OK',
                         placeid: location.LocationId || '',
                         lat: location.DisplayPosition.Latitude || '0',
                         lon: location.DisplayPosition.Longitude || '0',
                         name: location.Address.Label || 'Unknown',
                         road: location.Address.Street || '',
                         number: location.Address.HouseNumber || '',
                         postcode: location.Address.PostalCode || '',
                         city: location.Address.City || '',
                         state: location.Address.County || location.Address.District || location.Address.State || '',
                         country: location.Address.Country || ''
            }) ;
        } else {
            callback( {
                        error: true,
                        status: 'FAILED'
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

    constructor(type, hereid, herecode) {
      this.osm = new OSM() ;
      this.here = new HERE(hereid, herecode) ;
      this.optsearch = type ;
      this.optgeocode = type ;
    }

    setkey(hereid, herecode) {
        this.here.setkey(hereid, herecode) ;
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

