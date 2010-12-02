/***************************************************************************
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
 ***************************************************************************/

/****************************************************************************
 * Segments:
 *
 * - Segments are VistaImages of the type 'bit' with a number of
 *   additional attributes.
 * - Segments are stored in a hash map. Each of them is identified by a
 *   unique integer id.
 * - The currently active segment is identified by the member variable
 *   m_selection.
 ****************************************************************************/
#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include <qcolor.h>

#include "vistaimage.h"


#define DATAMANAGER DataManager::instance()
#define _SEG_MAP_TYPE std::map<const int, VistaSegment*>

/**
 * This class provides an interface to access the image data from the anatomie
 * and segment images. That includes loading and saving procedures, a map for
 * storing the segment images and a number of support methods for accessing
 * various image attributes like resolution and color values.
 *
 * It's desinged with the singleton pattern in mind and so there should be only
 * one instance at runtime in use. This instance can be accessed via the
 * DATAMANAGER macro in the following way:
 *
 * \code DATAMANAGER->selection()\endcode
 *
 * This would return a pointer to the currently selected segment image.
 *
 * @author Hannes Niederhausen
 * @author Thomas Pr&ouml;ger
 */
class DataManager
{

private:
	template <class T> IVistaImage *tcs( VImage img );

protected:

	static DataManager *singleton;

	/** The original anatomie image. This is only used as a backup buffer. */
	IVistaImage *m_original;

	/** A copy of the original anatomie image. It's used as a working copy where
	 * one can delete fragments in the <i>anatomie</i> mode.*/
	IVistaImage *m_copy;

	/** The currently active segment image. This pointer refers to a
	 * VistaSegment object, stored in the hash map, which is the the currently
	 * selected segment affected by editing operations. */
	VistaSegment *m_selection;

	/** An associative map which contains all segments. All loaded or created
	 * segments are stored in this hash map and assigned to a unique integer id.
	 * This id is shown in the segment window and will be used as a kind of color
	 * value in a multi-segment-ubyte-image. This means all voxel of the segment
	 * with the id 'x' will have the ubyte value 'x' in the resulting vista
	 * image.
	 */
	_SEG_MAP_TYPE m_seg_map;

	/** Flag, indicating that a valid anatomie image has been loaded.
	 * If this flag is FALSE then most of the intern operations will fail and
	 * vlEdit will quit with an error message.*/
	bool m_valid_image;

	/** The segment counter. It records the number of segments and is increased
	 * everytime a segment is loaded or newly created. Therefore it is used to
	 * assure that a new segment get's an unique id. Be aware that the counter
	 * isn't decreased when a segment is deleted so there is no warranty that
	 * the value of this variable corresponds to the real number of loaded
	 * segments.
	 */
	int m_max_segnumber;

	/** The parameter 'ca' from the anatomie image file.*/
	float *m_ca;
	/** The parameter 'extent' from the anatomie image file.*/
	float *m_extent;
	/** The midpoint of the anatomie image. This value is only be used when
	 * a ca value is available.
	 */
	float *m_midPoint;
	/** The parameter 'voxel' from the anatomie image file.*/
	float *m_segResolution;

	/** The origin of the current color cycle.
	 * This variable holds the origin which is used for traversing the color
	 * cycle. At the end of each "round" the origin will be shifted for 30°.
	 * The start value is 0.
	 */
	int m_colorOrigin;
	/** The current color value during the traversion of the color cycle. */
	int m_colorPointer;

	/** The output type for saved segments.
	 * '0' codes ubyte and '1' bit.*/
	short m_output;


	/**
	 * Creates a VistaImage based on the given VImage image.
	 * The template sepecific type of VistaImage will be set according
	 * to the data representation of the VImage.
	 *
	 * @param[in] img a VImage which is used as base object for the resulting
	 * VistaImage.
	 * @return An empty VistaImage with the data representation of the given
	 * VImage.
	 */
	IVistaImage *createVistaImage( VImage img );

	/** Default constructor.
	 * According to the singleton desing pattern this object shouldn't be
	 * instantiated directly. Therefore it's constructor is declared protected.*/
	DataManager();

	/**
	 * protected method to check the segments attributes like voxel
	 * resolution.
	 *
	 * @param[in] seg the vista image which should be validated
	 * @return FALSE if there is an invalid attribute, TRUE otherwise
	 */
	bool isValidSegment( VImage seg );

	/* This method transforms the color values of a given Vista image into a ubyte color space. */
	IVistaImage *transColorSpace( VImage img );


public:

	/** The Singleton instance method.
	 * According to the singleton design pattern, every singleton object
	 * should provide this access method for the singleton instance. So every
	 * call to this objects members should be done via
	 *
	 * \code DataManager::instance()->method() \endcode
	 *
	 * This call is wrapped into the DATAMANAGER macro.
	 */
	static DataManager *instance() {
		if ( singleton == NULL )
			singleton = new DataManager();

		return singleton;
	}

	/** Default destructor */
	~DataManager();

	/**
	 * This method tries to load the given file.
	 * The filename should name a readable file in vista image format.
	 *
	 * @param filename String naming the file which should be loaded.
	 *
	 * \see loadVistaImage(FILE *fp)
	 */
	void loadVistaImage( const char *filename );

	/**
	 * This method loads the vista image informations into memory.
	 * It is subsequently called to load the vista image informations and split
	 * them into VistaImage objects. These objects are stored in an interal
	 * hash map.
	 *
	 * @param fp File pointer to a vista image file stream. The file should
	 * have been opened in read-only mode.
	 *
	 * \see loadVistaImage(const char* filename)
	 */
	void loadVistaImage( FILE *fp );

	/**
	 * This method loads the segment informations from a file.
	 * Loads the segment informations from a vista image named by the given
	 * string. It tries to open the file in read-only mode and assigns it to a
	 * file pointer.
	 *
	 * \param filename A string naming the vista image file which should be
	 * loaded.
	 * \return A list with the ids of the loaded segments.
	 *
	 * \see loadSegments(FILE *fp).
	 */
	std::vector<int> loadSegments( const char *filename );

	/**
	 * Loads all segment images from a vista image file.
	 *
	 * This method loads all segments from a vista image file and creates
	 * a VistaSegment for each image found. These new segments are stored in the
	 * internal segment hash map.
	 *
	 * \param fp File pointer to a vista image file stream containing the segment
	 * images. The file should have been opened in read-only mode.
	 * \return A list with the ids of the loaded segments.
	 *
	 * \see loadSegments(const char* filename)
	 */
	std::vector<int> loadSegments( FILE *fp );

	/**
	 * This method creates a new VistaSegment and adds it to the internal
	 * segment hash map. The pointer for the currently active segment will be
	 * set to this object.
	 *
	 * The attributes of the new VistaSegment will be based on the informations
	 * given in the VImage template object.
	 *
	 * \param img A VImage extracted from a vista image file which describes the
	 * segment to be created.
	 * \return The id of the newly created VistaSegment.
	 */
	int createSegment( VImage img );

	/**
	 * This method creates a new empty VistaSegment and adds it to the internal
	 * segment hash map. The pointer for the currently active segment will be
	 * set to this object.
	 *
	 * Hence there is no template image, the attributes of the new VistaSegment
	 * are based on the anatomie's attributes.
	 *
	 * \return The id of the newly created VistaSegment.
	 * */
	int createSegment();

	/**
	 * This method sets the pointer of the currently selected segment to the
	 * segment with the given id.
	 *
	 * \param id The id of the segment which should be selected.
	 * \return TRUE if a segment with the given id exists, else FALSE.
	 */
	bool selCurrentSegment( int id );

	/**
	 * This method removes the given segment from the internal segment hash map.
	 *
	 * Removes the segment with the given id from the segment hash map.
	 * ATTENTION: after deleting the segment, the pointer to the current segment
	 * will be set to NULL regardless if there are segments left in the list. If
	 * there a more segments they must be selected "manually". The reason for
	 * this is that the selection for the next segments depends on the segment's
	 * order in the segment dialog table and not from the internal order in the
	 * hash map.
	 *
	 *
	 * Entfernt das gewÃ¤hlte Segment aus der Segment-Map. Bei Misserfolg wird
	 * -1 ansonst 0 zurueckgegeben.
	 * ACHTUNG: Nach dem Entfernen eines Segmentes wird der laufende
	 * Segmentzeiger auf NULL gesetzt! Falls noch vorhanden, muß ein neues
	 * Segment "per Hand" gewaehlt werden. Das wurde deswegen gemacht, da die
	 * Auswahl des neuen Segments von der Ordnung innerhalb der Tabelle und nicht
	 * innerhalb internen HashMap abhaengt.
	 *
	 * \param id The id of the segment which should be removed.
	 * \return -1 if a segment with the given id couldn't be found, else 0.
	 */
	int removeSegment( int id );

	/**
	 * Returns the working copy of the anatomie image.
	 * @return A pointer to the working copy of the anatomie image.
	 */
	IVistaImage *image() {
		return m_copy;
	}

	/**
	 * Returns the currently selected segment.
	 * @return A pointer to the currently selected segment.
	 */
	VistaSegment *selection() {
		return m_selection;
	}

	/**
	 *
	 * Deletes the specified region from the anatomie or segment image.
	 *
	 * This method deletes a spheric region  with the given radius starting at
	 * the x/y/z-coordinates. The color value from all voxels inside the sphere
	 * will be set to 0.
	 *
	 * \param selection This flag indicates if the voxel should be removed from
	 * the anatomie or the active segment.
	 * \param band The band coordinate of the midpoint.
	 * \param row The row coordinate of the midpoint.
	 * \param column The column coordinate of the midpoint.
	 * \param r The radius of the sphere.
	 *
	 * \see selectVoxel(int band, int row, int column, int r)
	 */
	void deleteVoxel( bool selection, int band, int row, int column, int r );

	/**
	 * Deletes the specified region from the anatomie or segment image
	 * respecting the given delta color value.
	 * Only the color value of all voxels inside the sphere whose original color
	 * value lie within the delta intervall will be set to 0.
	 *
	 * \param selection This flag indicates if the voxel should be removed from
	 * the anatomie or the active segment.
	 * \param band The band coordinate of the midpoint.
	 * \param row The row coordinate of the midpoint.
	 * \param column The column coordinate of the midpoint.
	 * \param r The radius of the sphere.
	 * \param delta the maximum color tolerance for the grey value which should
	 * be deleted.
	 *
	 * \see selectSegment(int band, int row, int column, int r, int delta)
	 */
	void deleteSegment( bool selection,
						int band,
						int row,
						int column,
						int r,
						int delta );

	/**
	 * Saves the anatomie image.
	 *
	 * This method saves a the working copy of the anatomie image to a vista
	 * image file.
	 *
	 * @param filename A string naming the vista image file to which should be
	 * written.
	 */
	void save( const char *filename );

	/**
	 * Indicates if a valid anatomie image could be loaded.
	 *
	 * @return TRUE if an anatomie image has been loaded, else FALSE.
	 */
	bool isValid() {
		return m_valid_image;
	}

	/**
	 * Indicates if a valid VistaSegment is available for editing operations.
	 *
	 * \return TRUE if there is a valid VistaSegment, else FALSE.
	 */
	bool isValidSegment() {
		return !(
				   ( m_selection == NULL )
				   || m_seg_map.empty()
			   );
	}

	/**
	 * This method returns the ca value of the anatomie image.
	 *
	 * \return NULL if there is no ca value or a dynamic array of size 3 with
	 * the band, row and column coordinates.
	 */
	float *ca() {return m_ca;};

	/** This method returns the extent attribute of the anatomie image.
	 *
	 * \return NULL if there is no extent value or a dynamic array of size 3 with
	 * the band, row and column coordinates.
	 */
	float *extent() {return m_extent;}

	/**
	 * This method returns the midpoint of the anatomie image.
	 *
	 * @return A dynamic array of size 3 with the band, row and column
	 * coordinates of the midpoint.
	 */
	float *midPoint();

	/**
	* Selects the specified region in the segment image.
	*
	* This method selects a spheric region in the segment image with the given
	* radius starting at the x/y/z-coordinates. All Voxels within this sphere
	* will be marked with the value '1'.
	*
	* \param band The band coordinate of the midpoint.
	* \param row The row coordinate of the midpoint.
	* \param column The column coordinate of the midpoint.
	* \param r The radius of the sphere.
	*
	* \see deleteVoxel(bool selection, int band, int row, int column, int r)
	*/
	void selectVoxel( int band, int row, int column, int r );

	/**
	 * Selects the specified region in the segment image according to the given
	 * color delta value.
	 *
	 * This method selects a spheric region in the segment image with the given
	 * radius starting at the x/y/z-coordinates. All Voxels within this sphere
	 * whose anatomical counterparts are within the given color range, defined
	 * by the delta parameter and the color value at the cursor position, will
	 * be marked with the value '1'.
	 *
	 * \param band The band coordinate of the midpoint.
	 * \param row The row coordinate of the midpoint.
	 * \param column The column coordinate of the midpoint.
	 * \param r The radius of the sphere.
	 * \param delta the maximum color tolerance for the grey value which should
	 * be selected.
	 *
	 * \see
	 * deleteSegment(bool selection, int band, int row, int column, int r,
	 * int delta)
	 */
	void selectSegment( int band, int row, int column, int r, int delta );

	/**
	 * This method marks segment voxels depending on the segment resolution.
	 *
	 * When selecting a single anatomical voxel there could be a number of
	 * corresponding voxel in the selection image according to the selection's
	 * resolution. This method finds these selection voxel and sets their value
	 * to 'val'.
	 *
	 * \param band The band coordinate of the midpoint.
	 * \param row The row coordinate of the midpoint.
	 * \param column The column coordinate of the midpoint.
	 * \param val The value to which the voxel should be set.
	 *
	 * \see selectSegment(int band, int row, int column, int r, int delta),
	 * selectVoxel(int band, int row, int column, int r)
	 */
	void selectSingleVoxel( int band, int row, int column,  int val );

	/**
	 * Saves a number of segments.
	 *
	 * This method saves some or all of the segments according to the given
	 * parameter.
	 *
	 * \param filename String which names the save file.
	 * \param saveAll This flag indicates if all segments should be saved:
	 * - TRUE -> all segments are written to the given file.
	 * - FALSE -> only segments which are marked as visible are written to the
	 * file.
	 */
	void saveSelection ( const char *filename, const bool saveAll = false );

	/**
	 * This method returns the segments resolution.
	 *
	 * \return A float array of size 3 with the resolution factors for band, row
	 * and column.
	 */
	float *getSegResolution() {
		return m_segResolution;
	}

	/**
	 * This method sets the resolution values for band, row and column to the
	 * given value.
	 *
	 * In this implementation of vlEdit all values are the same. So we are
	 * working with isotropic voxels.
	 *
	 * \param res The resolution factor to which all voxel coordinates are set.
	 */
	void setSegResolution( float res );

	/**
	 * Returns a new QColor object.
	 *
	 * This Method returns a QColor obejct with a new color value according
	 * to the prior generated color. To get a new color value the color circle
	 * will be processed in 120° steps until it reaches 360°. Then the rotation
	 * origin will be shifted by 30° and next 120°-step-run will be started.
	 *
	 * \see m_colorOrigin and m_colorPointer
	 */
	QColor nextColor();

	/**
	 * Returns the segment with the given id.
	 *
	 * \return Returns a segment with this id or 0 when this segment does not
	 * exist.
	 */
	VistaSegment *segment( int id );

	/**
	 * Sets the output type for all saved segments.
	 *
	 * Sets the output type for all segments that will be saved during this
	 * session.
	 *
	 * \param type The output type. Accepted values are 0 (ubyte) and 1 (bit).
	 */
	void setOutputType( short type );

	/**
	 * Returns the configured output type for saving segments.
	 *
	 * \return The output type for save procedures: 0 (ubyte) or 1 (bit).
	 */
	short outputType() { return m_output; }

	/**
	 * Returns a list of names from all empty segments
	 *
	 * \return Vector with the integer names of all emtpy segments.
	 */
	vector<int> emptySegments();

	/**
	 * Returns a list of name from all invisible segments
	 *
	 * \return Vector with the interger names of all invisible segments.
	 */
	vector<int> invisibleSegments();

	/**
	 * Traverse through segment list and look out for a segment with the
	 * given name.
	 *
	 * \param name Interger name of the segment we are looking for.
	 * \return The number of segments with the given name.
	 */
	int findByName( int name );

	/*************** DEBUG *****************/
	void checkList();
	/*************** DEBUG *****************/

};

#endif
