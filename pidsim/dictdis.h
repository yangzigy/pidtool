#ifndef DICTDIS_H
#define DICTDIS_H

#include "json.h"
#include "common.h"
#include <QString>
#include "qlabel.h"
#include <QLayout>
#include <QLineEdit>

class CDictDis //显示块类
{
public: //总类型包括：值、对象、列表 三种。其中值又包括：double int string stringarray等
	//块的属性：
	string type="string"; //自己的类型
	string name="";
	string des="";
	Json::Value val; //当前值

	//块的界面元素,块的widget由外部提供
	//QLabel *lbname=new QLabel();
	QLineEdit *lbname=new QLineEdit();
	QLineEdit *le_value=new QLineEdit();
	//QLabel *lbdes=new QLabel();
	QLineEdit *lbdes=new QLineEdit();
	QGridLayout *grid=0;
	QWidget *subWidget=0; //子块面板,作为对象时才有用
	vector<shared_ptr<QWidget> > widgetlist; //列表子块面板，作为列表时才有用

	//块的子块
	vector<shared_ptr<CDictDis> > dictlist;
	~CDictDis()
	{
		delete lbname;
		delete le_value;
		delete lbdes;
		if(grid) delete grid;
		if(subWidget) delete subWidget;
	}
	CDictDis(Json::Value &v,QWidget *pwi) //输入json节点，容器
	{ //首先创建布局器进行显示
		grid = new QGridLayout(pwi);
		grid->setMargin(0);
		grid->setVerticalSpacing(0);
		pwi->setLayout(grid);
		if(v.isObject()) //若是对象(值，或 对象类型)
		{
			//确定对象是哪种类型
			string stype=v["type"].asString();
			type=stype;
			name=v["name"].asString();
			des=v["des"].asString();
			val=v["dft"];

			lbdes->setText(des.c_str());
			lbdes->setStyleSheet("background-color:rgb(230,230,255)");
			lbdes->setReadOnly(true);
			lbdes->setCursorPosition(0);
			lbname->setText(name.c_str());
			lbname->setMinimumWidth(150);
			lbname->setMaximumWidth(150);
			//lbname->setStyleSheet("font: bold; font-size:18px; color: rgb(241, 70, 62); background-color: green");
			lbname->setStyleSheet("font: bold;background-color:rgb(230,255,230)");
			lbname->setReadOnly(true);
			lbname->setCursorPosition(0);
			if(stype=="obj") //若对象是一个对象类型
			{
				grid->addWidget(lbname,0,0,2,1); //加入到栅格布局的第0行第0列
				grid->addWidget(lbdes,0,1);
				//创建一个widget交给子节点显示
				subWidget=new QWidget(pwi);
				grid->addWidget(subWidget,1,1); //加入到栅格布局的第0行第1列
				CDictDis *tp=new CDictDis(v["dft"],subWidget); //交给子节点去绘制自己的界面
				shared_ptr<CDictDis> tstp(tp);
				dictlist.push_back(tstp);
			}
			else if(stype=="stringarray") //若对象是一个字符列表
			{
			}
			else if(stype=="double")
			{
				newNormObj(v,QString().sprintf("%lf",v["dft"].asDouble()).toStdString().c_str());
			}
			else if(stype=="int")
			{
				newNormObj(v,QString().sprintf("%d",v["dft"].asInt()).toStdString().c_str());
			}
			else if(stype=="string")
			{
				newNormObj(v,v["dft"].asString().c_str());
			}
		}
		else if(v.isArray()) //若是列表
		{
			type="array";
			int i=0;
			if(name!="")
			{
				lbname->setText(name.c_str());
				grid->addWidget(lbname,i,0); //加入到栅格布局的第i行第0列
				i++;
			}
			for(auto &it:v) //对于列表中的每一项，创建一个widget显示
			{
				QWidget *tpwi=new QWidget(pwi);
				//tpwi->setStyleSheet("QWidget{border: 1px solid black;}");
				tpwi->setStyleSheet("QWidget{border-top: 1px solid black;}");
				grid->addWidget(tpwi,i,0); //加入到栅格布局的第i行第0列
				CDictDis *tp=new CDictDis(it,tpwi); //交给子节点去绘制自己的界面
				
				shared_ptr<QWidget> twtp(tpwi);
				widgetlist.push_back(twtp);

				shared_ptr<CDictDis> tstp(tp);
				dictlist.push_back(tstp);
				i++;
			}
		}
	}
	void newNormObj(Json::Value &v,const char *ps) //添加一个普通对象
	{
		le_value->setText(ps);
		le_value->setMinimumWidth(150);
		le_value->setMaximumWidth(150);
		le_value->setStyleSheet("background-color:rgb(255,230,230)");

		grid->addWidget(lbname,0,0);
		grid->addWidget(le_value,0,1);
		grid->addWidget(lbdes,0,2);
	}
	//////////////////////////////////////////////////////
	//为已建立的界面中添加数据
	void setData(Json::Value &v)
	{
		if(type=="array") //若是参数列表
		{
			//遍历所有的对象，在v中查找值
			for(auto &it:dictlist) //参数列表中的块使用的数据在同一级
			{
				it.get()->setData(v);
			}
		}
		else if(type=="obj") //若是对象
		{
			for(auto &it:dictlist) //参数列表中的块使用的数据在下一级
			{
				Json::Value ev;
				it.get()->setData(v[name.c_str()]);
			}
		}
		else if(type=="string")
		{
			if(v[name.c_str()].isString())
			{
				val=v[name.c_str()];
				le_value->setText(val.asString().c_str());
			}
		}
		else if(type=="int")
		{
			if(v[name.c_str()].isInt())
			{
				val=v[name.c_str()];
				le_value->setText(QString().sprintf("%d",val.asInt()));
			}
		}
		else if(type=="double")
		{
			if(v[name.c_str()].isDouble())
			{
				val=v[name.c_str()];
				le_value->setText(QString().sprintf("%lf",val.asDouble()));
			}
		}
	}
	//从界面中拿数据
	void getData(Json::Value &v)
	{
		if(type=="array") //若是参数列表
		{
			//遍历所有的对象，设置到v中
			for(auto &it:dictlist) //参数列表中的块使用的数据在同一级
			{
				it.get()->getData(v);
			}
		}
		else if(type=="obj") //若是对象
		{
			for(auto &it:dictlist) //参数列表中的块使用的数据在下一级
			{
				Json::Value ev;
				it.get()->getData(v[name.c_str()]);
			}
		}
		else if(type=="string")
		{
			val=le_value->text().toStdString();
			v[name.c_str()]=val;
		}
		else if(type=="int")
		{
			val=le_value->text().toInt();
			v[name.c_str()]=val;
		}
		else if(type=="double")
		{
			val=le_value->text().toDouble();
			v[name.c_str()]=val;
		}
	}
};

#endif // 

