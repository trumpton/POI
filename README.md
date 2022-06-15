# POI - Points of Interest and Track management.

<img src="./manual/POI_Manager_Screenshot.png">

# Background and Overview

This program is capable of managing points of interest files, which are saved in both .gpx and .ov2 formats for Garmin and TomTom GPSs.
In addition, it is capable of managing .gpx tracks (with waypoints).
The program can import and export tracks from/to a filesystem mounted Garmin GPS, such as the E-Trex series.  Exporting to a TomTom GPS launches and uses a third party web browser.

When walking tracks are saved, the first waypoint on the track is also saved to a tracks.gpx, and tracks.ov2 file, which is particularly useful if the first point on a walk is accessed by car.

This program is provided free of charge, and no warranty or guarantees are offered.

# Manual
The latest manual can be found here: 
https://github.com/trumpton/POI/blob/master/manual/POI_Manager_Manual_v0.1.pdf

# Configuration

The program has certain map tile servers built-in, but these can be overridden using the POI.ini file (an example of which exists in the src directory).

In addition to the map tile server URLs, the configuration recognises three different keywords, which are used to identify built-in features:

OSM - this is used to specify the base map, and the Nomaitm Open Streetmaps geocoding.
Here - this is used to specify the Here geocoding (application id and code required in the keys section)
Bing - this is used to specify the Bing aerial maps (key required in the keys section)

The program can also upload contacts to Google, and for this it requires a Google access key and secret
adding to the ini file.  Please see the ini file for details.

# Linux Building - Preparation

Ensure QT is installed.  This program has been designed to work with QT6.  
If you install directly from https://www.qt.io/download

Make sure that you point to the correct qmake file - this can be done by
specifying it explictly by editing the Makefile, or installing 'qtchooser'

If you install qtchooser, add a qtx.y.z.conf file to /etc/xdg/qtchooser
and ensure it contains 2 lines, the first being the path to the qt bin folder
and the second containing the path to the qt lib folder

Either load the POI.pro file in QtCreator

Or use make to compile:

	make build

Or create an AppImage release:

	make appimage

# Configuration File

The configuration is not loaded from the POI.ini file, which must be manually edited if keys are to be added.
The file is automatically created (copied from the exe folder or usr/share/POI folder) when the configuration 
directory is selected from the configuration menu.

# Debugging

Add the following line to the command line in order to use a chromium-based browser to debug embedded javascript:

  --remote-debugging-port=7890

Then connect the browser to:

  http://localhost:7890/

