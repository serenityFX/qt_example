#include "TimeTableWidget.h"

#include <QtGlobal>
#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QWidget>
#include <QDialog>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QScopedPointer>

#include <translation.h>

#include "Route.h"
#include "TimeCondition.h"
#include "RouteEditorWidget.h"

#include "src.gen/ui_TimeTableWidget.h"

// FIXME: tmp
#include <util.h>


namespace
{

enum
{
//	ROUTE_LIST_TABLE_COLUMN_ROUTE_IS_ACTIVE,
	ROUTE_LIST_TABLE_COLUMN_TIME,
	ROUTE_LIST_TABLE_COLUMN_TRAIN_NUMBER,
	ROUTE_LIST_TABLE_COLUMN_DESCRIPTION,
	ROUTE_LIST_TABLE_COLUMN_ROUTE,
};


DECLARE_TR_CONTEXT(TimeTableWidgetTr)

} // namespace


namespace TimeTableEditor
{

///////////////////////////////////////////////////////////////////////////////
////////// TimeTableWidget::Impl //////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class TimeTableWidget::Impl
{
public:
	Impl(TimeTableWidget *obj);

	void init(RouteStorage *routeStorage);
	void insertRouteRow(Route::ConstPtr route, int row);
	void clear();

	int currentRouteIndex() const;

	void insertRoute(Route::Ptr route, int index = -1);
	void removeRoute(int index);

	void addRoute();
	void cloneRoute(int index);
	void editRoute(int index);

	QString readableTimeCondition(Route::ConstPtr route) const;
	QString readableRouteElementList(Route::ConstPtr route) const;

private:
	Route::Ptr doEditRoute(int index, const QString &routeEditorTitle) const;

private:
	TimeTableWidget *_obj;

public:
	Ui::TimeTableWidgetUI _ui;

	RouteStorage *_routeStorage;
};


TimeTableWidget::Impl::Impl(TimeTableWidget *obj)
	: _obj(obj)
	, _routeStorage(NULL)
{
	Q_ASSERT(NULL != _obj);

	_ui.setupUi(_obj);

	QHeaderView *routeTableHorizontalHeader = _ui.routeListTable->horizontalHeader();
	Q_ASSERT(NULL != routeTableHorizontalHeader);
	routeTableHorizontalHeader->setClickable(false);
	routeTableHorizontalHeader->setResizeMode(ROUTE_LIST_TABLE_COLUMN_TIME, QHeaderView::Interactive);
	routeTableHorizontalHeader->setResizeMode(ROUTE_LIST_TABLE_COLUMN_TRAIN_NUMBER, QHeaderView::Interactive);
	routeTableHorizontalHeader->setResizeMode(ROUTE_LIST_TABLE_COLUMN_DESCRIPTION, QHeaderView::Interactive);
	routeTableHorizontalHeader->setResizeMode(ROUTE_LIST_TABLE_COLUMN_ROUTE, QHeaderView::Stretch);
// FIXME: tmp
routeTableHorizontalHeader->resizeSection(ROUTE_LIST_TABLE_COLUMN_TIME, 200);
routeTableHorizontalHeader->resizeSection(ROUTE_LIST_TABLE_COLUMN_TRAIN_NUMBER, 100);
routeTableHorizontalHeader->resizeSection(ROUTE_LIST_TABLE_COLUMN_DESCRIPTION, 200);

	QObject::connect(_ui.routeListTable, SIGNAL(itemSelectionChanged()), _obj, SLOT(onCurrentRouteChanged()));
	QObject::connect(_ui.routeListTable, SIGNAL(cellDoubleClicked(int, int)), _obj, SLOT(onCellDoubleClicked(int, int)));

	QObject::connect(_ui.addRouteButton, SIGNAL(clicked()), _obj, SLOT(onAddRoute()));
	QObject::connect(_ui.cloneRouteButton, SIGNAL(clicked()), _obj, SLOT(onCloneRoute()));
	QObject::connect(_ui.removeRouteButton, SIGNAL(clicked()), _obj, SLOT(onRemoveRoute()));

	init(_routeStorage);
}

void TimeTableWidget::Impl::init(RouteStorage *routeStorage)
{
	clear();

	if (NULL == routeStorage)
	{
		return;
	}

	_routeStorage = routeStorage;

	int routeCount = _routeStorage->getRouteCount();
	for (int i = 0; i < routeCount; ++i)
	{
		Route::ConstPtr route = _routeStorage->getRoute(i);
		Q_ASSERT(!!route);

		insertRouteRow(route, i);
	}

	_ui.routeListTable->selectRow((0 < routeCount) ? 0 : -1);
}

void TimeTableWidget::Impl::insertRouteRow(Route::ConstPtr route, int row)
{
	Q_ASSERT(!!route);
	Q_ASSERT((0 <= row) && (row <= _ui.routeListTable->rowCount()));

//	QTableWidgetItem *routeIsActiveItem = new QTableWidgetItem();
//	routeIsActiveItem->setCheckState(Qt::Checked);
//routeIsActiveItem->setFlags(routeIsActiveItem->flags() & ~Qt::ItemIsSelectable);

	QTableWidgetItem *timeItem = new QTableWidgetItem();
	timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
	timeItem->setText(readableTimeCondition(route));
//timeItem->setCheckState(Qt::Checked);

	QTableWidgetItem *trainNumberItem = new QTableWidgetItem();
	trainNumberItem->setFlags(trainNumberItem->flags() & ~Qt::ItemIsEditable);
	trainNumberItem->setTextAlignment(Qt::AlignCenter);
	trainNumberItem->setText(route->_trainNumber);

	QTableWidgetItem *descriptionItem = new QTableWidgetItem();
	descriptionItem->setFlags(descriptionItem->flags() & ~Qt::ItemIsEditable);
	descriptionItem->setTextAlignment(Qt::AlignCenter);
	descriptionItem->setText(route->_description);

	QTableWidgetItem *routeItem = new QTableWidgetItem();
	routeItem->setFlags(routeItem->flags() & ~Qt::ItemIsEditable);
	routeItem->setText(readableRouteElementList(route));

	_ui.routeListTable->insertRow(row);
//	_ui.routeListTable->setItem(row, ROUTE_LIST_TABLE_COLUMN_ROUTE_IS_ACTIVE, routeIsActiveItem);
	_ui.routeListTable->setItem(row, ROUTE_LIST_TABLE_COLUMN_TIME, timeItem);
	_ui.routeListTable->setItem(row, ROUTE_LIST_TABLE_COLUMN_TRAIN_NUMBER, trainNumberItem);
	_ui.routeListTable->setItem(row, ROUTE_LIST_TABLE_COLUMN_DESCRIPTION, descriptionItem);
	_ui.routeListTable->setItem(row, ROUTE_LIST_TABLE_COLUMN_ROUTE, routeItem);
}

void TimeTableWidget::Impl::clear()
{
	_routeStorage = NULL;

	_ui.routeListTable->setRowCount(0);
}

int TimeTableWidget::Impl::currentRouteIndex() const
{
	const QList<QTableWidgetItem *> selectedItemList = _ui.routeListTable->selectedItems();
	if (selectedItemList.isEmpty())
	{
		return -1;
	}

	QTableWidgetItem *item = selectedItemList.at(0);
	Q_ASSERT(NULL != item);
	Q_ASSERT(-1 != item->row());

	return item->row();
}

void TimeTableWidget::Impl::insertRoute(Route::Ptr route, int index)
{
	Q_ASSERT(!!route);
	Q_ASSERT((-1 == index) || ((0 <= index) && (index <= _ui.routeListTable->rowCount())));
	Q_ASSERT(NULL != _routeStorage);
	Q_ASSERT(_ui.routeListTable->rowCount() == _routeStorage->getRouteCount());

	int rowToInsert = (-1 == index) ? _ui.routeListTable->rowCount() : index;

	_routeStorage->insertRoute(route, rowToInsert);

	insertRouteRow(route, rowToInsert);

	_ui.routeListTable->selectRow(rowToInsert);

	emit _obj->modified();
}

void TimeTableWidget::Impl::removeRoute(int index)
{
	Q_ASSERT((0 <= index) && (index < _ui.routeListTable->rowCount()));
	Q_ASSERT(NULL != _routeStorage);
	Q_ASSERT(_ui.routeListTable->rowCount() == _routeStorage->getRouteCount());

	_routeStorage->removeRoute(index);
	_ui.routeListTable->removeRow(index);

	emit _obj->modified();
}

void TimeTableWidget::Impl::addRoute()
{
	Route::Ptr route = doEditRoute(-1, TimeTableWidgetTr::tr("Add route"));
	if (!!route)
	{
		insertRoute(route);
	}
}

void TimeTableWidget::Impl::cloneRoute(int index)
{
	Q_ASSERT((0 <= index) && (index < _ui.routeListTable->rowCount()));

	Route::Ptr route = doEditRoute(index, TimeTableWidgetTr::tr("Clone route"));
	if (!!route)
	{
		insertRoute(route);
	}

}

void TimeTableWidget::Impl::editRoute(int index)
{
	Q_ASSERT((0 <= index) && (index < _ui.routeListTable->rowCount()));

	Route::Ptr route = doEditRoute(index, TimeTableWidgetTr::tr("Route properties"));
	if (!!route)
	{
		removeRoute(index);
		insertRoute(route, index);
	}
}

QString TimeTableWidget::Impl::readableTimeCondition(Route::ConstPtr route) const
{
	Q_ASSERT(!!route);

	return TimeCondition(route->_timeCondition).readableTimeCondition();
}

QString TimeTableWidget::Impl::readableRouteElementList(Route::ConstPtr route) const
{
	Q_ASSERT(!!route);

	QStringList routeElementNameList;
	for (int i = 0; i < route->_routePartList.size(); ++i)
	{
		const Route::Part &routePart = route->_routePartList.at(i);

		BaseRouteElement::ConstPtr baseRouteElement = routePart._routeElement;
		Q_ASSERT(!!baseRouteElement);

		const QString &routeElementName = ((!baseRouteElement->_friendlyName.isEmpty()) ? baseRouteElement->_friendlyName: baseRouteElement->_name);

		routeElementNameList << routeElementName;
	}

	return routeElementNameList.join(" -> ");
}

Route::Ptr TimeTableWidget::Impl::doEditRoute(int index, const QString &routeEditorTitle) const
{
	Q_ASSERT((-1 == index) || ((0 <= index) && (index < _ui.routeListTable->rowCount())));
	Q_ASSERT(NULL != _routeStorage);
	Q_ASSERT(_ui.routeListTable->rowCount() == _routeStorage->getRouteCount());

	RouteEditorWidget::Config config(*_routeStorage, index);
	QScopedPointer<RouteEditorWidget> routeEditorWidget(new RouteEditorWidget(config, _obj));

	routeEditorWidget->setWindowTitle(routeEditorTitle);

	if (QDialog::Accepted != routeEditorWidget->exec())
	{
		return Route::Ptr();
	}

	Route::Ptr route = routeEditorWidget->getRoute();
	Q_ASSERT(!!route);

	return route;
}


///////////////////////////////////////////////////////////////////////////////
////////// TimeTableWidget ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TimeTableWidget::TimeTableWidget(QWidget *parent, Qt::WindowFlags flags)
	: QWidget(parent, flags)
	, _impl(new Impl(this))
{
}

TimeTableWidget::~TimeTableWidget()
{
}

void TimeTableWidget::init(RouteStorage &routeStorage)
{
	_impl->init(&routeStorage);
}

void TimeTableWidget::onCurrentRouteChanged()
{
	bool hasSelectedRoute = (-1 != _impl->currentRouteIndex());

	_impl->_ui.cloneRouteButton->setEnabled(hasSelectedRoute);
	_impl->_ui.removeRouteButton->setEnabled(hasSelectedRoute);
}

void TimeTableWidget::onCellDoubleClicked(int row, int column)
{
	Q_ASSERT((0 <= row) && (row < _impl->_ui.routeListTable->rowCount()));
	Q_UNUSED(column);

	_impl->editRoute(row);
}

void TimeTableWidget::onAddRoute()
{
	_impl->addRoute();
}

void TimeTableWidget::onCloneRoute()
{
	int index = _impl->currentRouteIndex();
	Q_ASSERT((0 <= index) && (index < _impl->_ui.routeListTable->rowCount()));

	_impl->cloneRoute(index);
}

void TimeTableWidget::onRemoveRoute()
{
	int index = _impl->currentRouteIndex();
	Q_ASSERT((0 <= index) && (index < _impl->_ui.routeListTable->rowCount()));

	_impl->removeRoute(index);
}

void TimeTableWidget::onRouteEdit()
{
	int index = _impl->currentRouteIndex();
	Q_ASSERT((0 <= index) && (index < _impl->_ui.routeListTable->rowCount()));

	_impl->editRoute(index);
}

} // namespace TimeTableEditor
