#
# POI Linux Makefile
#

#################################################
#
# Makefile configuration
#

# Makefile uses one of the following methods
# to find qmake in order of preference:
#
#  1. Specify the QMAKE variable in the Makefile
#  2. Use the QT_SELECT environment variable
#  3. Fallback to the QTVERSION variable in the Makefile
#
# Note: 2 and 3 above require that qtchooser is installed.
#

#QMAKE := /usr/bin/qmake
QTVERSION := qt6lts

# Application specific configuration
# Name of application exe file inside the AppImage

APPNAME := POI

# Name of the application as defined in the desktop file

FULLAPPNAME := POI_Manager

# Library to exclude from build

EXCLUDELIB := libnss

# Source folder relative to Makefile
# Source folder must contain appname.pro file

SOURCE := ./src

# Optional share data, which will be copied
# to usr/share/appname

SHARE := ./src/share

# Location of appname.desktop and appname.png

APPIMAGESRC := ./src/appimage

# Output folder for release executable

RELEASE := ./release

#################################################
# Working variables - Here be dragons

ARCH := $(shell arch)
BUILD := ./build_${ARCH}
TOOLSDIR := ./tools
LINUXDEPLOY := linuxdeploy-${ARCH}.AppImage
LINUXDEPLOYQT := linuxdeploy-plugin-qt-${ARCH}.AppImage

GITVER := $(shell git describe --tags)
QV := $(if $(QT_SELECT),$(QT_SELECT),$(QTVERSION))
QMAKEEXE := $(if ${QMAKE},${QMAKE},$(shell echo "`qtchooser -qt=${QV} -print-env | grep QTTOOLDIR | cut -d\\" -f2`/qmake"))
OUTPUTFILENAME := ${FULLAPPNAME}_${GITVER}_${ARCH}.AppImage

help:
	#
	# Makefile for Linux build
	#
	#
	# make clean     - Remove the build files
	# make spotless  - Remove the build files and output executable
	# make all	 - Builds everything
	#
	# make qmake     - Runs qmake, ready for build / compilation
	# make build     - Builds application from Makefile in build folder
	# make appimage  - Builds appimage from application in build folder
	#
	# Current ${APPNAME} version : ${GITVER}
	#
	# Using qmake : ${QMAKEEXE}
	#
	#   To modify qmake version:
	#
	#   1. export QT_SELECT=5 ; make
	#   2. Edit Makefile and specify QMAKE version directly
	#   3. Edit Makefile and specify QTVERSION directly
	#
	#   Note: 1 and 3 require qtchooser to be installed
	#

clean:
	/bin/rm -rf ${BUILD}

spotless: clean
	/bin/rm -f ${TOOLSDIR}/${LINUXDEPLOY}
	/bin/rm -f ${TOOLSDIR}/${LINUXDEPLOYQT}
	# AppImages in ${RELEASE} are not autmatically removed

all: appimage

qmake: ${BUILD}/Makefile
	
build: ${BUILD}/${APPNAME}

appimage: tools build ${RELEASE}/${OUTPUTFILENAME}

#
# Download linuxdeploy tools
#

tools: ${TOOLSDIR}/${LINUXDEPLOY} ${TOOLSDIR}/${LINUXDEPLOYQT}

${TOOLSDIR}/${LINUXDEPLOY}:
	mkdir -p ${TOOLSDIR} && \
	cd ${TOOLSDIR} && \
	wget -N https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/${LINUXDEPLOY} && \
	chmod +x ${LINUXDEPLOY}

${TOOLSDIR}/${LINUXDEPLOYQT}:
        # QtWebEngineProcess - not currently supported {12/6/2022}
        # https://github.com/linuxdeploy/linuxdeploy-plugin-qt/issues/112
        # so alternative linuxdeploy-plugin-qt is used
        # https://github.com/koord-live/linuxdeploy-plugin-qt/releases
	# wget -N https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/${LINUXDEPLOYQT}

	mkdir -p ${TOOLSDIR} && \
	cd ${TOOLSDIR} && \
	wget -N https://github.com/koord-live/linuxdeploy-plugin-qt/releases/download/continuous/${LINUXDEPLOYQT} && \
	chmod +x ${LINUXDEPLOYQT}

#
# Build the appimage
#

${RELEASE}/${OUTPUTFILENAME}:

	mkdir -p ${RELEASE}
	export QMAKE=${QMAKEEXE} && cd ${BUILD} && \
		../${TOOLSDIR}/${LINUXDEPLOY} \
        	--desktop-file "../${APPIMAGESRC}/${APPNAME}.desktop" \
        	--icon-file "../${APPIMAGESRC}/${APPNAME}.png" \
        	--exclude-library "${EXCLUDELIB}*" \
        	--appdir appdir \
        	--executable ${APPNAME} \
		--plugin installmenu \
        	--plugin qt

# libnss causes problems, so is unbundled
# note: the exclude-library is not passes to the qt plugin
# so this step is done by hand

	@[ "${EXCLUDELIB}" ] && /bin/rm -f ${BUILD}/appdir/usr/lib/${EXCLUDELIB}*

# Copy application-specific data files from src/share to appimage/share/appname

	@[ "${SHARE}" ] && mkdir -p "${BUILD}/appdir/usr/share/${APPNAME}"
	@[ "${SHARE}" ] && cp -R ${SHARE}/* ${BUILD}/appdir/usr/share/${APPNAME}


# Now create the output image

	export OUTPUT=../${RELEASE}/${OUTPUTFILENAME} && cd ${BUILD} && \
		../${TOOLSDIR}/${LINUXDEPLOY} \
		--desktop-file "../${APPIMAGESRC}/${APPNAME}.desktop" \
		--icon-file "../${APPIMAGESRC}/${APPNAME}.png" \
		--appdir appdir \
		--exclude-library "${EXCLUDELIB}*" \
		--output appimage

	#
	# AppImage file has been placed in ${RELEASE} folder
	#
#
# Compile the application using the makefile
#

${BUILD}/${APPNAME}: ${BUILD}/Makefile
	cd ${BUILD} && \
		make

#
# Create the makefile using Qt
#

${BUILD}/Makefile: 
	mkdir -p ${BUILD}
	cd ${BUILD} && \
		${QMAKEEXE} -o Makefile ../${SOURCE}/${APPNAME}.pro -spec linux-g++

