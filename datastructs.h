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

enum WmDisplayUnits {
	WmWavenumbers,
	WmGHz
};

}

Q_DECLARE_METATYPE(NicerOhms::WmDisplayUnits)

#endif // DATASTRUCTS_H
