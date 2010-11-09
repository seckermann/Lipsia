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
#ifndef POINT_H
#define POINT_H

/**
Diese Klasse repräsentiert einen Punkt in Pixelkoordinaten.
Sie wird von der PointSetklasse benutzt.

@author Hannes Niederhausen
*/
class Point{
protected:
	int m_x;
	int m_y;
	int m_z;

public:
	Point();
    Point(int x, int y, int z);
	Point(const Point &src);
	
    ~Point();
	
	int x() {return m_x;};
	int y() {return m_y;};
	int z() {return m_z;};
	
	void setX(int x){m_x=x;};
	void setY(int y){m_y=y;};
	void setZ(int z){m_z=z;};
	
	void dump();
	
	/**
	 *    Berechnet die 26 Nachbarn und schreibt sie in das übergebene Array.
	 * @param p ein Feld von Punkten welches Platz für 26 Punkte haben muss
	 */
	void getNeighbours(Point* p);
	
	/**
	 *    Vergleicht das Object mit dem übergebenen Punkt p.
	 *    Ein Punkt ist gleich eines anderen wenn alle Koordinaten übereinstimmen, 
	 *    kleiner wenn seine x Koordinate kleiner der des anderen Punktes ist und größer falls
	 *    seine X-Koordinate größer der des anderen ist.<br>
	 *    Ist die X-Koordinate beider Punkte gleich so wird genauso mit der Y-Koordinate verfahren, und bei
	 *    möglicher Gleichheit noch mit der Z-Koordinate.
	 * @param p der Vergleichspunkt
	 * @return -1 falls p größer this
	 * 			0 falls sie gleich sind
	 			1 falls this größer p
	 */
	int compare (Point* p);
	
	
	/**
	 *    Diese Klasse überprüft der Punkt p ein NAchbar des aufrufenden Objektes ist,
	 *    d.h. ob sich mindestens eine Koordinate der beiden Punkte nur um 1 unterscheiden.
	 * @param p der Punkt der möglicherweise ein Nachbar ist
	 * @return <b>true</b> falls p ein Nachbar ist, sonst <b>false</b>
	 */
	bool isNeighbour(Point *p);

	
	/**
	 *    Überprüft ob zwei Punkte gleich sind.
	 * @param p der zu vergleichende Punkt
	 * @return <b>true</b> falls die Punkte gleich sind, sonst <b>false</b>
	 */
	bool equals(Point *p);
	
	bool operator< (Point &p){return this->compare(&p)==1;};
	
	bool operator== (Point &p){return this->equals(&p)==true;};
	
	bool operator!= (Point &p){return this->equals(&p)==false;};
};

#endif
