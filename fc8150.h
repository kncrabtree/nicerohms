#ifndef FC8150_H
#define FC8150_H

#include "frequencycounter.h"

class FC8150 : public FrequencyCounter
{
    Q_OBJECT
public:
    FC8150(QObject *parent = nullptr);
    ~FC8150();

public slots:
    bool testConnection();
    void initialize();
    Scan prepareForScan(Scan scan);
    void readPointData();

protected:
    double read();

};

#endif // FC8150_H
