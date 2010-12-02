#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qrect.h>

#include "Configurator.h"

/**
    @author Hannes Niederhausen <niederhausen@cbs.mpg.de>
*/
class PlotWidget : public QWidget
{
	Q_OBJECT
public:
	PlotWidget( Configurator *conf, QWidget *parent = 0, const char *name = 0 );

	~PlotWidget();

	void setSync( bool sync ) {
		m_sync = sync;
		update();
	};

	void setDrawGrid( bool drawGrid ) {
		m_drawGrid = drawGrid;
		updateImage();
	};

protected slots:
	void update();

protected:
	void paintEvent( QPaintEvent * );
	void resizeEvent( QResizeEvent * );


private:
	void updateImage();
	void drawGrid( QPainter *painter, QRect bbox );
	void drawCurve( QPainter *, QRect );

	int m_minVal;
	int m_maxVal;
	bool m_sync;
	bool m_drawGrid;
	int m_band;
	int m_row;
	int m_col;

	QPixmap *m_buffer;

	Configurator *m_conf;



};

#endif
