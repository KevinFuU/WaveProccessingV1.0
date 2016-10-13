#include "Helper.h"

#include <QtCore/QVector>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QDebug>
#include <algorithm>
#include "PaintParam.h"

//#define LIMIT_DATA_SHAPE

float Helper::X_INTERVAL = 15;
float Helper::Y_RATIO = 0.75;
float Helper::CIF = 5.0;

using namespace kvl;

Helper::Helper(const SeisData& seisData) :
	sdata_(seisData)
{
	init();
}

QSize Helper::getPaintSize() 
{
	return paintWidgetSize();
}

QSize Helper::paintWidgetSize()
{
	return QSize((sub_.n + 1) * X_INTERVAL + Interval::XShift * 2, YLength() + Interval::YShift);
}

float Helper::YLength() const 
{ 
	return sub_.m * sub_.dt * 1000 * Y_RATIO + Interval::YShift; 
}

void Helper::init()
{
	sub_.m = sdata_.getM();
	sub_.n = sdata_.getN();
	sub_.dt = sdata_.getDt();
	sub_.dx = sdata_.getDx();
	isBalance_ = false;
}

void Helper::paintOrigData(QPainter* painter, QPaintEvent* event)
{
	painter->setPen(QPen(Qt::black, 1.0));
	paintAxis(painter);
	paintLines(painter);
	paintPolygons(painter);
}

void Helper::paintTraceEqualization(QPainter *painter, QPaintEvent *event)
{
	//if TraceEqual has done.It shouldn't be procedded anymore.
	if(!isBalance_)
	{
		auto sdataPtr = sdata_.getSeisDataPtr();
		auto smaxColsPtr = sdata_.getMaxColsPtr();
		auto smax = sdata_.getMaxData();
		for (unsigned int i = 0; i < sub_.n; ++i)
		{
			for (unsigned int j = 0; j < sub_.m; ++j)
			{
				(*sdataPtr)(j, i) = (*smaxColsPtr)[i] != 0 ?  
					(*sdataPtr)(j, i) / ((*smaxColsPtr)[i] / smax) : 0;
			}
		}

		isBalance_ = true;
	}

	paintOrigData(painter, event);
}

void Helper::paintLines(QPainter *&painter)
{
	painter->save();
	painter->setPen(QPen(Qt::black, 1.0));
	painter->setRenderHint(QPainter::Antialiasing);
	auto sdataMax = sdata_.getMaxData();
	auto sRecordPtr = sdata_.getSeisDataPtr();
	for (unsigned int i = 0; i < sub_.n; ++i)
	{
		auto l = i + 1;
		QVector<QPointF> linePoints(sub_.m);

		for (unsigned int j = 0; j < sub_.m; ++j)
		{
			double sRecord = (*sRecordPtr)(j, i) * CIF;

			#ifdef LIMIT_DATA_SHAPE
			if (fabs(sRecord) > sdataMax)
				sRecord = 
				fabs(sRecord) / sRecord * sdataMax;
			#endif

			linePoints[j].setX(l * X_INTERVAL + Interval::XShift + X_INTERVAL * sRecord * Interval::TraceLimit / sdataMax);
			linePoints[j].setY(j * sub_.dt * 1000 * Y_RATIO + Interval::YShift);
			//~[1]
		}
		painter->drawPolyline(linePoints);
	}
	painter->restore();
}

void Helper::paintPolygons(QPainter *&painter)
{
	auto sRecordPtr = sdata_.getSeisDataPtr();
	auto sdataMax = sdata_.getMaxData();
	double sRecord;
	for (unsigned int i = 0; i < sdata_.getN(); ++i)
	{
		QVector<float> xsf(sub_.m);  //x坐标集
		QVector<float> ysf(sub_.m);  //y坐标集
		double smax;
		unsigned int k, l = i + 1;
		for (unsigned int j = 0; j < sub_.m; ++j)
		{
			int ns = -1;
			if ((*sRecordPtr)(j, i) > 0)
			{
				ns++;
				xsf[ns] = (float)(l * X_INTERVAL);
				ysf[ns] = (float)(j * sub_.dt * 1000.0 * Y_RATIO);
				smax = 0.0;
				for (k = j; k < sub_.m; ++k)
				{
					if ((*sRecordPtr)(k, i) > 0)
					{
						ns++;
						if (k == sub_.m - 1)
						{
							xsf[ns] = (float)(l * X_INTERVAL);
							ysf[ns] = (float)(k * sub_.dt * 1000.0 * Y_RATIO);
							ns++;
						}
						else
						{
							
							sRecord = (*sRecordPtr)(k, i) * CIF;
							#ifdef LIMIT_DATA_SHAPE
							if (fabs(sRecord) > sdataMax)
								sRecord = fabs(sRecord) / sRecord * sdataMax;
							#endif
							xsf[ns] = (float)(l * X_INTERVAL + X_INTERVAL * sRecord * Interval::TraceLimit / sdataMax);
							ysf[ns] = (float)(k * sub_.dt * 1000.0 * Y_RATIO);
							if (smax < sRecord) smax = sRecord;
						}
					}
					else
					{
						ns++;
						xsf[ns] = (float)(l * X_INTERVAL);
						ysf[ns] = (float)(k * sub_.dt * 1000.0 * Y_RATIO);
						ns++;
						break;
					}
				}
				j = k;
				if (ns > 3 && 4.0 * smax / sdataMax > 0.5)
				{
					QVector<QPointF> gonPoints(ns);
					for (k = 0; k < ns; ++k)
					{
						gonPoints[k].setX(xsf[k] + Interval::XShift);
						gonPoints[k].setY(ysf[k] + Interval::YShift);
					}
					painter->setBrush(Qt::black);
					painter->drawPolygon(gonPoints);
				}
			}
		}
	}
}

void Helper::paintAxis(QPainter *&painter)
{
	painter->save();
	painter->setPen(QPen(Qt::black, 1.0));

	#pragma region 绘制框线
	QPointF start(Interval::XShift, Interval::YShift);
	QPointF xEnd(X_INTERVAL * (sub_.n + 1) + Interval::XShift, Interval::YShift);
	QPointF yEnd(Interval::XShift, YLength());
	QPointF end(X_INTERVAL * (sub_.n + 1) + Interval::XShift, YLength());
	painter->drawLine(start, xEnd);
	painter->drawLine(start, yEnd);
	painter->drawLine(yEnd, end);
	painter->drawLine(xEnd, end);
	#pragma endregion

	//set font.
	QFont font = painter->font();
	font.setPointSize(24);
	font.setFamily("Times");

	#pragma region x-axis
	int xmaxNum = QString::number(sub_.n * sub_.dx).size();
	for(unsigned int i = 0; i < sub_.n;)
	{
		auto str = QString::number((i + 1) * sub_.dx);
		auto width = std::max((float)xmaxNum * 10, X_INTERVAL);
		QRect xtextRect = QRect(X_INTERVAL * (i + 1) + Interval::XShift - width / 2, 26, width, font.pointSize());
		painter->drawText(xtextRect, Qt::AlignCenter, str);

		//handle x-axis coordinate interval.
		auto gap = (width + X_INTERVAL - 1) / X_INTERVAL;
		i += gap;
	}

	#pragma endregion

	//dashline.
	QPen pen = QPen(Qt::DashLine);
	painter->setPen(pen);

	#pragma region y-axis
	auto ymaxNum = QString("%1").arg(sub_.m * sub_.dt * 1000, 0, 'f', 1).size();
	for (auto j = Interval::YAxisInterval / Y_RATIO; j < sub_.m * sub_.dt * 1000;)
	{
		auto width = static_cast<float>(ymaxNum * 8);
		QRect ytextRect = QRect(Interval::XShift - width - 2, j * Y_RATIO + Interval::YShift - font.pointSize() / 2, width, font.pointSize());
		auto yAxisCoor = QString("%1").arg(j, 0, 'f', 1);
		painter->drawText(ytextRect, Qt::AlignVCenter | Qt::AlignRight, yAxisCoor);

		QPointF yAxisStart = QPointF(Interval::XShift, j * Y_RATIO + Interval::YShift);
		QPointF yAxisEnd = QPointF(X_INTERVAL * (sub_.n + 1) + Interval::XShift, j * Y_RATIO + Interval::YShift);
		painter->drawLine(yAxisStart, yAxisEnd);
		
		//fixed distance of interval.
		j += Interval::YAxisInterval / Y_RATIO;
	}

	#pragma endregion

	painter->restore();
}