
#include "pid.h"
#include "json.h"

//################################################################################
//						对外接口
//################################################################################
extern "C"
{
	float ctrl_fun_basepid(float exp,float y); //pid控制函数
	float model_fun_arma(float u); //Arma模型函数
	const char *cmd_fun(const char *s); //指令函数
}
PID_CON m_pid=
{
	0,0,0, 0,0,0,0,//状态
	0.5,	10,	10,	0.2, //滤波，P，I，D
	0.1,100,-100, //周期(s)，上下限
	100,0.1,	0.01, //积分分离阈值，D环节低通，输入死区
	100,100,	10, //I、D 最大值(不用置max),小范围逼近e阈值(不用置0)
};
//################################################################################
//						工具
//################################################################################
#ifndef SOMAKE
int main(int argc, const char *argv[]) //测试用
{
//////////////////////////////////////////////////////////////////
//模型部分
	//设置参数
	Json::Value rstv;
	rstv["arma"]="set_cfg";
	rstv["cfg"]["B_list"][0]=0;
	rstv["cfg"]["B_list"][1]=0.9;
	rstv["cfg"]["A_list"][0]=0.1;
	Json::FastWriter writer;
	string s=writer.write(rstv);
	cmd_fun(s.c_str());
//读取参数
	rstv.clear();
	rstv["arma"]="get_cfg";
	s=writer.write(rstv);
	s=cmd_fun(s.c_str());
	printf("%s\n",s.c_str());
//阶跃响应
	for(int i=0;i<30;i++)
	{
		float r=model_fun_arma(10);
		printf("%.2f\n",r);
	}
//////////////////////////////////////////////////////////////////
//控制部分
	cout<<"pid sim"<<endl;
	int i;
	float u=0,y=0,ea=0;
	default_random_engine rand_e;
	normal_distribution<> rand_norm_small(0,1); //均值、标准差
	for(i=0;i<50;i++)
	{
		u=ctrl_fun_basepid(30,y);
		printf("%.2f %.2f %.2f %.2f %.2f\n",
			m_pid.e*10,m_pid.u,m_pid.up,m_pid.ui,m_pid.ud);
		//y=y*0.9+u*0.1;
		y=model_fun_arma(u);
		y+=rand_norm_small(rand_e);
		ea+=rand_norm_small(rand_e);
		ea*=0.9;
		y+=ea;
	}
	return 0;
}
#endif
//################################################################################
//						控制函数
//################################################################################
//char dbg_buf[4096]={0};
float ctrl_fun_basepid(float exp,float y)
{
	float u=0;
	u=base_pid(exp-y,&m_pid);
	//构造调试字符串e u p i d
	//sprintf(dbg_buf,"%.2f %.2f %.2f %.2f %.2f\n",
		//m_pid.e,m_pid.u,m_pid.up,m_pid.ui,m_pid.ud);
	return u;
}
//################################################################################
//						arma模型
//################################################################################
//Arma模型：
//y(t) = b(0) + b(1)y(t-1) + …… + b(m)y(t-m) +
//		 a(0)x(t) + a(1)x(t-1) + …… + a(n)x(t-n)
vector<float> A_list; //n+1个
vector<float> x_list; //n+1个,x(t) …… x(t-n)
vector<float> B_list; //m+1个
vector<float> y_list; //m个,y(t-1) …… y(t-m)
float model_fun_arma(float u)
{
	float r=0;
	//移动数据
	for(int i=x_list.size()-1;i>0;i--)
	{
		x_list[i]=x_list[i-1];
	}
	x_list[0]=u;
	//计算
	for(int i=0;i<A_list.size();i++)
	{
		r+=A_list[i]*x_list[i];
	}
	r+=B_list[0];
	for(int i=1;i<B_list.size();i++)
	{
		r+=B_list[i]*y_list[i-1];
	}
	//移动y
	for(int i=y_list.size()-1;i>0;i--)
	{
		y_list[i]=y_list[i-1];
	}
	y_list[0]=r;
	return r;
}
//################################################################################
//						指令函数
//################################################################################
const char *cmd_fun(const char *s)
{
	static string rsts="";
	Json::Value v;
	Json::Reader reader;
	reader.parse(s,v);
	if(v["basepid"].isString())
	{
		string scmd=v["basepid"].asString();
		if(scmd=="curve")
		{
			Json::Value rstv;
			rstv["curve"][0]=m_pid.up;
			rstv["curve"][1]=m_pid.ui;
			rstv["curve"][2]=m_pid.ud;
			Json::FastWriter writer;
			rsts=writer.write(rstv);
		}
		else if(scmd=="set_cfg" && v["cfg"].isObject()) //设置配置项
		{
			double d=0;
			d=m_pid.k_in; jsonget(v["cfg"],"k_in",d); m_pid.k_in=d;
			d=m_pid.P; jsonget(v["cfg"],"P",d); m_pid.P=d;
			d=m_pid.I; jsonget(v["cfg"],"I",d); m_pid.I=d;
			d=m_pid.D; jsonget(v["cfg"],"D",d); m_pid.D=d;
			d=m_pid.T; jsonget(v["cfg"],"T",d); m_pid.T=d;
			d=m_pid.max; jsonget(v["cfg"],"max",d); m_pid.max=d;
			d=m_pid.min; jsonget(v["cfg"],"min",d); m_pid.min=d;
			d=m_pid.E; jsonget(v["cfg"],"E",d); m_pid.E=d;
			d=m_pid.Dc; jsonget(v["cfg"],"Dc",d); m_pid.Dc=d;
			d=m_pid.die; jsonget(v["cfg"],"die",d); m_pid.die=d;
			d=m_pid.max_i; jsonget(v["cfg"],"max_i",d); m_pid.max_i=d;
			d=m_pid.max_d; jsonget(v["cfg"],"max_d",d); m_pid.max_d=d;
			d=m_pid.high_e; jsonget(v["cfg"],"high_e",d); m_pid.high_e=d;
		}
		else if(scmd=="get_cfg") //获取配置项
		{
			Json::Value rstv;
			rstv["k_in"]=m_pid.k_in;
			rstv["P"]=m_pid.P;
			rstv["I"]=m_pid.I;
			rstv["D"]=m_pid.D;
			rstv["T"]=m_pid.T;//单位秒
			rstv["max"]=m_pid.max;//输出上限
			rstv["min"]=m_pid.min;//输出下限
			//扩展参数
			rstv["E"]=m_pid.E;//积分分离阈值（若不用，则置为最高误差输入）
			rstv["Dc"]=m_pid.Dc;//D环节低通系数
			rstv["die"]=m_pid.die;//输入死区绝对值,死区保持输出不变(若不用，则置零)
			//加速逼近
			rstv["max_i"]=m_pid.max_i;//I环节累积值最大值（若不用，则置为max）
			rstv["max_d"]=m_pid.max_d;//d环节最大值,（若不用，则置为max或以上）
			rstv["high_e"]=m_pid.high_e;//小范围逼近e阈值（若不用，则置为0）
			rstv["curve"][0]="up";
			rstv["curve"][1]="ui";
			rstv["curve"][2]="ud";
			Json::FastWriter writer;
			rsts=writer.write(rstv);
		}
	}
	else if(v["arma"].isString())
	{
		string scmd=v["arma"].asString();
		if(scmd=="set_cfg" && v["cfg"].isObject()) //设置配置项
		{
			if(v["cfg"]["A_list"].isArray())
			{
				A_list.resize(v["cfg"]["A_list"].size());
				x_list.resize(A_list.size());
				for(int i=0;i<A_list.size();i++)
				{
					A_list[i]=v["cfg"]["A_list"][i].asDouble();
				}
			}
			if(v["cfg"]["B_list"].isArray())
			{
				B_list.resize(v["cfg"]["B_list"].size());
				y_list.resize((B_list.size()-1)>0?(B_list.size()-1):0);
				for(int i=0;i<B_list.size();i++)
				{
					B_list[i]=v["cfg"]["B_list"][i].asDouble();
				}
			}
		}
		else if(scmd=="get_cfg") //获取配置项
		{
			Json::Value rstv;
			for(int i=0;i<A_list.size();i++)
			{
				rstv["A_list"][i]=A_list[i];
			}
			for(int i=0;i<B_list.size();i++)
			{
				rstv["B_list"][i]=B_list[i];
			}
			Json::FastWriter writer;
			rsts=writer.write(rstv);
		}
	}
	return rsts.c_str();
}

