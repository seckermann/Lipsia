/****************************************************************
 *
 * design.h
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
 * $Id: design.h 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


#ifndef OF_H
#define OF_H

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
// #include <viaio/VGraph.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/file.h>
//#include <viaio/os.h>

/* QT - Librarys */
#include <qiconset.h> 
#include <qframe.h>
#include <qimage.h> 
#include <qkeycode.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtoolbar.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qstatusbar.h> 
#include <qpicture.h> 
#include <qfont.h> 
#include <qtoolbar.h> 
#include <qlabel.h>
#include <qpixmap.h>
#include <qcolor.h> 
#include <qlcdnumber.h> 
#include <qtoolbutton.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlcdnumber.h>
#include <qmainwindow.h>
#include "designCW.h"

class Design : public QMainWindow
{
    Q_OBJECT
public:
    Design( QWidget *parent=0, const char *name=0 );
    ~Design();
    QMenuBar *menubar;
    QPopupMenu *condmenu;
    QStatusBar *statusbar;

public slots:
    void setSession(int);
    void setCondition(int);
    void wertView( double );
    void writeStatus(double);

private:
    designCW   *center;
    QToolButton *toolb;
    QLCDNumber *wert;
    QLabel *text;
    QFont appFont;
};

#endif // OF_H
