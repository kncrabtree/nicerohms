#ifndef SCAN_H
#define SCAN_H

#include <QSharedDataPointer>

#include <QDateTime>

class ScanData;

class Scan
{
public:
	Scan();
	Scan(const Scan &);
	Scan &operator=(const Scan &);
	~Scan();

	bool hardwareSuccess() const;

	void setHardwareFailed();
	void setInitialized();

private:
	QSharedDataPointer<ScanData> data;
};


class ScanData : public QSharedData
{
public:
	ScanData() : number(0), isInitialized(false), hardwareSuccess(true) {}

	int number;
	bool isInitialized;
	bool hardwareSuccess;
	QDateTime startTime;
};

#endif // SCAN_H
