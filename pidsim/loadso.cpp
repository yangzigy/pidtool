
#include "loadso.h"
extern "C"
{
#include <dlfcn.h>
}

int CSoFile::load(const char *soname)
{
	close();
	handle = dlopen(soname, RTLD_GLOBAL);     //打开soname.c_str()指定的动态库
	if (!handle)
	{
		printf("so file load error:%s\n", dlerror());
		return 1;
	}
	printf("load so:%s\n",soname);
	sofilename=soname;
	dlerror();
	return 0;
}
void CSoFile::close(void)
{
	if(handle) dlclose(handle);      //关闭调用动态库句柄
	handle=0;
	sofilename="";
	functions.clear();
}
int CSoFile::add_fun(const char *name)
{
	if(!handle) return 1;
	char *error;
	void *pf=dlsym(handle,name);    //指针pf指向test在当前内存中的地址
	if ((error = dlerror()) != NULL)
	{
		printf ("so file :%s function error:%s:%s,%X\n",
			sofilename.c_str(),name,error,(void*)pf);
		return 2;
	}
	functions[name]=pf;
	printf ("so file :%s add a function:%s\n", sofilename.c_str(),name);
	return 0;
}

