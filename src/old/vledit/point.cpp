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
#include <iostream>
#include <stdlib.h>

#include "point.h"

Point::Point( )
{
}

Point::Point( int x, int y, int z )
{
	m_x=x;
	m_y=y;
	m_z=z;
}

Point::Point(const Point & src )
{
	m_x=src.m_x;
	m_y=src.m_y;
	m_z=src.m_z;
}


Point::~Point()
{
}

bool Point::isNeighbour( Point * p )
{
	if (abs(m_x-p->x())==1)
		return true;
		
	if (abs(m_y-p->y())==1)
		return true;
		
	if (abs(m_z-p->z())==1)
		return true;

	return false;
}

int Point::compare( Point * p )
{

	if (m_x<p->x())
		return -1;
	else
		if (m_x>x())
			return 1;
	
	if (m_y<p->y())
		return -1;
	else
		if (m_y>y())
			return 1;
			
	if (m_z<p->z())
		return -1;
	else
		if (m_z>z())
			return 1;
			
	return 0;
}

bool Point::equals( Point * p )
{
	return ( (m_x==p->x())&&(m_y==p->y())&&(m_z==p->z()) );
}

void Point::dump( )
{
	std::cout<<"X: "<<m_x<<" Y: "<<m_y<<" Z: "<<m_z<<std::endl;
}

void Point::getNeighbours( Point * p )
{
	p[0].m_x=m_x-1;
	p[0].m_y=m_y;
	p[0].m_z=m_z;
	
	p[1].m_x=m_x-1;
	p[1].m_y=m_y+1;
	p[1].m_z=m_z;
	
	p[2].m_x=m_x-1;
	p[2].m_y=m_y+1;
	p[2].m_z=m_z-1;

	p[3].m_x=m_x-1;
	p[3].m_y=m_y+1;
	p[3].m_z=m_z+1;
	
	p[4].m_x=m_x-1;
	p[4].m_y=m_y;
	p[4].m_z=m_z-1;
	
	p[5].m_x=m_x-1;
	p[5].m_y=m_y;
	p[5].m_z=m_z+1;
	
	p[6].m_x=m_x-1;
	p[6].m_y=m_y-1;
	p[6].m_z=m_z-1;
	
	p[7].m_x=m_x-1;
	p[7].m_y=m_y-1;
	p[7].m_z=m_z+1;
	
	p[8].m_x=m_x-1;
	p[8].m_y=m_y-1;
	p[8].m_z=m_z;
	
	
	p[9].m_x=m_x+1;
	p[9].m_y=m_y;
	p[9].m_z=m_z;
	
	p[10].m_x=m_x+1;
	p[10].m_y=m_y+1;
	p[10].m_z=m_z;
	
	p[11].m_x=m_x+1;
	p[11].m_y=m_y+1;
	p[11].m_z=m_z-1;

	p[12].m_x=m_x+1;
	p[12].m_y=m_y+1;
	p[12].m_z=m_z+1;
	
	p[13].m_x=m_x+1;
	p[13].m_y=m_y;
	p[13].m_z=m_z-1;
	
	p[14].m_x=m_x+1;
	p[14].m_y=m_y;
	p[14].m_z=m_z+1;
	
	p[15].m_x=m_x+1;
	p[15].m_y=m_y-1;
	p[15].m_z=m_z-1;
	
	p[16].m_x=m_x+1;
	p[16].m_y=m_y-1;
	p[16].m_z=m_z+1;
	
	p[17].m_x=m_x+1;
	p[17].m_y=m_y-1;
	p[17].m_z=m_z;
	
	p[18].m_x=m_x;
	p[18].m_y=m_y+1;
	p[18].m_z=m_z-1;
	
	p[19].m_x=m_x;
	p[19].m_y=m_y+1;
	p[19].m_z=m_z;
	
	p[20].m_x=m_x;
	p[20].m_y=m_y+1;
	p[20].m_z=m_z+1;
	
	p[21].m_x=m_x;
	p[21].m_y=m_y;
	p[21].m_z=m_z-1;
	
	p[22].m_x=m_x;
	p[22].m_y=m_y;
	p[22].m_z=m_z+1;
	
	p[23].m_x=m_x;
	p[23].m_y=m_y-1;
	p[23].m_z=m_z-1;
	
	p[24].m_x=m_x;
	p[24].m_y=m_y-1;
	p[24].m_z=m_z+1;
	
	p[25].m_x=m_x;
	p[25].m_y=m_y-1;
	p[25].m_z=m_z;
}

