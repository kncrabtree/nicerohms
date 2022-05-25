HEADERS += \
    $$PWD/gpibcontroller.h \

SOURCES += \
    $$PWD/gpibcontroller.cpp


DEFINES += NICEROHMS_GPIBCONTROLLER=$$GPIBCONTROLLER

equals(GPIBCONTROLLER,0) {
    HEADERS += $$PWD/virtualgpibcontroller.h
	SOURCES += $$PWD/virtualgpibcontroller.cpp
}

equals(GPIBCONTROLLER,1) {
    HEADERS += $$PWD/prologixusb.h
	SOURCES += $$PWD/prologixusb.cpp
}
