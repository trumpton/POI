#-------------------------------------------------
#
# Project created by QtCreator 2015-08-14T18:42:24
#
#-------------------------------------------------

QT += core gui network xml
QT += webenginewidgets

# Windows
#QT +=  webchannel

#CONFIG += qml_debug
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = POI
TEMPLATE = app

GIT_VERSION = $$system(git --git-dir $$PWD/../.git --work-tree $$PWD describe --always --tags)
DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"

SOURCES += main.cpp\
        mainwindow.cpp \
    poicollection.cpp \
    configuration.cpp \
    cookies.cpp \
    prompt.cpp \
    mainwindow_mapcallback.cpp \
    mainwindow_menu.cpp \
    mainwindow_editdetails.cpp \
    mainwindow_lists.cpp \
    easyexif/exif.cpp \
    undo.cpp \
    mapswidget.cpp \
    photoimportdialog.cpp \
    segmentchooser.cpp

HEADERS  += mainwindow.h \
    poicollection.h \
    configuration.h \
    cookies.h \
    urls.h \
    prompt.h \
    version.h \
    easyexif/exif.h \
    undo.h \
    mapswidget.h \
    photoimportdialog.h \
    segmentchooser.h

FORMS    += mainwindow.ui \
    configuration.ui \
    prompt.ui \
    photoimportdialog.ui \
    segmentchooser.ui

RESOURCES += \
    icons.qrc \
    mapicons.qrc \
    openlayers.qrc

OTHER_FILES +=

