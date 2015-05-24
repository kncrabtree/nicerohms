#-------------------------------------------------
#
# Project created by QtCreator 2015-05-23T09:29:32
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = nicerohms
TEMPLATE = app

include(gui.pri)
include(data.pri)
include(hardware.pri)
include(implementations.pri)

SOURCES += main.cpp

unix:!macx: LIBS += -lqwt

#Hardware configuration
#Set the following variables to select hardware implementations
#Note: if you change the config, you need to rebuild the whole project!

#Laser. 0 = virtual, 1 = PZT control via MTD694B
DEFINES += NICEROHMS_LASER=0

#Lockins. 0 = virtual, 1 = Stanford SR530
DEFINES += NICEROHMS_LOCKIN1=0 NICERHOMS_LOCKIN2=0

#Wavemeter. 0 = virtual, 1 = Bristol 621
DEFINES += NICEROHMS_WAVEMETER=0

#Cavity PZT Driver. 0 = virtual, 1 = MTD694B
DEFINES += NICEROHMS_CAVITYPZT=0

#IO Board. 0 = virtual, 1 = LabJack U6
DEFINES += NICEROHMS_IOBOARD=0



QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS += -std=c++11
