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
 *
 *   $Id: vlbrainplane.cpp 3581 2009-06-04 07:33:10Z proeger $ 
 ***************************************************************************/

#include "vlbrainplane.h"
#include "datamanager.h"
#include "uiconfig.h"

#define GL_GLEXT_LEGACY
#define GLX_GLXEXT_LEGACY
#include <GL/glx.h>
#include <GL/glext.h>
#include <vector>

#include <qstring.h>

PFNGLACTIVETEXTUREARBPROC 	pglActiveTextureARB=NULL;
PFNGLMULTITEXCOORD2FARBPROC pglMultiTexCoord2fARB=NULL;

vlBrainPlane::vlBrainPlane(int type)
{
    initExtensions();
    m_type=type;
    glGenTextures(1, &m_texID);
    glGenTextures(1, &m_selID);
    m_anaPlanePos = -1;
    m_selPlanePos = -1;
    createTexture(true);
}

vlBrainPlane::~vlBrainPlane()
{
    glDeleteTextures(1, &m_texID);
    glDeleteTextures(1, &m_selID);
}

/* Wird zur Zeit ausschliesslich in createTexture(bool) verwendet. */
template <class T> T* vlBrainPlane::getData(bool selection)
{

    // temporäre Variablen
    T* array;
    int* selectionColor;
 
	// ANATOMIEDATEN
    if (!selection){
		IVistaImage* img=DATAMANAGER->image();

		 // Array mit den Bilddaten des VImages
	    // (band x row x column)
	    T*** tmpArray=VPixelArray(img->src(), T);
	
	    int pos=0;
	
	    // Array mit Pixelwerten für die Textur.
	    array=new T[m_anaTexWidth*m_anaTexHeight];
	    //alle Daten auf 0 setzen
	    memset(array,0,sizeof(T)*m_anaTexWidth*m_anaTexHeight);
	    //füllen der Bilddaten in das array
	    for (int i=0; i<m_anaHeight; i++) {
			for (int j=0; j<m_anaWidth; j++) {
			    switch (m_type) {
				case AXIAL:
				    array[pos] = tmpArray[m_anaPlanePos][i][j];
				    break;
				case SAGITTAL:
				    array[pos] = tmpArray[i][j][m_anaPlanePos];
				    break;
				case CORONAL:
				    array[pos] = tmpArray[i][m_anaPlanePos][j];
			    }
			    pos++;
			}
			pos+=(m_anaTexWidth-m_anaWidth);
	    }
	    return array;
    }
    // SEGMENTATIONSDATEN
    else{
        // Array mit Pixelwerten für die Textur.
        array=new T[m_selTexWidth*m_selTexHeight*4];
        //alle Daten auf 0 setzen
        memset(array,0,sizeof(T)*m_selTexWidth*m_selTexHeight*4);
        
        // Iterator ueber die geordnete List aller Segmente
        vector<int>::iterator iter = UICONFIG->segList()->begin();
        while(iter != UICONFIG->segList()->end()) {
            
            VistaSegment* seg;
            seg=DATAMANAGER->segment(*iter);
            if(seg->visible) {
      
                //Farbwert des Segmentes laden
                selectionColor=seg->color;
            
                // Array mit den Bilddaten des VImages
                // (band x row x column)
                T*** tmpArray=VPixelArray(seg->src(), T);
        
                // Die Position innerhalb des Pixelarrays
                int pos=0;
            
                // Der Pixelwert innerhalb der internene Segmentrepreaesentation
                short bitVal = 0;
                
                //Füllen der Bilddaten in das array
                for (int i=0; i<m_selHeight; i++) {
                    for (int j=0; j<m_selWidth; j++) {              
                        switch (m_type) {
                        case AXIAL:
                            bitVal = tmpArray[m_selPlanePos][i][j];
                            break;
                        case SAGITTAL:
                            bitVal = tmpArray[i][j][m_selPlanePos];
                            break;
                        case CORONAL:
                            bitVal = tmpArray[i][m_selPlanePos][j];
                            break;
                        default:
                            bitVal = 0;
                        }
                        // Falls an dieser Stelle ein Farbwert ist und noch kein
                        // anderer Farbwert definiert ist: 
                        if(bitVal != 0) {
                            // Eintragen der RGBA-Komponenten der Farbe
                            for(int c=0;c<4;c++) {
                                array[pos++] = selectionColor[c];
                            }
                        }else { // Ansonst die naechsten vier '0'en ueberspringen.
                            pos+=4;
                        }
                    }
                    // Da die Zieltextur eine Breite von 2^n hat und damit 
                    // wahrscheinlich breiter ist als das darzustellende Bild,
                    // werden hier die restlichen Pixel dieser Reihe uebersprungen.
                    pos+=(m_selTexWidth-m_selWidth)*4;
                }

            }
            // die naechste Segment-ID in der Liste.
            iter++;
            
        }
	
	    return array;    	
    } // end else (!selection)
    
}

/**
 * Berechnet die nächst groesste 2er Potenz von arg.
 * @param arg 
 * @return 
 */
int vlBrainPlane::getValidSize(int arg)
{
    int r=1;

    while (r<arg)
    {
	r<<=1;
    }
    return r;
}

void vlBrainPlane::createTexture(bool newImage) {
    GLenum type; //datentyp des Pixels

    // setze die Größe der Bilddaten auf den neuesten Stand.
    updateImageSize();
    
    void*	anaPixel=NULL;
    VUByte*   segPixel=NULL;


    if (DATAMANAGER->isValid()) {
		//welchen datantypen haben wir??
	
		//Laden des "Grundbildes" 
        // NEW: due to possible colorspace problems vlEdit will convert all anatomy data from VShort 
        // and VSByte to VUByte. This switch statement is DEPRECATED and should be replaced by a 
        // VShort-only version
		switch(VPixelRepn(DATAMANAGER->image()->src()))
		{
		    case VUByteRepn:	type=GL_UNSIGNED_BYTE;
					anaPixel=getData<VUByte>(false);
					break;
		    case VSByteRepn:	type=GL_BYTE;
					anaPixel=getData<VSByte>(false);
					break;
		    case VShortRepn:	type=GL_SHORT; 
					anaPixel=getData<VShort>(false);
					break;
		    default:
					qWarning("Datentyp nicht zulaessig");abort();
		}
    } else {
		anaPixel=getData<char>(false);
    }
    if (anaPixel==NULL) {
		qWarning("Pixeldaten sind NULL, Fehler beim Erstellen der Textur!!");
		return;
    }

    //Wird ein neues Bild angelegt?!
    if (newImage) {
        // new texture -> new filter values
        // get the latest filter values
        refreshFilter();

        // create anatomie texture
        glBindTexture(GL_TEXTURE_2D, m_texID);
        glTexImage2D(GL_TEXTURE_2D, 0, 4, m_anaTexWidth, m_anaTexHeight, 0, 
                GL_LUMINANCE, type, anaPixel);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        //create segment texture if exists
        if((segPixel=getData<VUByte>(true)) != NULL ){
            glBindTexture(GL_TEXTURE_2D, m_selID);
            glTexImage2D(GL_TEXTURE_2D, 0, 4, m_selTexWidth, m_selTexHeight, 0, 
                GL_RGBA, GL_UNSIGNED_BYTE, segPixel);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_filter);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_filter);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        }
      // es wird kein neues Bild angelegt
      } else {
        glBindTexture(GL_TEXTURE_2D, m_texID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_anaTexWidth, m_anaTexHeight, 
            GL_LUMINANCE, type, anaPixel);
        //Laden der Bilddaten fuer Selektionstextur wenn vorhanden
        if((segPixel=getData<VUByte>(true)) != NULL){
            glBindTexture(GL_TEXTURE_2D, m_selID);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_selTexWidth, m_selTexHeight, 
                GL_RGBA, GL_UNSIGNED_BYTE, segPixel);
        }

      }
      glFlush();

      // Free Memory
      
      if (anaPixel!=NULL) {
         if (DATAMANAGER->isValid()) {
         //welchen datantypen haben wir??
            
         // Wir löschen nur typisierten Speicher.
         switch(VPixelRepn(DATAMANAGER->image()->src()))
         {
            case VUByteRepn: delete[] (VUByte*)anaPixel;
                  break;
            case VSByteRepn: delete[] (VSByte*)anaPixel;
                  break;
            case VShortRepn: delete[] (VShort*)anaPixel;
                  break;
            default:
                  qWarning("Datentyp nicht zulaessig");abort();
         }
         } else {
            delete [] (char*)anaPixel;
         }
      }
      if(segPixel != NULL)
         delete[] (VUByte*)segPixel;
}

void vlBrainPlane::draw()
{

    // Multitexturing
    // Texturumgebung0 = Anatomie
    pglActiveTextureARB(GL_TEXTURE0_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texID);
    // Anatomietextur ersetzt jedes Vorgaengerpixel
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    // Texturumgebung1 = Segmentation
    pglActiveTextureARB(GL_TEXTURE1_ARB);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_selID);
    // Segmenttextur ersetzt Vorgaengerpixel entsprechend seines Alpha-Werts.
    // Der Gesamtalphawert des Fragements entspricht dem Vorgaengeralphawert,
    // in diesem Fall 1.0 der Anatomietextur.
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    
    glPushMatrix();

    float scale=UICONFIG->scale();
    glScalef(scale, scale, scale);
    drawQuad();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    pglActiveTextureARB(GL_TEXTURE0_ARB);
    glDisable(GL_TEXTURE_2D);

}

void vlBrainPlane::move(int delta)
{
    // grundsätzlich bewegen wir uns in der Anantomie
    m_anaPlanePos+=delta;
    m_anaPlanePos = m_anaPlanePos < 0 ? 0 : m_anaPlanePos;
    m_anaPlanePos = m_anaPlanePos > m_anaMaxPos ? m_anaMaxPos : m_anaPlanePos;

    // Falls es ein gueltiges Segment gibt, muß auch hier die Ebene 
    // angepaßt werden.
    if(DATAMANAGER->isValidSegment()) {
	// die Bewegung innerhalb des Segments ist von der gewählten Auflösung 
	// abhängig.
	// relative Resolution
	float relRes = DATAMANAGER->image()->getResolution()[0] /
	    DATAMANAGER->selection()->getResolution()[0];
		int segDelta = (int)(relRes * delta);
		// Bewegung innerhalb der Selektion
		m_selPlanePos+=segDelta;
		m_selPlanePos = m_selPlanePos < 0 ? 0 : m_selPlanePos;
		m_selPlanePos = m_selPlanePos > m_selMaxPos ? m_selMaxPos : m_selPlanePos;
    }
    createTexture(false);
}

void vlBrainPlane::setPlane(int p)
{
    // Grundsätzlich bewegen wir uns innerhalb der Anatomie
    m_anaPlanePos=p; 
    m_anaPlanePos = m_anaPlanePos < 0 ? 0 : m_anaPlanePos;
    m_anaPlanePos = m_anaPlanePos > m_anaMaxPos ? m_anaMaxPos : m_anaPlanePos;

    // Falls es ein gueltiges Segment gibt, muß auch hier die Ebene 
    // angepaßt werden.
    if(DATAMANAGER->isValidSegment()) {
	// Die Position innerhalb des Segments muß wieder an die Auflösung angepaßt 
	// werden.
	float relRes = DATAMANAGER->image()->getResolution()[0] /
	    DATAMANAGER->selection()->getResolution()[0];
		m_selPlanePos = (int)(p * relRes);
		m_selPlanePos = m_selPlanePos < 0 ? 0 : m_selPlanePos;
		m_selPlanePos = m_selPlanePos > m_selMaxPos ? m_selMaxPos : m_selPlanePos;
    }

    createTexture(false);
}

void vlBrainPlane::deleteVoxel( int x, int y, int radius )
{
	
    if (!DATAMANAGER->isValid())
	return;
	
	// Selektionsmodus ??
	bool sel = (UICONFIG->getMode() == 1);

    switch (m_type) {
	case CORONAL: 	DATAMANAGER->deleteVoxel(sel, y, m_anaPlanePos, x, radius);
			break;
	case SAGITTAL: 	DATAMANAGER->deleteVoxel(sel, y, x, m_anaPlanePos, radius);
			break;
	case AXIAL: 	DATAMANAGER->deleteVoxel(sel, m_anaPlanePos, y, x, radius);
			break;
    }
    createTexture(false);
}

void vlBrainPlane::deleteSegment( int x, int y, int radius, int delta )
{
    
    if (!DATAMANAGER->isValid())
		return;

	// Selektionsmodus ??
	bool sel = (UICONFIG->getMode() == 1);

    switch (m_type) {
	case CORONAL: 	DATAMANAGER->deleteSegment(sel, y, m_anaPlanePos, x, radius, delta);
			break;
	case SAGITTAL: 	DATAMANAGER->deleteSegment(sel, y, x, m_anaPlanePos, radius, delta);
			break;
	case AXIAL: 	DATAMANAGER->deleteSegment(sel, m_anaPlanePos, y, x, radius, delta);
			break;
    }
    createTexture(false);
    
}

void vlBrainPlane::drawQuad( )
{
    // Die relativen Texturenkoordinaten sind bei Anatomie und Segmente 
    // unterschiedlich.
    float anaTexW=(float)m_anaWidth/(float)m_anaTexWidth;
    float anaTexH=(float)m_anaHeight/(float)m_anaTexHeight;

    float selTexW = anaTexW, selTexH = anaTexH;

    if(DATAMANAGER->isValidSegment()) {
		selTexW=(float)m_selWidth/(float)m_selTexWidth;
		selTexH=(float)m_selHeight/(float)m_selTexHeight;
    }

    glBegin(GL_QUADS);
    //glTexCoord2f(0, 0);
    pglMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);
    pglMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, 0);
    glVertex3f(-m_anaWidth/2, m_anaHeight/2, 0);

    //glTexCoord2f(0, texH);
    pglMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, anaTexH);
    pglMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0, selTexH);
    glVertex3f(-m_anaWidth/2, -m_anaHeight/2, 0);

    //glTexCoord2f(texW, texH);
    pglMultiTexCoord2fARB(GL_TEXTURE0_ARB, anaTexW, anaTexH);
    pglMultiTexCoord2fARB(GL_TEXTURE1_ARB, selTexW, selTexH);
    glVertex3f(m_anaWidth/2, -m_anaHeight/2, 0);

    //glTexCoord2f(texW, 0);
    pglMultiTexCoord2fARB(GL_TEXTURE0_ARB, anaTexW, 0);
    pglMultiTexCoord2fARB(GL_TEXTURE1_ARB, selTexW, 0);
    glVertex3f(m_anaWidth/2, m_anaHeight/2, 0);
    glEnd();
}

void vlBrainPlane::initExtensions( )
{
    //sind die Funktionen schon initialisiert?
    if (pglActiveTextureARB!=NULL)
	return;

    pglActiveTextureARB=(PFNGLACTIVETEXTUREARBPROC) glXGetProcAddressARB(
        (GLubyte*)"glActiveTextureARB");
    pglMultiTexCoord2fARB=(PFNGLMULTITEXCOORD2FARBPROC) glXGetProcAddressARB(
        (GLubyte*)"glMultiTexCoord2fARB");
}

void vlBrainPlane::markSegment( int x, int y )
{
    if (!DATAMANAGER->isValid())
		return;
		
    // Koordinaten kommen als Texturkoordinaten der Anatomie an
    // --> Umwandlung

    switch (m_type) {
	case CORONAL: DATAMANAGER->selectSegment(y, m_anaPlanePos, x, UICONFIG->radius()
				, UICONFIG->delta());
			break;
	case SAGITTAL: DATAMANAGER->selectSegment(y, x, m_anaPlanePos, UICONFIG->radius()
				, UICONFIG->delta());
			break;
	case AXIAL: DATAMANAGER->selectSegment(m_anaPlanePos, y, x, UICONFIG->radius()
				, UICONFIG->delta());
			break;
    }
    
    createTexture(false);
}

void vlBrainPlane::markVoxel( int x, int y )
{
    if (!DATAMANAGER->isValid())
	return;
	  
    switch (m_type) {
	case CORONAL: DATAMANAGER->selectVoxel(y, m_anaPlanePos, x, UICONFIG->radius());
			break;
	case SAGITTAL: DATAMANAGER->selectVoxel(y, x, m_anaPlanePos, UICONFIG->radius());
			break;
	case AXIAL: DATAMANAGER->selectVoxel(m_anaPlanePos, y, x, UICONFIG->radius());
			break;
    }
    
    createTexture(false);
}

void vlBrainPlane::refreshFilter() {

    // Option NN
    if(UICONFIG->getFilter().compare(NN_FILTER) == 0) {
		this->m_filter = GL_NEAREST;
		return;
    }
    //Option Bilin
    if(UICONFIG->getFilter().compare(BILIN_FILTER) == 0) {
		this->m_filter = GL_LINEAR;
		return;
    }

}

void vlBrainPlane::updateImageSize() {

    // Fallback wenn kein Datensatz für die Anatomie geladen werden konnte.
    if (!DATAMANAGER->isValid()) {
		qWarning("Error getting image data!");
		return;
    }
    
    // Eine Anatomie gibt es immer
    IVistaImage* anatomie = DATAMANAGER->image();
    if(anatomie != NULL){
	switch(m_type) {
	    case AXIAL:
			m_anaWidth = anatomie->width();
			m_anaHeight = anatomie->height();
			m_anaMaxPos = anatomie->depth()-1;
			break;
	    case SAGITTAL:
			m_anaWidth = anatomie->height();
			m_anaHeight = anatomie->depth();
			m_anaMaxPos = anatomie->width()-1;
			break;
	    case CORONAL:
			m_anaWidth = anatomie->width();
			m_anaHeight = anatomie->depth();
			m_anaMaxPos = anatomie->height()-1;
	}
	// korrigiere m_planePos falls notwendig
	if(m_anaPlanePos == -1)
	    m_anaPlanePos = m_anaMaxPos / 2;
		// bestimme die korrekte Texturgröße
		m_anaTexWidth = getValidSize(m_anaWidth);
		m_anaTexHeight = getValidSize(m_anaHeight);
    }
    else{
		qFatal("Error getting anatomie image size.");
        return;
    }
    // Noch kein Segment vorhanden
    if(!DATAMANAGER->isValidSegment()){
		// Falls noch kein gueltiges Segment vorhanden werden die 
		// Maße anhand der eingestellten Auflösung und den Maßen
        // der Anatomie bestimmt.
		float res = anatomie->getResolution()[0] / DATAMANAGER->getSegResolution()[0];
		switch (m_type){
		    case SAGITTAL:
				m_selWidth = (int)(res * anatomie->height());
				m_selHeight = (int)(res * anatomie->depth());
				m_selMaxPos = (int)(res * anatomie->width())-1;
				break;
		    case AXIAL:
				m_selWidth = (int)(res * anatomie->width());
				m_selHeight = (int)(res * anatomie->height());
				m_selMaxPos = (int)(res * anatomie->depth())-1;
				break;
		    case CORONAL:
				m_selWidth = (int)(res * anatomie->width());
				m_selHeight = (int)(res * anatomie->depth());
				m_selMaxPos = (int)(res * anatomie->height())-1;
			}
    }
    // Es gibt ein aktives Segment
    else{
	IVistaImage* selection = DATAMANAGER->selection();
	if(selection != NULL){
	    switch(m_type) {
		case AXIAL:
		    m_selWidth = selection->width();
		    m_selHeight = selection->height();
		    m_selMaxPos = selection->depth()-1;
		    break;
		case SAGITTAL:
		    m_selWidth = selection->height();
		    m_selHeight = selection->depth();
		    m_selMaxPos = selection->width()-1;
		    break;
		case CORONAL:
		    m_selWidth = selection->width();
		    m_selHeight = selection->depth();
		    m_selMaxPos = selection->height()-1;
	    }
	}
    }

    // korrigiere planePos wenn notwendig
    if(m_selPlanePos == -1)
	m_selPlanePos = (int)(m_selMaxPos / 2);
    // ermittle Texturgröße 
    m_selTexWidth = getValidSize(m_selWidth);
    m_selTexHeight = getValidSize(m_selHeight);

    // das Seitenverhältnis muß gleich bleiben. 
    if(m_selTexWidth != m_selTexHeight){
	if(m_selTexWidth > m_selTexHeight)
	    m_selTexHeight = m_selTexWidth;
	else
	    m_selTexWidth = m_selTexHeight;
    }
}

