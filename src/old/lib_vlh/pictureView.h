/****************************************************************
**
** Definition of Bild class
**
** Added interpolation for zooming images
** Gert Wollny <wollny@cns.mpg.de>
****************************************************************/

#ifndef PICTUREVIEW_H
#define PICTUREVIEW_H

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

#include "prefs.h"

#include "interpolator.hh"


extern VImage *src, *fnc;

class pictureView : public QWidget
{
	Q_OBJECT
public:
	pictureView( QWidget *parent = 0, const char *name = 0, prefs *pr = 0, int typ = 0, int ifile = 0, int files = 0, double *ca = 0, double *cp = 0, double *extent = 0, double *fixpoint = 0, int crossoff = 0, double ppp = 0.0, double nnn = 0.0, int *scalingfaktor = 0 );

	QSizePolicy sizePolicy() const;
	QPixmap pm, cpm;
	double nnmax, ppmax;
	QColor *rgbfarbe_fix, *rgbfarbe, *rgbfarbeoverlay;
	void talEcht( int x, int y, int z );
	int recreate;
	const QScaleImage &get_image()const {
		return *image;
	};

	void paint( QPainter * );
public slots:
	void repaintf();
	void posChanged( int );
	void negChanged( int );
	void talCross( int, int, int );
	void colorMap();
	void changeInterpol( interpol_type ipt );

signals:
	void viewChanged( );
	void crossPosit( /*int*/float, /*int*/float, /*int*/float, QString );
	void echtPosit( /*int*/float, /*int*/float, /*int*/float, QString );
	void colbarRepaint();
	void zWert( double );
	void z2Wert( double );
	void sendtoserver();

protected:
	void paintEvent( QPaintEvent * );
	void mousePressEvent( QMouseEvent *e );
	void mouseReleaseEvent( QMouseEvent *e );
	void mouseMoveEvent( QMouseEvent *e );

private:
	CMagBase *get_interpolator( int s1, int s2 ) const;

	prefs *pr;
	int type_m, rows, bands, columns, framefaktor, fnc_rows, fnc_bands, fnc_columns, files_m;
	double sc1, sc2;
	double *ca_m, *cp_m, *extent_m, *fixpoint_m;
	int ifile_m, crossoff_m;
	double ppp_m, nnn_m;
	int Xplus, Yplus, Zplus, newrows, newcols, newbands;
	int Xmax, Ymax, Zmax, *sf, letzte;
	interpol_type m_ipt;
	QScaleImage *imagebackup;
	QScaleImage *image;
	float prz;
};

#endif // PICTUREVIEW_H
