HEADERS += \
    $$PWD/frequencycomb.h

SOURCES += \
    $$PWD/frequencycomb.cpp

DEFINES += NICEROHMS_FREQCOMB=$$FREQCOMB

equals(FREQCOMB,0) {
    HEADERS += $$PWD/virtualfreqcomb.h
	SOURCES += $$PWD/virtualfreqcomb.cpp
}

equals(FREQCOMB,1) {
    HEADERS += $$PWD/menlocomb.h
	SOURCES += $$PWD/menlocomb.cpp
}
