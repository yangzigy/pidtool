#ifndef LOADSO_H
#define LOADSO_H

#include "main.h"

class CSoFile
{
public:
	CSoFile(){}
	CSoFile(const CSoFile &p)
	{
		sofilename=p.sofilename;
		handle=p.handle;
		functions=p.functions;
		((CSoFile*)&p)->handle=0;
	}
	~CSoFile()
	{
	}
	int load(const char *soname);
	void close(void);
	string sofilename;
	void *handle;
	map<string,void*> functions; //此动态库的函数列表
	void* get_fun(const char *name)
	{
		if(functions.count(name)>0)
		{
			return functions[name];
		}
		return 0;
	}
	int add_fun(const char *name);
};

#endif

