#ifndef VIMAGEMANAGER_H_
#define VIMAGEMANAGER_H_

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <qptrlist.h>
#include <qstring.h>

/**
 * This class manages all the VImage stuff, which means, loading the
 * functional data and creating the 3D images for every timeslice.
 * After that he provides methods to get the ubyte data
 * of the current timeclice for the 3 views.
 *
 * @author Hannes Niederhausen 2007
 */
class VImageManager
{
public:
	VImageManager();
	virtual ~VImageManager();

	/**
	 * Loads the given file and creates for every timeslice a VImage
	 */
	void init( VAttrList list );

	unsigned char *coronarData() {return m_coronarData;};
	unsigned char *axialData() {return m_axialData;};
	unsigned char *sagittalData() {return m_sagittalData;};

	int bands() {
		return VImageNBands( ( m_imageList.at( 0 ) ) );
	};
	int rows() {
		return VImageNRows( ( m_imageList.at( 0 ) ) );
	};
	int cols() {
		return VImageNColumns( ( m_imageList.at( 0 ) ) );
	};

	int xmin() {
		return m_xmin;
	};
	int xmax() {
		return m_xmax;
	};


	void updateViewData( int col, int row, int band, int currTime );

	unsigned short getValue( int band, int row, int col, int time );

	int numOfTimeSteps() {
		return m_imageList.count();
	};

	void setAnamean( double anamean ) {
		m_anamean = anamean;
	};

	void setAnaalpha( double anaaplha ) {
		m_anaalpha = anaaplha;
	};

	double getAnaalpha() {
		return m_anaalpha;
	};

	void setHistgrammCutOff( double black, double white ) {
		m_black = black;
		m_white = white;
	};

private:
	double m_anamean;
	double m_anaalpha;
	int m_xmin, m_xmax;
	int m_currTime;
	double m_black;
	double m_white;
	double m_scaleValue;
	QPtrList<V_ImageRec> m_imageList;


	void prepareScaleValue();
	VImage scaleImage( VImage src, VImage dest, double factor );
	VImage vtimestep( VAttrList, VImage, int );

	unsigned char *m_coronarData;
	unsigned char *m_sagittalData;
	unsigned char *m_axialData;

};

#endif /*VIMAGEMANAGER_H_*/
