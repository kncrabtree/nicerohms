HEADERS += \
    $$PWD/aomsynthesizer.h

SOURCES += \
    $$PWD/aomsynthesizer.cpp

DEFINES += NICEROHMS_AOMSYNTH=$$AOMSYNTH

equals(AOMSYNTH,0) {
    HEADERS += $$PWD/virtualaomsynthesizer.h
	SOURCES += $$PWD/virtualaomsynthesizer.cpp
}

equals(AOMSYNTH,1) {
    HEADERS += $$PWD/aom8657a.h
	SOURCES += $$PWD/aom8657a.cpp
}
