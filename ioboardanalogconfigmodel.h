#ifndef IOBOARDANALOGCONFIGMODEL_H
#define IOBOARDANALOGCONFIGMODEL_H

#include <QAbstractTableModel>
#include <QList>

#include "datastructs.h"

class IOBoardAnalogConfigModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	IOBoardAnalogConfigModel(QObject *parent = nullptr);
	~IOBoardAnalogConfigModel();

	struct AnalogChannelConfig {
		bool enabled;
		QString name;
		NicerOhms::LabJackRange range;
	};

	void saveToSettings();

private:
	QList<AnalogChannelConfig> d_channelConfigList;

	// QAbstractItemModel interface
public:
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
};

#include <QStyledItemDelegate>

class RangeDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	RangeDelegate(QObject *parent = nullptr);
	~RangeDelegate();

	// QAbstractItemDelegate interface
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
	void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // IOBOARDANALOGCONFIGMODEL_H
