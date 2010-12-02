#ifndef IMAGEWIDGET_H_
#define IMAGEWIDGET_H_

#include <qwidget.h>
#include <qevent.h>
#include <qcolor.h>

#include "Configurator.h"
#include "interpolator.hh"

class ImageWidget : public QWidget
{
	Q_OBJECT
public:
	ImageWidget ( Configurator *conf, QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
	virtual ~ImageWidget();

	void setImageData( unsigned char *data, int width, int height ) {
		m_imgWidth = width;
		m_imgHeight = height;
		m_imgData = data;
	};

	int imgWidth() {return m_imgWidth;};
	int imgHeight() {return m_imgHeight;};

	double scaleX() {return m_scaleX;};
	double scaleY() {return m_scaleY;};

	int xOffset() {return m_xOffset;};
	int yOffset() {return m_yOffset;};

	void setCrossPos( int crossX, int crossY ) {
		m_crossX = crossX;
		m_crossY = crossY;
	};

	void update();

signals:
	void positionChanged( ImageWidget *, int x, int y );
	void crossPositionChanged( ImageWidget *, int x, int y );

protected:
	void paintEvent( QPaintEvent * );
	void mouseMoveEvent( QMouseEvent * );
	void mouseReleaseEvent( QMouseEvent *e );
	void leaveEvent( QEvent * );
	void wheelEvent ( QWheelEvent *e );


private:
	void getRealCoords( int *x, int *y, QMouseEvent *e );

	unsigned char *m_imgData;
	int m_imgWidth;
	int m_imgHeight;

	int m_xOffset;
	int m_yOffset;

	double m_scaleX;
	double m_scaleY;

	double m_scale;

	bool m_recreate;

	int m_crossX;
	int m_crossY;

	QScaleImage *m_imgBuffer;

	Configurator *m_configurator;

};

#endif /*IMAGEWIDGET_H_*/
