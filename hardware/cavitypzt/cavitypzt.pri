HEADERS += \
    $$PWD/cavitypztdriver.h

SOURCES += \
    $$PWD/cavitypztdriver.cpp

DEFINES += NICEROHMS_CAVITYPZT=$$CAVITYPZT

equals(CAVITYPZT, 0) {
    HEADERS += $$PWD/virtualcavitypzt.h
	SOURCES += $$PWD/virtualcavitypzt.cpp
}

equals(CAVITYPZT, 1) {
    HEADERS += $$PWD/cavitymdt694b.h
	SOURCES += $$PWD/cavitymdt694b.cpp
}
