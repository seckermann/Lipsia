/****************************************************************
 *
 * designCW.h
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
 * $Id: designCW.h 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#ifndef SETUPCW_H
#define SETUPCW_H

#include <viaio/Vlib.h>

/* Qt - Libraries */
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qwidget.h>
#include <qevent.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qstringlist.h>
/* #include <qlist.h> */
#include <qwidget.h>
#include <qprinter.h> 
#include <qpaintdevicemetrics.h> 
#include <qslider.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtabwidget.h> 
#include <qcheckbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qtabdialog.h>
#include <qscrollbar.h>
#include "bild1.h"
#include "bild2.h"
#include "bild3.h"

class designCW : public QTabWidget
{
    Q_OBJECT

public:
  designCW( QWidget *parent, const char *name = 0, double version=0, double TR=0, int sessanz=0, int *sesslength=0, int *sesscov=0, VImage src=0, VImage src1=0, VImage orth=0, VImage gamma=0);
    ~designCW();
    Bild *bild1;
    Bild2 *bild2;
    Bild3 *bild3, *bild4, *bild5, *bild6;

public slots:
    void changeScroll1( int zoom );
    void changeScroll2( int zoom );

protected:
    void setupTab1();
    void setupTab3();
    void setupTab4();
    void setupTab5();
    void setupTab6();
    void setupTab2();

private:
    QLineEdit *d0, *d1, *d01;
    double version_m;
    double TR_m;
    int sessanz_m;
    int *sesslength_m, *sesscov_m;
    VImage src_m, src1_m, orth_m, gamma_m;
    QCheckBox  *useDB;
    QScrollBar *sb1, *sb2;
};

#endif
