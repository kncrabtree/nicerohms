.#-------------------------------------------------
#
# Project created by QtCreator 2015-05-23T09:29:32
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport xml

TARGET = nicerohms
TEMPLATE = app
CONFIG += c++17

!exists(config/config.pri) {
    error('config.pri file not found in config directory. Please copy config/config.pri.template to config/config.pri, and then edit config/config.pri as needed.')
}

include(config/config.pri)
include(acquisition/acquisition.pri)
include(gui/gui.pri)
include(data/data.pri)
include(hardware/hardware.pri)

SOURCES += main.cpp
DISTFILES += \
    settings.readme

RESOURCES += \
    resources/resources.qrc

