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
#ifndef VLCAMERA_H
#define VLCAMERA_H

/**
Diese Klasse repräsentiert die Kamera der GLWidgets. Vorerst nutzt sie lediglich die Position, d.h.
Rotation wird (noch) nicht unterstützt.
@author Hannes Niederhausen
*/
class vlCamera
{

protected:
	/**Positionen der Kamera in Worldspace*/
	float posX, posY, posZ;

public:
	vlCamera( float x = 0, float y = 0, float z = 0 );

	~vlCamera();

	void setGlTranslation();

	void moveX( float x ) {posX += x;};

	void moveY( float y ) {posY += y;};

	void moveZ( float z ) {posZ += z;};

	void move ( float x, float y, float z ) {posX += x; posY += y; posZ += z;};

	/**
	 * Liefert die Position der Kamera als integer array in x-y-z-Reihenfolge.
	 */
	void getPosition( float pos[] );

	float x() {return posX;};

	float y() {return posY;};

	float z() {return posZ;};

	void setPos( float x, float y, float z );

};

#endif
