#include "ioboardanalogconfigmodel.h"

#include <QSettings>
#include <QApplication>
#include <QComboBox>

IOBoardAnalogConfigModel::IOBoardAnalogConfigModel(QObject *parent) :
	QAbstractTableModel(parent)
{
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	int numChannels = s.value(QString("ioboard/numAnalogChannels"),14).toInt();

	s.beginGroup(QString("lastScanConfig"));
	s.beginReadArray(QString("ioboardAnalog"));
	for(int i=0; i<numChannels; i++)
	{
		s.setArrayIndex(i);
		bool en = s.value(QString("enabled"),false).toBool();
		QString name = s.value(QString("name"),QString("")).toString();
		NicerOhms::LabJackRange range = static_cast<NicerOhms::LabJackRange>(s.value(QString("range"),NicerOhms::LJR10V).toInt());

		AnalogChannelConfig cc;
		cc.enabled = en;
		cc.name = name;
		cc.range = range;

		beginInsertRows(QModelIndex(),i,i);
		d_channelConfigList.append(cc);
		endInsertRows();
	}

	s.endArray();
	s.endGroup();


}

IOBoardAnalogConfigModel::~IOBoardAnalogConfigModel()
{

}

void IOBoardAnalogConfigModel::saveToSettings()
{
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	s.beginGroup(QString("lastScanConfig"));
	s.beginWriteArray(QString("ioboardAnalog"));
	for(int i=0; i<d_channelConfigList.size(); i++)
	{
		s.setArrayIndex(i);
		s.setValue(QString("enabled"),d_channelConfigList.at(i).enabled);
		s.setValue(QString("name"),d_channelConfigList.at(i).name);
		s.setValue(QString("range"),d_channelConfigList.at(i).range);
	}

	s.endArray();
	s.endGroup();
}



int IOBoardAnalogConfigModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return d_channelConfigList.size();
}

int IOBoardAnalogConfigModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return 3;
}

QVariant IOBoardAnalogConfigModel::data(const QModelIndex &index, int role) const
{
	QVariant out;
	if(index.row() >= d_channelConfigList.size() || index.row() < 0)
		return out;

	if(role == Qt::CheckStateRole)
	{
		if(index.column() == 0)
			out = (d_channelConfigList.at(index.row()).enabled ? Qt::Checked : Qt::Unchecked);
	}
	else if(role == Qt::DisplayRole)
	{
		switch (index.column()) {
		case 0:
			break;
		case 1:
			out = d_channelConfigList.at(index.row()).name;
			break;
		case 2:
		{
			QString s;
			switch (d_channelConfigList.at(index.row()).range) {
			case NicerOhms::LJR10V:
				s = QString("10 V");
				break;
			case NicerOhms::LJR1V:
				s = QString("1 V");
				break;
			case NicerOhms::LJR100mV:
				s = QString("100 mV");
				break;
			case NicerOhms::LJR10mV:
				s = QString("10 mV");
				break;
			default:
				break;
			}
			out = s;
			break;
		}
		default:
			break;
		}
	}
	else if(role == Qt::EditRole)
	{
		switch(index.column()) {
		case 0:
//			out = (d_channelConfigList.at(index.row()).enabled ? Qt::Checked : Qt::Unchecked);
			break;
		case 1:
			out = d_channelConfigList.at(index.row()).name;
			break;
		case 2:
		{
			int i = 0;
			switch (d_channelConfigList.at(index.row()).range) {
			case NicerOhms::LJR10V:
				i = 0;
				break;
			case NicerOhms::LJR1V:
				i = 1;
				break;
			case NicerOhms::LJR100mV:
				i = 2;
				break;
			case NicerOhms::LJR10mV:
				i = 3;
				break;
			default:
				break;
			}
			out = i;
			break;
		}
		}
	}

	return out;
}

bool IOBoardAnalogConfigModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(index.row() < 0 || index.row() >= d_channelConfigList.size() || index.column() < 0 || index.column() > 2)
		return false;

	if(role == Qt::CheckStateRole)
	{
		if(index.column() == 0)
		{
			d_channelConfigList[index.row()].enabled = value.toBool();
			emit dataChanged(index,index);
			return true;
		}
	}



	bool out = true;
	switch(index.column()) {
	case 1:
		d_channelConfigList[index.row()].name = value.toString();
		emit dataChanged(index,index);
		break;
	case 2:
		NicerOhms::LabJackRange r;
		switch(value.toInt()) {
		case 0:
			r = NicerOhms::LJR10V;
			break;
		case 1:
			r = NicerOhms::LJR1V;
			break;
		case 2:
			r = NicerOhms::LJR100mV;
			break;
		case 3:
			r = NicerOhms::LJR10mV;
			break;
		default:
			out = false;
			break;
		}
		if(out)
		{
			d_channelConfigList[index.row()].range = r;
			emit dataChanged(index,index);
		}
	}
	return out;
}

QVariant IOBoardAnalogConfigModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QVariant out;
	if(role != Qt::DisplayRole)
		return out;

	if(orientation == Qt::Horizontal)
	{
		switch (section) {
		case 0:
			out = QString("On");
			break;
		case 1:
			out = QString("Name");
			break;
		case 2:
			out = QString("Range");
			break;
		}
	}
	else
	{
		if(section >= 0 && section < d_channelConfigList.size())
			out = QString("AIN%1").arg(section+1);
	}

	return out;
}

Qt::ItemFlags IOBoardAnalogConfigModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags out = Qt::NoItemFlags;
	switch(index.column()) {
	case 0:
		out = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
		break;
	case 1:
		out = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		break;
	case 2:
		out = Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		break;
	default:
		break;
	}

	return out;
}

RangeDelegate::RangeDelegate(QObject *parent) :
	QStyledItemDelegate(parent)
{
}

RangeDelegate::~RangeDelegate()
{
}

QWidget *RangeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(index)
	Q_UNUSED(option)

	QComboBox *editor = new QComboBox(parent);

	editor->addItem(QString("10 V"));
	editor->addItem(QString("1 V"));
	editor->addItem(QString("100 mV"));
	editor->addItem(QString("10 mV"));

	return editor;
}

void RangeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	static_cast<QComboBox*>(editor)->setCurrentIndex(index.model()->data(index,Qt::EditRole).toInt());
}

void RangeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	model->setData(index,static_cast<QComboBox*>(editor)->currentIndex(),Qt::EditRole);
}

void RangeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(index)
	editor->setGeometry(option.rect);
}
