#ifndef BATCHMANAGER_H
#define BATCHMANAGER_H

#include <QObject>

#include "datastructs.h"
#include "scan.h"

class BatchManager : public QObject
{
    Q_OBJECT
public:
    enum BatchType
    {
	   SingleScan
    };

    explicit BatchManager(BatchType b);
    ~BatchManager();

signals:
    void logMessage(QString,NicerOhms::LogMessageCode = NicerOhms::LogNormal);
    void beginScan(Scan);
    void batchComplete(bool aborted);

public slots:
    void scanComplete(const Scan s);
    void beginNextScan();

protected:
    BatchType d_type;

    virtual void writeReport() =0;
    virtual void processScan(const Scan s) =0;
    virtual Scan nextScan() =0;
    virtual bool isComplete() =0;
};

#endif //BATCHMANAGER_H
