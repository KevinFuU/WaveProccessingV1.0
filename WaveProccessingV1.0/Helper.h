#pragma once

#ifndef HELPER_H_
#define HELPER_H_
#include "SeisData.h"
#include <QSize>
#include <QObject>

class QPainter;
class QPaintEvent;

//handle paint status(origin, balance).
enum class DataStatus { origin, balance };

//This class is designed for helping paint seismic data.
//It shouldn't be derived and copyable.
class Helper final : public QObject
{
	Q_OBJECT
public:
	Helper(const kvl::SeisData &);
	Helper(const Helper&) = delete;
	Helper& operator=(const Helper&) = delete;

	void paintOrigData(QPainter *painter, QPaintEvent *event);
	void paintTraceEqualization(QPainter *painter, QPaintEvent *event);

	QSize getPaintSize();
	kvl::SubInfo getSubInfo() { return sub_; }
	static float getXInterval() { return X_INTERVAL; }
	static float getYRatio() { return Y_RATIO; }

	//Zoom Button
	void xZoomOut() 
	{ 
		X_INTERVAL *= 1.5; 
//		psize_ = paintWidgetSize();
	}
	void yZoomOut()
	{ 
		Y_RATIO *= 2; 
//		psize_ = paintWidgetSize();
	}
	void xZoomIn() 
	{ 
		X_INTERVAL /= 1.5; 
//		psize_ = paintWidgetSize();
	}
	void yZoomIn() 
	{
		Y_RATIO /= 2.0F;
//		psize_ = paintWidgetSize();
	}
	void ampZoomOut() 
	{ 
		CIF *= 1.2F;
//		psize_ = paintWidgetSize();
	}
	void ampZoomIn() 
	{ 
		CIF /= 1.2F;
//		psize_ = paintWidgetSize();
	}

private:
	kvl::SubInfo sub_;
	bool isBalance_;
	kvl::SeisData sdata_;

	static float X_INTERVAL;
	static float Y_RATIO;
	static float CIF;

	void paintLines(QPainter *&painter);
	void paintPolygons(QPainter *&painter);
	void paintAxis(QPainter *&painter);
	void init();
	float YLength() const;

	QSize paintWidgetSize();
};


#endif // !HELPER_H_
