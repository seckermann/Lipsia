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
#ifndef VLGLWIDGET_H
#define VLGLWIDGET_H

#include <qgl.h>

#include "vlbrainplane.h"
#include "vlcamera.h"
#include "vlaim.h"

#define VIEW		1
#define DELETE		2
#define USE_DELTA	4
#define MARK		8

/**
@author Hannes Niederhausen
*/
class vlGLWidget : public QGLWidget
{
Q_OBJECT
protected:
	/** Die Ebene mit der Gehirn Textur*/
	vlBrainPlane *plane;

	/**Schnittebenentyp*/
	int type;

	/** Die Kamera des 3D Raumes*/
	vlCamera	camera;

	/** letzte X Koordinate bei einem abgefangenem MausMoveEvent*/
	int mouseX;

	/** letzte Y Koordinate bei einem abgefangenem MausMoveEvent*/
	int mouseY;

	/**Name des Widgets in abh??ngigkeit des Types*/
	QString name;

	/**Das Zielkreuz*/
	vlAim	aim;
	
	/**Gr????e der Box die zum l??schen benutzt wird*/
	int cursorSize;

	/**Die maximale Farbabweichung zur Segmentierung*/
	int m_deltaGrey;

	/**Flag f??r den L??schmodus (0 nicht l??schen, 1 komplett l??schen 2 Segment l??schen)*/
	int m_delete;

	/**
	 *    Initialisiert das Widget f??r den sp??teren Gebrauch
	 */
	void initializeGL();

	/**
	 *    Schreibt den Namen des Widgets in den oberen Bereich des Widgets
	 */
	void drawName();


	/**
	 *        Berechnet die 3DWelt Koordinaten auf der z=0 eBENE
	 *        Anhand der 2D Mauskkordinaten
	 * @param mouseX  X-Koordniante der Maus 
	 * @param mouseY  X-Koordniante der Maus 
	 * @param x X-kordinate in der 3D Welt
	 * @param y Y-kordinate in der 3D Welt
	 */
	void calculateWorldCoord(int mouseX, int mouseY, float *x, float *y);

	/**
	 *    Berechnet aus den ??bergebenen Weltkoordinaten, mit Hilde des plane-Objektes die Texturkoordinaten
	 *    (Vistakoordinaten).
	 * @param worldX 
	 * @param worldY 
	 * @param x 
	 * @param y 
	 */
	void calculateTexCoord(float worldX, float worldY, int *x, int *y);

	/**
	 *    Behandelt die gemeinsamen berechnungsroutinen f??r die MouseEvents die sich mit
	 *    der Ebenen??nderung befassen
	 * @param e 
	 */
	void commonMouseClick(QMouseEvent *e);

	/**
	 *    Bewegt den Fokus der verschiedenen Ansicht entsprechend der
	 *    Mausposition.
	 * @param px x Koordinate in Texturkoordinaten
	 * @param py y Koordinate in Texturkoordinaten
	 */
	void moveView(int px, int py);

	/**
	 *    L??scht Teile aus dem view entsprechend der Mausposition.
	 * @param px x Koordinate in Texturkoordinaten
	 * @param py y Koordinate in Texturkoordinaten
	 */
	void removePart(int px, int py);
	
	/**
	 *    Markiert ein Segment mit der in UIConfig gespeicherten Gr????e ausgehend
	 *    von der Mausposition
	 * @param px x Koordinate in Texturkoordinaten
	 * @param py y Koordinate in Texturkoordinaten
	 */
	void markPart(int px, int py);

public:
    /**
     * Der Konstruktor.
     * @param parent das Elternwidget oder NULL
     * @param type der Typ der Schnittebene die in diesem Widget dargestellt werden soll
     */
    vlGLWidget(QWidget *parent, int type=CORONAL);

    ~vlGLWidget();

    /**
     * Ver??ndert die OpenGL Perspektive entsprechend der neuen H??he/Breite
     * @param w 
     * @param h 
     */
    void resizeGL(int w, int h);

	/**
	 *    Setzt die gr????e des L??schcursors
	 * @param v 
	 */
	void setCursorSize(int v) {cursorSize=v;};

	/**
	 *    Diese Routine ist f??r das Zeichnen des Inhaltes zust??ndig
	 * @param  
	 */
	void paintGL(void);

	/**
	 *    Diese Methode wird bei jedem Mausradereignis aufgerufen.
	 * @param e 
	 */
	void wheelEvent(QWheelEvent *e);

	/**
	 *    Diese Methode reagiert auf ein Mausbewegeungsereignis.
	 * @param e 
	 */
	void mouseMoveEvent(QMouseEvent *e);

	/**
	 *    Diese Methode behandelt die Tastatur-Ereignisse
	 * @param e 
	 */
	void keyPressEvent (QKeyEvent * e);

	/**
	 *    Diese Methode behandelt die Tastatur-Ereignisse
	 * @param e 
	 */
	void mouseReleaseEvent (QMouseEvent * e);

    /**
     * Diese Methode wird aufgerufen sobald die Maustaste nach unten gedrückt wurde.
     * @param e
     */
    void mousePressEvent (QMouseEvent *e);

	/**
	 *    Setzt das delete Flag
	 * @param val 
	 */
	void setDelete(int val) {m_delete=val;};


	/**
	 *    gibt den Zustand des delete flags zur??ck
	 * @return 
	 */
	int getDelete() {return m_delete;};
	
	/**
	 * gibt das Fadenkreuzobjekt zurueck
	 */
	vlAim *getAim() {return &aim;};
	
	/**
	 * gibt die Ebene fuer die Gehirntextur zurueck
	 */
	vlBrainPlane *getBrainPlane() {return plane;};
	
	/**
	 * gibt die Kamera des aktuellen Widgets zurueck
	 */
	vlCamera * getCamera() {return &camera;};

	/**
	 *	Setzt die maximale Farbabweichung f??r die Segmentierung
	 *	@param val
	 */
	void setGreyDelta(int val) {m_deltaGrey=val;};

	/**
	 *    Setzt den Zustand des Widgets (Zielkreutposition, ausgew??hlte Schnittebene, Zoom)
	 *    auf den Defaultwert zur??ck.
	 */
	void reset();

public slots:
	virtual void recreateView(bool rebuildTex);

	virtual void reaim();
	

signals:
	/**
	 *    Das Signal um alle Texturen neu zu zeichnen.
	 * @param  
	 */
	virtual void recreateViews(bool);

	/**
	 *    Das Signal wird gesendet wenn die Maus sich bewegt. ??bergeben werden die Koordinaten
	 *    in Texturkoordinaten.
	 * @param band 
	 * @param row 
	 * @param column 
	 */
	virtual void mouseMoved(int band, int row, int column);
	
	/**
	 * Dieses Signal wird gesendet wenn Teile des Segmentes hinzugefuegt oder entfernt wurden.
	 */
	virtual void volumeChanged();

};

#endif
