
#include "main.h"
#include "json.h"

//################################################################################
//						对外接口
//################################################################################
extern "C"
{
	float model_fun_arma(float u);
	const char *cmd_arma(const char *s); //指令函数
}
//################################################################################
//						工具
//################################################################################
#ifndef SOMAKE
int main(int argc, const char *argv[])
{
	//设置参数
	Json::Value rstv;
	rstv["arma"]="set_cfg";
	rstv["cfg"]["B_list"][0]=0;
	rstv["cfg"]["B_list"][1]=0.9;
	rstv["cfg"]["A_list"][0]=0.1;
	Json::FastWriter writer;
	string s=writer.write(rstv);
	cmd_arma(s.c_str());
//读取参数
	rstv.clear();
	rstv["arma"]="get_cfg";
	s=writer.write(rstv);
	s=cmd_arma(s.c_str());
	printf("%s\n",s.c_str());
//阶跃响应
	for(int i=0;i<30;i++)
	{
		float r=model_fun_arma(10);
		printf("%.2f\n",r);
	}
	return 0;
}
#endif

//################################################################################
//						处理函数
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
const char *cmd_arma(const char *s)
{
	static string rsts="";
	Json::Value v;
	Json::Reader reader;
	reader.parse(s,v);
	if(v["arma"].isString())
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

