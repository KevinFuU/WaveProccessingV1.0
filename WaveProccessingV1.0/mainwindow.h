#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include <QMouseEvent>
#include <QScrollArea>

class PaintWidget;
class QLabel;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

protected:
	void mousePressEvent(QMouseEvent *event);
	//	void moveEvent(QMoveEvent *event);
private:
	enum class FileDirection { Next = 0, Back };

	QStringList fileNames_;
	QString currentFileName_;

	Ui::MainWindowClass ui;
	PaintWidget *paintWidget_;
	QScrollArea *scroll_;
	QToolBar *fileToolBar_;
	QLabel *filenameLabel_;
	QLabel *seisRecordLabel_;
	QLabel *sFileInfoLabel_;

	void openCurrentFile();
	void drawCurrentFile();
	void initWindow();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void traceEqualization();
	void showFkWidget();

	void xZoomOutUpdate();
	void xZoomInUpdate();
	void yZoomOutUpdate();
	void yZoomInUpdate();
	void ampZoomOutUpdate();
	void ampZoomInUpdate();


	void nextSeisFile();
	void preSeisFile();
	void updateCurrentFile(FileDirection);
};

#endif // MAINWINDOW_H
