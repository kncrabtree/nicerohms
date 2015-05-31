#include "ioboarddigitalconfigmodel.h"

#include <QSettings>
#include <QApplication>

IOBoardDigitalConfigModel::IOBoardDigitalConfigModel(QObject *parent) :
	QAbstractTableModel(parent)
{
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	QString sk = s.value(QString("ioboard/subKey"),QString("virtual")).toString();
	int numPins = s.value(QString("ioboard/%1/numDigitalChannels").arg(sk),20).toInt();
	d_reservedPins = s.value(QString("ioboard/%1/reservedDigitalChannels").arg(sk),2).toInt();

	s.beginGroup(QString("lastScanConfig"));
	s.beginReadArray(QString("ioboardDigital"));
	for(int i=d_reservedPins; i<numPins; i++)
	{
		s.setArrayIndex(i);
		bool en = s.value(QString("enabled"),false).toBool();
		QString name = s.value(QString("name"),QString("")).toString();

		beginInsertRows(QModelIndex(),i-d_reservedPins,i-d_reservedPins);
		d_channelConfigList.append(qMakePair(en,name));
		endInsertRows();
	}
	s.endArray();
	s.endGroup();

}

IOBoardDigitalConfigModel::~IOBoardDigitalConfigModel()
{

}

void IOBoardDigitalConfigModel::saveToSettings()
{
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	s.beginGroup(QString("lastScanConfig"));
	s.beginWriteArray(QString("ioboardDigital"));
	for(int i=0; i<d_channelConfigList.size(); i++)
	{
		s.setArrayIndex(i+d_reservedPins);
		s.setValue(QString("enabled"),d_channelConfigList.at(i).first);
		s.setValue(QString("name"),d_channelConfigList.at(i).second);
	}
	s.endArray();
	s.endGroup();
	s.sync();
}



int IOBoardDigitalConfigModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return d_channelConfigList.size();
}

int IOBoardDigitalConfigModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return 2;
}

QVariant IOBoardDigitalConfigModel::data(const QModelIndex &index, int role) const
{
	QVariant out;
	if(index.row() >= d_channelConfigList.size() || index.row() < 0)
		return out;

	if(role == Qt::CheckStateRole)
	{
		if(index.column() == 0)
			out = (d_channelConfigList.at(index.row()).first ? Qt::Checked : Qt::Unchecked);
	}
	else if(role == Qt::DisplayRole)
	{
		switch (index.column()) {
		case 0:
			break;
		case 1:
			out = d_channelConfigList.at(index.row()).second;
			break;
		default:
			break;
		}
	}
	else if(role == Qt::EditRole)
	{
		switch(index.column()) {
		case 0:
			break;
		case 1:
			out = d_channelConfigList.at(index.row()).second;
			break;
		}
	}

	return out;
}

bool IOBoardDigitalConfigModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(index.row() < 0 || index.row() >= d_channelConfigList.size() || index.column() < 0 || index.column() > 2)
		return false;

	if(role == Qt::CheckStateRole)
	{
		if(index.column() == 0)
		{
			d_channelConfigList[index.row()].first = value.toBool();
			emit dataChanged(index,index);
			return true;
		}
	}

	if(role != Qt::EditRole)
		return false;


	if(index.column() == 1)
	{
		d_channelConfigList[index.row()].second = value.toString();
		emit dataChanged(index,index);
		return true;
	}

	return false;
}

QVariant IOBoardDigitalConfigModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(role == Qt::DisplayRole)
	{
		if(orientation == Qt::Horizontal)
		{
			if(section == 0)
				return QString("On");
			if(section == 1)
				return QString("Name");
		}
		else if(orientation == Qt::Vertical)
		{
			if(section >= 0 && section < d_channelConfigList.size())
				return QString("DIN%1").arg(section+d_reservedPins);
		}
	}

	return QVariant();
}

Qt::ItemFlags IOBoardDigitalConfigModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags out = Qt::NoItemFlags;
	switch(index.column()) {
	case 0:
		out = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
		break;
	case 1:
		out = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		break;
	default:
		break;
	}

	return out;
}
