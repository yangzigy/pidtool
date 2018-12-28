#include "mainwindow.h"
#include <QApplication>

Json::Value config; //配置对象
MainWindow *pwin;

vector<string> list_dir(const char *path,const char *filter); //列出目录中指定文件
//////////////////////////////////////////////////////////////
//					入口
//////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
///////////////////////////////////////////////////////
//				配置
///////////////////////////////////////////////////////
	start_program();
	string name=exepath+"config.txt";
	if(argc==2) //若指定了配置文件
	{
		name=argv[1];
	}
	string text=read_textfile(name.c_str());
	Json::Reader reader;
	reader.parse(text.c_str(),config,false); //可以有注释,false不会复制
	pjson(config); //输出配置字符
//启动QT部分
	QApplication a(argc, argv);
	MainWindow w; //一定放在配置读取以后
	pwin=&w;
	w.show();
//启动时加载文件夹中的模型和控制算法
	try
	{
		vector<string> files=list_dir((exepath+"model").c_str(),"*.mdjson");
		model_ini(files);
		files=list_dir((exepath+"ctrl").c_str(),"*.ctrljson");
		ctrl_ini(files);
	}
	catch(string s)
	{
		cout<<s<<endl;
		QMessageBox::information(&w,"error", s.c_str(), QMessageBox::Yes);
		return 1;
	}
	cout<<"ctrl and model functions: "<<sp_ctrl.size()<<"   "<<sp_md.size()<<endl;
	w.ui_initial();
	return a.exec();
}

vector<string> list_dir(const char *path,const char *filter)
{
	QDir dir(path);
	if(!dir.exists())
	{
		return vector<string>();
	}
	//查看路径中后缀为.json格式的文件
	QStringList filters;
	filters<<QString(filter);
	dir.setFilter(QDir::Files | QDir::NoSymLinks); //设置类型过滤器，只为文件格式
	dir.setNameFilters(filters);  //设置文件名称过滤器，只为filters格式
	vector<string> rst;
	for(int i=0;i<dir.count();i++)
	{
		string s=path;
		rst.push_back(s+"/"+dir[i].toStdString());
	}
	return rst;
}
