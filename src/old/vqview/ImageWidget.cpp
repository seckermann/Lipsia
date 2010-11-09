#include "ImageWidget.h"

#include <math.h>

#include <qpainter.h>
#include <qpalette.h>

#define ROUND(x)	(int)(x+0.5)
#define NP -128
ImageWidget::ImageWidget(Configurator* conf, QWidget * parent, const char * name, WFlags f) :
	QWidget(parent, name, f) {
	m_configurator = conf;
	m_imgData = NULL;
	m_imgBuffer = NULL;
	m_recreate = true;
	m_xOffset = 0;
	m_yOffset = 0;
	setUpdatesEnabled(TRUE);
	setMouseTracking(true);
}

ImageWidget::~ImageWidget() {
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* e) {
	if (e->button()==Qt::LeftButton) {
		int x, y;
		getRealCoords( &x, &y, e);
		emit crossPositionChanged(this, x, y);	
		mouseMoveEvent( e);
	}
}

void ImageWidget::getRealCoords(int *x, int *y, QMouseEvent *e) {
	*x = (int) (((double) (e->x()+m_xOffset))/m_scale);
	*y = (int) (((double) (e->y()+m_yOffset))/m_scale);
	
}

void ImageWidget::leaveEvent(QEvent *) {
	m_configurator->setMousePosition(m_configurator->band(),
			m_configurator->row(), m_configurator->col());
}

void ImageWidget::wheelEvent( QWheelEvent *e) {
	double zoom = m_configurator->zoom();
	int delta = e->delta();	
	zoom += (delta<0) ? -0.05 : 0.05 ;
	if (zoom>2)
		zoom = 2;
	if (zoom<0.4)
		zoom = 0.4;
	m_configurator->setZoom(zoom);
}

void ImageWidget::mouseMoveEvent(QMouseEvent* e) {
	int x, y;
	getRealCoords( &x, &y, e);
	emit positionChanged(this, x, y);
	if (e->state()==Qt::LeftButton) {
		emit crossPositionChanged(this, x, y);
	}
}



void ImageWidget::update() {
	m_recreate = true;
	repaint(false);
}

void ImageWidget::paintEvent(QPaintEvent*) {
	QPainter painter(this);
	QPalette palette;
	if (m_imgData==NULL) {
		return;
	}
	if (m_recreate) {
		
		if (m_imgBuffer!=NULL)
			delete m_imgBuffer;
		m_imgBuffer = new QScaleImage(m_imgWidth, m_imgHeight);

		// fill the image with our imgData
		int counter = 0;
		for (int y=0; y<m_imgHeight; y++)
			for (int x=0; x<m_imgWidth; x++) {
				int val = m_imgData[counter];
				( *m_imgBuffer)(x, y) = qRgb(val, val, val);
				counter++;
			}

		// scale the image with the choosen interpolation algorithm
		CMagBase* scaler = m_configurator->currInterpolator();

		m_scaleX = width()/m_imgWidth * m_configurator->zoom();
		m_scaleY = height()/m_imgHeight * m_configurator->zoom();

		m_scale = (scaleX()<scaleY()) ? scaleX() : scaleY();
		scaler->setScalingRatio(m_scale);

		if (m_configurator->interpolationtype()==ip_bicub) {
			dynamic_cast<CBicubSplineMag*>(scaler)->setParam(-0.5);
		}

		//scaling image
		QScaleImage* tmp = (*scaler)(*m_imgBuffer);
		delete m_imgBuffer;
		m_imgBuffer = tmp;
		//m_recreate = false;
	}
	
	int cx = (int)(m_crossX*m_scale);
	int cy = (int)(m_crossY*m_scale);
	
	// check if we need to move offsets
	int leftBorder = width()-40;
	int bottomBorder = height() -40;
	if (cx>leftBorder) {
		m_xOffset = cx-leftBorder;
		if ( (width()+m_xOffset) > m_imgBuffer->width() )
			m_xOffset = (m_imgBuffer->width() - width());
		// should never happen, but it does...
		if (m_xOffset<0)
			m_xOffset = 0;
		cx -=m_xOffset;
	} else {
		m_xOffset = 0;	
	}
	if (cy>bottomBorder) {
		m_yOffset = cy-bottomBorder;
		if ( (height()+m_yOffset) > m_imgBuffer->height() )
			m_yOffset = (m_imgBuffer->height() - height());
		// should never happen, but it does...
		if (m_yOffset<0)
			m_yOffset = 0;
		cy -= m_yOffset;
	} else {
		m_yOffset=0;
	}
	
	painter.setBrush(Qt::black);
	painter.drawImage(-m_xOffset, -m_yOffset, *m_imgBuffer);
	
	// only draw the region next to the imgBuffer black to avoid flickering
	painter.drawRect(0, m_imgBuffer->height(), width(), height()-m_imgBuffer->height());
	painter.drawRect(m_imgBuffer->width(), 0, width()-m_imgBuffer->width(), height());
	// drawing a aiming cross
	painter.setPen(QColor(255, 255, 255));
	painter.drawLine(0, cy, cx-10, cy);
	painter.drawLine(cx+10, cy, width(), cy);
	
	painter.drawLine(cx, 0, cx, cy-10);
	painter.drawLine(cx, cy+10, cx, height());
	
	painter.drawPoint(cx, cy);
	
}
