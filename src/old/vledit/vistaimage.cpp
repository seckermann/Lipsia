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
 ********************************************************************/
#include "vistaimage.h"
#include "datamanager.h"
#include <stdlib.h>
#include <string>

IVistaImage::IVistaImage()
{
	image = NULL;
	m_res = NULL;
}

IVistaImage::IVistaImage( VImage img )
{
	image = img;
	m_res = NULL;

}

IVistaImage::IVistaImage( const int x, const int y, const int z, VRepnKind rep )
{
	image = VCreateImage( x, y, z, rep );

	/* initialize image data */
	switch( rep ) {
	case VBitRepn:
		setAll<VBit>( 0 );
		break;
	case VUByteRepn:
		setAll<VUByte>( 0 );
		break;
	case VSByteRepn:
		setAll<VSByte>( 0 );
		break;
	case VShortRepn:
		setAll<VShort>( 0 );
		break;
	default:
		qWarning( "Error initializing segment image. Unsupported datatype" );
		abort();
	}

	m_res = NULL;
}

IVistaImage::~IVistaImage()
{
	if ( image != NULL ) {
		VDestroyImage( image );
		image = NULL;
	}

	// reservierten Speicher freigeben.
	if( m_res != NULL )
		delete( m_res );

}

int IVistaImage::get( int band, int row, int column )
{
	int result = 0;

	if ( ( band >= 0 ) && ( band < bands() ) && ( row >= 0 ) && ( row < rows() ) && ( column > 0 ) && ( column < columns() ) ) {

		if ( VPixelRepn( image ) == VUByteRepn )
			result = VPixel ( image, band, row, column, VUByte );

		if ( VPixelRepn( image ) == VSByteRepn )
			result = VPixel ( image, band, row, column, VSByte );

		if ( VPixelRepn( image ) == VShortRepn )
			result = VPixel ( image, band, row, column, VShort );
	}

	return result;
}

float *IVistaImage::getResolution()
{

	// Die Aufloesung wurde noch nicht aus der Attributsliste geholt und gespeichert.
	if ( m_res == NULL ) {
		//m_res wurde noch nicht definiert
		m_res = new float [3];

		// Aufloesung mu�noch aus der Attributsliste ausgelesen werden
		VAttrList list = VImageAttrList( src() );
		VString value;
		vector<string> vals;

		// Attribut "voxel" auslesen
		VGetAttr( list, "voxel", NULL, VStringRepn, &value );

		// Werte trennen ( Trennzeichen: " " )
		splitString( value, " ", &vals );

		// Liste durchgehen, nach float konvertieren und in m_res speichern.
		vector<string>::iterator iter = vals.begin();
		int counter = 0;

		while( ( iter != vals.end() ) && ( counter < 3 ) ) {
			counter++;
			float  tmpVal = atof( ( *iter ).c_str() );
			m_res[counter - 1] = tmpVal;
			iter++;
		}
	}

	return m_res;

}

/*
 * Tool functions - not part of the VistaSegment interface.
 */

void IVistaImage::splitString( string str, string delim, vector<string>* results )
{
	unsigned int cutAt = 0;

	while( ( cutAt = str.find_first_of( delim, 0 ) ) < str.length() ) {
		if( cutAt > 0 ) {
			results->push_back( str.substr( 0, cutAt ) );
		}

		str = str.substr( cutAt + 1 );
	}

	if( str.length() > 0 )

	{
		results->push_back( str );
	}
}

template <class T> void IVistaImage::setAll( T value )
{

	int i;
	T *p = ( T * )VImageData( image );

	for( i = 0; i < VImageNPixels( image ); i++ ) {
		*( p++ ) = value;
	}
}

/******************************************************
 * VistaSegment
 * ****************************************************/

void VistaSegment::init()
{

	// initialize color
	// fallback color value
	//Red
	color[0] = -1;
	//Green
	color[1] = -1;
	//Blue
	color[2] = -1;
	//Alpha
	color[3] = 130;

	// Segment ist "von Natur aus" sichtbar
	visible = true;

	// color value from vista image
	VString str;

	// if color attribute is found -> set color value
	// else keep fallback value -1
	if( VGetAttr(
			VImageAttrList( image ),
			"color",
			NULL,
			VStringRepn,
			( VPointer ) &str ) == VAttrFound )
		sscanf( str, "%d %d %d", color, color + 1, color + 2 );

	// default value - no valid name
	name = 0;

	// if seg_name is found then set name value
	// else keep default val 0
	if( VGetAttr(
			VImageAttrList( image ),
			"seg_name",
			NULL,
			VUByteRepn,
			( VPointer ) &name ) != VAttrFound )
		name = 0;


	//initialize Volume
	countVolume();

	// Aufloesung
	m_res = new float[3];

	// ein neues Segment bekommt die vorkonfigurierte Aufl�ung
	if( DATAMANAGER->getSegResolution()[0] == 0.0 ) {
		m_res[0] = DATAMANAGER->image()->getResolution()[0];
		m_res[1] = DATAMANAGER->image()->getResolution()[1];
		m_res[2] = DATAMANAGER->image()->getResolution()[2];
	} else  {
		m_res[0] = DATAMANAGER->getSegResolution()[0];
		m_res[1] = DATAMANAGER->getSegResolution()[1];
		m_res[2] = DATAMANAGER->getSegResolution()[2];
	}

}

VistaSegment::~VistaSegment()
{
	if( m_res != NULL ) {
		delete m_res;
	}
}

double VistaSegment::getVolume()
{

	// Die Abmessungen der Voxel
	float  *measures;
	// Das Voxelvolumen
	double vovo = 0;

	measures = getResolution();

	vovo = measures[0] * measures[1] * measures[2];

	return volume * vovo;
}

void VistaSegment::countVolume()
{

	long mv = 0;
	int x = 0, y = 0, z = 0;

	//Bestimme die Anzahl der markierten Elemente
	for ( x = 0; x < columns(); x++ )
		for ( y = 0; y < rows(); y++ ) {
			for ( z = 0; z < bands(); z++ ) {
				if ( at( z, y, x ) != 0 )
					mv++;
			}
		}

	//speichern
	volume = mv;

}

void VistaSegment::changeVolume( int volDiff )
{

	volume += volDiff;
	// kein Volumen < 0
	volume = volume < 0 ? 0 : volume;

}

void VistaSegment::setColor( int red, int green, int blue )
{

	color[0] = red;
	color[1] = green;
	color[2] = blue;

}


