#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTextCodec>

extern Json::Value config; //配置对象

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	QMargins tmpmarg(1,1,1,1);
	chartView0 = new MQChartView();
	chart0=chartView0->chart0;
//	chartView0->setRenderHint(QPainter::Antialiasing, true); //
	//chart0->setTheme(QChart::ChartThemeBrownSand); //
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
	float e_curv_amp=ui->le_e_amp->text().toFloat();
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
		[this,&e_curv_amp](u8 series_n,int x,float y) //曲线设置函数
		{
			if(series_n>=chart0->series().size()) return ;
			if(series_n==0) y*=e_curv_amp;
			((QLineSeries*)chart0->series()[series_n])->append(x,y);
		}
	);
	on_bt_fitscreen_clicked();
	//评估值
	ui->lb_eval_std->setText(sFormat("均方差：%.2f",eval_std).c_str());
	ui->lb_eval_max_e->setText(sFormat("最大偏离：%.2f",eval_max_err).c_str());
}
void MainWindow::on_bt_recognize_clicked() //系统辨识
{//取得模型
	CFilePath fp_model;
	fp_model=ui->cb_model->currentText().toStdString();
	auto pmodel=sp_md[fp_model.name_ext];
	if(pmodel->recname=="")
	{
		string s=sFormat("模型:%s 无辨识函数",pmodel->id.c_str());
		QMessageBox::information(this,"error", s.c_str(), QMessageBox::Yes);
	}
	//打开辨识输入数据,第一列输入，第二列输出
	auto name=QFileDialog::getOpenFileName(0,"","","csv文件(*.csv)");
	if(name=="") return ;
	string text=read_textfile(name.toStdString().c_str());
	vector<string> vs=com_split(text,"\n");
	if(vs.size()<2) return ;
	vector<string> line;
	vector<float> ulist; //输入列表
	vector<float> ylist; //输出列表;
	for(int i=0;i<vs.size();i++) //每一行
	{
		line=com_split(vs[i],",");
		if(line.size()<2) continue;
		float u=0,y=0;
		sscanf(line[0].c_str(),"%f",&u);
		sscanf(line[1].c_str(),"%f",&y);
		ulist.push_back(u);
		ylist.push_back(y);
	}
	//设置参数,清除状态
	Json::Value rstv;
	Json::StyledWriter writer;
	string s;
	rstv[pmodel->funname]="set_cfg";
	rstv["cfg"]=pmodel->cfg;
	s=writer.write(rstv);
	pmodel->cmd_fun(s.c_str());
	//传入辨识函数，开始辨识
	float r=pmodel->rec_fun(&(ulist[0]),&(ylist[0]),ulist.size());
	s=sFormat("辨识结果：%.2f，是否保存",r);
	auto qr=QMessageBox::information(this,"是否保存", s.c_str(), QMessageBox::Yes | QMessageBox::No);
	if(qr==QMessageBox::No) return ;
	//若要保存
	//获取参数
	rstv.clear();
	rstv[pmodel->funname]="get_cfg";
	s=writer.write(rstv);
	s=pmodel->cmd_fun(s.c_str());
	Json::Value cfg;
	Json::Reader reader;
	reader.parse(s,cfg);
	rstv=pmodel->toJson();
	rstv["cfg"]=cfg;
	//询问保存文件名
	bool ok = FALSE;
	QString qtext = QInputDialog::getText(this,tr("保存路径"),tr("请输入文件名:"),QLineEdit::Normal,"",&ok);
	if(ok && !qtext.isEmpty())
	{
		s=qtext.toStdString()+".mdjson";
		if(s==pmodel->id) //若与之前相等，则提示
		{
			QMessageBox::information(this,"error", "文件名与现有模型重复", QMessageBox::Yes);
			return ;
		}
		s=CSysModel::dirname+"/"+s; //文件名
		CComFile cf;
		cf.open(s.c_str(),"w");
		s=writer.write(rstv);
		cf.write((void*)s.c_str(),s.size());
		cf.close();
		//重新加载模型部分
		extern vector<string> list_dir(const char *path,const char *filter); //列出目录中指定文件
		vector<string> files=list_dir(CSysModel::dirname.c_str(),"*.mdjson");
		model_ini(files);
		ui->cb_model->clear();
		for(auto &it:sp_md) //控制算法列表
		{
			ui->cb_model->addItem(it.first.c_str());
		}
	}
}
void MainWindow::on_bt_import_data_clicked() //导入数据
{
	auto name=QFileDialog::getOpenFileName(0,"","","csv文件(*.csv)");
	QTextCodec *code = QTextCodec::codecForName("gb2312");//解决中文路径问题
	string namestd = code->fromUnicode(name).data();
	if(namestd!="")
	{
		string text=read_textfile(namestd.c_str());
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
	QTextCodec *code = QTextCodec::codecForName("gb2312");//解决中文路径问题
	string namestd = code->fromUnicode(name).data();
	if(namestd!="")
	{
		CComFile cf;
		cf.open(namestd.c_str(),"w");
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
void MainWindow::on_bt_help_clicked() //帮助
{
	QFile nFile(":/help.html");
	if(!nFile.open(QFile::ReadOnly))
	{
		qDebug() << "could not open file for reading";
		return;
	}
	string nText =nFile.readAll().data();
	nText=com_replace(nText,"src=\"","src=\"qrc:/");
	widget_help->setStyleSheet("font-size:14px;");
	widget_help->setMinimumWidth(700);
	widget_help->setMinimumHeight(500);
	widget_help->setHtml(nText.c_str());
	widget_help->show();
}

