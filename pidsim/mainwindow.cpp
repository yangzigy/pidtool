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
	chart_serial_0->setName("e");
	chart_serial_0->setUseOpenGL(true); //使用OpenGL加速显示
	chart0->addSeries(chart_serial_0);
	chart_serial_1= new QLineSeries(chart0);
	chart_serial_1->setName("u");
	chart_serial_1->setUseOpenGL(true); //使用OpenGL加速显示
	chart0->addSeries(chart_serial_1);
	chart0->createDefaultAxes();
	chart0->axisX()->setRange(0, 100);
	chart0->axisY()->setRange(-100, 100);
	//static_cast<QValueAxis *>(chart0->axisX())->setLabelFormat("%d  ");
	//static_cast<QValueAxis *>(chart0->axisY())->setLabelFormat("%d  ");
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
	for(auto &it:sp_ctrl) //控制算法列表
	{
		ui->cb_ctrlalg->addItem(it.first.c_str());
	}
	for(auto &it:sp_md) //模型列表
	{
		ui->cb_model->addItem(it.first.c_str());
	}
	is_init_ctrls=1;
	ui->cb_ctrlalg->setCurrentIndex(0);
	on_cb_ctrlalg_currentIndexChanged(ui->cb_ctrlalg->currentText());
}

void MainWindow::on_cb_ctrlalg_currentIndexChanged(const QString &arg1) //选择算法
{
	if(is_init_ctrls==0) return;
	if(subWidget) //若需要重新布局
	{
		QLayoutItem *child;
		while(child=ui->groupBox_5->layout()->takeAt(0))
		{
			delete child;
		}
	//	delete subWidget;
		delete pdict;
		pdict=0;
	}
	DBGL;
	cout<<arg1.toStdString()<<endl;
	pjson(sp_ctrl[arg1.toStdString()]->cfg);
	pjson(sp_ctrl[arg1.toStdString()]->cfgdes);
	subWidget=new QWidget();

	ui->gridLayout_8->addWidget(subWidget,0,0); //加入到栅格布局的第0行第0列
	pdict=new CDictDis(sp_ctrl[arg1.toStdString()]->cfgdes,subWidget);
}
void MainWindow::on_bt_fitscreen_clicked() //适应屏幕
{
	float max=-0xffffffff,min=0xffffffff;
	int curv_len=((QLineSeries*)(chart0->series()[0]))->points().size();
	for(auto &it:chart0->series())
	{
		for(int i=0;i<(((QLineSeries*)it)->points()).size();i++)
		{
			float y=((QLineSeries*)it)->points()[i].y();
			if(y>max) max=y; if(y<min) min=y;
		}
	}
	chart0->axisX()->setRange(0,curv_len+2);
	chart0->axisY()->setRange(min,max);
}

void MainWindow::on_bt_sim_clicked() //开始仿真
{
///////////////////////////////////////////////////////////////////////////
//配置
	en_noise_norm=ui->cb_noise_norm->isChecked()?1:0;
	nnorm_mean=ui->le_nnorm_mean->text().toDouble();
	nnorm_std=ui->le_nnorm_std->text().toDouble();
	en_noise_acc=ui->cb_noise_acc->isChecked()?1:0;
	nacc_std=ui->le_nacc_std->text().toDouble();
//取得当前的仿真数据源--期望数据
	string expdatafilename=ui->le_exp_filename->text().toStdString();
	CFilePath fp_model;
	CFilePath fp_ctrl;
//取得模型
	fp_model=ui->cb_model->currentText().toStdString();
//取得算法
	fp_ctrl=ui->cb_ctrlalg->currentText().toStdString();
	Json::Value dict_cfg; //从dict控件中取得的cfg
	pdict->getData(dict_cfg);
	u8 curv_n=2; //总曲线数
//调用运行函数
	sim_proc(expdatafilename,fp_model,fp_ctrl,dict_cfg,
		[this,&curv_n](Json::Value &v) //设置曲线
		{
			//pjson(v);
			chart0->removeAllSeries(); //首先去掉所有曲线
			chart_serial_0= new QLineSeries(chart0);
			chart_serial_0->setName("e");
			chart_serial_0->setUseOpenGL(true); //使用OpenGL加速显示
			chart0->addSeries(chart_serial_0);
			chart_serial_1= new QLineSeries(chart0);
			chart_serial_1->setName("u");
			chart_serial_1->setUseOpenGL(true); //使用OpenGL加速显示
			chart0->addSeries(chart_serial_1);
			if(v.isArray())
			{
				for(auto &it:v)
				{
					QLineSeries *tmpseries=new QLineSeries(chart0);
					tmpseries->setName(it.asString().c_str());
					tmpseries->setUseOpenGL(true); //使用OpenGL加速显示
					chart0->addSeries(tmpseries);
					curv_n++;
				}
			}
			chart0->createDefaultAxes();
			return curv_n;
		},
		[this](u8 series_n,int x,float y) //曲线设置函数
		{
			if(series_n>=chart0->series().size()) return ;
			((QLineSeries*)chart0->series()[series_n])->append(x,y);
		}
	);
	on_bt_fitscreen_clicked();
	//评估值
	ui->lb_eval_std->setText(sFormat("%.2f",eval_std).c_str());
}

void MainWindow::on_bt_import_data_clicked() //导入数据
{
	auto name=QFileDialog::getOpenFileName(0,"","","csv文件(*.csv)");
	if(name!="")
	{
		string text=read_textfile(name.toStdString().c_str());
		vector<string> vs=com_split(text,"\n");
		if(vs.size()<2) return ;
		//第一行
		vector<string> line=com_split(vs[0],",");
		if(line.size()<=0 || line.size()>10) return ;
		chart0->removeAllSeries(); //首先去掉所有曲线
		for(auto &it:line)
		{
			QLineSeries *tmpseries=new QLineSeries(chart0);
			tmpseries->setName(it.c_str());
			tmpseries->setUseOpenGL(true); //使用OpenGL加速显示
			chart0->addSeries(tmpseries);
		}
		chart0->createDefaultAxes();
		int curv_n=line.size();
		chart0->createDefaultAxes();
		for(int i=1;i<vs.size();i++) //每一行
		{
			line=com_split(vs[i],",");
			if(line.size()!=curv_n) continue;
			for(int j=0;j<line.size();j++)
			{
				float y=0;
				sscanf(line[j].c_str(),"%f",&y);
				((QLineSeries*)chart0->series()[j])->append(i-1,y);
			}
		}
		on_bt_fitscreen_clicked();
	}
}

void MainWindow::on_bt_save_data_clicked() //导出数据
{
	auto name=QFileDialog::getSaveFileName (0,"","","csv文件(*.csv)");
	if(name!="")
	{
		CComFile cf;
		cf.open(name.toStdString().c_str(),"w");
		//首先写第一行
		string line="";
		for(auto &it:chart0->series())
		{
			line+=it->name().toStdString();
			line+=",";
		}
		line.pop_back();
		line+="\n";
		cf.write((char*)line.c_str(),line.size());
		int s_n=chart0->series().size();
		int d_n=((QLineSeries*)(chart0->series()[0]))->points().size();
		for(int i=0;i<d_n;i++)
		{
			line="";
			for(int j=0;j<s_n;j++)
			{
				float y=((QLineSeries*)(chart0->series()[j]))->points()[i].y();
				line+=sFormat("%.2f,",y);
			}
			line.pop_back();
			line+="\n";
			cf.write((char*)line.c_str(),line.size());
		}
	}
}

void MainWindow::on_bt_recognize_clicked() //系统辨识
{

}
