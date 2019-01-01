#ifndef APPFLOW_H
#define APPFLOW_H

#include "common.h"
#include "json.h"

typedef float (*CTRL_FUN)(float exp,float y); //控制函数接口
typedef float (*MODEL_FUN)(float u); //模型函数接口
typedef const char *(*CMD_FUN)(const char *s); //指令函数

class CAlgObj //程序对象
{
public:
	CAlgObj(){}
	string id; //对象唯一ID
	string dllname; //dll名称
	string funname; //函数名称
	Json::Value cfg; //参数
	CMD_FUN cmd_fun; //通信指令函数
	virtual Json::Value toJson(void)
	{
		Json::Value v;
		v["dllname"]=dllname;
		v["funname"]=funname;
		v["cfg"]=cfg;
		return v;
	}
	virtual int fromJson(Json::Value &v)
	{
		if(v["dllname"].isString() && 
			v["funname"].isString())
		{
			dllname=v["dllname"].asString();
			funname=v["funname"].asString();
			cfg=v["cfg"];
			return 0;
		}
		return 1;
	}
	virtual void loadso(void); //加载动态库和函数
};
class CCtrlAlg : public CAlgObj //控制对象
{
public:
	CTRL_FUN fun; //算法调用函数
	Json::Value cfgdes; //参数描述
	static string dirname;
	virtual Json::Value toJson(void)
	{
		Json::Value v=CAlgObj::toJson();
		v["cfgdes"]=cfgdes;
		return v;
	}
	virtual int fromJson(Json::Value &v)
	{
		int r=CAlgObj::fromJson(v);
		if(v["cfgdes"].isArray())
		{
			cfgdes=v["cfgdes"];
		}
		return r;
	}
	virtual void loadso(void); //加载动态库和函数
};
class CSysModel : public CAlgObj //模型对象
{
public:
	MODEL_FUN fun; //算法调用函数
	static string dirname;
	virtual void loadso(void); //加载动态库和函数
};

///////////////////////////////////////////////////////////////////
//程序体系数据
extern map<string,shared_ptr<CCtrlAlg> > sp_ctrl; //控制算法列表
extern map<string,shared_ptr<CSysModel> > sp_md; //模型列表

///////////////////////////////////////////////////////////////////
//初始化
template <class Tmap,class Talg>
void part_ini(vector<string> &files,Tmap &sp) //将文件列表转换为内存对象，放在map中
{
	sp.clear();
	for(auto &it:files) //输入的文件列表不含全路径
	{
		string fullpath=exepath+Talg::dirname+"/"+it;
		Json::Value v;
		string text=read_textfile(fullpath.c_str());
		Json::Reader reader;
		reader.parse(text.c_str(),v,false); //可以有注释,false不会复制
		cout<<"get a config file"<<endl;
		pjson(v); //输出配置字符
		if(!v.isObject()) continue;
		Talg tc;
		if(tc.fromJson(v)) continue;
		tc.id=it;
		sp[it]=make_shared<Talg>(tc);
	}
	if(sp.size()<=0) throw string("no functions")+__FILE__+sFormat(":%d\n",__LINE__);
}
void ctrl_ini(vector<string> &files);
void model_ini(vector<string> &files);
///////////////////////////////////////////////////////////////////
//仿真流程及配置
extern int en_noise_norm; //正太分布噪声
extern float nnorm_mean; //均值
extern float nnorm_std; //均方差
extern int en_noise_acc; //累积正太分布噪声
extern float nacc_std; //均方差
extern float eval_std; //评估效果
void sim_proc(string expdatafilename, //期望数据文件名
				CFilePath fp_model, //模型文件名
				CFilePath fp_ctrl, //控制文件名
				Json::Value dict_cfg, //从dict控件中取得的cfg
				function<int (Json::Value &v)> setcurv, //设置曲线函数,返回总数量
				function<void (u8 series_n,int x,float y)> curvdata //曲线数据函数
				);

#endif

