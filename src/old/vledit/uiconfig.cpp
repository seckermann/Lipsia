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
#include "uiconfig.h"
#include "datamanager.h"

UIConfig *UIConfig::m_instance = NULL;

UIConfig::UIConfig()
{
	m_Scale = 1;
	m_band = 0;
	m_column = 0;
	m_row = 0;
	debug = 0;
	setFilter( NN_FILTER );
	setCoordMode( ANA_MODE );
	m_delta_treshold = 0;
	m_translate = true;
	m_sync = true;
	m_aim = true;

}

UIConfig::~UIConfig()
{

}

void UIConfig::addScale( float scale )
{
	m_Scale += scale;

	if ( m_Scale < LOWER_SCALE_LIMIT )
		m_Scale = LOWER_SCALE_LIMIT;

	if ( m_Scale > UPPER_SCALE_LIMIT )
		m_Scale = UPPER_SCALE_LIMIT;

	emit scaleChanged();
}

void UIConfig::setScale( float scale )
{
	m_Scale = scale;

	if ( m_Scale < LOWER_SCALE_LIMIT )
		m_Scale = LOWER_SCALE_LIMIT;

	if ( m_Scale > UPPER_SCALE_LIMIT )
		m_Scale = UPPER_SCALE_LIMIT;

	emit scaleChanged();
}

void UIConfig::setAim( float band, float row, float column )
{
	if ( DATAMANAGER->isValid() ) {
		if ( band != -1 ) {
			m_band = ( band > DATAMANAGER->image()->bands() ) ?
					 ( int )DATAMANAGER->image()->bands() / 2 : band;
		}

		if ( row != -1 ) {
			m_row = ( row > DATAMANAGER->image()->rows() ) ?
					( int )DATAMANAGER->image()->rows() / 2 : row;
		}

		if ( column != -1 ) {
			m_column = ( column > DATAMANAGER->image()->columns() ) ?
					   ( int )DATAMANAGER->image()->columns() / 2 : column;
		}

		emit aimChanged();
	}

}

void UIConfig::setRow( float row )
{
	if ( row < 0 )
		m_row = 0;
	else
		m_row = ( row > DATAMANAGER->image()->rows() ) ?
				DATAMANAGER->image()->rows() : row;

	emit aimChanged();
}

void UIConfig::setBand( float band )
{
	if ( band < 0 )
		m_band = 0;
	else
		m_band = ( band > DATAMANAGER->image()->bands() ) ?
				 DATAMANAGER->image()->bands() : band;

	emit aimChanged();
}

void UIConfig::setColumn( float column )
{
	if ( column < 0 )
		m_column = 0;
	else
		m_column = ( column > DATAMANAGER->image()->columns() ) ?
				   DATAMANAGER->image()->columns() : column;

	emit aimChanged();
}

void UIConfig::mouseReleased()
{
	emit sendCoordinates( 0 );
}

void UIConfig::setFilter( const QString filter )
{

	// setze den Filter auf den neuen Wert
	m_filter = filter;
	//benachrichtige den Rest der vlEdit-Welt.
	emit filterChanged( filter );

}

void UIConfig::recreateViews()
{

	emit recreateViews( false );

}

void UIConfig::setMousePos( int band, int row, int col )
{

	m_mousePos[0] = band;
	m_mousePos[1] = row;
	m_mousePos[2] = col;
}

int UIConfig::idByRow( int row )
{
	if( row < 0 )
		return row;

	int segList_size = m_seg_list.size();
	int segList_index = segList_size - row - 1;
	return m_seg_list[segList_index];
}

void UIConfig::checkList()
{

	char *stars = "*******************************";
	cout << stars << endl << "Checking seg_list:" << endl;
	cout << "size: " << m_seg_list.size() << endl;

	vector<int>::iterator iter;

	for( iter = m_seg_list.begin(); iter != m_seg_list.end(); iter++ ) {
		if( *iter != NULL )
			cout << *iter << endl;
		else
			cout << "NULL" << endl;
	}

	cout << stars << endl;

}
