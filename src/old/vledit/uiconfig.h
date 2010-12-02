/***************************************************************************
 *   Copyright (C) 2005 by Hannes Niederhausen                             *
 *   niederhausen@cbs.mpg.de                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef UICONFIG_H
#define UICONFIG_H

#include <qobject.h>
#include <map>
#include <vector>

#define UICONFIG    UIConfig::instance()
/* Obere Skalierungsgrenze */
#define UPPER_SCALE_LIMIT 10.0
/* Untere Skalierungsgrenze */
#define LOWER_SCALE_LIMIT 0.2
/* Skalierungsschritt */
#define SCALE_STEP 0.5

// Die beiden Darstellungsfilter, die von OpenGL unterstuetzt werden.
#define NN_FILTER "NN"
#define BILIN_FILTER "Bilin"

// Die Anzeigemodi fuer die Koordinaten in der Statusleiste.
#define ANA_MODE "Ana"
#define SEG_MODE "Seg"
#define TAL_MODE "Tal"

/**
 * Diese Klasse enth&auml;t alle globalen Konfigurationsdaten, wie Zoom,
 * Maus- und Zielkreuzkoordinaten, Darstellungsfilter und Bearbeitungsmodus.
 * @author Hannes Niederhausen
 */
class UIConfig : public QObject
{
	Q_OBJECT
protected:
	/** Die Instanz des Singletons*/
	static UIConfig *m_instance;

	/**der Skalierungsfaktor fuer die Ansichten*/
	float m_Scale;

	/**band position of aim*/
	float m_band;
	/**row position of aim*/
	float m_row;
	/**column position of aim*/
	float m_column;
	/**current number of segment*/
	int m_currSegmentNumber;
	/**delta value for segmentation*/
	int m_delta;
	/**radius of the sphere*/
	int m_radius;
	/** Der Bearbeitungsmodus in dem vledit gestartet wurde. 0 = ana, 1 = seg */
	short mode;
	/** currently selected zoom filter */
	QString m_filter;
	/** sorted list for managing saving and printing order of the segments. This
	 * vector holds a list of segment ids in ascending order. This means that the
	 * segment with the lowest drawing and saing priority comes first and the
	 * most dominant segment comes last. In the segment window, the segments are
	 * shown in reversed order with the most important segment first. */
	std::vector<int> m_seg_list;
	// Der Modus in dem die Koordinaten in der Statusleiste angezeit werden.
	QString m_coordMode;
	// the last known mouse position
	int m_mousePos[3];

	// the currently selected delta treshold value
	int m_delta_treshold;

	// translate camera view?
	bool m_translate;

	// synchronize with vlserv?
	bool m_sync;

	// is aim visible
	bool m_aim;

	UIConfig();

public:

	/**
	 *    Gibt die Instanz des Singletons zurueck
	 * @return
	 */
	static UIConfig *instance() {
		if ( m_instance == NULL )
			m_instance = new UIConfig();

		return m_instance;
	};


	~UIConfig();

	/**
	 *    addiert zum derzeitigen Zoomfaktor den Wert Scale und ueberprueft dann den Zoomfaktor auf
	 *    die Grenzen des Intervalles [LOWER_SCALE_LIMIT,UPPER_SCALE_LIMIT]. Werden diese ????berschritten wird der Zoomfaktor
	 *    gleich der Grenze gesetzt.
	 * @param scale
	 * @return
	 */
	void addScale( float scale );

	/**
	 *    Setzt den Zoomfaktor auf den Wert von scale, wenn er im Intervall [LOWER_SCALE_LIMIT,UPPER_SCALE_LIMIT] ist,
	 *    sonst an die Grenzen.
	 * @param scale
	 */
	void setScale( float scale );


	/**
	 *    returns zoom factor of views
	 * @return
	 */
	float scale() {return m_Scale;};

	/**
	 *    returns aim column position
	 * @return
	 */
	float column() {return m_column;};

	/**
	 *    returns aim band position
	 * @return
	 */
	float band() {return m_band;};

	/**
	 *    returns aim row position
	 * @return
	 */
	float row() {return m_row;};

	/**
	 *    the radius of the editing sphere
	 * @return
	 */
	int radius() {return m_radius;};

	/**
	 *    returns delta value
	 * @return
	 */
	int delta() {return m_delta;};


	/**
	 *    sets aim row position
	 * @param row
	 */
	void setRow( float row );

	/**
	 *    sets aim column position
	 * @param column
	 */
	void setColumn( float column );

	/**
	 *   sets aim band position
	 * @param row
	 */
	void setBand( float band );

	void mouseReleased();


	/**
	 * Sets the aimposition to the given values. if a value is -1, the value is
	 * ignored.
	 * If a value is larger than the size of the currently loaded image, the
	 * value is set to this maximum. The given parameters are texture
	 * coordinates without zoom.
	 * @param band the band position
	 * @param row  the row position
	 * @param column the column position
	 */
	void setAim( float band, float row, float column );

	void setDelta( int delta ) {m_delta = delta;};

	void setRadius( int radius ) {m_radius = radius;};

	/**
	 *    Setzt die aktuelle Nummer fuer die Segmentierungsauswahl
	 * @param i segment nummer
	 */
	void setCurrSegmentNumber( int i ) {
		m_currSegmentNumber = i;
		emit recreateViews( false );
	};

	/**
	 *    Gibt die Segmentnummer zur??ck.
	 * @return
	 */
	int currSegmentNumber() {return m_currSegmentNumber;};


	/**
	 * Global verfuegbarer Indikator für Debugprozesse. Dieses Flag hat die Wert 0 (false) und 1 (true).
	 * Dieses Flag dient zur Unterstützung der konditionalen Breakpoints des Debuggers. Konditionale
	 * Brekapoints können auf den Wert dieses Flags testen.
	 */
	short debug;

	/** Returns the current editing mode as const VShort value. Valid values are
	* 0 and 1 */
	const short getMode() {
		return mode;
	}

	/** Sets the current editing mode to the given value. */
	void setMode( const short mode ) {
		this->mode = mode;
	}

	/** sets the current filter to the given value and emits a filterChanged signal. */
	void setFilter( const QString );

	/** returns the name value of the currently set rendering filter */
	QString getFilter() { return m_filter; }

	/** a wrapper method to send the recreateViews(bool) signal */
	void recreateViews();

	/** Returns a pointer to segments order vector. This can be used to
	 * manipulate  the segments order. the entries are in ascending order
	 * with the most important object at the end of the list.*/
	std::vector<int>* segList() {return &m_seg_list;}

	/** Returns the id of the segment in the given table row.
	 * @param row The row index ni the color or volume table
	 * @return The id of the segment in this row
	 */
	int idByRow( int row );

	// returns the currently selected delta treshold value
	int deltaTr() { return m_delta_treshold; }

	// Sets the currently selected delta treshold to the given value. This should be done with
	// a mousePressEvent in vlGLWindow.
	void setDeltaTr( int dtr ) {m_delta_treshold = dtr;}

	// This flag indicates, that the camera should be translated to given position
	void setTranslate( bool trans ) { m_translate = trans; }

	// This flag indicates, that the camera should be translated to given position
	bool translate() { return m_translate; }

	// Returns the configured display mode for coordinates.
	QString coordMode() { return m_coordMode;}

	// Sets the configured display mode for coordinates.
	void setCoordMode( QString cmode ) { m_coordMode = cmode; }

	// Returns the last known mouse position
	int *mousePos() { return m_mousePos; }

	// Sets the last known mouse pos to the given voxel coordinates.
	void setMousePos( int, int, int );

	// get syncronization state
	bool sync() { return m_sync; }

	// set synchronization state; FALSE: No sync with server, TRUE: synchronize with server
	void setSync( bool sync ) { m_sync = sync; if ( sync ) emit sendCoordinates( 1 );}

	// should the red crosshairs be displayed or not?
	bool isAimVisible() { return m_aim; }

	// sets the visibility of the red crosshairs.
	void setAimVisible( bool aim ) {
		m_aim = aim;
	}

	//***********DEBUG*******************
	void checkList();
	//***********DEBUG*******************

signals:
	/**
	 *    This signal is send if the aim position changed
	 */
	void aimChanged();

	/**
	 *    This signal is send if the scale changed
	 */
	void scaleChanged();

	/**
	*   This signal is send if the selected segment id has changed
	*/
	void recreateViews( bool );

	/**
	*   This signal is send if new coords should ne transmitted
	*/
	void sendCoordinates( int );

	/**
	 *  This signal is send when the filter value has been changed interally
	 */
	void filterChanged( const QString &filter );

};

#endif
