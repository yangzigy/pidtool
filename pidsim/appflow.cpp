#include "appflow.h"
#include <QDir>
extern "C"
{
#include <dlfcn.h>
}

map<string,shared_ptr<CCtrlAlg> > sp_ctrl; //控制算法列表
map<string,shared_ptr<CSysModel> > sp_md; //模型列表

static map<string,void*> sofiles; //加载的所有动态库文件,文件名：文件指针

void CAlgObj::loadso(void)
{
	if(sofiles.count(dllname)<=0) //若还没打开这个动态库
	{
		char *error;
		void *tp=dlopen((exepath+"lib/"+dllname).c_str(), RTLD_GLOBAL);
		if(!tp)
			throw string("load dll:")+dllname+" failed\n";
		sofiles[dllname] = tp;
		cmd_fun=(CMD_FUN)dlsym(sofiles[dllname],"cmd_fun");
		if ((error = dlerror()) != NULL)
			throw string("load dll:")+dllname+" : cmd_fun failed\n";
	}
}
void CCtrlAlg::loadso(void)
{
	CAlgObj::loadso();
	char *error;
	fun=(CTRL_FUN)dlsym(sofiles[dllname],funname.c_str());
	if ((error = dlerror()) != NULL)
		throw string("load dll:"+dllname+" : "+funname+" failed\n");
}
void CSysModel::loadso(void)
{
	CAlgObj::loadso();
	char *error;
	fun=(MODEL_FUN)dlsym(sofiles[dllname],funname.c_str());
	if ((error = dlerror()) != NULL)
		throw string("load dll:"+dllname+" : "+funname+" failed\n");
}
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

