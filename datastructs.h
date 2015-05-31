#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <QMetaType>

namespace NicerOhms {

enum LogMessageCode {
    LogNormal,
    LogWarning,
    LogError,
    LogHighlight,
    LogDebug
};

enum LabJackRange {
	LJR10V,
	LJR1V,
	LJR100mV,
	LJR10mV
};

}

#endif // DATASTRUCTS_H
