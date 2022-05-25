HEADERS += \
    $$PWD/wavemeter.h

SOURCES += \
    $$PWD/wavemeter.cpp

DEFINES += NICEROHMS_WAVEMETER=$$WAVEMETER

equals(WAVEMETER,0) {
    HEADERS += $$PWD/virtualwavemeter.h
	SOURCES += $$PWD/virtualwavemeter.cpp
}

equals(WAVEMETER,1) {
    HEADERS += $$PWD/bristol621.h
	SOURCES += $$PWD/bristol621.cpp
}
