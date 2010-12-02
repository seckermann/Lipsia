/****************************************************************
 *
 * lView.h
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Heiko Mentzel, 1999, <lipsia@cbs.mpg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id: dock.h 756 2004-03-11 17:57:36Z karstenm $
 *
 *****************************************************************/

#ifndef MENU_H
#define MENU_H

/* From the std. library: */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>   /* Interprozesskommunikation */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

/* QT - Librarys */
#include <qiconset.h>
#include <qframe.h>
#include <qimage.h>
#include <qkeycode.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qtoolbar.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qstatusbar.h>
#include <qpicture.h>
#include <qfont.h>
#include <qdir.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qcombobox.h>
#include <qtoolbar.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qlcdnumber.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qmainwindow.h>
#include <qsocketnotifier.h>



class lDock : public QMainWindow
{
	Q_OBJECT
public:
	lDock( QWidget *parent = 0, const char *name = 0 );
	~lDock();

signals:


public slots:
	void syncronize( int );

private:

	QSocketNotifier *notif_m;
	QToolBar   *Stools;
	QLabel     *echt, *pidlabel;
	int        fd_fifo, *vlslot, *fifoo;
};

#endif // MENU_H
