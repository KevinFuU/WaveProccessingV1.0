#pragma once
#ifndef FKWIDGET_H_
#define FKWIDGET_H_

#include <QWidget>
#include <memory>
#include "SeisData.h"

class FkWidget : public QWidget
{
	Q_OBJECT
	using u32_t = unsigned int;
	using fmat2_ptr = std::shared_ptr<arma::fmat>;
	using cx_fmat2_ptr = std::shared_ptr<arma::cx_fmat>;
public:
	FkWidget(QWidget *parent, const kvl::SeisData& sdata);
	~FkWidget() {}

protected:
	void paintEvent(QPaintEvent *event);

private:
	kvl::SeisData sdata_;
	QImage *image_;

	fmat2_ptr sPaintDataPtr(fmat2_ptr &origPtr);
	cx_fmat2_ptr fft2SeisData(fmat2_ptr &sdataPtr);
	void drawImage(cx_fmat2_ptr &);
	QRgb getRgbColor(float v, float vmin, float vmax);
};

#endif