#-------------------------------------------------
#
# Project created by QtCreator 2015-05-23T09:29:32
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = nicer-ohms
TEMPLATE = app

include(gui.pri)
include(data.pri)
include(hardware.pri)

SOURCES += main.cpp

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS += -std=c++11
