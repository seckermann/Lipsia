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
#ifndef VLBRAINPLANE_H
#define VLBRAINPLANE_H

#include <vistaimage.h>
#include <GL/gl.h>

//wenn CORONAL nicht definiert is, ist keins der Makros defniert
#ifndef CORONAL
	#define CORONAL			0
	#define SAGITTAL		2
	#define AXIAL			4
#endif

/**
 * Diese Klasse stellt die texturierte Ebene dar.
@author Hannes Niederhausen
*/
class vlBrainPlane{
protected:
	// Texture IDs
	GLuint m_texID;
	GLuint m_selID;

	/**
	 * Anatomie
	 */

	// die Maﬂe der Anatomie
	int m_anaWidth;
	int m_anaHeight;
	
	// die Maﬂe der Anatomietextur
	int m_anaTexWidth;
	int m_anaTexHeight;
	//
	// die Position der Ebene innerhalb der Anatomie
	int m_anaPlanePos;
	// die maximale Position der Ebene innerhalb der Anatomie
	int m_anaMaxPos;
	
	/**
	 * Selektion
	 */
	
	// Die Maﬂe der Selektion
	int m_selWidth;
	int m_selHeight;
	
	// Die Maﬂe der Selektionstextur
	int m_selTexWidth;
	int m_selTexHeight;
	
	// die Position der Ebene innerhalb des Selektion
	int m_selPlanePos;
	// die maximale Position der Ebene innerhalb der Selektion
	int m_selMaxPos;
	

	short m_type;
	
	/** The texture filter for the anatomie AND segmentation texture.
	 * Valid values are GL_NEAREST and GL_LINEAR according to the OpenGL
	 * documentation for glTexParameterf(). */
	GLfloat m_filter;

	/**
	 *    Gibt ein Array der 2D Daten zurueck oder NULL falls keine Daten vorhanden
	 * @param selection Wenn <i>true</i> werden die Daten aus der Auswahl genommen
	 * ansonst aus der Anatomie.
	 * @return 
	 */
	template <class T> T* getData(bool selection);	

	/**
	 * Sets the the geometry values for anatomie and selection images.
	 */
	void updateImageSize();

	/**
	 * Returns the next power value of two greater or equal than the given argument.
	 */
	int getValidSize(int arg);

	/**
	 *    Die Zeichenroutine f??r das Rechteck
	 */
	void drawQuad();
	
	/**
	 *    Initialisiert die Funktionen f¸r Multitexturing
	 */
	void initExtensions();

	/**
	 *   Transformiert Koordinaten der Anatomietextur aud Koordinaten der 
	 *   Selektionstextur. Dabei wird der Bildbreich NICHT verlassen.
	 */
	void transformCoords(int, int, int&, int&);
	
public:
    vlBrainPlane(int type);
    ~vlBrainPlane();

	void createTexture(bool newImage);
	void draw();

	void move(int delta);

	int width(){return m_anaWidth;};

	int height(){return m_anaHeight;};
	
	void setWidth(const unsigned short width) {m_anaWidth = width;};
	void setHeight(const unsigned short height) {m_anaHeight = height;};
	
	int texHeight() {return m_anaTexHeight;};
	
	int texWidth() {return m_anaTexWidth;};

	void setPlane(int p);
	
	int getPlane() {return m_anaPlanePos;};
	
	void deleteVoxel(int x, int y, int radius);

	void deleteSegment(int x, int y, int radius, int delta);
	
	void markSegment(int x, int y);
	
	void markVoxel(int x, int y);
	
	/** Refreshs the current value of the filter from UICONFIG */
	void refreshFilter();
};

#endif
