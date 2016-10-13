#include "PaintWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QLabel>
#include <QDebug>
#include <QPoint>
PaintWidget::PaintWidget(help_ptr helper, QWidget *parent) :
	helper_(helper), QWidget(parent), flag_(DataStatus::origin)
{
	init();
}

void PaintWidget::init()
{
	//initialize size.
	currentSize();
	resize(paintSize_);

	//set background.
	setAutoFillBackground(true);
	QPalette palette;
	palette.setColor(QPalette::Background, Qt::white);
	setPalette(palette);
}

void PaintWidget::currentSize()
{
	auto size = helper_->getPaintSize();
	paintSize_.setWidth(size.width());
	paintSize_.setHeight(size.height());
}

void PaintWidget::paintEvent(QPaintEvent *event)
{
	if (helper_ != nullptr)
	{
		QPainter painter;
		painter.begin(this);
		switch (flag_)
		{
		case DataStatus::origin:
			helper_->paintOrigData(&painter, event);
			break;
		case DataStatus::balance:
			helper_->paintTraceEqualization(&painter, event);
			break;
		default:
			break;
		}
		painter.end();
	}
}
