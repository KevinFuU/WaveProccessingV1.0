#include "mainwindow.h"

#include "FileRW.h"
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QStatusBar>
#include <QScrollBar>
#include <QLabel>
#include <QDebug>
#include <utility>
#include "PaintParam.h"
#include "PaintWidget.h"
#include "FkWindow.h"

using namespace kvl;

float Interval::MaxCentralWindowHeight = 0.0;
float Interval::MaxCentralWindowWidth = 0.0;

static SeisData seisdata;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), scroll_(new QScrollArea), paintWidget_(nullptr)
{
	ui.setupUi(this);
	initWindow();
}

void MainWindow::initWindow()
{
	this->setWindowState(Qt::WindowMaximized);
	//background color.
	setAutoFillBackground(true);
	QPalette palette;
	palette.setColor(QPalette::Background, Qt::white);
	setPalette(palette);

	createMenus();
	createToolBars();
	createStatusBar();
}

void MainWindow::createMenus()
{
	/*菜单栏选项*/
	auto openAction = new QAction(QIcon(":/MainWindow/file-open-save"), tr("&Open"), this);
	openAction->setShortcut(QKeySequence::Open);
	openAction->setStatusTip(tr("Open an existing file"));

	auto traceEqualAction = new QAction(QIcon(":/MainWindow/png-process"),
		tr("Trace Equalization"), this);
	traceEqualAction->setStatusTip(tr("Trace Equalization"));

	auto fkFilterAction = new QAction(QIcon(":/MainWindow/png-process"),
		tr("F-K Filtering"), this);
	fkFilterAction->setStatusTip((tr("F-K Filtering")));

	QMenu *file = menuBar()->addMenu(tr("File"));
	file->addAction(openAction);

	QMenu *process = menuBar()->addMenu(tr("Process"));
	process->addAction(traceEqualAction);
	process->addAction(fkFilterAction);

	connect(openAction, &QAction::triggered, this, &MainWindow::openCurrentFile);
	connect(traceEqualAction, &QAction::triggered, this, &MainWindow::traceEqualization);
	connect(fkFilterAction, &QAction::triggered, this, &MainWindow::showFkWidget);
}

void MainWindow::createToolBars()
{
	auto xZoomOutAction = new QAction(QIcon(":/MainWindow/xZoomOut"),
		QString::fromLocal8Bit("横向扩大"), this);
	ui.mainToolBar->addAction(xZoomOutAction);
	connect(xZoomOutAction, &QAction::triggered, this, &MainWindow::xZoomOutUpdate);

	auto xZoomInAction = new QAction(QIcon(":/MainWindow/xZoomIn"),
		QString::fromLocal8Bit("横向缩小"), this);
	ui.mainToolBar->addAction(xZoomInAction);
	connect(xZoomInAction, &QAction::triggered, this, &MainWindow::xZoomInUpdate);

	auto yZoomOutAction = new QAction(QIcon(":/MainWindow/yZoomOut"),
		QString::fromLocal8Bit("纵向扩大"), this);
	ui.mainToolBar->addAction(yZoomOutAction);
	connect(yZoomOutAction, &QAction::triggered, this, &MainWindow::yZoomOutUpdate);

	auto yZoomInAction = new QAction(QIcon(":/MainWindow/yZoomIn"),
		QString::fromLocal8Bit("纵向缩小"), this);
	ui.mainToolBar->addAction(yZoomInAction);
	connect(yZoomInAction, &QAction::triggered, this, &MainWindow::yZoomInUpdate);

	auto ampZoomOutAction = new QAction(QIcon(":/MainWindow/aZoomOut"),
		QString::fromLocal8Bit("振幅放大"), this);
	ui.mainToolBar->addAction(ampZoomOutAction);
	connect(ampZoomOutAction, &QAction::triggered, this, &MainWindow::ampZoomOutUpdate);

	auto ampZoomInAction = new QAction(QIcon(":/MainWindow/aZoomIn"),
		QString::fromLocal8Bit("振幅缩小"), this);
	ui.mainToolBar->addAction(ampZoomInAction);
	connect(ampZoomInAction, &QAction::triggered, this, &MainWindow::ampZoomInUpdate);

	fileToolBar_ = addToolBar(tr("File"));
	auto nextFileAction = new QAction(QIcon(":/MainWindow/nextFile"),
		QString::fromLocal8Bit("下一个文件"), this);
	fileToolBar_->addAction(nextFileAction);
	connect(nextFileAction, &QAction::triggered, this, &MainWindow::nextSeisFile);

	auto preFileAction = new QAction(QIcon(":/MainWindow/preFile"),
		QString::fromLocal8Bit("上一个文件"), this);
	fileToolBar_->addAction(preFileAction);
	connect(preFileAction, &QAction::triggered, this, &MainWindow::preSeisFile);

	ui.mainToolBar->setVisible(false);
	fileToolBar_->setVisible(false);
}

void MainWindow::createStatusBar()
{
	filenameLabel_ = new QLabel("None");
	filenameLabel_->setAlignment(Qt::AlignCenter);
	filenameLabel_->setMinimumSize(filenameLabel_->sizeHint());
	filenameLabel_->setContentsMargins(QMargins(100, 2, 100, 2));

	seisRecordLabel_ = new QLabel;
	seisRecordLabel_->setIndent(2);
	seisRecordLabel_->setAlignment(Qt::AlignCenter);

	sFileInfoLabel_ = new QLabel;
	sFileInfoLabel_->setAlignment(Qt::AlignCenter);
	sFileInfoLabel_->setMinimumSize(sFileInfoLabel_->sizeHint());
	sFileInfoLabel_->setContentsMargins(QMargins(25, 2, 25, 2));

	statusBar()->addWidget(sFileInfoLabel_);
	statusBar()->addWidget(seisRecordLabel_, 1);
	statusBar()->addWidget(filenameLabel_);
}

void MainWindow::showFkWidget()
{
	FKWindow *fkWindow = new FKWindow(new QWidget, seisdata);
	fkWindow->show();
}


void MainWindow::nextSeisFile()
{
	updateCurrentFile(FileDirection::Next);
	drawCurrentFile();
}

void MainWindow::preSeisFile()
{
	updateCurrentFile(FileDirection::Back);
	drawCurrentFile();
}

void MainWindow::updateCurrentFile(FileDirection direction)
{
	auto currentFileIndex = fileNames_.indexOf(currentFileName_);
	auto n = fileNames_.size();
	if (direction == FileDirection::Next)
	{
		if (currentFileIndex < n - 1 && currentFileIndex >= 0) ++currentFileIndex;
		else if (currentFileIndex == n - 1) currentFileIndex = 0;
		else
		{
			throw std::logic_error("invalid file name.");
		}
	}

	if (direction == FileDirection::Back)
	{
		if (currentFileIndex <= n - 1 && currentFileIndex > 0) --currentFileIndex;
		else if (currentFileIndex == 0) currentFileIndex = n - 1;
		else
		{
			throw std::logic_error("invalid file name.");
		}
	}

	currentFileName_ = fileNames_[currentFileIndex];
}

void MainWindow::openCurrentFile()
{
	fileNames_ = QFileDialog::getOpenFileNames(this,
		tr("Open File"), ".", tr("All (*.sg2 *.SG2 *.sga *.zxq *.cdp *.dat);; \
			Seg (*.sg2, *.SG2);; Sega (*.sga);; Zxq (*.cdp *.zxq)"));
	if (fileNames_.empty())
	{
		QMessageBox::warning(this, tr("Warning"),
			tr("You didn't select a file"));
		return;
	}

	currentFileName_ = fileNames_[0];

	drawCurrentFile();

	ui.mainToolBar->setVisible(true);
	fileToolBar_->setVisible(true);
}

void MainWindow::drawCurrentFile()
{
	FileRW fr;
	if (!currentFileName_.isEmpty()) //如果文件打开成功
	{
		if (!fr.readSeisFile(currentFileName_))  //如果读取文件失败
		{
			QMessageBox::warning(this, tr("Read seismic file"),
				tr("读取文件:%1失败!").arg(currentFileName_));
			return;
		}
		else
		{
			scroll_->setAlignment(Qt::AlignHCenter);
			setCentralWidget(scroll_);

			seisdata = fr.getData();
			auto helper = std::make_shared<Helper>(seisdata);
			paintWidget_ = new PaintWidget(helper, this);
			scroll_->setWidget(paintWidget_);

			//set statusbar.
			QFileInfo fi = QFileInfo(currentFileName_);
			filenameLabel_->setText(fi.fileName());

			SubInfo subInfo = paintWidget_->getSubInfo();
			sFileInfoLabel_->setText(QString::fromLocal8Bit("道数:%1     采样点数:%2     采样间隔:%3ms")
				.arg(QString::number(subInfo.n), QString::number(subInfo.m),
					QString("%1").arg(subInfo.dt * 1000, 0, 'f', 2)));
		}
	}
}

void MainWindow::traceEqualization()
{
	paintWidget_->setPaintStatus(DataStatus::balance);
	paintWidget_->update();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
	if (paintWidget_ != nullptr)
	{
		auto scrollVerValue = scroll_->verticalScrollBar()->value();

		//bound of seismic image.
		auto minHeight = scroll_->pos().y() + Interval::YShift;
		auto minWidth = scroll_->widget()->pos().x() + Interval::XShift;
		auto maxHeight = scroll_->pos().y() + paintWidget_->height() - Interval::YShift;
		auto maxWidth = scroll_->widget()->pos().x() + paintWidget_->width() - Interval::XShift;

		auto subInfo = paintWidget_->getSubInfo();

		if (event->x() > minWidth && event->x() < maxWidth &&
			event->y() > minHeight && event->y() < maxHeight)
		{
			float xDistance = event->x() - minWidth;
			auto xInterval = PaintWidget::xInterval();
			int x = 0;

			//get the number of trace 
			if (xDistance < xInterval * 1.5)
				x = 1;
			else if (xDistance < xInterval * (subInfo.n + 0.5))
				x = (xDistance - 0.5 * xInterval) / xInterval + 1;
			else
				x = subInfo.n;

			//get sample time t.
			float yDistance = event->y() - minHeight + scrollVerValue;
			auto yRatio = PaintWidget::yRatio();
			float y = yDistance / yRatio;

			seisRecordLabel_->setText(QString("n = %1    t = %2ms")
				.arg(QString::number(x), QString("%1").arg(y, 0, 'f', 1)));
		}
	}
}

MainWindow::~MainWindow()
{
}

//控制缩放
void MainWindow::xZoomOutUpdate()
{
	paintWidget_->xZoomOutUpdate();
}

void MainWindow::xZoomInUpdate()
{
	paintWidget_->xZoomInUpdate();
}

void MainWindow::yZoomOutUpdate()
{
	paintWidget_->yZoomOutUpdate();
}

void MainWindow::yZoomInUpdate()
{
	paintWidget_->yZoomInUpdate();
}

void MainWindow::ampZoomOutUpdate()
{
	paintWidget_->ampZoomOutUpdate();
}

void MainWindow::ampZoomInUpdate()
{
	paintWidget_->ampZoomInUpdate();
}
//
