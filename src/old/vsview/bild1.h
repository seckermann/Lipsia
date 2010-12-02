/****************************************************************
 *
 * vlview:
 * Definition of Bild class
 *
 * Copyright (C) 1998 MPI of Cognitive Neuroscience, Leipzig
 *
 * Author Heiko Mentzel 1998, <mentzel@cns.mpg.de>
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
 *****************************************************************
 *
 * History:
 * ========
 *  Heiko Mentzel -- 07/27/2001 -- shows now also zmaps of only 1 bands
 *
 *****************************************************************/

#ifndef BILD1_H
#define BILD1_H

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
#include <qcursor.h>

extern int YY, XX, ZZ;

class Bild : public QWidget
{
	Q_OBJECT
public:
	Bild( QWidget *parent = 0, const char *name = 0 );

	QPixmap pm, cpm;
	QColor *rgbfarbe;
	QImage image;
	int recreate;

public slots:
	void repaintf();
	void colorMap();

signals:
	void Wert( int, int, double );

protected:
	void paintEvent( QPaintEvent * );
	void mouseMoveEvent( QMouseEvent *e );
	void leaveEvent ( QEvent *e );

private:
	int type_m, ifile_m, crossoff_m, files_m;
	double sc1, sc2;
	QRgb grau;
	double *ca_m, *cp_m, *extent_m, *fixpoint_m;
	QImage colorimage;
};

#endif // BILD1_H
