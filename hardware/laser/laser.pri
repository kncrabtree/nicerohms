HEADERS += \
    $$PWD/laser.h

SOURCES += \
    $$PWD/laser.cpp

DEFINES += NICEROHMS_LASER=$$LASER

equals(LASER,0) {
    HEADERS += $$PWD/virtuallaser.h
	SOURCES += $$PWD/virtuallaser.cpp
}

equals(LASER,1) {
    HEADERS += $$PWD/lasermdt694b.h
	SOURCES += $$PWD/lasermdt694b.cpp
}
