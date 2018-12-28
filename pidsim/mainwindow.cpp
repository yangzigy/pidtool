#include "mainwindow.h"
#include "ui_mainwindow.h"

extern Json::Value config; //配置对象

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	chart0 = new QChart();
	QMargins tmpmarg(5,5,5,5);
	chart0->setMargins(tmpmarg);
	chartView0 = new QChartView(chart0);
//	chartView0->setRenderHint(QPainter::Antialiasing, true); //
//	chart0->setTheme(QChart::ChartThemeDark); //
//	set_theme(chart0->theme());
	//chart0->setTitle("Line chart0");
	chart_serial_0= new QLineSeries(chart0);
	chart_serial_0->setName("CH0");
	chart_serial_0->setUseOpenGL(true); //使用OpenGL加速显示
	chart0->addSeries(chart_serial_0);
	chart_serial_1= new QLineSeries(chart0);
	chart_serial_1->setName("CH1");
	chart_serial_1->setUseOpenGL(true); //使用OpenGL加速显示
	chart0->addSeries(chart_serial_1);
	chart0->createDefaultAxes();
	chart0->axisX()->setRange(0, config["记录长度"].asInt());
	chart0->axisY()->setRange(-0x3ffff/2, 0x3ffff/2);
	static_cast<QValueAxis *>(chart0->axisX())->setLabelFormat("%d  ");
	static_cast<QValueAxis *>(chart0->axisY())->setLabelFormat("%d  ");
	chartView0->setRubberBand(QChartView::RectangleRubberBand);

	ui->grid_serial->addWidget(chartView0,0,0);
	ui->grid_serial->setContentsMargins(tmpmarg);
	//connect(this,SIGNAL(signal_rx_cb()),this,SLOT(rx_cb_pro()));
}

MainWindow::~MainWindow()
{
	delete ui;
	exit(0); //在win下使用opengl加速，退出时报错，这样强制退出
}
void MainWindow::set_theme(QChart::ChartTheme theme)
{
	QPalette pal = window()->palette();
	if (theme == QChart::ChartThemeLight) {
		pal.setColor(QPalette::Window, QRgb(0xf0f0f0));//f0f0f0是白灰色
		pal.setColor(QPalette::WindowText, QRgb(0x404044));
		//![8]
	} else if (theme == QChart::ChartThemeDark) {
		pal.setColor(QPalette::Window, QRgb(0x121218));//121218是蓝黑色
		pal.setColor(QPalette::WindowText, QRgb(0xd6d6d6));//0xd6d6d6是白灰色
	} else if (theme == QChart::ChartThemeBlueCerulean) {
		pal.setColor(QPalette::Window, QRgb(0x40434a));//黑灰
		pal.setColor(QPalette::WindowText, QRgb(0xd6d6d6));//白灰
	} else if (theme == QChart::ChartThemeBrownSand) {
		pal.setColor(QPalette::Window, QRgb(0x9e8965));
		pal.setColor(QPalette::WindowText, QRgb(0x404044));//0x404044是黑灰色
	} else if (theme == QChart::ChartThemeBlueNcs) {
		pal.setColor(QPalette::Window, QRgb(0x018bba));
		pal.setColor(QPalette::WindowText, QRgb(0x404044));
	} else if (theme == QChart::ChartThemeHighContrast) {
		pal.setColor(QPalette::Window, QRgb(0xffab03));
		pal.setColor(QPalette::WindowText, QRgb(0x181818));
	} else if (theme == QChart::ChartThemeBlueIcy) {
		pal.setColor(QPalette::Window, QRgb(0xcee7f0));
		pal.setColor(QPalette::WindowText, QRgb(0x404044));
	} else {
		pal.setColor(QPalette::Window, QRgb(0xf0f0f0));
		pal.setColor(QPalette::WindowText, QRgb(0x404044));
	}
	window()->setPalette(pal);
}
void MainWindow::ui_initial(void)
{
	for(auto &it:sp_ctrl)
	{
	}
	for(auto &it:sp_md)
	{
	}
}
