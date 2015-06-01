#ifndef IOBOARDDIGITALCONFIGMODEL_H
#define IOBOARDDIGITALCONFIGMODEL_H

#include <QAbstractTableModel>

#include <QList>

class IOBoardDigitalConfigModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	IOBoardDigitalConfigModel(QObject *parent = nullptr);
	~IOBoardDigitalConfigModel();

	void saveToSettings();
	QList<QPair<int,bool>> getConfig();

	// QAbstractItemModel interface
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;



private:
	QList<QPair<bool,QString>> d_channelConfigList;
	int d_reservedPins;

};

#endif // IOBOARDDIGITALCONFIGMODEL_H
