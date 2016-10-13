#include "FkWindow.h"
#include <type_traits>
#include <QColor>
#include <QPainter>
#include <QScrollArea>
#include <QAction>
#include <QToolBar>
#include <cmath>
using namespace kvl;
using namespace arma;

FKWindow::FKWindow(QWidget *parent, const kvl::SeisData& sdata) :
	QMainWindow(parent), sdata_(sdata), fkScroll_(new QScrollArea)
{
	this->setWindowState(Qt::WindowMaximized);
	setAutoFillBackground(true);
	QPalette palette;
	palette.setColor(QPalette::Background, Qt::white);
	setPalette(palette);

	auto startAction = new QAction(QIcon(":/MainWindow/png-process"),
		QString::fromLocal8Bit("¿ªÊ¼"), this);
	QToolBar *start = addToolBar(tr("Start"));
	start->addAction(startAction);
	connect(startAction, &QAction::triggered, this, &FKWindow::showFkData);
}



void FKWindow::showFkData()
{
	setCentralWidget(fkScroll_);
	fkWidget_ = new FkWidget(this, sdata_);
	fkScroll_->setAlignment(Qt::AlignHCenter);
	fkScroll_->setWidget(fkWidget_);
}


