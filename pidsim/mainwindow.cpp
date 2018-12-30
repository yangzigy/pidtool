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
	}
	DBGL;
	cout<<arg1.toStdString()<<endl;
	pjson(sp_ctrl[arg1.toStdString()]->cfg);
	pjson(sp_ctrl[arg1.toStdString()]->cfgdes);
	subWidget=new QWidget();

	ui->gridLayout_8->addWidget(subWidget,0,0); //加入到栅格布局的第0行第0列
	pdict=new CDictDis(sp_ctrl[arg1.toStdString()]->cfgdes,subWidget);
}

void MainWindow::on_bt_sim_clicked() //开始仿真
{
///////////////////////////////////////////////////////////////////////////
//取得当前的仿真数据源--期望数据
	string expdatafilename=ui->le_exp_filename->text().toStdString();
	string text=read_textfile(expdatafilename.c_str());
	vector<string> vs=com_split(text,"\n");
	if(vs.size()<=0) //若文件不对
	{
		QMessageBox::information(this,"error", "exp 文件错误", QMessageBox::Yes);
	}
	vector<float> expdata;
	expdata.resize(vs.size());
	for(int i=0;i<vs.size();i++)
	{
		sscanf(vs[i].c_str(),"%f",&(expdata[i]));
	}
	Json::Value rstv;
	CFilePath fp_model;
	CFilePath fp_ctrl;
	Json::FastWriter writer;
	Json::Reader reader;
	string s;
///////////////////////////////////////////////////////////////////////////
//取得模型
	fp_model=ui->cb_model->currentText().toStdString();
	auto pmodel=sp_md[fp_model.name_ext];
	//设置参数
	rstv[fp_model.name]="set_cfg";
	rstv["cfg"]=pmodel->cfg;
	s=writer.write(rstv);
	pmodel->cmd_fun(s.c_str());
///////////////////////////////////////////////////////////////////////////
//取得算法
	fp_ctrl=ui->cb_ctrlalg->currentText().toStdString();
	auto pctrl=sp_ctrl[fp_ctrl.name_ext];
	//CMD_FUN ctrl_cmd_fun=pctrl->cmd_fun;
	//设置参数
	rstv.clear();
	//获取参数
	rstv.clear();
	rstv[fp_ctrl.name]="get_cfg";
	s=writer.write(rstv);
	s=pctrl->cmd_fun(s.c_str());
	rstv.clear();
	reader.parse(s,rstv);
	//设置曲线
	chart0->removeAllSeries(); //首先去掉所有曲线
	chart_serial_0= new QLineSeries(chart0);
	chart_serial_0->setName("e");
	chart_serial_0->setUseOpenGL(true); //使用OpenGL加速显示
	chart0->addSeries(chart_serial_0);
	chart_serial_1= new QLineSeries(chart0);
	chart_serial_1->setName("u");
	chart_serial_1->setUseOpenGL(true); //使用OpenGL加速显示
	chart0->addSeries(chart_serial_1);
	if(rstv["curve"].isArray())
	{
		for(auto &it:rstv["curve"])
		{
			QLineSeries *tmpseries=new QLineSeries(chart0);
			tmpseries->setName(it.asString().c_str());
			tmpseries->setUseOpenGL(true); //使用OpenGL加速显示
			chart0->addSeries(tmpseries);
		}
	}
	chart0->createDefaultAxes();
///////////////////////////////////////////////////////////////////////////
//主流程
	cout<<"pid sim"<<endl;
	int i;
	float u=0,y=0,ea=0;
	//噪音
	default_random_engine rand_e;
	normal_distribution<> rand_norm_small(0,1); //均值、标准差
	float max=-0xffffffff,min=0xffffffff;
	for(int i=0;i<expdata.size();i++)
	{
		u=pctrl->fun(expdata[i],y);
	//取得曲线
		double d=expdata[i]-y;
		printf("%.2f %.2f\n",u,d);
		chart_serial_0->append(i,d);
		chart_serial_1->append(i,u);
		if(u>max) max=u; if(u<min) min=u;
		if(d>max) max=d; if(d<min) min=d;
		rstv.clear();
		rstv[fp_ctrl.name]="curve";
		s=writer.write(rstv);
		s=pctrl->cmd_fun(s.c_str());
		rstv.clear();
		reader.parse(s,rstv);
		if(rstv["curve"].isArray() && rstv["curve"].size()==chart0->series().size()-2)
		{
			for(int j=0;j<rstv["curve"].size();j++)
			{
				d=rstv["curve"][j].asDouble();
				((QLineSeries*)chart0->series()[j+2])->append(i,d);
				if(d>max) max=d; if(d<min) min=d;
			}
		}
	//调用模型
		y=pmodel->fun(u);
		y+=rand_norm_small(rand_e);
		ea+=rand_norm_small(rand_e);
		ea*=0.9;
		y+=ea;
	}
	chart0->axisX()->setRange(0,expdata.size());
	chart0->axisY()->setRange(min,max);
}
