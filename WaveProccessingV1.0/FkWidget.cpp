#include "FkWidget.h"
#include <cmath>
#include <QColor>
#include <QPainter>

using namespace kvl;
using namespace arma;

using byte = unsigned char;

SubInfo sub = SubInfo();

static uword rows = 0;
static uword cols = 0;

FkWidget::FkWidget(QWidget *parent, const kvl::SeisData& sdata) :
	QWidget(parent)
{
	resize(512, 2048);

	sub = sdata.getSubInfo();
	fmat mat(*sdata.getSeisDataPtr());
	sdata_ = SeisData(std::make_shared<fmat>(mat), sub.dt, sub.dx);
	image_ = new QImage(1024, 4192, QImage::Format_RGB32);

	setAutoFillBackground(true);
	QPalette palette;
	palette.setColor(QPalette::Background, Qt::white);
	setPalette(palette);
}


void FkWidget::paintEvent(QPaintEvent *event)
{
	auto origDataPtr = sdata_.getSeisDataPtr();
	auto sdataPtr = sPaintDataPtr(origDataPtr);
	auto fkDataPtr = fft2SeisData(sdataPtr);

	drawImage(fkDataPtr);

	QPainter widgetPainter(this);
	widgetPainter.drawImage(0, 0, *image_);
}

auto FkWidget::sPaintDataPtr(fmat2_ptr &origPtr)->
std::remove_reference<decltype(origPtr)>::type
{
	fmat targ(*origPtr);
	for (int i = 0; i < (int)sub.n; ++i)
	{
		for (int j = 0; j < (int)sub.m; ++j)
		{
			//当i+j为奇数,数据取负;当i+j为偶数,取正.
			int parity = (i + j) & 1;
			int mask = ((~parity + 1) * 2 + 1); //mask = -1, if (i+j) 为奇.
			targ(j, i) = mask * targ(j, i);
		}
	}
	return std::make_shared<fmat>(targ);
}

FkWidget::cx_fmat2_ptr FkWidget::fft2SeisData(fmat2_ptr &sdataPtr)
{
	fmat zeroMat(arma::size(*sdataPtr), fill::zeros);
	cx_fmat sdata(*sdataPtr, zeroMat);
	rows = sub.m;
	cols = sub.n;
	//判断列数如果小于512,填充。否则,为最近的2幂数。
	if (cols <= 512) cols = 512;
	else
	{
		int temp = cols;
		cols = 1;
		while (temp >>= 1)
			cols <<= 1;
	}

	int temp = rows;
	rows = 1;
	while (temp >>= 1)
		rows <<= 1;

	cx_fmat fft2Data = fft2(sdata, rows, cols);
	fft2Data.save("fft2.csv", arma_ascii);
	return std::make_shared<cx_fmat>(fft2Data);
}

void FkWidget::drawImage(cx_fmat2_ptr &fkDataPtr)
{
	fmat ampl(rows, cols);

	for (uword i = 0; i < cols; ++i)
	{
		for (uword j = 0; j < rows; ++j)
		{
			ampl(j, i) = sqrt((*fkDataPtr)(j, i).real() * (*fkDataPtr)(j, i).real()
				+ (*fkDataPtr)(j, i).imag() * (*fkDataPtr)(j, i).imag());
		}
	}

	std::vector<float> maxAmpls(cols);
	for (uword i = 0; i < cols; ++i)
		maxAmpls[i] = ampl.row(i).max();

	for (uword i = 0; i < cols; i++)
	{
		for (uword j = 0; j < rows; j++)
		{
			ampl(j, i) /= maxAmpls[i];
		}
	}

	ampl.save("ampl.csv", arma_ascii);

	for (uword i = 0; i < cols; ++i)
	{
		for (uword j = 0; j < rows; ++j)
		{
			auto v = ampl(j, i);
			if (v > 1.0) v = 1.0;
			int red = (int)((1 - v) * 255);
			int blue = (int)((1 - v) * 255);
			int green = int((1 - v) * 255);

			image_->setPixel(i, j, qRgb(red, green, blue));
		}
	}
}

QRgb FkWidget::getRgbColor(float v, float vmin, float vmax)
{
	float r = 1.0, g = 1.0, b = 1.0;

	float dv;
	if (v < vmin) v = vmin;
	if (v > vmax) v = vmax;

	dv = vmax - vmin;

	if (v < (vmin + 0.25 * dv))
	{
		r = 0;
		g = 4 * (v - vmin) / dv;
	}
	else if (v < (vmin + 0.5 * dv))
	{
		r = 0;
		g = 1 + 4 * (vmin + 0.25 * dv - v) / dv;
	}
	else if (v < (vmin + 0.75 * dv)) {
		r = 4 * (v - vmin - 0.5 * dv) / dv;
		b = 0;
	}
	else {
		g = 1 + 4 * (vmin + 0.75 * dv - v) / dv;
		b = 0;
	}

	return qRgb(r * 255, g * 255, b * 255);
}