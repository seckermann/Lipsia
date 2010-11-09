/********************************************************************
 *   Copyright (C) 2005 by Hannes Niederhausen
 *   niederhausen@cbs.mpg.de
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *******************************************************************/
#ifndef VISTAIMAGE_H
#define VISTAIMAGE_H

#include <viaio/VImage.h>
#include <vector>
#include <cstdlib>
#include <typeinfo>
#include <stdlib.h>
#include <qapplication.h>

using namespace std;

/**
 * Interface fr die VistaImage Klasse. Diese Klasse ist notwendig um Templateparameter unabh�ngige
 * Instanzen der VistaImage Klasse zu deklarieren.
 */
class IVistaImage
{
protected:
	VImage image;
	VPointer data;	
	/** resolution data saved as float[3] array */
	float* m_res;
	
	/** splits a given String into parts according to the given
	 * delimeter and saves the result in a vector */
	void splitString(string, string, vector<string>*);

    /** initalize the internal VImage with the given value */
    template <class T> void setAll(T value);

public:
	IVistaImage();

	IVistaImage(VImage src);

	/**
	 *    Creates a new VistaImage with the given size and Pixelrepresentation
	 * @param x number of bands
	 * @param y number of rows
	 * @param z number of columns
	 * @param rep kind of datatype for a voxel
	 */
	IVistaImage(const int x, const int y, const int z, VRepnKind rep);

	virtual ~IVistaImage();

	/**
	 *    Definiert die PixelRepr�entation des bergebenen VImages
	 * @param image das VIMage dessen Repr�entation ben�igt wird
	 * @return die typeid der PixelRepr�entation (Datentyp eines Pixels im Bild)
	 */
	const std::type_info& getType(){
		switch(VPixelRepn(image))
		{
			case VBitRepn:		return typeid(VBit);break;
			case VUByteRepn:	return typeid(VUByte);break;
			case VSByteRepn:	return typeid(VSByte);break;
			case VShortRepn:	return typeid(VShort);break;
			case VLongRepn:		return typeid(VLong);break;
			default:
				qFatal("Datentyp nicht zul�sig");abort();
		}
	}

/**
	 *    Definiert die PixelRepräsentation des übergebenen VImages
	 * @param image das VIMage dessen Repräsentation benötigt wird
	 * @return die typeid der PixelRepräsentation (Datentyp eines Pixels im Bild)
	 */
	const char* getTypeString(){
		switch(VPixelRepn(image))
		{
			case VBitRepn:		return "VBit";break;
			case VUByteRepn:	return "VUByte";break;
			case VSByteRepn:	return "VSByte";break;
			case VShortRepn:	return "VShort";break;
			case VLongRepn:		return "VLong";break;
			default:
				qFatal("Datentyp nicht zul�sig");abort();
		}
	}

	/**
	 *    Gibt die Breite des Bildes zurück.
	 * @return 
	 */
	inline int width()
	{
		return VImageNColumns(image);
	}

	/**
	 *    Gibt die H�e des Bildes zurck
	 * @return 
	 */
	inline int height()
	{
		return VImageNRows(image);
	}
	
	int get(int band, int row, int column);

	/**
	 *    Gibt die Tiefe (aka Anzahl der Bänder) zurück.
	 * @return 
	 */
	inline int depth()
	{
		return VImageNBands(image);
	}

	inline int bands()
	{
		return VImageNBands(image);
	}

	inline int rows()
	{
		return VImageNRows(image);
	}
	inline int columns()
	{
		return VImageNColumns(image);
	}

	/**
	 *    Gibt die Bytegr�e der Pixeldaten zurck.
	 * @return 
	 */
	inline int size()
	{
		return VImageSize(image);
	}

	/**
	 *    Gibt das zugrunde liegende VImage zurck
	 * @return 
	 */
	inline VImage src()
	{
		return image;
	}
	
	/** 
	 * Gibt die Aufl�ung des VImages zurck.  Siehe auch m_res. */
	float* getResolution();
	
};

/**
 * Diese Klasse repr�entiert ein Vista Image.
 * @author Hannes Niederhausen
 */
template <class T> class VistaImage : public IVistaImage
{
public:

	VistaImage();

    /**
     * Erstellt ein VistaImage und nutzt die Daten aus dem bergebenen VImage
     * @param img das VImage welches genutzt werden soll.
     */
    VistaImage(VImage img) : IVistaImage(img) {
		
	};

	/**
	 *    Erstellt ein neues leeres VistaImage.
	 * @param x Breite
	 * @param y Höhe
	 * @param z Tiefe
	 */
	VistaImage(const int x, const int y, const int z, VRepnKind rep)
		: IVistaImage(x,y,z,rep) 
	{
		
	};

    //virtual ~VistaImage() {};


	/**
	 *    Gibt die Adresse des Wertes an der Position pos zurück.
	 * @param pos die Position
	 * @return die Adresse des Wert an der übergebenen Position 
	 */
	inline T &at(unsigned int pos)
	{
		return ((T*)data)[pos];
	}

	inline T &at(int band, int row, int column)
	{
		return *(T*)VPixelPtr(image, band, row, column);
	}
	
	/**
	 *    Gibt den Wert an der Position pos zurück.
	 * @param pos die Position
	 * @return der Wert an der übergebenen Position
	 */
	inline T at(unsigned int pos)const{return ((T*)data)[pos];}

	/**
	 * Setzt das komplette Bild auf den übergebenen Wert.
	 * @param value 
	 */
	inline void reset(const T value){
		if(value!=0) {
			for(int i=this->size()-1;i>=0;i--)
				at(i)=value;
		}
		else 
			bzero(data,sizeof(T)*this->size());
	}
};

/**
* Diese Klasse repraesentiert eine Auswahl (Segment) innerhalb eines VistaImages.
* Ein Segment ist ein VistaImage vom Typ Vbit mit zusaetzlichen Attributen fuer
* Eigennamen, ID und Farbwert.
* 
* @author Thomas Proeger
* @date 22.07.06
*/
class VistaSegment : public VistaImage<VBit>
{

private:
	void init();

protected:
	/**
	 * Zaehlt die markierten Voxel des Segments und speichert das Ergebnis in volume.
	 *  Wird verwendet wenn ein Segment aus einem VImage erzeugt wird (Segment laden).
	 */
	void countVolume();
	
public:
	
	/**
     * The segments name is an integer value.
     * 
     * There are two cases to consider:
     * 
     * - a UByte/Float image
     * When a UByte image is loaded then there are multiple segments stored
     * in one image. So the color values of the segment voxel will be used as 
     * the segments names. 
     * When saving segments as UByte images then the name will be used 
     * as segment specific value and all Voxels of a segment will be 
     * repesented with this value in the resulting image.
     * 
     * - a Bit image
     * When loading a bit image then all image voxels are represented as '1'.
     * So the segments name can be found in a new vista header tag, called
     * "seg_name". If there is no such tag then the internal segment id will be
     * used instead.
     *
     * IMPORTANT! 
     * Though it can be initialized with the internal id it's not guaranteed that
     * every segment has a unique name. So never confuse it with an id
     * value!
     * 
	*/
	int name;
	/**
	* Zugeordneter Farbwert. Jedem Segment kann ein charakteristischer Farbwert zugeordnet werden.
	* Dieser Wert wird als RGBA(Red,Green,Blue,Alpha)-Komponente gespeichert. 
	*/
	int color[4];
	/**
	* Volumen des Segments in Anzahl der markierten Voxel ohne Beruecksichtigung der 
	* Voxelgroesse. Das Volumen in mm wird von getVolume() geliefert.
	*/
	int volume;
    
    /**
     * Setzt den Sichtbarkeitsstatus eines Segments. Sichtbare Segmente werden
     * im Editor angezeigt und werden beim Mehrfachspeichern bercksichtig.
     * ACHTUNG: Es koennen auch unsichtbare Segmente gespeichert (Einzeln) und 
     * bearbeitet werden. Alle Segmente sind zu Beginn sichtbar.
     */
    bool visible;

	/**
	* leerer Standardkonstruktor
	*/
	VistaSegment(){
		init();
	}
	
	/**
	* 
	*/
	VistaSegment(VImage img) : VistaImage<VBit>(img){
		init();
	}

	VistaSegment(const int x, const int y, const int z) 
		: VistaImage<VBit>(x,y,z,VBitRepn) {
		init();
	}
    
    ~VistaSegment();

	/**
	* We need to assure that we return VBit values. Without this little maneuver VistaSegment::at would
	* return (unsigned char &) values.
	*/
	inline VBit &at(int band, int row, int column)
	{
		return *(VBit*)VPixelPtr(image, band, row, column);
	}
	
	/**
	* Liefert das Volumen des Segments unter Beruecksichtigung der Voxelaufloesung.
	*/
	double getVolume();
	
	/**
	 * Alters the current value of volume according to the given difference
	 */
	void changeVolume(int volDiff);
	
	/**
	 * Sets the color array to the given rgb values
	 */
	void setColor(int, int, int);

};

#endif
