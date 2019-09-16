/*
文件名：pid.c
作者：北京交通大学 自控1102 杨孜
时间：2013-9-11
功能：

*/
#include "pid.h"//认为main.h中已经包含了math.h

#define PID		(*p)
//死区PID(带抗饱和积分,积分分离,不完全微分（适合噪声严重的系统）)
float base_pid(float e,PID_CON *p)//传入误差
{
	float tmpu=0;	 //基础输出
	float di=0;
	PID.e=e*PID.k_in + PID.e*(1-PID.k_in); //对误差低通滤波
	if(fabs(PID.e) < PID.die)//死区
	{
		return PID.u;
	}
	//PID
	PID.up=PID.e * PID.P;
	PID.ui=PID.e_acc;
	PID.ud=PID.ud * (1-PID.Dc) + ((PID.e - PID.e_pre) * PID.D / PID.T) * PID.Dc;//不完全微分
	//小范围加速逼近（P增加:1+2*sqrt(1-(e/K))）
	if(PID.high_e>0.000001) //P变参系数
	{
		tmpu=1-fabs(PID.e)/PID.high_e;
		MINMAX(tmpu,0,1);
		PID.up*=1+2*sqrt(tmpu);
	}
	//d限幅
	MINMAX(PID.ud,-PID.max_d,PID.max_d);
	//准备数据
	PID.e_pre=PID.e;
	tmpu = PID.uf + PID.up + PID.ui + PID.ud;//输出基础+P+I+D
	if(tmpu > PID.max)//限幅+抗饱和积分
	{
		tmpu=PID.max;
		if(e < 0 && e>-PID.E)//积分分离
		{
			di=PID.e*PID.I*PID.T;
		}
	}
	else if(tmpu < PID.min)
	{
		tmpu=PID.min;
		if(e > 0 && e<PID.E)//积分分离
		{
			di=PID.e*PID.I*PID.T;
		}
	}
	else
	{
		if(fabs(e) < PID.E) //积分分离
		{
			di=PID.e*PID.I*PID.T;
		}
	}
	PID.e_acc+=di;
	//积分值限幅（抑制超调）
	MINMAX(PID.e_acc,-PID.max_i,PID.max_i);
	//输出
	PID.u=tmpu;
	return PID.u;
}

