/***************************************************************************
 *   Copyright (C) 2005 by Thomas Proeger
 *   proeger@cbs.mpg.de                                               *
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

#ifndef VLSEGMENTWINDOW_H_
#define VLSEGMENTWINDOW_H_

#include <qpoint.h>
#include <qobject.h>
#include "vltable.h"
#include "segmentwindow.h"
#include "vlcolorboxtableitem.h"

#define PIXMAP_LENGTH 18

//gueltige Werte fuer das Flag "event"
//Sie sind notwendig um den currentChanged() event
//mitzuteilen was fuer ein Ereignis der Aenderung 
//voraus ging.
//NEW und DELETE: alle internen Aktualsierungen wurden schon 
//vorgenommen -> weiter
//CLICK: die "current row" wurde mittels eines Mausklicks veraendert.
//Diese Aenderung muﬂ intern noch beruecksichtigt werden.
#define EVENT_NONE 0
#define EVENT_NEW 1
#define EVENT_DELETE 2
#define EVENT_CLICK 4

class vlSegmentWindow : public SegmentWindow {

	Q_OBJECT

private:

    vlTable* volumeTable;
    vlTable* colorTable;

    // tiny event handling mechanism
    short event;

	/** creates a new entry in the segment window tables. */
	void newEntry(int id);

protected:
	void closeEvent(QCloseEvent*);
    
    // this method has to be called when the table content, and therefore the 
    // segment order, has been changed.
    void updateGUI();

public:
	vlSegmentWindow(
		QWidget* parent = 0,
		const char* name = 0);
	
	~vlSegmentWindow();
	
public slots:

	// create a new empty segment
	virtual void fileNew();

	// open a segment
	virtual void fileOpen();
	
	// delete a segment
	virtual void editCut();

    // current row in volume table changed
    void volumeTable_currentChanged(int, int);
	
    // current row in color table changed
    void colorTable_currentChanged(int, int);
	
	// the mouse has been clicked in the volume table
	virtual void volumeTable_pressed(int, int, int, const QPoint &);

	// the mouse has been clicked in the color table
	virtual void colorTable_pressed(int, int, int, const QPoint &);

	// the color of the segment was changed
	virtual void colorChanged(int);

	// the volume of the selected segment has changed
	void volumeChanged();

    // the "Save All Segments" button was pressed.
    void fileSaveAllAction_activated();
    
    // The "Save Visible Segments" button was pressed. 
    void fileSaveVisibleAction_activated();
    
    // the visibility of an segment in the given row has been changed
    void visibilityChanged(bool, int); 
    
    // the selected segment has been moved a step up
    void segmentUpAction_activated();
    
    // the selected segment has been moved a step down
    void segmentDownAction_activated();

signals:

	// this signal is send when the segment window is about to be closed.
	void segmentWindowClosed();

};

#endif /*VLSEGMENTWINDOW_H_*/
