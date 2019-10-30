#ifndef MQCHART_H
#define MQCHART_H

#include "json.h"
#include "common.h"
#include <QDialog>
#include <QtCharts>
#include <QLabel>

class MQChartView : public QChartView
{
public:
	MQChartView(QWidget *parent = nullptr):QChartView()
	{
		chart0 = new QChart();
		QMargins tmpmarg(0,0,0,0);
		chart0->setMargins(tmpmarg);

		m_coord=new QGraphicsSimpleTextItem(chart0);
		QFont font;
		font.setPixelSize(25);
		m_coord->setFont(font);

		m_line=new QGraphicsLineItem(chart0);
		QPen pen(Qt::black,1,Qt::DashDotLine);
		m_line->setPen(pen);
		m_line->setLine(0,0,0,100);

		setChart(chart0);
		setRubberBand(QChartView::RectangleRubberBand);
	}
	QChart *chart0;
	QGraphicsSimpleTextItem *m_coord; //曲线值文字
	QGraphicsLineItem *m_line; //竖线
	void mousePressEvent(QMouseEvent * event)
	{
		qreal x = (event->pos()).x();
		qreal y = (event->pos()).y();
		double xVal = chart0->mapToValue(event->pos()).x();
		double yVal = chart0->mapToValue(event->pos()).y();

		//查找各曲线的值
		string s="";
		float pre_x=0;
		float pre_y=0;
		auto ser=chart0->series();
		for(int i=0;i<ser.size();i++)
		{
			QLineSeries* ls=(QLineSeries*)ser[i];
			s+=ls->name().toStdString()+":";
			auto ps=ls->pointsVector();
			for(int j=0;j<ps.size();j++)
			{
				auto &tp=ps[j];
				float tx=tp.x();
				float ty=tp.y();
				if(pre_x<xVal && xVal<=tx) 
				{
					float o0=xVal-pre_x;
					float o1=tx-xVal;
					float out=o1*pre_y + o0*ty;
					out/=(o0+o1);
					s+=sFormat("%1.0f ",out);
					break;
				}
				pre_x=tx;
				pre_y=ty;
			}
		}
		//m_coord->setPos(x+5, y);
		m_coord->setPos(5, 5);
		//m_coord->setText(QString::asprintf("x:%.3f,y:%1.0f",xVal, yVal));
		m_coord->setText(s.c_str());
		m_line->setLine(x,0,x,height());

		QChartView::mousePressEvent(event);
	}
};

#endif // 

