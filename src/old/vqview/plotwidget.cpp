#include "plotwidget.h"

#include <qpainter.h>
#include <qfontmetrics.h> 
#include <qpointarray.h>

// margins for painting area
#define   TOP_MARGIN	10
#define   BOTTOM_MARGIN 25
#define	  LEFT_MARGIN	10
#define	  RIGHT_MARGIN  10

#define  ROUND(x)	((int) (x+0.5))

PlotWidget::PlotWidget(Configurator* conf, QWidget * parent, const char * name)
 : QWidget(parent, name, 0)
{
	setMinimumSize(600, 200);
	m_conf = conf;
	connect(m_conf, SIGNAL(crossPositionChanged()),
			this, SLOT(update()));
	m_buffer=NULL;
	m_drawGrid = true;
	m_sync = true;
}


PlotWidget::~PlotWidget()
{
}

void PlotWidget::resizeEvent(QResizeEvent*)
{
	update();
}

void PlotWidget::paintEvent(QPaintEvent*)
{

	if (m_buffer==NULL) {
		m_buffer = new QPixmap(width(), height());
	
		QPainter *painter = new QPainter(m_buffer);
		QFontMetrics fm = painter->fontMetrics();
		
		if ( (m_minVal==0)  && (m_maxVal==0) ) {
			painter->setFont(QFont("arial", 20));
			
			QString text = "Raw data is constant 0";
			int tw = fm.width(text);
			painter->setBrush(Qt::white);
			painter->drawRect(0, 0, width(), height());
			painter->drawText(width()/2-tw/2, height()/2, text);
			
		} else {
			painter->setFont(QFont("arial", 10));
			// clear image to white background
			painter->setBrush(Qt::white);
			painter->drawRect(0,0, width(), height());
			
			painter->setBrush(Qt::black);
			
			// calulating the axes
			QFontMetrics fm = painter->fontMetrics();
			int tw = fm.width(QString::number(m_minVal));
			
			int minY = TOP_MARGIN;
			int maxY = height() - BOTTOM_MARGIN;
			int minX = tw+LEFT_MARGIN+5;
			int maxX = width()-RIGHT_MARGIN;
			
			painter->drawLine(minX, minY, minX, maxY);		// y-axis
					
			painter->drawLine(minX-5, maxY, maxX, maxY); // x-axis, including minimum line for value
			
			// drawing min value
			painter->drawText(LEFT_MARGIN, height()-(BOTTOM_MARGIN-(fm.height()/2)), QString::number(m_minVal));
			
			drawGrid(painter, QRect(minX, minY, maxX-minX, maxY-minY));
			drawCurve(painter, QRect(minX, minY, maxX-minX, maxY-minY));
		}
		// deleting buffer painter so we can delete the buffer when it's time
		delete painter;
	}
	QPixmap tmp = *m_buffer;
	QPainter* p = new QPainter(this);
	p->drawPixmap(0, 0, tmp);
	delete p;
}

void PlotWidget::drawCurve(QPainter* p, QRect bbox)
{
	QPointArray points(m_conf->numOfTimeValues());
	
	int x = 0;
	int y = 0;
	
	
	int lastX = -1;
	int minX = bbox.x();
	int minY = bbox.y()+bbox.height();	// the y coord of the min value..
	
	
	double yStep = (bbox.height())/(double)(m_maxVal-m_minVal);
	double xStep = (bbox.width()/(double)m_conf->numOfTimeValues());
	
	int pointIndex = 0;
	
	for (int t=0; t<m_conf->numOfTimeValues(); t++) {
		int val = m_conf->imageManager().getValue(m_band, m_row, m_col, t)-m_minVal;
		
		y = ROUND(minY - (val*yStep));
		x = ROUND(minX + (t*xStep));
		// we draw only the points with new x coords
		if (x!=lastX) {
			points.setPoint(pointIndex, x, y);
			pointIndex++;
			lastX = x;
		}
	}
	
	//p->drawPolyline(points, 0, pointIndex);
	
	QPoint p1 = points.point(0);
	QPoint p2 = points.point(0);
	for (int i=1; i<pointIndex; i++) {
		p1 = p2;
		p2 = points.point(i);
		p->drawLine(p1, p2);
	}
	
	
}

void PlotWidget::drawGrid(QPainter* p, QRect bbox)
{	
	
	int nSteps = 1;
	if (m_drawGrid) {
		nSteps = 5;
	}
	
	int heightStepPx = bbox.height()/nSteps;
	int stepVal = (m_maxVal-m_minVal)/nSteps;
	QFontMetrics fm = p->fontMetrics();
	int currY = bbox.y()+bbox.height()-heightStepPx;
	int currVal = m_minVal+stepVal;
	
	int tw = fm.width(QString::number(m_minVal))+5;
	
	for (int i=0; i<nSteps-1; i++) {
		p->drawLine(bbox.x(), currY, bbox.x()+bbox.x(), currY);
		p->setPen(QColor(133, 133, 133));
		p->drawLine(bbox.x(), currY, bbox.x()+bbox.width(), currY);
		p->setPen(QColor(0, 0, 0));
		p->drawLine(bbox.x()-5, currY, bbox.x(), currY);
		p->drawText(bbox.x()-tw, currY+(fm.height()/2), QString::number(currVal));
		currY-=heightStepPx;
		currVal+=stepVal;
	}
	
	if (m_drawGrid) {
		p->setPen(QColor(133, 133, 133));
		p->drawLine(bbox.x(), bbox.y(), bbox.x()+bbox.width(), bbox.y());
	}
	p->setPen(QColor(0, 0, 0));
	p->drawLine(bbox.x()-5, bbox.y(), bbox.x(), bbox.y());
	p->drawText(bbox.x()-tw, bbox.y()+(fm.height()/2), QString::number(m_maxVal));
	
	// drawing vertical lines...
	stepVal = m_conf->numOfTimeValues()/16;
	int widthStepPx = bbox.width()/16;
	
	currVal = stepVal;
	int currX = bbox.x()+widthStepPx;
	
	currY = bbox.y()+bbox.height();
	for (int i=0; i<16; i++) {
		p->drawLine(currX, currY, currX, currY+5);
		
		tw = currX - fm.width(QString::number(currVal))/2;
		p->drawText(tw, currY+5+fm.height(), QString::number(currVal));
		if (m_drawGrid) {
			p->setPen(QColor(133, 133, 133));
			p->drawLine(currX, currY, currX, bbox.y());
			p->setPen(QColor(0, 0, 0));
		}
		
		currX+=widthStepPx;
		currVal+=stepVal;
	}
		
	
}

void PlotWidget::update() 
{
	if (m_sync) 
		updateImage();
}

void PlotWidget::updateImage() 
{
	if (m_sync) {
		m_band = m_conf->band();
		m_row = m_conf->row();
		m_col = m_conf->col();
		
		m_minVal =  32000;
		m_maxVal = -32000;
	
		for (int i=0; i<m_conf->numOfTimeValues(); i++) {
			int val = m_conf->imageManager().getValue(m_band, m_row, m_col, i);
			if (val <m_minVal)
				m_minVal = val;
			if (val >m_maxVal)
				m_maxVal = val;
		}
		if ( (m_maxVal!=0) && (m_minVal!=0) ) {
			int dist = (int) ((m_maxVal-m_minVal)*0.1);
			m_minVal -= dist;
			m_maxVal += dist;
		}
	}
	if (m_buffer!=NULL) {
		delete m_buffer;
		m_buffer = NULL;
	}

	repaint();
}
