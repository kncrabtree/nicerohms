#include "validationmodel.h"

#include <QSettings>
#include <QApplication>

ValidationModel::ValidationModel(QObject *parent) :
     QAbstractTableModel(parent)
{
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	s.beginGroup(QString("lastScanConfig"));
	int total = s.beginReadArray(QString("validation"));
	for(int i=0; i<total; i++)
	{
		s.setArrayIndex(i);
		QString key = s.value(QString("key"),QString("")).toString();
		double min = s.value(QString("min"),0.0).toDouble();
		double max = s.value(QString("max"),1.0).toDouble();
		bool abort = s.value(QString("abort"),true).toBool();

		if(!key.isEmpty())
			addNewItem(key,min,max,abort);
	}
	s.endArray();
	s.endGroup();
}

ValidationModel::~ValidationModel()
{

}



int ValidationModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	
	return d_validationList.size();
}

int ValidationModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)
	
	return 4;
}

QVariant ValidationModel::data(const QModelIndex &index, int role) const
{
	QVariant out;
	if(index.row() < 0 || index.row() >= d_validationList.size())
		return out;
	
	if(role == Qt::DisplayRole)
	{
		switch (index.column()) {
		case 0:
			out = d_validationList.at(index.row()).key;
			break;
		case 1:
			out = QString::number(d_validationList.at(index.row()).min,'g');
			break;
		case 2:
			out = QString::number(d_validationList.at(index.row()).max,'g');
			break;
		case 3:
			out = (d_validationList.at(index.row()).abort ? QString("Abort") : QString("Redo"));
			break;
		default:
			break;
		}
	}
	else if(role == Qt::EditRole)
	{
		switch (index.column()) {
		case 0:
			out = d_validationList.at(index.row()).key;
			break;
		case 1:
			out = d_validationList.at(index.row()).min;
			break;
		case 2:
			out = d_validationList.at(index.row()).max;
			break;
		case 3:
			out = d_validationList.at(index.row()).abort;
			break;
		default:
			break;
		}
	}
	
	return out;
}

bool ValidationModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(role != Qt::EditRole)
		return false;
	
	if(index.row() < 0 || index.row() >= d_validationList.size())
		return false;
	
	bool success = false;
	switch(index.column()) {
	case 0:
		d_validationList[index.row()].key = value.toString();
		success = true;
		break;
	case 1:
		d_validationList[index.row()].min = value.toDouble();
		success = true;
		break;
	case 2:
		d_validationList[index.row()].max = value.toDouble();
		success = true;
		break;
	case 3:
		d_validationList[index.row()].abort = value.toBool();
		success = true;
		break;
	}
	
	if(success)
		emit dataChanged(index,index);
	
	return success;
}

QVariant ValidationModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	QVariant out;
	
	if(orientation == Qt::Horizontal)
	{
		if(role == Qt::DisplayRole)
		{
			switch (section) {
			case 0:
				out = QString("Key");
				break;
			case 1:
				out = QString("Min");
				break;
			case 2:
				out = QString("Max");
				break;
			case 3:
				out = QString("Act");
				break;
			default:
				break;
			}
		}
		else if(role == Qt::ToolTipRole)
		{
			switch (section) {
			case 0:
				out = QString("The key for the validation item.\nThis is usually the string shown on the plots, but there are some exceptions.\nFor instance, refer to analog and digital channels of IOBoard by e.g. AIN3 or DIN7.");
				break;
			case 1:
				out = QString("Minimum valid value. If the value read is lower, the specified action will be taken.");
				break;
			case 2:
				out = QString("Maximum valid value. If the value read is higher, the specified action will be taken.");
				break;
			case 3:
				out = QString("Action to be taken if measured value is invalid.\nOptions are to abort the scan or redo the point.");
				break;
			default:
				break;
			}
		}
	}
	else if(orientation == Qt::Vertical)
	{
		if(role == Qt::DisplayRole)
			out = section+1;
	}
	
	return out;
}

bool ValidationModel::removeRows(int row, int count, const QModelIndex &parent)
{
	if(row < 0 || row+count > d_validationList.size() || d_validationList.isEmpty())
	        return false;
	
	beginRemoveRows(parent,row,row+count-1);
	for(int i=0; i<count; i++)
		d_validationList.removeAt(row);
	endRemoveRows();
	
	return true;
}

Qt::ItemFlags ValidationModel::flags(const QModelIndex &index) const
{
	if(index.row() < 0 || index.row() >= d_validationList.size())
		return Qt::NoItemFlags;
	
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void ValidationModel::addNewItem(QString k, double min, double max, bool abort)
{
	NicerOhms::ValidationItem v;
	v.key = k;
	v.min = min;
	v.max = max;
	v.abort = abort;
	
	beginInsertRows(QModelIndex(),d_validationList.size(),d_validationList.size());
	d_validationList.append(v);
	endInsertRows();
}

void ValidationModel::saveToSettings()
{
	//first, remove any items with empty keys
	for(int i = d_validationList.size()-1; i >= 0; i--)
	{
		if(d_validationList.at(i).key.isEmpty())
			d_validationList.removeAt(i);
	}

	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	s.beginGroup(QString("lastScanConfig"));
	s.beginWriteArray(QString("validation"));
	for(int i=0; i<d_validationList.size();i++)
	{
		s.setArrayIndex(i);
		s.setValue(QString("key"),d_validationList.at(i).key);
		s.setValue(QString("min"),qMin(d_validationList.at(i).min,d_validationList.at(i).max));
		s.setValue(QString("max"),qMax(d_validationList.at(i).min,d_validationList.at(i).max));
		s.setValue(QString("abort"),d_validationList.at(i).abort);
	}
	s.endArray();
	s.endGroup();


}

QList<NicerOhms::ValidationItem> ValidationModel::getList()
{
	return d_validationList;
}

DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(QObject *parent) :
     QStyledItemDelegate(parent)
{
	
}

QWidget *DoubleSpinBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(option)
	Q_UNUSED(index)
	
	QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
	editor->setMinimum(-1e200);
	editor->setMaximum(1e200);
	editor->setDecimals(3);
	
	return editor;
}

void DoubleSpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	if(index.column() == 1 || index.column() == 2)
		static_cast<QDoubleSpinBox*>(editor)->setValue(index.model()->data(index,Qt::EditRole).toDouble());
		
}

void DoubleSpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	double val = static_cast<QDoubleSpinBox*>(editor)->value();
	model->setData(index,val);
}

void DoubleSpinBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	editor->setGeometry(option.rect);
	Q_UNUSED(index)
}

CompleterLineEditDelegate::CompleterLineEditDelegate(QObject *parent) :
     QStyledItemDelegate(parent)
{
	
}

QWidget *CompleterLineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	Q_UNUSED(option)
	Q_UNUSED(index)
	
	QLineEdit *le = new QLineEdit(parent);
	
	QSettings s(QSettings::SystemScope,QApplication::organizationName(),QApplication::applicationName());
	QStringList knownKeys = s.value(QString("knownValidationKeys"),QString("")).toString().split(QChar(';'),QString::SkipEmptyParts);
	if(!knownKeys.isEmpty())
	{
		QCompleter *comp = new QCompleter(knownKeys,le);
		comp->setCaseSensitivity(Qt::CaseInsensitive);
		le->setCompleter(comp);
	}

	return le;
}

void CompleterLineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	if(index.column() == 0)
		static_cast<QLineEdit*>(editor)->setText(index.model()->data(index,Qt::EditRole).toString());
}

void CompleterLineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	QString key = static_cast<QLineEdit*>(editor)->text();
	model->setData(index,key);
}

void CompleterLineEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	editor->setGeometry(option.rect);
	Q_UNUSED(index)
}


ActionComboBoxDelegate::ActionComboBoxDelegate(QObject *parent) :
     QStyledItemDelegate(parent)
{
	
}

QWidget *ActionComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QComboBox *cb = new QComboBox(parent);
	
	cb->addItem(QString("Abort"),true);
	cb->addItem(QString("Redo"),false);
	
	Q_UNUSED(option)
	Q_UNUSED(index)

	return cb;
}

void ActionComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	bool abort = index.model()->data(index,Qt::EditRole).toBool();
	QComboBox *cb = static_cast<QComboBox*>(editor);
	if(abort)
		cb->setCurrentIndex(0);
	else
		cb->setCurrentIndex(1);
}

void ActionComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	model->setData(index,static_cast<QComboBox*>(editor)->currentData());
}

void ActionComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	editor->setGeometry(option.rect);
	Q_UNUSED(index)
}
