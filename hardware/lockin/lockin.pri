HEADERS += \
    $$PWD/lockin.h

SOURCES += \
    $$PWD/lockin.cpp

LOCKIN_HW = VirtualLockIn SR530

for(num, 0..$$size(LOCKINS)) {
N = $$member(LOCKINS,$$num)
count(N,1) {
DEFINES += NICEROHMS_LOCKIN_$$num=$$member(LOCKIN_HW,$$N)
}
}

contains(LOCKINS,0) {
    DEFINES += NICEROHMS_VIRTUALLOCKIN
	HEADERS += $$PWD/virtuallockin.h
	SOURCES += $$PWD/virtuallockin.cpp
}

contains(LOCKINS,1) {
    DEFINES += NICEROHMS_LOCKINSR530
	HEADERS += $$PWD/virtuallockin.h
	SOURCES += $$PWD/virtuallockin.cpp
}
