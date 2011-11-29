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
#include "datamanager.h"
#include "uiconfig.h"
#include "point.h"
#include <viaio/VGraph.h>
#include <viaio/headerinfo.h>
#include <viaio/VImage.h>
#include <vector>
#include <math.h>
#include <string.h>

#include <qstring.h>
#include <qstringlist.h>

extern "C" {
#include <via.h>
}

using namespace std;

DataManager *DataManager::singleton = NULL;

DataManager::DataManager()
{
	m_original = NULL;
	m_selection = NULL;
	m_copy = NULL;
	m_valid_image = false;
	m_max_segnumber = 0;
	m_ca = NULL;
	m_extent = NULL;
	m_segResolution = new float[3];
	m_colorOrigin = 0;
	m_colorPointer = m_colorOrigin;
	m_midPoint = NULL;
}


DataManager::~DataManager()
{
	// free allocated memory

	if ( m_ca != NULL ) {
		delete[] m_ca;
		m_ca = NULL;
	}

	if ( m_segResolution != NULL )
		delete[] m_segResolution;

	if ( m_extent != NULL )
		delete[] m_extent;

	if ( m_midPoint != NULL )
		delete[] m_midPoint;
}


IVistaImage *DataManager::createVistaImage( VImage img )
{
	switch( VPixelRepn( img ) ) {
	case VBitRepn:
		return new VistaImage<VBit>( img );
		//        case VFloatRepn:  return new VistaImage<VFloat> (img);break;
		//        case VDoubleRepn: return new VistaImage<VDouble>(img);break;
	case VUByteRepn:
		return new VistaImage<VUByte> ( img );
		break;
	case VSByteRepn:
		return new VistaImage<VSByte> ( img );
		break;
	case VShortRepn:
		return new VistaImage<VShort> ( img );
		break;
		//        case VLongRepn:       return new VistaImage<VLong>  (img);break;
	default:
		qWarning( "Invalid datatype" );
	}

	return NULL;
}

void DataManager::loadVistaImage( const char *filename )
{
	FILE *fp;

	// open file read-only
	fp = fopen( filename, "rb" );

	if ( fp == NULL ) {
		qWarning( "Invalid filename" );
		return;
	}

	loadVistaImage( fp );

}

std::vector<int> DataManager::loadSegments( const char *filename )
{
	// open file read-only
	FILE *fp;
	fp = fopen( filename, "rb" );

	if ( fp == NULL ) {
		qWarning( "Error loading segment: Invalid filename" );
		std::vector<int> v;
		return v;
	}

	return loadSegments( fp );
}

void DataManager::deleteVoxel( bool sel, int band, int row, int column, int r )
{

	int r_quad = r * r;

	if ( band >= m_copy->bands() )
		return;

	if ( row >= m_copy->rows() )
		return;

	if ( column >= m_copy->columns() )
		return;

	// delete from selection
	if( sel ) {
		if( !isValidSegment() ) {
			qWarning( "SelectVoxel error: no valid segment!" );
			return;
		}

		// convert anatomie coordinates to segment coordinates
		float resFactor = DATAMANAGER->image()->getResolution()[0] /
						  DATAMANAGER->selection()->getResolution()[0];

		int ba = ( int )rintf( band * resFactor );
		int ro = ( int )rintf( row * resFactor );
		int co = ( int )rintf( column * resFactor );

		int startC = ( int )( co - r );
		int startR = ( int )( ro - r );
		int startB = ( int )( ba - r );

		int endC = ( int )( co + r );
		int endR = ( int )( ro + r );
		int endB = ( int )( ba + r );

		// prevend undefined values.
		if ( startC < 0 )
			startC = 0;

		if ( startR < 0 )
			startR = 0;

		if ( startB < 0 )
			startB = 0;

		if ( endC >= m_selection->width() )
			endC = m_selection->width() - 1;

		if ( endR >= m_selection->height() )
			endR = m_selection->height() - 1;

		if ( endB >= m_selection->depth() )
			endB = m_selection->depth() - 1;

		// count voxel whose value has been changed. (1 -> 0)
		int removedVoxel = 0;

		for ( int i = startC + 1; i < endC; i++ ) {
			for( int j = startR + 1; j < endR; j++ ) {
				for( int k = startB + 1; k < endB; k++ ) {
					int x = co - i;
					int y = ro - j;
					int z = ba - k;

					if ( x *x + y *y + z *z <= r_quad ) {
						if( m_selection->at( k, j, i ) != 0 ) {
							m_selection->at( k, j, i ) = 0;
							removedVoxel++;
						}
					}
				}
			}
		}

		// calculate new volume
		m_selection->changeVolume( -removedVoxel );
	}// END delete from selection

	// delete from anatomie texture
	else {
		int startC = ( int )( column - r );
		int startR = ( int )( row - r );
		int startB = ( int )( band - r );

		int endC = ( int )( column + r );
		int endR = ( int )( row + r );
		int endB = ( int )( band + r );

		// prevent undefined values
		if ( startC < 0 )
			startC = 0;

		if ( startR < 0 )
			startR = 0;

		if ( startB < 0 )
			startB = 0;

		if ( endC >= m_copy->width() )
			endC = m_copy->width() - 1;

		if ( endR >= m_copy->height() )
			endR = m_copy->height() - 1;

		if ( endB >= m_copy->depth() )
			endB = m_copy->depth() - 1;

		/* try to cast the img to the correct pixel representation. VByte or
		 * VShort*/

		/* VUByte */
		VistaImage<VUByte> *anaImage = dynamic_cast<VistaImage<VUByte>* >( m_copy );

		//cast image
		if ( !anaImage ) {

			/* VShort */
			VistaImage<VShort> *anaImage = dynamic_cast<VistaImage<VShort>* >( m_copy );

			if ( !anaImage ) {
				qWarning( "Konnte Bild nicht casten" );
				return;
			}

			// images casted to VShort
			for ( int i = startC + 1; i < endC; i++ ) {
				for( int j = startR + 1; j < endR; j++ ) {
					for( int k = startB + 1; k < endB; k++ ) {
						int x = column - i;
						int y = row - j;
						int z = band - k;

						if ( x *x + y *y + z *z <= r_quad )
							anaImage->at( k, j, i ) = 0;
					}
				}
			}

			return;
		} // End cast to VShort

		// image casted to VUByte
		for ( int i = startC + 1; i < endC; i++ ) {
			for( int j = startR + 1; j < endR; j++ ) {
				for( int k = startB + 1; k < endB; k++ ) {
					int x = column - i;
					int y = row - j;
					int z = band - k;

					if ( x *x + y *y + z *z <= r_quad )
						anaImage->at( k, j, i ) = 0;
				}
			}
		}
	}// END delete from anatomie texture
}

void DataManager::save( const char *filename )
{
	FILE *fp;
	// list of VImages which should be saved.
	VImage imgList[1];

	// valid anatomie image?
	if( !isValid() )
		return;

	fp = fopen( filename, "wb" );

	if ( fp == NULL ) {
		qWarning( "Fehler beim �fnen der Datei" );
		return;
	}

	// VImage from anatomical work copy
	imgList[0] = m_copy->src();

	if ( !VWriteImages( fp, NULL, 1, imgList ) )
		qWarning( "Fehler beim Schreiben der Imagedaten" );

	fclose( fp );
}

void DataManager::deleteSegment( bool sel, int band, int row, int column, int r, int delta )
{
	int i, j, k;

	int x, y, z;

	// the radius to the second
	int r_quad = r * r;

	// the subdelta near the cursor position.
	// It's a fragment of the given delta.
	int subdelta = ( int ) ( delta / 5 );

	// prevend undefined values

	if ( band >= m_copy->bands() )
		return;

	if ( row >= m_copy->rows() )
		return;

	if ( column >= m_copy->columns() )
		return;

	int startC = ( int )( column - r );
	int startR = ( int )( row - r );
	int startB = ( int )( band - r );

	int endC = ( int )( column + r );
	int endR = ( int )( row + r );
	int endB = ( int )( band + r );

	if ( startC < 0 )
		startC = 0;

	if ( startR < 0 )
		startR = 0;

	if ( startB < 0 )
		startB = 0;

	if ( endC >= m_copy->width() )
		endC = m_copy->width() - 1;

	if ( endR >= m_copy->height() )
		endR = m_copy->height() - 1;

	if ( endB >= m_copy->depth() )
		endB = m_copy->depth() - 1 ;

	/* try to cast the img to the correct pixel representation. VUByte
	 * or VShort. */

	/* VUByte */
	VistaImage<VUByte> *anaImage = dynamic_cast<VistaImage<VUByte>* >( m_copy );

	// cast the image
	if ( !anaImage ) {
		/* VShort */
		VistaImage<VShort> *anaImage = dynamic_cast<VistaImage<VShort>* >( m_copy );

		if ( !anaImage ) {
			qWarning( "Konnte Bild nicht casten" );
			return;
		}

		// image casted to VShort

		// the anatomical color at the given coordinates.
		int orig_color = ( int )anaImage->at( band, row, column );

		// delete from selection
		if( sel ) {
			if( !isValidSegment() )
				return;

			for ( i = startC; i <= endC; i++ ) {
				for( j = startR; j <= endR; j++ ) {
					for( k = startB; k <= endB; k++ ) {
						x = column - i;
						y = row - j;
						z = band - k;

						if ( x *x + y *y + z *z <= r_quad ) {
							// don't uses the "transparent" black anatomical
							// background as reference
							if( anaImage->at( k, j, i ) == 0 )
								continue;

							if( ( abs( UICONFIG->deltaTr() - anaImage->at( k, j, i ) ) <= delta )
								&& ( abs( orig_color - anaImage->at( k, j, i ) ) <= subdelta ) )
								selectSingleVoxel( k, j, i, 0 );
						}
					}
				}
			}

			return;
		} // End delete from selection

		// delete from the anatomical image
		for ( i = startC; i <= endC; i++ ) {
			for( j = startR; j <= endR; j++ ) {
				for( k = startB; k <= endB; k++ ) {
					x = column - i;
					y = row - j;
					z = band - k;

					if ( x *x + y *y + z *z <= r_quad ) {
						// don't uses the "transparent" black anatomical
						// background as reference
						if( anaImage->at( k, j, i ) == 0 )
							continue;

						if( ( abs( UICONFIG->deltaTr() - anaImage->at( k, j, i ) ) <= delta )
							&& ( abs( orig_color - anaImage->at( k, j, i ) ) <= subdelta ) )
							anaImage->at( k, j, i ) = 0;
					}
				}
			}
		}

		return;
	} // End convert to VShort

	// successfully converted to VUByte

	// The anatomical color at the given coordinates.
	int orig_color = ( int )anaImage->at( band, row, column );

	// delete from selection
	if( sel ) {
		if( !isValidSegment() )
			return;

		for ( i = startC; i <= endC; i++ ) {
			for( j = startR; j <= endR; j++ ) {
				for( k = startB; k <= endB; k++ ) {
					x = column - i;
					y = row - j;
					z = band - k;

					if ( x *x + y *y + z *z <= r_quad ) {
						// don't uses the "transparent" black anatomical
						// background as reference
						if( anaImage->at( k, j, i ) == 0 )
							continue;

						if( ( abs( UICONFIG->deltaTr() - anaImage->at( k, j, i ) ) <= delta )
							&& ( abs( orig_color - anaImage->at( k, j, i ) ) <= subdelta ) )
							selectSingleVoxel( k, j, i, 0 );
					}
				}
			}
		}

		return;
	} // End delete from selection

	// delete from anatomie

	for ( i = startC; i <= endC; i++ ) {
		for( j = startR; j <= endR; j++ ) {
			for( k = startB; k <= endB; k++ ) {
				x = column - i;
				y = row - j;
				z = band - k;

				if ( x *x + y *y + z *z <= r_quad ) {
					// don't uses the "transparent" black anatomical
					// background as reference
					if( anaImage->at( k, j, i ) == 0 )
						continue;

					if( ( abs( UICONFIG->deltaTr() - anaImage->at( k, j, i ) ) <= delta )
						&& ( abs( orig_color - anaImage->at( k, j, i ) ) <= subdelta ) )
						anaImage->at( k, j, i ) = 0;
				}
			}
		}
	}// END delete from anatomie
}

float *DataManager::midPoint( )
{
	if ( !isValid() )
		return NULL;

	// if there is no midpoint data create it.
	if ( ( m_midPoint == NULL ) ) {
		m_midPoint = new float[3];
		m_midPoint[0] = ( float ) m_original->bands() / 2;
		m_midPoint[1] = ( float ) m_original->rows() / 2;
		m_midPoint[2] = ( float ) m_original->columns() / 2;
	}

	return m_midPoint;
}

std::vector<int> DataManager::loadSegments( FILE *fp )
{
	VAttrList list;            // attribute list for vista header data
	VAttrListPosn posn;        // iterator over attributes
	VImage src_image = NULL;   // the source image buffer found in the vista file
	int nsegments = 0;          // overall number of segments
	//the list of internal indices of found vista images
	std::vector<int> indexList;

	// Hashmap with preloaded image data
	// key: const VImage image found in vista file
	// data: int* integer array with  segment name flags for uByte images,
	//             in a bit image --> NULL
	//             IMPORTANT: the array memory must be freed with 'delete[]'
	std::map<const VImage, bool *> image_map;

	// get attribute list
	list = VReadFile( fp, NULL );

	if ( !list ) {
		qWarning( "Error loading segment: Invalid file type" );
		return indexList;
	}

	//*****************************************************************
	// first run through image elements
	// -- get vimages and number of segments --
	//*****************************************************************


	for( VFirstAttr( list, &posn ); VAttrExists( &posn ); VNextAttr( &posn ) ) {
		// VImage data found
		if( VGetAttrRepn( &posn ) == VImageRepn ) {
			VGetAttrValue( &posn, NULL, VImageRepn, &src_image );

			// check if attributes are ok
			if( !isValidSegment( src_image ) )
				continue;

			VUByte *p_src;
			bool *name_array;
			VImage tmp;

			// some variables needed in the switch statement
			switch( VPixelRepn( src_image ) ) {
				// bit image
			case VBitRepn:
				image_map[src_image] = NULL;
				nsegments++;
				break;
				//  float image
			case VFloatRepn:
				// convert src_image to ubyte image
				tmp = VConvertImageCopy(
						  src_image,
						  NULL,
						  VAllBands,
						  VUByteRepn );
				src_image = tmp;
				// IMPORTANT
				// we'll do without using the break command since we want
				// to continue with the converted src_image.
				// ubyte image
			case VUByteRepn:
				name_array = new bool[256];
				// initialize array with '0' (=false)
				memset( name_array, 0, 256 );
				p_src = & VPixel( src_image, 0, 0, 0, VUByte );

				for( int j = 0; j < VImageNPixels( src_image ); j++ ) {
					// if there is a valid color and there there was no VImage with
					// this id before
					if( ( *p_src != 0 ) && ( name_array[*p_src] == false ) ) {
						nsegments++;
						name_array[*p_src] = true;
					}

					p_src++;
					image_map[src_image] = name_array;
				}

				break;
			default:
				break;
			}
		}
	}

	qDebug( "Number of segments found: %i", nsegments );

	//*****************************************************************
	// second run through vista image elements
	// -- load image data into vista segments --
	//*****************************************************************

	std::map<const VImage, bool *>::iterator image_iter = image_map.begin();
	// a hash map with <const VUByte, VImage> pairs
	// key: const VUByte the segments integer name
	// VImage: a VBit copy container of the original image
	std::map<const VUByte, VImage> segimage_map;
	std::map<const VUByte, VImage>::iterator seg_iter;

	while( image_iter != image_map.end() ) {

		int id = 0; //id buffer
		bool *name_array;
		// a hash map with segment integer names mapping to color strings
		// key: const VUByte the segments integer name
		// QStringList the list of color R-G-B-String as found in the "color" attribute
		std::map<const VUByte, QStringList> segcolors_map;
		std::map<const VUByte, QStringList>::iterator color_iter;
		// string buffer
		VString content;
		//string list buffer for string color vals
		QStringList colorList;
		// get attribute list from original image
		VAttrList attr_list = VImageAttrList( ( *image_iter ).first );
		// string list buffer for RGB strings
		QStringList tmp_list;

		switch( VPixelRepn( ( *image_iter ).first ) ) {
			// VBit IMAGE
		case VBitRepn:
			//create new VistaImage based on image data
			id = createSegment( VCopyImage( ( *image_iter ).first, NULL, VAllBands ) );

			if( id > 0 ) {
				indexList.push_back( id );
			}

			break;
			// VUByte IMAGE
		case VUByteRepn:

			// fatal error
			if( ( *image_iter ).second == NULL ) {
				qFatal( "Error getting name flags from ubyte image. Abort" );
				return indexList;
			}

			if( VGetAttr( attr_list, "color", NULL, VStringRepn, &content )
				== VAttrFound ) {
				colorList = QStringList::split( " ", content );
				// traverse list while it containts more than 3 elements
				int i = 0;

				while( colorList.size() - i > 3 ) {

					// check valid values
					// create a new string list and store element 2,3,4 in it
					tmp_list = QStringList( colorList[i + 1] );
					tmp_list.push_back( colorList[i + 2] );
					tmp_list.push_back( colorList[i + 3] );
					// store list in hasmap
					segcolors_map[colorList[i].toUShort()] = tmp_list;
					// step to next color values;
					i += 4;
				}
			}

			name_array = ( *image_iter ).second;

			// since we know the array size we can do such bogus
			for( int i = 0; i < 256; i++ ) {
				// if there is a segment with the name 'i' then create
				// container
				if( name_array[i] ) {
					segimage_map[i] = VCreateImage(
										  VImageNBands( ( *image_iter ).first ),
										  VImageNRows( ( *image_iter ).first ),
										  VImageNColumns( ( *image_iter ).first ),
										  VBitRepn );
					//copy attributes from original image
					VImageAttrList( segimage_map[i] ) =
						VCopyAttrList( VImageAttrList( ( *image_iter ).first ) );

					// color attribute
					// remove color attribute,
					VAttrListPosn posn;

					if( VLookupAttr( VImageAttrList( segimage_map[i] ), "color", &posn ) )
						VDeleteAttr( &posn );

					//if a color value for this segment
					// was found in the vista image then copy val into the container
					// image, else a new color value will be set later by the method
					// DataManager::createSegment(VImage)
					color_iter = segcolors_map.find( i );

					if( color_iter != segcolors_map.end() ) {
						QString color = ( *color_iter ).second.join( " " );
						VSetAttr( VImageAttrList( segimage_map[i] ), "color",
								  NULL, VStringRepn, color.latin1() );
					}

					// set image name to i, temporary. This attribut must be
					// removed when the corresponding VistaSegment is saved
					// as a VUByte image
					VSetAttr( VImageAttrList( segimage_map[i] ), "seg_name",
							  NULL, VUByteRepn, i );
				}
			}

			// Traverse original ubyte image and extract segment data.
			// Therefore we look for ubyte values != 0 and set a 1
			// val in the corresponding bit container at the given
			// coordinates.
			for( int band = 0; band < VImageNBands( ( *image_iter ).first ); band++ ) {
				for( int row = 0; row < VImageNRows( ( *image_iter ).first ); row++ ) {
					for( int col = 0; col < VImageNColumns( ( *image_iter ).first ); col++ ) {
						VUByte pixel = VPixel( ( *image_iter ).first, band, row, col, VUByte );

						if ( pixel != 0 )
							VPixel( segimage_map[pixel], band, row, col, VBit ) = 1;
					}
				}
			}

			// create VistaSegment from bit images.
			seg_iter = segimage_map.begin();

			while( seg_iter != segimage_map.end() ) {

				id = createSegment( ( *seg_iter ).second );

				if( id > 0 ) {
					indexList.push_back( id );
				}

				seg_iter++;
			}

			break;
		default:
			break;
		} // END switch pixel representation

		// free array memory
		delete[] ( *image_iter ).second;
		image_iter++;
	} // END iteration over image map

	// free file pointer
	fclose( fp );

	return indexList;
}

void DataManager::loadVistaImage( FILE *fp )
{
	VAttrList list;     //attribute list from vista image header
	VAttrListPosn posn; //iterator over header attributes
	VImage src = NULL;  //source image buffer


	//load file
	list = VReadFile( fp, NULL );

	if ( !list ) {
		qWarning( "Invalid file type" );
		return;
	}

	//discard old image data
	if ( m_original != NULL ) {
		delete m_original;
		m_original = NULL;
	}

	if ( m_copy != NULL ) {
		delete m_copy;
		m_copy = NULL;
	}

	//run through attributes and search image data
	for ( VFirstAttr( list, &posn ); VAttrExists( &posn ); VNextAttr( &posn ) ) {
		//get image data
		if ( VGetAttrRepn ( &posn ) == VImageRepn ) {
			VGetAttrValue ( &posn, NULL, VImageRepn, &src );

			//create working copy
			m_original = createVistaImage( src );
			// create a color scaled working copy
			m_copy = transColorSpace( src );

			m_valid_image = true;
		}
	}

	//nun holen wir uns die n�igen Attribute
	VString str;

	//ca
	if ( VGetAttr ( VAttrList ( src->attributes ), "ca", NULL, VStringRepn, ( VPointer ) & str ) == VAttrFound ) {
		if ( m_ca == NULL )
			m_ca = new float[3];

		int r = sscanf( str, "%f %f %f", &m_ca[0], &m_ca[1], &m_ca[2] );

		//wrong ca string format.
		if( r < 3 ) {
			delete m_ca;
			m_ca = NULL;
		}

	} else {
		//if no ca found -> remove old one
		if ( m_ca != NULL ) {
			delete m_ca;
			m_ca = NULL;
		}
	}

	//extent
	if ( VGetAttr ( VAttrList ( src->attributes ), "extent", NULL, VStringRepn, ( VPointer ) & str ) == VAttrFound ) {
		if ( m_extent == NULL )
			m_extent = new float[3];

		int r = sscanf( str, "%f %f %f", &m_extent[0], &m_extent[1], &m_extent[2] );

		//wrong extent string format
		if( r < 3 ) {
			delete m_extent;
			m_extent = NULL;
		}

	} else {
		//if no extent found  -> remove old one
		if ( m_extent != NULL ) {
			delete m_extent;
			m_extent = NULL;
		}

	}

	//TODO DEBUG
//	qWarning( QString("Resolution (Anatomie): ") + QString::number( m_original->getResolution()[0] )
//			  + QString("x") + QString::number( m_original->getResolution()[1] )
//			  + QString("x") + QString::number( m_original->getResolution()[2] ) );
}


void DataManager::selectSegment( int band, int row, int column, int r, int delta )
{

	if( !isValidSegment() ) {
		qWarning( "selectSegment error: no valid segment!" );
		return;
	}

	// Subdelta value: a delta value which referes to the current mouse Position
	// (the given band, row, column)
	int subdelta = ( int )( delta / 5 );

	// check ranges
	if ( band >= m_copy->bands() )
		return;

	if ( row >= m_copy->rows() )
		return;

	if ( column >= m_copy->columns() )
		return;

	int startC = ( int )( column - r );
	int startR = ( int )( row - r );
	int startB = ( int )( band - r );

	int endC = ( int )( column + r );
	int endR = ( int )( row + r );
	int endB = ( int )( band + r );

	// check for undefnied values
	if ( startC < 0 )
		startC = 0;

	if ( startR < 0 )
		startR = 0;

	if ( startB < 0 )
		startB = 0;

	if ( endC >= m_copy->width() )
		endC = m_copy->width() - 1;

	if ( endR >= m_copy->height() )
		endR = m_copy->height() - 1;

	if ( endB >= m_copy->depth() )
		endB = m_copy->depth() - 1;

	// the anatomie is used as reference for the delta value.
	VistaImage<VUByte> *anaImage = dynamic_cast<VistaImage<VUByte>* >( m_copy );

	//casten des VImages
	if ( !anaImage ) {

		VistaImage<VShort> *anaImage = dynamic_cast<VistaImage<VShort>* >( m_copy );

		if ( !anaImage ) {
			qWarning( "Konnte Bild nicht casten" );
			return;
		}

		// successfully convert to VShort

		// the radius squared
		int r_quad = r * r;

		// color value on the anatomie at the given position
		int orig_color = ( int )anaImage->at( band, row, column );

		// select voxel within the calculated ranges
		for ( int i = startC + 1; i < endC; i++ ) {
			for( int j = startR + 1; j < endR; j++ ) {
				for( int k = startB + 1; k < endB; k++ ) {
					int x = column - i;
					int y = row - j;
					int z = band - k;

					if ( x *x + y *y + z *z <= r_quad ) {
						if( ( abs( UICONFIG->deltaTr() - anaImage->at( k, j, i ) ) <= delta )
							&& ( abs( orig_color - anaImage->at( k, j, i ) ) <= subdelta ) ) {
							selectSingleVoxel( k, j, i, 1 );
						}
					}
				}
			}
		}

		return;
	}// End convert to short

	// successfully converted to VUByte

	// the radius to the square
	int r_quad = r * r;

	// color value in the anatomie at  the given position
	int orig_color = ( int )anaImage->at( band, row, column );

	// select voxel within the calculated ranges.
	for ( int i = startC + 1; i < endC; i++ ) {
		for( int j = startR + 1; j < endR; j++ ) {
			for( int k = startB + 1; k < endB; k++ ) {
				int x = column - i;
				int y = row - j;
				int z = band - k;

				if ( x *x + y *y + z *z <= r_quad )
					if ( ( abs( UICONFIG->deltaTr() - anaImage->at( k, j, i ) ) <= delta )
						 && ( abs( orig_color - anaImage->at( k, j, i ) ) <= subdelta ) ) {
						selectSingleVoxel( k, j, i, 1 );
					}
			}
		}
	}

}

void DataManager::saveSelection( const char *filename , const bool saveAll )
{
	// dynamic array of VImages to save
	VImage *imgList;
	// number of VImages to save
	int num_images = 0;
	// tmp. image buffer
	VImage src;

	/***********************************************
	 * adjust list of segments to save
	 **********************************************/

	// create copy of segment id list
	vector<int> segList = *UICONFIG->segList();
	// To stabilise any iterator after a delete operation the
	// STL-documentation suggests to create a vector of static size.
	segList.reserve( segList.capacity() + 1 );

	vector<int>::iterator iter = segList.begin();

	// iterate over id list and remove all ids from invisible (if not
	// saveAll flag set) or empty segments.
	while( iter != segList.end() ) {
		if( ( segment( *iter )->getVolume() == 0.0 )
			|| ( ( !saveAll ) && ( !segment( *iter )->visible ) ) ) {
			segList.erase( iter );
		} else {
			iter++;
		}
	}

	// number of segments left for saving
	num_images = segList.size();

	// if nothing left to save -> quit
	if( num_images <= 0 ) {
		return;
	}

	// reserve memory for image list.
	imgList = ( VImage * )VMalloc( sizeof( VImage ) * num_images );

	/*************************************
	 * save all segments in VUByte repn
	 ************************************/
	if( m_output == 0 ) {

		std::vector<int>::iterator iter = segList.begin();
		// The vlEdit specific segment id will represent the color value of the
		// corresponding segment in the VUByte image. Accepted values are 1-255.
		// This means, every voxel from a segment with id n will be saved as
		// color value n in the resulting VImage. Therefore at most 255 segments
		// can be saved in a single vista image in VUByte format.
		int index = 0;
		// VUByte container
		VImage container = NULL;
		// color string
		QString strColor;
		// string buffer
		char *strBuf = new char[100];
		// pointer to dynamic array
		int *color;

		// iterate over id list copy and save every segment
		while( iter != segList.end() ) {
			// the corresponding segment
			VistaSegment *seg = segment( *iter );
			// id of current segment
			index = seg->name;
			// segment VImage
			src = seg->src();
			// color array
			color = seg->color;

			// If there is no container image, create a new one using the first
			// segment as template. Since the id of the segment needn't to be 1
			// it will be overidden in one of the following steps.
			if ( container == NULL ) {
				// this will create a VUByte VImage container witch '1' color
				// values
				container = VConvertImageCopy( seg->src(), NULL,
											   VAllBands, VUByteRepn );
			}

			// add segment to list

			// construct color string, Format: "$name $R $G $B"
			sprintf( strBuf, " %d %d %d %d", seg->name, color[0], color[1], color[2] );
			strColor += strBuf;

			// copy VImage to the container
			// rule: - the color value is the index value
			//       - if there is a value !=0 it could be overriden by the color
			//          color value from a higher ranked segment

			// traverse source and destination images with pixel pointer
			VBit *p_src =  & VPixel( src, 0, 0, 0, VBit );
			VUByte *p_cont = &  VPixel( container, 0, 0, 0, VUByte );

			// paranoia test
			if( VImageNPixels( src ) != VImageNPixels( container ) ) {
				qFatal( "source and container image with different size!" );
			}

			for( int i = 0; i < VImageNPixels( src ); i++ ) {
				// Since we use multiplication we have to be sure not
				// to override an existing color value from a prior segment with
				// the transparent '0' value of the current segment.
				if( *p_src != 0 ) {
					*p_cont = *p_src * index;
				}

				p_cont++;
				p_src++;
			}

			iter++;
		}

		// remove trailing whitespaces
		strColor = strColor.stripWhiteSpace();
		// set color header attribute
		VSetAttr( VImageAttrList ( container ), "color", NULL,
				  VStringRepn, ( VString )( strColor.latin1() ) );

		// remove "seg_name" attribute from attribute list
		VAttrListPosn posn;

		if( VLookupAttr( VImageAttrList( container ), "seg_name", &posn ) )
			VDeleteAttr( &posn );

		// add container image to save list
		imgList[0] = container;
		// IMPORTANT: the number of images to save is ONLY 1 !!!
		num_images = 1;

		delete[] strBuf;
	}
	/***********************************
	 * save segments to bit images
	 ***********************************/
	else {
		// iterator over id list
		std::vector<int>::reverse_iterator riter = segList.rbegin();
		// index buffer
		int index = 0;
		VString str = ( VString )VMalloc( sizeof( char ) * 100 );

		// traverse list of segment ids and create VImages for saving
		while( riter != segList.rend() ) {
			// VImage of current segment
			src = segment( *riter )->src();
			// color value of current segment
			int *color = segment( *riter )->color;

			// save color string
			sprintf( str, "%d %d %d", color[0], color[1], color[2] );
			VSetAttr( VImageAttrList ( src ), "color", NULL, VStringRepn, ( VString )str );

			// save segment name
			sprintf( str, "%d", segment( *riter )->name );
			VSetAttr( VImageAttrList ( src ), "seg_name", NULL, VStringRepn, ( VString )str );

			// add image to list
			imgList[index++] = src;

			riter++;
		}

		VFree( str );
	}

	// write to vista file
	FILE *fp;

	// open file for writing
	fp = fopen( filename, "wb" );

	if ( fp == NULL ) {
		qWarning( "error opening file" );
		return;
	}

	// write all VImages to file
	if ( !VWriteImages( fp, NULL, num_images, imgList ) )
		qWarning( "Fehler beim Schreiben der Imagedaten" );

	// clean up
	fclose( fp );
	VFree( imgList );

}

void DataManager::selectVoxel( int band, int row, int column, int r )
{
	// no valid segment found
	if( !isValidSegment() ) {
		qWarning( "SelectVoxel error: no valid segment!" );
		return;
	}

	// check ranges
	if ( band >= m_copy->bands() )
		return;

	if ( row >= m_copy->rows() )
		return;

	if ( column >= m_copy->columns() )
		return;

	// transform anatomical coordinates to segment coordinates
	float resFactor = DATAMANAGER->image()->getResolution()[0] /
					  DATAMANAGER->selection()->getResolution()[0];

	// respect segment resolution
	//    int ba = (int)rintf(band * resFactor);
	//    int ro = (int)rintf(row * resFactor);
	//    int co = (int)rintf(column * resFactor);

	int ba = ( int )floor( band * resFactor );
	int ro = ( int )floor( row * resFactor );
	int co = ( int )floor( column * resFactor );


	int startC = ( int )( co - r );
	int startR = ( int )( ro - r );
	int startB = ( int )( ba - r );

	int endC = ( int )( co + r );
	int endR = ( int )( ro + r );
	int endB = ( int )( ba + r );

	// check ranges
	if ( startC < 0 )
		startC = 0;

	if ( startR < 0 )
		startR = 0;

	if ( startB < 0 )
		startB = 0;

	if ( endC >= m_selection->width() )
		endC = m_selection->width() - 1;

	if ( endR >= m_selection->height() )
		endR = m_selection->height() - 1;

	if ( endB >= m_selection->depth() )
		endB = m_selection->depth() - 1;

	// radius to the square
	int r_quad = r * r;
	// number of voxels with altered state
	int markedVoxel = 0;

	for ( int i = startC + 1; i < endC; i++ ) {
		for( int j = startR + 1; j < endR; j++ ) {
			for( int k = startB + 1; k < endB; k++ ) {
				int x = co - i;
				int y = ro - j;
				int z = ba - k;

				if ( x *x + y *y + z *z <= r_quad ) {
					if( m_selection->at( k, j, i ) != 1 ) {
						m_selection->at( k, j, i ) = 1;
						markedVoxel++;
					}
				}
			}
		}
	}

	// calculate new volume
	m_selection->changeVolume( markedVoxel );

}

void DataManager::selectSingleVoxel( int band, int row, int column, int value )
{

	// transform anatomical coordinates into resoution dependend segment coords.
	float resFactor = DATAMANAGER->image()->getResolution()[0] /
					  DATAMANAGER->selection()->getResolution()[0];

	// Case 1: res_ana_ < res_sel
	if( resFactor > 1 ) {
		// A selection in the anatomie marks 2x2x2 segment voxel at least.
		// IMPORTANT: the pixel index starts with 0

		// band
		int b_max = ( int )floor( ( band + 1 ) * resFactor - 1 );
		int b_min = ( int )floor( band * resFactor );

		// row
		int r_max = ( int )floor( ( row + 1 ) * resFactor - 1 );
		int r_min = ( int )floor( row * resFactor );

		// column
		int c_max = ( int )floor( ( column + 1 ) * resFactor - 1 );
		int c_min = ( int )floor( column * resFactor );

		// check range
		b_min = b_min < 0 ? 0 : b_min;
		r_min = r_min < 0 ? 0 : r_min;
		c_min = c_min < 0 ? 0 : c_min;

		b_max = b_max >= m_selection->bands() ? m_selection->bands() - 1 : b_max;
		r_max = r_max >= m_selection->rows() ? m_selection->rows() - 1 : r_max;
		c_max = c_max >= m_selection->columns() ? m_selection->columns() - 1 : c_max;

		// number of affected voxels
		int markedVoxel = 0;

		// now we have a cuboid with measures
		// [b_min,b_max] x [r_min, r_max] x [c_min, c_max].
		for( int b = b_min; b <= b_max; b++ )
			for( int r = r_min; r <= r_max; r++ )
				for( int c = c_min; c <= c_max; c++ ) {
					if( m_selection->at( b, r, c ) != value ) {
						m_selection->at( b, r, c ) = value;
						markedVoxel++;
					}
				}

		m_selection->changeVolume( value == 0 ? -markedVoxel : markedVoxel );
		return;
	}

	// 2. Fall res_ana == res_sel -> same resolution; no changes
	if( resFactor == 1 ) {
		if( m_selection->at( band, row, column ) != value ) {
			m_selection->at( band, row, column ) = value;
			m_selection->changeVolume( value == 0 ? -1 : 1 );
		}

		return;
	}

	//3. Fall res_ana > res_sel
	if( resFactor < 1 ) {

		int b = ( int )rintf( band * resFactor );
		int r = ( int )rintf( row * resFactor );
		int c = ( int )rintf( column * resFactor );

		if( m_selection->at( b, r, c ) != value ) {
			m_selection->at( b, r, c ) = value;
			m_selection->changeVolume( value == 0 ? -1 : 1 );
		}
	}

}

int DataManager::createSegment( VImage img )
{

	// create a VistaSegment based on the given image.
	if( img != NULL ) {
		m_seg_map[++m_max_segnumber] = new VistaSegment( img );
	}

	// setColor
	// no color value found in vista image -> set default value
	if( m_seg_map[m_max_segnumber]->color[0] == -1 ) {
		QColor c = nextColor();
		m_seg_map[m_max_segnumber]->setColor( c.red(), c.green(), c.blue() );
	}

	// name
	// no seg_name attribute found by VistaSegment -> take internal id
	if( m_seg_map[m_max_segnumber]->name == 0 ) {
		m_seg_map[m_max_segnumber]->name = m_max_segnumber;
	}

	// select segment
	selCurrentSegment( m_max_segnumber );

	return m_max_segnumber;
}

int DataManager::createSegment()
{

	/*
	 * Since there is no template image given we will use the anatomical
	 * geometry to create a new segment.
	 *
	 * IMPORTANT: vlEdit only accepts isotropic voxels therefore we can use
	 * any resolution coordinate as calculation factor.
	 */

	IVistaImage *anatomie = DATAMANAGER->image();
	float resolution = m_segResolution[0];
	float ana_reso = anatomie->getResolution()[0];

	int bands = ( int )( ana_reso / resolution * anatomie->depth() );
	int rows = ( int )( ana_reso / resolution * anatomie->height() );
	int columns = ( int )( ana_reso / resolution * anatomie->width() );

	m_seg_map[++m_max_segnumber] = new VistaSegment(
		bands,
		rows,
		columns
	);

	// Since the VistaSegment is empty we mu� copy all header informations from
	// the anatomie image.
	VCopyImageAttrs( m_original->src(), m_seg_map[m_max_segnumber]->src() );

	// We still need to adjust some attributes in the newly created VistaSegment
	VAttrList attrlist = VImageAttrList( m_seg_map[m_max_segnumber]->src() );

	// string buffer for new attribute values
	char *setAttrStr = new char[100];
	// to read attributes we only need an "empty" character pointer
	VString getAttrStr;

	// set voxel attribute
	sprintf( setAttrStr, "%1.6f %1.6f %1.6f", resolution, resolution, resolution );
	VSetAttr( attrlist, "voxel", NULL, VStringRepn, ( VString )setAttrStr );

	// float buffer
	float ca0, ca1, ca2;
	// relativ resolution
	float resFactor = DATAMANAGER->image()->getResolution()[0] /
					  m_seg_map[m_max_segnumber]->getResolution()[0];

	//ca
	if ( VGetAttr( attrlist, "ca", NULL, VStringRepn, ( VPointer ) & getAttrStr ) == VAttrFound ) {
		sscanf( getAttrStr, "%f %f %f", &ca0, &ca1, &ca2 );
		sprintf( setAttrStr, "%1.2f %1.2f %1.2f",
				 ca0 * resFactor, ca1 * resFactor, ca2 * resFactor );
		VSetAttr( attrlist, "ca", NULL, VStringRepn, ( VString )setAttrStr );
	}

	//cp
	if ( VGetAttr( attrlist, "cp", NULL, VStringRepn, ( VPointer ) & getAttrStr ) == VAttrFound ) {
		sscanf( getAttrStr, "%f %f %f", &ca0, &ca1, &ca2 );
		sprintf( setAttrStr, "%1.2f %1.2f %1.2f",
				 ca0 * resFactor, ca1 * resFactor, ca2 * resFactor );
		VSetAttr( attrlist, "cp", NULL, VStringRepn, ( VString )setAttrStr );
	}

	//fixpoint
	if ( VGetAttr( attrlist, "fixpoint", NULL, VStringRepn, ( VPointer ) & getAttrStr ) == VAttrFound ) {
		sscanf( getAttrStr, "%f %f %f", &ca0, &ca1, &ca2 );
		sprintf( setAttrStr, "%1.2f %1.2f %1.2f",
				 ca0 * resFactor, ca1 * resFactor, ca2 * resFactor );
		VSetAttr( attrlist, "fixpoint", NULL, VStringRepn, ( VString )setAttrStr );
	}

	// set color
	QColor c = nextColor();
	m_seg_map[m_max_segnumber]->setColor( c.red(), c.green(), c.blue() );

	// set segment name
	m_seg_map[m_max_segnumber]->name = m_max_segnumber;

	// select newly created segment
	selCurrentSegment( m_max_segnumber );

	delete[] setAttrStr;

	return m_max_segnumber;
}

bool DataManager::selCurrentSegment( int id )
{

	// id = 0 is a fallback value which is used in when the last segment has
	// been removed from the list and the datamanager should know about that.
	// i.e. setting m_selection to NULL.
	if( id == 0 ) {
		m_selection = NULL;
		return false;
	}

	// in all other cases we should check for a valif id.
	if( m_seg_map.find( id ) == m_seg_map.end() ) {
		m_selection = NULL;
		std::cerr << "selCurrentSegment(WARNING): id (" << id << ") not found!" << endl;
		return false;
	}

	// set pointer to segment with given id or NULL if there is no such id in
	// the hash map.
	m_selection = m_seg_map[id];

	return true;
}

int DataManager::removeSegment( int id )
{

	// if map emtpy return
	if( m_seg_map.empty() )
		return -1;

	m_seg_map.erase( id );

	// reset segment pointer
	// IMPORTANT: the segment pointer must be set by hand to ensure that a
	// valid segment is still selected.
	m_selection = NULL;

	return 0;
}

void DataManager::setSegResolution( float res )
{
	if( m_copy == NULL ) {
		qFatal( "Error loading anatomie!" );
		exit( -1 );
	}

	// no resolution given -> use anatomical resolution
	if( res <= 0 ) {
		res = m_copy->getResolution()[0];
	} else {
		if( res < m_copy->getResolution()[0] ) {
			qFatal( "Error: segment resolution smaller than resolution of anatomie!" );
			exit( -1 );
		}
	}

	// set resolution to the given value
	for( int i = 0; i < 3; i++ )
		m_segResolution[i] = res;

	//TODO DEBUG
//	qWarning( QString("Resolution (Segment): ") + QString::number( m_segResolution[0] )
//			  + QString("x") + QString::number( m_segResolution[1] )
//			  + QString("x") + QString::number( m_segResolution[2] ) );

}

QColor DataManager::nextColor()
{

	QColor color ( m_colorPointer, 255, 255, QColor::Hsv );

	// translate color pointer
	m_colorPointer += 120;
	// don't leave the intervall [0, 359]!
	m_colorPointer = m_colorPointer >= 360 ?
					 m_colorPointer - 360 : m_colorPointer;

	// translate origin after a complete circle
	if( m_colorPointer == m_colorOrigin ) {
		m_colorOrigin += 30;
		m_colorOrigin =  m_colorOrigin >= 360 ?
						 m_colorOrigin - 360 : m_colorOrigin;
		m_colorPointer = m_colorOrigin;
	}

	// return new color
	return color;

}

VistaSegment *DataManager::segment( int id )
{

	// if valid id return segment pointer
	if( ( id > 0 ) && ( id <= m_max_segnumber ) )
		return m_seg_map[id];

	return 0;
}

void DataManager::setOutputType( short type )
{

	// set only valid output types.
	if( ( type == 0 ) || ( type == 1 ) )
		m_output = type;
	else {
		qWarning( "wrong output type: %i", type );

	}
}

bool DataManager::isValidSegment( VImage img )
{

	// check voxel parameter
	VAttrList attrList = VImageAttrList( img );
	VString content;
	VGetAttr( attrList, "voxel", NULL, VStringRepn, &content );

	QStringList resList = QStringList::split( " ", content );

	// wrong number of intergers in string (format error)
	if( resList.size() != 3 ) {
		qWarning( "Error reading attribute (voxel): wrong number of arguments" );
		return false;
	}

	QStringList::const_iterator iter = resList.begin();

	if( ( ( *( iter++ ) ).toFloat() != m_segResolution[0] )
		|| ( ( *( iter++ ) ).toFloat() != m_segResolution[1] )
		|| ( ( *( iter++ ) ).toFloat() != m_segResolution[2] ) ) {
		iter = resList.begin();
		QString segment_res(
			( *( iter++ ) ) + QString(", ") +
			( *( iter++ ) ) + QString(", ") +
			( *( iter++ ) ) );
		QString config_res(
			QString::number( m_segResolution[0] ) + QString(", ") +
			QString::number( m_segResolution[1] ) + QString(", ") +
			QString::number( m_segResolution[2] ) );
		QString error(
			QString("Error loading vimage: resolution (") +
			segment_res +
			QString(") is not compatible with current settings (") +
			config_res + QString(").") );
//		qWarning( error.latin1() );
		return false;
	}

	return true;
}

vector<int> DataManager::emptySegments()
{

	vector<int> segs;

	// traverse vector list and collect all EMPTY segments
	vector<int>::iterator iter = UICONFIG->segList()->begin();

	while( iter != UICONFIG->segList()->end() ) {
		if( segment( *iter )->getVolume() == 0.0 )
			segs.push_back( segment( *iter )->name );

		iter++;
	}

	return segs;
}

vector<int> DataManager::invisibleSegments()
{

	vector<int> segs;

	// traverse vector list and collect all INVISIBLE segments
	vector<int>::iterator iter = UICONFIG->segList()->begin();

	while( iter != UICONFIG->segList()->end() ) {
		if( !segment( *iter )->visible )
			segs.push_back( segment( *iter )->name );

		iter++;
	}

	return segs;
}

int DataManager::findByName( int name )
{

	int count = 0;

	// out of UByte range
	if( ( name < 1 ) || ( name > 255 ) )
		return count;

	_SEG_MAP_TYPE::iterator iter = m_seg_map.begin();

	while( iter != m_seg_map.end() ) {
		if( ( *iter ).second->name == name )
			count++;

		iter++;
	}

	return count;
}

void DataManager::checkList()
{

	char *stars = "*************************";

	std::cerr << stars << std::endl << "Check segment list" << std::endl;
	std::cerr << "Size:" << m_seg_map.size() << std::endl;



	_SEG_MAP_TYPE::iterator iter = m_seg_map.begin();

	while( iter != m_seg_map.end() ) {
		std::cerr << "id: " << ( *iter ).first << "\t";

		if( ( *iter ).second == NULL )
			std::cerr << "NULL" << std::endl;
		else
			std::cerr << ( *iter ).second->name << std::endl;

		iter++;
	}

	cout << stars << endl;

}

IVistaImage *DataManager::transColorSpace( VImage img )
{

	/* call helper method according to the data representation */
	switch( VPixelRepn( img ) ) {
	case VUByteRepn:
		/* nothing to do */
		return createVistaImage( VCopyImage( img, NULL , VAllBands ) );
	case VSByteRepn:
		return tcs<VSByte>( img );
	case VShortRepn:
		return tcs<VShort>( img );
	default:
		qWarning( "Datentyp nicht zulaessig" );
		abort();
	}
}

template <class T> IVistaImage *DataManager::tcs( VImage img )
{

	/* 'cut-off' threshold values */
	static VDouble black = 0.01;
	static VDouble white = 0.01;

	int nbands = VImageNBands( img );
	int nrows  = VImageNRows( img );
	int ncols  = VImageNColumns( img );
	int npixels = nbands * nrows * ncols;
	int smin = ( int )VRepnMinValue( VPixelRepn( img ) );
	int smax = ( int )VRepnMaxValue( VPixelRepn( img ) );
	int dim = smax - smin + 1;
	int i, j;

	IVistaImage *dest_img = new VistaImage<VUByte>( nbands, nrows, ncols, VUByteRepn );
	VImageAttrList( dest_img->src() ) = VCopyAttrList( VImageAttrList( img ) );

	/* create histogram */
	float *histo = ( float * ) VMalloc( sizeof( float ) * dim );

	for ( j = 0; j < dim; j++ ) histo[j] = 0;

	T *src_pp = ( T * ) VImageData( img );

	for ( i = 0; i < ( int )( npixels / 4.0 ); i++ ) {
		j = *src_pp;
		src_pp += 4;
		j -= smin;
		histo[j]++;
	}

	float sum = 0;

	for ( j = 0; j < dim; j++ ) sum += histo[j];

	/* convert to fractional values */
	for ( j = 0; j < dim; j++ ) histo[j] /= sum;

	/* skip lower border */
	sum = 0;

	for ( j = 0; j < dim; j++ ) {
		sum += histo[j];

		if ( sum > black ) break;
	}

	int xmin = j + smin;

	/* skip higher border */
	sum = 0;

	for ( j = dim; j > 0; j-- ) {
		sum += histo[j];

		if ( sum > white ) break;
	}

	int xmax = j + smin;

	/* free memory */
	VFree( histo );

	/* get scope (scale factor) */
	float alpha = ( float )255.0 / ( float )( xmax - xmin );

	/* transform color values */
	src_pp = ( T * ) VImageData( img );
	VUByte *dest_pp = ( VUByte * ) VImageData( dest_img->src() );

	for( i = 0; i < npixels; i++ ) {
		int mycol = ( int )rint( ( double )( ( float )( *( src_pp++ ) - xmin ) * alpha ) );

		if( mycol < 0 ) mycol = 1;

		if( mycol > 255 ) mycol = 255;

		*( dest_pp++ ) = ( VUByte )mycol;
	}

	return dest_img;
}

