/*
文件名：pid.h
作者：北京交通大学 自控1102 杨孜
时间：2013-9-11
功能：

*/
#ifndef PID_H
#define PID_H

#include "main.h"

typedef struct
{
//状态
	float e;//误差
	float e_pre;//上次误差
	float e_acc;//累计误差
	float up;//P输出
	float ui;//I输出
	float ud;//D输出
	float uf;//前馈输出，要求外部输入,外部可做复杂算法
	float u;//总输出
//参数
	float k_in;
	float P;
	float I;
	float D;
	float T;//单位秒
	float max;//输出上限
	float min;//输出下限
//扩展参数
	float E;//积分分离阈值（若不用，则置为最高误差输入）
	float Dc;//D环节低通系数
	float die;//输入死区绝对值,死区保持输出不变(若不用，则置零)
//加速逼近
	float max_i;//I环节累积值最大值（若不用，则置为max）
	float max_d;//d环节最大值,（若不用，则置为max或以上）
	float high_e;//小范围逼近e阈值（若不用，则置为0）
} PID_CON;

//基础pid，实现的变形包括：
//1、输入滤波
//2、积分分离
//3、抗饱和积分
//4、不完全微分
//5、输入死区
//6、小范围加速逼近（P增加:1+2*sqrt(1-(e/K))）
//7、积分值限幅（抑制超调）
//8、D值限幅（抑制超调）
float base_pid(float e,PID_CON *p);//传入期望值、传感值

/////////////////////////////////////////////////////////////////////
//简单系统辨识，1阶系统，增量学习，两次输入更新一次参数
//参数包括：
//	k，系统参数，是输入的系数
//	Amp，是输入增益，由于没有去偏置，所以不太准
typedef struct
{
	float k; //系统参数
	float Amp; //增益
	float x1; //前1个值
	float y1; //前1个值
	float y2; //前2个值
	float learn_speed; //学习速度
	int stat; //0为初始化，1为第一个值，2为计算
	int err; //错误，0正常，>=1出错
} SIMP_REC;
void srec_ini(SIMP_REC *sr);
float srec_run(float x,float y1,SIMP_REC *sr);
void srec_learn(float x,float y,SIMP_REC *sr);

#endif

