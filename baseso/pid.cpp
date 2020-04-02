/*
文件名：pid.cpp
时间：2013-9-11
功能：
	通用pid和简单系统辨识
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
		PID.e=0;
		//return PID.u;
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
////////////////////////////////////////////////////////////////////////////////
//简单辨识：
//y=(1-k)y + kAmpx    k是系统参数，Amp是增益
//y=Ay + Bx
float srec_run(float x,float y1,SIMP_REC *sr)
{
	return (1-sr->k)*y1 + sr->k*sr->Amp*x;
}
void srec_ini(SIMP_REC *sr)
{
	sr->k=0.1f;//float k; //系统参数
	sr->Amp=1;//float Amp; //增益
	sr->x1=0;//float x1; //前1个值
	sr->y1=0;//float y1; //前1个值
	sr->y2=0;//float y2; //前2个值
	sr->learn_speed=0.2f;//float learn_speed; //学习速度
	sr->stat=0;//int stat; //0为初始化，1为第一个值，2为计算
	sr->err=0;//int err; //错误，0正常，1出错
}
void srec_learn(float x,float y,SIMP_REC *sr)
{
	if(sr->stat==0) //若是初始化
	{
		sr->stat=1;
		goto END;
	}
	else if(sr->stat==1)
	{
		sr->stat=2;
	}
	else if(sr->stat==2)
	{
		sr->err=0;
		//第二次，开始计算
		//Ay1 + Bx = y
		//Ay2 + Bx1 = y1
		float y1=sr->y1,y2=sr->y2,x1=sr->x1;
		//Ay1y2 + Bxy2 = yy2
		//Ay2y1 + Bx1y1 = y1y1
		//B=(yy2-y1y1)/(xy2-x1y1)
		float tmp=x*y2-x1*y1;
		if(fabs(tmp)<1e-6)
		{
			sr->err=1;
			goto END;
		}
		float B=(y*y2-y1*y1)/tmp;
		if(fabs(y1)<1e-6)
		{
			sr->err=2;
			goto END;
		}
		float A=(y - B*x)/y1;
		//printf("	A:%.1f*y1:%.1f + B:%.1f*x:%.1f - y:%.1f\n", A,y1,B,x,y);
		//更新变量
		float k=1-A;
		if(k<0 || k>(1-1e-6))
		{
			printf("k=%.5f\n",k);
			sr->err=3;
			goto END;
		}
		float Amp=B/k;
		if(Amp<0)
		{
			sr->err=4;
			goto END;
		}
		sr->k=sr->k*(1-sr->learn_speed) + sr->learn_speed*k;
		sr->Amp=sr->Amp*(1-sr->learn_speed) + sr->learn_speed*Amp;
		//printf("	k:%.3f,Amp:%.1f\n",k,Amp);
		//最后改状态
		sr->stat=1;
	}
END:
	//状态变量向后滑动
	sr->y2=sr->y1;
	sr->x1=x; sr->y1=y;
}

