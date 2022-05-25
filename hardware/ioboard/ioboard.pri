HEADERS += \
    $$PWD/ioboard.h

SOURCES += \
    $$PWD/ioboard.cpp

DEFINES += NICEROHMS_IOBOARD=$$IOBOARD

equals(IOBOARD,0) {
    HEADERS += $$PWD/virtualioboard.h
	SOURCES += $$PWD/virtualioboard.cpp
}

equals(IOBOARD,1) {
    HEADERS += $$PWD/ioboardu6.h \
	    $$PWD/u6.h
		SOURCES += $$PWD/ioboardu6.cpp \
	    $$PWD/u6.cpp

    LIBS += -llabjackusb
}
