#ifndef TIME_TABLE_WIDGET_H_13FABB9B_BC3C_474A_AB0A_C3320A63F19D
#define TIME_TABLE_WIDGET_H_13FABB9B_BC3C_474A_AB0A_C3320A63F19D

#include <QObject>
#include <QWidget>
#include <QScopedPointer>


namespace TimeTableEditor
{

class RouteStorage;


class TimeTableWidget : public QWidget
{
	Q_OBJECT

public:
	TimeTableWidget(QWidget *parent = NULL, Qt::WindowFlags flags = 0);
	~TimeTableWidget();

	void init(RouteStorage &routeStorage);

private slots:
	void onCurrentRouteChanged();
	void onCellDoubleClicked(int row, int column);
	void onAddRoute();
	void onCloneRoute();
	void onRemoveRoute();
	void onRouteEdit();

signals:
	void modified();

private:
	class Impl;
	QScopedPointer<Impl> _impl;

	Q_DISABLE_COPY(TimeTableWidget)
};

} // namespace TimeTableEditor

#endif // TIME_TABLE_WIDGET_H_13FABB9B_BC3C_474A_AB0A_C3320A63F19D
