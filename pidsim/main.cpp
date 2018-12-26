#include "mainwindow.h"
#include <QApplication>
#include "json.h"

Json::Value config; //配置对象
MainWindow *pwin;

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

	QApplication a(argc, argv);
	MainWindow w; //一定放在配置读取以后
	pwin=&w;
	w.show();
	return a.exec();
}

