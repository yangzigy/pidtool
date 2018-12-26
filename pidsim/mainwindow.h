#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QtCharts>
#include <QtWidgets/QGraphicsView>
#include <QtCharts/QChartGlobal>
#include <QtCharts/QAbstractAxis>
#include "qmessagebox.h"
#include "json.h"

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
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
