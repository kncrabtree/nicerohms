#-------------------------------------------------
#
# Project created by QtCreator 2015-05-23T09:29:32
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = nicer-ohms
TEMPLATE = app

include(gui.pri)
include(data.pri)
include(hardware.pri)
include(implementations.pri)

SOURCES += main.cpp

unix:!macx: LIBS += -lqwt

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS += -std=c++11
