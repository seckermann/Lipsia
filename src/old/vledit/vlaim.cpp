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
#include "vlaim.h"
#include "uiconfig.h"

#include <GL/gl.h>

vlAim::vlAim( float x, float y )
{
	m_posX = x;
	m_posY = y;
}

vlAim::~vlAim()
{
}

void vlAim::draw()
{
	// crossfade is only shown when visible flag is set to true
	if( UICONFIG->isAimVisible() ) {

		glDisable( GL_TEXTURE_2D );
		glLineWidth( 1 );
		glColor3f( 1, 0, 0 );
		glBegin( GL_LINES );
		//waagerechte Achse
		glVertex3f( m_posX, -1000, .1 );
		glVertex3f( m_posX,  m_posY - 10, .1 );

		glVertex3f( m_posX,  m_posY + 10, .1 );
		glVertex3f( m_posX,  1000, .1 );
		//senkrechte Achse
		glVertex3f( -1000, m_posY, .1 );
		glVertex3f( m_posX - 10, m_posY, .1 );

		glVertex3f( m_posX + 10, m_posY, .1 );
		glVertex3f( 1000, m_posY, .1 );
		glEnd();
		//Punkt in der Mitte
		glBegin( GL_POINTS );
		glVertex3f( m_posX, m_posY, .1 );
		glEnd();

	}

}

void vlAim::setPos( float x, float y )
{
	m_posX = x;
	m_posY = -y;
}


