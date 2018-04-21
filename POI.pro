#-------------------------------------------------
#
# Project created by QtCreator 2015-08-14T18:42:24
#
#-------------------------------------------------

QT += core gui network

# Linux
QT += webkitwidgets

# Windows
# QT +=  webchannel webenginewidgets

CONFIG += qml_debug
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = POI
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    googlemapswidget.cpp \
    poicollection.cpp \
    configuration.cpp \
    tomtomwidget.cpp \
    cookies.cpp

HEADERS  += mainwindow.h \
    googlemapswidget.h \
    poicollection.h \
    configuration.h \
    tomtomwidget.h \
    WebAccess.h \
    cookies.h

FORMS    += mainwindow.ui \
    configuration.ui

RESOURCES += \
    resources.qrc

OTHER_FILES +=

DISTFILES +=
