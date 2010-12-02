#include "VImageManager.h"

#include <stdio.h>
#include <memory.h>
#include <math.h>

extern "C" {
#include <via.h>
}
#define DEBUGING 0
#define NP -32768
#define MFT 65535
#define ABS(x)      (x<0)?-x:x

VImageManager::VImageManager()
{
	m_black = 0.01;
	m_white = 0.01;
	m_xmax = -32700;
	m_xmin = 32000;

}

VImageManager::~VImageManager()
{
}

void VImageManager::init( VAttrList list )
{
	VAttrListPosn posn;
	int nbands = 0;
	int nrows = 0;
	int ncols = 0;
	int timeSlices = 2;
	VImage src;
	VString str;

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn )
			continue;

		VGetAttrValue ( &posn, NULL, VImageRepn, &src );

		if ( VPixelRepn( src ) != VShortRepn )
			continue;

		// check if we have a functional data image
		if ( ( VGetAttr ( VImageAttrList ( src ), "MPIL_vista_0", NULL,
						  VStringRepn, ( VPointer ) & str ) == VAttrFound ) ||
			 ( VGetAttr ( VImageAttrList ( src ), "repetition_time", NULL,
						  VStringRepn, ( VPointer ) & str ) == VAttrFound ) ) {
			if ( VImageNRows( src ) > nrows )
				nrows = VImageNRows( src );

			if ( VImageNColumns( src ) > ncols )
				ncols = VImageNColumns( src );

			if ( VImageNBands( src ) > timeSlices )
				timeSlices = VImageNBands( src );

			nbands++;
		}
	}

	if ( nbands == 0 )
		VError( "No raw data found" );

	// now loading the vimages
	int currentTimeSlice = 0;   // curr slice

	for ( currentTimeSlice = 0; currentTimeSlice < timeSlices; currentTimeSlice++ ) {
		VImage dest = VCreateImage( nbands, nrows, ncols, VShortRepn );
		vtimestep( list, dest, currentTimeSlice );
		m_imageList.append( dest );
	}

	prepareScaleValue();



	// creating data arrays
	int size = ncols * nbands;
	m_coronarData = new unsigned char[size];
	memset( m_coronarData, 0, size * sizeof( unsigned char ) );

	size = ncols * nrows;
	m_axialData = new unsigned char[size];
	memset( m_axialData, 0, size * sizeof( unsigned char ) );

	size = nbands * nrows;
	m_sagittalData = new unsigned char[size];
	memset( m_sagittalData, 0, size * sizeof( unsigned char ) );
}

VImage VImageManager::scaleImage( VImage src, VImage dest, double factor )
{
	int nPixel = VImageSize( src ) / VPixelSize( src );

	if ( dest == NULL ) {
		dest = VCreateImage(
				   VImageNBands( src ),
				   VImageNRows( src ),
				   VImageNColumns( src ),
				   VUByteRepn );
	}


	VUByte *destPtr = ( VUByte * ) VImageData( dest );
	VShort *srcPtr = ( VShort * ) VImageData( src );

	srcPtr = ( VShort * ) VImageData( src );

	for ( int i = 0; i < nPixel; i++ ) {
		if ( *srcPtr <= m_xmin )
			*destPtr = 0;
		else if ( *srcPtr >= m_xmax )
			*destPtr = 255;
		else {
			int val = *srcPtr - m_xmin;
			*destPtr = ( VUByte ) ( val * factor );
		}

		srcPtr++;
		destPtr++;
	}

	return dest;
}

void VImageManager::prepareScaleValue()
{
	QPtrList<V_ImageRec>::iterator it = m_imageList.begin();

	unsigned int lastIndex = 90;

	if ( lastIndex >= m_imageList.count() )
		lastIndex = m_imageList.count();

	for ( unsigned int i = 0; i < lastIndex; i++ ) {

		VImage src = m_imageList.at( i );
		int ncols = VImageNColumns( src );
		int nrows = VImageNRows( src );
		int nbands = VImageNFrames( src );

		int npixels = nbands * nrows * ncols;
		int smin = ( int )VRepnMinValue( VShortRepn );
		int smax = ( int )VRepnMaxValue( VShortRepn );
		int i = 0, j = 0;
		int dim = 2 * smax + 1;

		float *histo = new float[dim];

		for ( j = 0; j < dim; j++ ) histo[j] = 0;

		VShort *src_pp = ( VShort * ) VImageData( src );

		for ( i = 0; i < ( int )( npixels ); i++ ) {
			j = *src_pp;
			src_pp ++;
			j -= smin;
			histo[j]++;
		}

		float sum = 0;

		for ( j = 0; j < dim; j++ ) sum += histo[j];

		for ( j = 0; j < dim; j++ ) histo[j] /= sum;

		sum  = 0;

		for ( j = 0; j < dim; j++ ) {
			sum += histo[j];

			if ( sum > m_black ) break;
		}

		int xmin = j + smin;

		sum = 0;

		for ( j = dim; j > 0; j-- ) {
			sum += histo[j];

			if ( sum > m_white ) break;
		}

		int xmax = j + smin;


		if ( xmin < m_xmin ) m_xmin = xmin;

		if ( xmax > m_xmax ) m_xmax = xmax;
	}
}
VImage VImageManager::vtimestep( VAttrList list, VImage dest, int step )
{
	VFillImage( dest, VAllBands, 0 );
	VPointer src_pp = NULL;
	VPointer dest_pp = NULL;
	VAttrListPosn posn;
	VShort *ptr1 = NULL;
	VShort *ptr2 = NULL;
	VString str;
	VImage src;

	int n = 0;
	int npixels = 0;

	bool revert = false;

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn )
			continue;

		VGetAttrValue ( &posn, NULL, VImageRepn, &src );

		if ( VPixelRepn( src ) != VShortRepn )
			continue;


		if ( ( VGetAttr ( VImageAttrList ( src ), "MPIL_vista_0", NULL,
						  VStringRepn, ( VPointer ) & str ) != VAttrFound ) &&
			 ( VGetAttr ( VImageAttrList ( src ), "repetition_time", NULL,
						  VStringRepn, ( VPointer ) & str ) != VAttrFound ) )
			continue;


		/* doch nicht rumdrehen!
		   if (VGetAttr (VImageAttrList (src), "extent", NULL,
		   VStringRepn, (VPointer) & str) != VAttrFound)
		   revert = true;
		*/

		if ( n == 0 )
			VCopyImageAttrs ( src, dest );

		if ( VImageNRows( src ) > 1 ) {
			if ( VSelectBand ( "vtimestep", src, step, &npixels, &src_pp ) == FALSE )
				VError( "err reading data" );

			int destBand = ( revert ) ? VImageNBands( dest ) - n - 1 : n;

			dest_pp = VPixelPtr( dest, destBand, 0, 0 );

			ptr1 = ( VShort * ) src_pp;
			ptr2 = ( VShort * ) dest_pp;
			memcpy ( ptr2, ptr1, npixels * sizeof( VShort ) );
		}

		n++;
	}

	return dest;
}
unsigned short VImageManager::getValue( int band, int row, int col, int time )
{
	VImage image = m_imageList.at( time );

	return ( VShort )VPixel ( image, band, row, col, VShort );
}

void VImageManager::updateViewData( int col, int row, int band, int currTime )
{
	// retrieving image for the current time
	VImage img = m_imageList.at( currTime );
	m_currTime = currTime;
	// initializing helpers
	int imgCol = 0,
		imgRow = 0,
		imgBand = 0;
	VShort *** imgPtr = VPixelArray( img, VShort );
	int size = 0;

	int nrows = VImageNRows( img );
	int ncols = VImageNColumns( img );
	int nbands = VImageNBands( img );

	short tmp;

	//creating coronar image
	size = ncols * nbands;
	imgRow = row;

	for ( int i = 0; i < size; i++ ) {

		tmp = ( short ) ( imgPtr[imgBand][imgRow][imgCol] );

		if ( tmp > 0 )
			tmp = ( int )rint( ( double )( ( float )( tmp - m_anamean ) * m_anaalpha ) );

		if ( tmp < 0 )
			tmp = 0;

		if ( tmp > 255 )
			tmp = 255;

		m_coronarData[i] = ( unsigned char ) tmp;
		imgCol++;

		if ( imgCol >= ncols ) {
			imgCol = 0;
			imgBand++;
		}

	}

	//creating sagittal image
	size = nbands * nrows;
	imgCol = col;
	imgRow = 0;
	imgBand = 0;

	for ( int i = 0; i < size; i++ ) {
		tmp = ( short ) ( imgPtr[imgBand][imgRow][imgCol] );

		if ( tmp > 0 )
			tmp = ( int )rint( ( double )( ( float )( tmp - m_anamean ) * m_anaalpha ) );

		if ( tmp < 0 )
			tmp = 0;

		if ( tmp > 255 )
			tmp = 255;

		m_sagittalData[i] = ( unsigned char ) tmp;

		imgRow++;

		if ( imgRow >= nrows ) {
			imgRow = 0;
			imgBand++;
		}

	}

	//creating axial image
	size = ncols * nrows;
	imgRow = 0;
	imgCol = 0;
	imgBand = band;

	for ( int i = 0; i < size; i++ ) {
		tmp = ( short ) ( imgPtr[imgBand][imgRow][imgCol] );

		if ( tmp > 0 )
			tmp = ( int )rint( ( double )( ( float )( tmp - m_anamean ) * m_anaalpha ) );

		if ( tmp < 0 )
			tmp = 0;

		if ( tmp > 255 )
			tmp = 255;

		m_axialData[i] = ( unsigned char ) tmp;

		imgCol++;

		if ( imgCol >= ncols ) {
			imgCol = 0;
			imgRow++;
		}

	}


}
