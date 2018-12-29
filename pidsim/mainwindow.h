#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <qdir>
#include <QtCharts>
#include <QtWidgets/QGraphicsView>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QAbstractAxis>
#include "qmessagebox.h"
#include "common.h"
#include "json.h"
#include "appflow.h"
#include "dictdis.h"

using namespace QtCharts;

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

	void ui_initial(void);

	CDictDis *pdict=0;
	QWidget *subWidget=0;
	int is_init_ctrls=0; //算法列表是否已经完成

	QChartView *chartView0;
	QChart *chart0;
	QLineSeries *chart_serial_0;
	QLineSeries *chart_serial_1;
	QChartView *chartView1;
	QDateTimeAxis *mAxis0;
	QValueAxis *mAyis0;
	void set_theme(QChart::ChartTheme theme);
	//signals:
	//void signal_rx_cb(); //接收信号
	//public slots:
private slots:
	void on_cb_ctrlalg_currentIndexChanged(const QString &arg1);

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
