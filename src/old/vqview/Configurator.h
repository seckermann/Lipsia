#ifndef CONFIGURATOR_H_
#define CONFIGURATOR_H_

#include <qobject.h>
#include <qvaluelist.h>
#include <qstring.h>

#include <iostream>
#include <memory>
#include <math.h>

#include "interpolator.hh"
#include "VImageManager.h"
/**
 * This class contains all the data of an running application.
 *
 * e.g. The current filename, the VImageContainer
 * brightness and contrast for rendering the images, current time
 */
class Configurator : public QObject
{
	Q_OBJECT
public:
	Configurator( int *argc, char **argv );
	virtual ~Configurator();

	void setInterpolationtype( interpol_type t );
	interpol_type interpolationtype() {
		return m_interpolationtype;
	};
	CMagBase *currInterpolator() {
		return m_currInterpolator;
	};
	float zoom() {
		return m_zoom;
	};

	QString filename() {return m_filename;};
	VImageManager &imageManager() {return m_imgManager;};

	void lockPosition();
	void setPosition( int band, int row, int col );
	void setMousePosition( int band, int row, int col );

	void setMouseValid( bool val ) {
		m_mouseValid = val;
		emit mousePositionChanged();
	}

	bool mouseValid() {
		return m_mouseValid;
	}

	void setZoom( double zoom ) {
		m_zoom = zoom;
		emit zoomChanged();
	};

	void setTime( int time ) {
		m_time = time;

		if ( time >= imageManager().numOfTimeSteps() )
			m_time = 0;

		m_imgManager.updateViewData( m_col, m_row, m_band, m_time );
		emit timeValueChanged();
	};

	void setFps( int fps ) {
		m_fps = fps;
	};

	int time() {
		return m_time;
	};

	int numOfTimeValues() {
		return m_imgManager.numOfTimeSteps();
	};

	int row() {
		return m_row;
	};

	int band() {
		return m_band;
	};

	int col() {
		return m_col;
	};

	int mouseRow() {
		return m_mouseRow;
	};

	int mouseBand() {
		return m_mouseBand;
	};

	int mouseCol() {
		return m_mouseCol;
	};

	void setBrightness( int b ) {
		m_brightness = b;
		float stretch = 200;
		float ymax = ( float )m_imgManager.xmax();
		float ymin = ( float )m_imgManager.xmin();
		float ymaxmin = ( float )( ymax - ymin );
		m_shift = ( float )b * ( ymax - ymin ) / m_stretch;
		m_imgManager.setAnaalpha( m_factor * 255.0 / ymaxmin );
		m_imgManager.setAnamean( ymin - m_shift + ( ymaxmin - 255.0 / m_imgManager.getAnaalpha() ) / 2.0 );
		m_imgManager.updateViewData( m_col, m_row, m_band, m_time );
		emit colorsChanged();
	};

	void setContrast( int c ) {
		m_contrast = c;
		double stretch = 1.02;
		float ymax = ( float )m_imgManager.xmax();
		float ymin = ( float )m_imgManager.xmin();
		float ymaxmin = ( float )( ymax - ymin );
		m_factor = ( float )pow( m_stretchfact, ( double )c );
		m_imgManager.setAnaalpha( m_factor * 255.0 / ymaxmin );
		m_imgManager.setAnamean( ymin - m_shift + ( ymaxmin - 255.0 / m_imgManager.getAnaalpha() ) / 2.0 );
		m_imgManager.updateViewData( m_col, m_row, m_band, m_time );
		emit colorsChanged();
	};

	double contrast() {
		return m_contrast;
	};

	double brightness() {
		return m_brightness;
	};

	int fps() {
		return m_fps;
	};

public slots:
	void changeTimeValue( int val ) {
		setTime( val );
	}

signals:
	void mousePositionChanged();
	void crossPositionChanged();
	void timeValueChanged();
	void zoomChanged();
	void colorsChanged();

private:

	void parseVistaOptions( int *argc, char **argv );

	int m_brightness;
	int m_contrast;
	float m_shift;
	float m_factor;
	float m_stretch;
	double m_stretchfact;

	int m_currentTime;
	float m_zoom;

	interpol_type  m_interpolationtype;

	QString m_filename;
	CMagBase *m_currInterpolator;
	VImageManager m_imgManager;

	// the position of the cross
	int m_row;
	int m_band;
	int m_col;

	// the position of the mouse
	int m_mouseRow;
	int m_mouseBand;
	int m_mouseCol;
	// flag if mouse position is used
	bool m_mouseValid;

	// the frames per second
	int m_fps;

	// the timer position
	int m_time;
	// the maximum time value
	int m_maxTime;
};

#endif /*CONFIGURATOR_H_*/
