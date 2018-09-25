# POI - Points of Interest and Track management.

# Background and Overview

This program is capable of managing points of interest files, which are saved in both .gpx and .ov2 formats for Garmin and TomTom GPSs.
In addition, it is capable of managing .gpx tracks (with waypoints).
The program can import and export tracks from/to a filesystem mounted Garmin GPS, such as the E-Trex series.  Exporting to a TomTom GPS launches and uses a third party web browser.

When walking tracks are saved, the first waypoint on the track is also saved to a tracks.gpx, and tracks.ov2 file, which is particularly useful if the first point on a walk is accessed by car.

This program is provided free of charge, and no warranty or guarantees are offered.

# Configuration

The program has certain map tile servers built-in, but these can be overridden using the POI.ini file (an example of which exists in the src directory).

In addition to the map tile server URLs, the configuration recognises three different keywords, which are used to identify built-in features:

OSM - this is used to specify the base map, and the Nomaitm Open Streetmaps geocoding.
Here - this is used to specify the Here geocoding (application id and code required in the keys section)
Bing - this is used to specify the Bing aerial maps (key required in the keys section)


