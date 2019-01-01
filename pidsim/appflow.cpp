#include "appflow.h"
#include <QDir>
extern "C"
{
#include <dlfcn.h>
}
//程序流程数据
map<string,shared_ptr<CCtrlAlg> > sp_ctrl; //控制算法列表
map<string,shared_ptr<CSysModel> > sp_md; //模型列表
static map<string,void*> sofiles; //加载的所有动态库文件,文件名：文件指针
//////////////////////////////////////////////////////////////////////////////
//						程序流程函数
//////////////////////////////////////////////////////////////////////////////
void ctrl_ini(vector<string> &files)
{
	part_ini<map<string,shared_ptr<CCtrlAlg> >,CCtrlAlg>(files,sp_ctrl);
	//从配置中读取真正的函数
	for(auto &it:sp_ctrl)
	{
		it.second->loadso();
	}
}
void model_ini(vector<string> &files)
{
	part_ini<map<string,shared_ptr<CSysModel> >,CSysModel>(files,sp_md);
	//从配置中读取真正的函数
	for(auto &it:sp_md)
	{
		it.second->loadso();
	}
}
//控制仿真流程
//控制仿真配置
extern void msgbox_yes(string title,string text); //yes对话框
int en_noise_norm=1; //正太分布噪声
float nnorm_mean=0; //均值
float nnorm_std=1; //均方差
int en_noise_acc=1; //累积正太分布噪声
float nacc_std=1; //均方差
float eval_std=0; //评估效果
int eval_jump_n=10; //评估跳过值
//流程函数
void sim_proc(string expdatafilename, //期望数据文件名
				CFilePath fp_model, //模型文件名
				CFilePath fp_ctrl, //控制文件名
				Json::Value dict_cfg, //从dict控件中取得的cfg
				function<int (Json::Value &v)> setcurv, //设置曲线函数,返回总数量
				function<void (u8 series_n,int x,float y)> curvdata //曲线数据函数
				)
{
///////////////////////////////////////////////////////////////////////////
//取得当前的仿真数据源--期望数据
	string text=read_textfile(expdatafilename.c_str());
	vector<string> vs=com_split(text,"\n");
	if(vs.size()<=0) //若文件不对
	{
		msgbox_yes("error", "exp 文件错误");
		return ;
	}
	vector<float> expdata;
	expdata.resize(vs.size());
	for(int i=0;i<vs.size();i++)
	{
		sscanf(vs[i].c_str(),"%f",&(expdata[i]));
	}
	Json::Value rstv;
	Json::FastWriter writer;
	Json::Reader reader;
	string s;
///////////////////////////////////////////////////////////////////////////
//取得模型
	auto pmodel=sp_md[fp_model.name_ext];
	//设置参数,清除状态
	rstv[pmodel->funname]="set_cfg";
	rstv["cfg"]=pmodel->cfg;
	s=writer.write(rstv);
	pmodel->cmd_fun(s.c_str());
///////////////////////////////////////////////////////////////////////////
//取得算法
	auto pctrl=sp_ctrl[fp_ctrl.name_ext];
	//CMD_FUN ctrl_cmd_fun=pctrl->cmd_fun;
	//设置参数
	rstv.clear();
	rstv["cfg"]=dict_cfg;
	rstv[pctrl->funname]="set_cfg";
	s=writer.write(rstv);
	pctrl->cmd_fun(s.c_str());
	//清除状态
	rstv.clear();
	rstv[pctrl->funname]="ini";
	s=writer.write(rstv);
	pctrl->cmd_fun(s.c_str());
	//获取参数
	rstv.clear();
	rstv[pctrl->funname]="get_cfg";
	s=writer.write(rstv);
	s=pctrl->cmd_fun(s.c_str());
	rstv.clear();
	reader.parse(s,rstv);
	//设置曲线
	int curv_n=setcurv(rstv["curve"]);
	if(curv_n<2 || curv_n>10) //限制曲线数量
	{
		msgbox_yes("error", "curve number must between 0 and 10");
		return ;
	}
///////////////////////////////////////////////////////////////////////////
//主流程
	cout<<"pid sim"<<endl;
	int i;
	float u=0,y=0,ea=0;
	eval_std=0;
	//噪音
	default_random_engine rand_e(time(0));
	normal_distribution<> rand_norm(nnorm_mean,nnorm_std); //均值、标准差
	normal_distribution<> rand_acc(0,nacc_std); //均值、标准差
	for(int i=0;i<expdata.size();i++)
	{
	//控制函数
		u=pctrl->fun(expdata[i],y); 
	//评估效果
		if(i>=eval_jump_n) eval_std+=(expdata[i]-y)*(expdata[i]-y);
	//曲线操作
		double d=expdata[i]-y;
		//printf("%.2f %.2f\n",u,d);
		curvdata(0,i,d);
		curvdata(1,i,u);
		rstv.clear();
		rstv[pctrl->funname]="curve";
		s=writer.write(rstv);
		s=pctrl->cmd_fun(s.c_str());
		rstv.clear();
		reader.parse(s,rstv);
		if(rstv["curve"].isArray() && rstv["curve"].size()==curv_n-2)
		{
			for(int j=0;j<rstv["curve"].size();j++)
			{
				d=rstv["curve"][j].asDouble();
				curvdata(j+2,i,d);
			}
		}
		else cout<<"curve number error"<<endl;
	//调用模型
		y=pmodel->fun(u);
	//加入噪声
		if(en_noise_norm) //使能正太分布噪声
		{
			y+=rand_norm(rand_e);
		}
		ea+=rand_acc(rand_e);
		ea*=0.9;
		if(en_noise_acc) //使能累积正太分布噪声
		{
			y+=ea;
		}
	}
	eval_std/=(expdata.size()==0?1:expdata.size());
	eval_std=sqrt(eval_std);
}

//////////////////////////////////////////////////////////////////////////////
//						算法对象成员函数
//////////////////////////////////////////////////////////////////////////////
void CAlgObj::loadso(void)
{
	char *error;
	if(sofiles.count(dllname)<=0) //若还没打开这个动态库
	{
		void *tp=dlopen((exepath+"lib/"+dllname).c_str(), RTLD_GLOBAL);
		if(!tp)
			throw string("load dll:")+dllname+" failed. "+__FILE__+sFormat(":%d\n",__LINE__);
		sofiles[dllname] = tp;
	}
	cmd_fun=(CMD_FUN)dlsym(sofiles[dllname],"cmd_fun");
	if ((error = dlerror()) != NULL)
		throw string("load dll:")+dllname+" : cmd_fun failed. "+__FILE__+sFormat(":%d\n",__LINE__);
}
string CCtrlAlg::dirname="ctrl";
void CCtrlAlg::loadso(void)
{
	CAlgObj::loadso();
	char *error;
	fun=(CTRL_FUN)dlsym(sofiles[dllname],funname.c_str());
	if ((error = dlerror()) != NULL)
		throw string("load dll:"+dllname+" : "+funname+" failed. "+__FILE__+sFormat(":%d\n",__LINE__));
}
string CSysModel::dirname="model";
void CSysModel::loadso(void)
{
	CAlgObj::loadso();
	char *error;
	fun=(MODEL_FUN)dlsym(sofiles[dllname],funname.c_str());
	if ((error = dlerror()) != NULL)
		throw string("load dll:"+dllname+" : "+funname+" failed. "+__FILE__+sFormat(":%d\n",__LINE__));
}

