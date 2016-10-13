#pragma once

#ifndef PAINTWIDGET_H_
#define PAINTWIDGET_H_

#include "Helper.h"
#include <QWidget>
#include <QMouseEvent>

class QPaintEvent;
class QLabel;

class PaintWidget : public QWidget
{
	using help_ptr = std::shared_ptr<Helper>;
	Q_OBJECT
public:
	PaintWidget(help_ptr helper, QWidget *parent);
	PaintWidget(const PaintWidget&) = delete;
	PaintWidget& operator=(const PaintWidget&) = delete;

	void xZoomOutUpdate() 
	{ 
		helper_->xZoomOut(); currentSize(); resize(paintSize_); 
	}
	void xZoomInUpdate()  
	{ 
		helper_->xZoomIn(); currentSize(); resize(paintSize_); 
	}
	void yZoomOutUpdate() 
	{
		helper_->yZoomOut(); currentSize(); resize(paintSize_);
	}
	void yZoomInUpdate() 
	{
		helper_->yZoomIn(); currentSize(); resize(paintSize_);
	}
	void ampZoomOutUpdate() 
	{
		helper_->ampZoomOut(); update();
	}
	void ampZoomInUpdate() 
	{
		helper_->ampZoomIn(); update();
	}

	void setPaintStatus(DataStatus ds) { flag_ = ds; }
	kvl::SubInfo getSubInfo() { return helper_->getSubInfo(); }

	static float xInterval() { return Helper::getXInterval(); }
	static float yRatio() { return Helper::getYRatio(); }
protected:
	void paintEvent(QPaintEvent *event);

private:
	help_ptr helper_;
	QSize paintSize_;
	DataStatus flag_;

	void init();
	void currentSize();
};
#endif // !PAINTWIDGET_H_


