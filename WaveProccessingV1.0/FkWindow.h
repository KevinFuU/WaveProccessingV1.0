#pragma once
#ifndef FKWINDOW_H_
#define FKWINDOW_H_

#include <QMainWindow>
#include "FkWidget.h"

class QPaintEvent;
class QScrollArea;

class FKWindow : public QMainWindow
{
	Q_OBJECT
public:
	FKWindow(QWidget *parent, const kvl::SeisData& sdata);
	~FKWindow() {}

private:
	FkWidget *fkWidget_;
	QScrollArea *fkScroll_;
	kvl::SeisData sdata_;

	void showFkData();
};

#endif // !1