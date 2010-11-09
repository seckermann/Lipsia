/****************************************************************
 *
 * bild3.h
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
 * $Id: bild3.h 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#ifndef BILD3_H
#define BILD3_H

/* Std. Include Files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Vista Include Files */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* Qt Include Files */
#include <qpainter.h>
#include <qwidget.h>
#include <qcolor.h> 
#include <qpicture.h> 
#include <qfont.h> 
#include <qimage.h> 
#include <qlabel.h>


class Bild3 : public QWidget
{
    Q_OBJECT
public:
  Bild3( QWidget *parent=0, const char *name=0, double repetition_time=0, int mode=0, int sessanz=0, int *sesslength=0, int *sesscov=0, VImage src=0, VImage src1=0, VImage orth=0 );

    QSizePolicy sizePolicy() const;

public slots:
    void repaintf();
    void getSessionData( int session=0);
    void position1( int pos=0 );
    void position2( int pos=0 );

signals:
    void wertView( double );
    void neuZoom1( int zoom );
    void neuZoom2( int zoom );
    void sendMaxCut( double );

protected:
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent *e );
    void mousePressEvent( QMouseEvent *e );

private:
    double TR;
    int mode_m;
    int zoom1, posit1, zoom2, posit2;
    int ses_1, ses_2, begin_1, begin_2;
    int sessanz_m;
    int *sesslength_m, *sesscov_m;
    VImage src_m, src1_m, orth_m;
};

#endif // BILD3_H
