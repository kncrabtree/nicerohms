HEADERS += \
    $$PWD/frequencycounter.h

SOURCES += \
    $$PWD/frequencycounter.cpp

DEFINES += NICERHOMS_FREQCOUNTER=$$FREQCOUNTER

equals(FREQCOUNTER,0) {
    HEADERS += $$PWD/fc8150.h
	SOURCES += $$PWD/fc8150.cpp
}

equals(FREQCOUNTER,1) {
    HEADERS += $$PWD/fc8150.h
	SOURCES += $$PWD/fc8150.cpp
}
