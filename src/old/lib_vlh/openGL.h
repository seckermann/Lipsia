/****************************************************************************
** $Id: openGL.h 3181 2008-04-01 15:19:44Z karstenm $
*****************************************************************************/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/VGraph.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <qcolor.h>

#ifndef OPENGL_H
#define OPENGL_H

#include <qgl.h>

#include "prefs.h"

class MyGLDrawer : public QGLWidget
{
	Q_OBJECT        // must include this if you use Qt signals/slots

public:
	MyGLDrawer( QWidget *parent = 0, const char *name = 0, VImage *src_ = 0, VImage *fnc_ = 0, prefs *pr_ = 0, double ppp = 0.0, double nnn = 0.0, double *ca_ = 0, double *cp_ = 0, double *extent_ = 0, int ifile = 0 );
	~MyGLDrawer();
	QColor *posfarbe, *negfarbe;
	double nnmax, ppmax;
	GLfloat xRot, yRot, zRot;
	short color_onoff, lines_onoff, polygons_onoff, fog_onoff, click_exact, crosses_onoff; /* added by A. Hagert */
	short graphtype, lastcolor[3]; /* added by A. Hagert */
	short move_total_x, move_total_y; /* by A. Hagert -> moving graph with mouse */
	VString col_tab_file;  /* added by A. Hagert */
	GLfloat/*uint*/ *slist1, *slist2, *slist3, *slist4, maxslist;  /* changed by A. Hagert: for float graphs */
	int showcross;

signals:
	void zeichneOGL();
	void kreuzBewegt();
	void z2Wert( double );
	void crossPosit( float/*int*/, float/*int*/, float/*int*/, QString );  /* changed by A. Hagert: for float graphs */
	void crossLabel( double/*int*/ );     /* changed by A. Hagert: for float graphs (LCD-display takes double but not float) */
	void mouseRotated ( int, int, int );  /* added by A. Hagert */
	void got_color_min_max( float, float ); /* added by A. Hagert */
	void sendtoserver();

public slots:
	void setXRotation( int degrees );
	void setYRotation( int degrees );
	void setZRotation( int degrees );
	void crossChange( );
	void posChanged( int );
	void negChanged( int );
	void posChanged( );
	void negChanged( );
	void optionsOnOff( QWidget *parent );
	void bewegeKreuz( int x, int y );
	void initializeGL( );
	void talCross( float/*int*/, float/*int*/, float/*int*/ );  /* changed by A. Hagert: for float graphs */
	void clean();                                               /* added by A. Hagert */
	void zoom( float z );                                       /* added by A. Hagert */
	void move( float x, float y, float z );                 /* added by A. Hagert */
	void findMaxZ();                                            /* added by A. Hagert */
	void findMinZ();                                            /* added by A. Hagert */
	void findMinMaxZ( int );                                    /* added by A. Hagert */
	void Disable_fog();                                         /* added by A. Hagert */
	void Enable_fog();                                      /* added by A. Hagert */
	void coordIN();                                         /* added by A. Hagert */
	void move_cross();                                  /* added by A. Hagert */


protected:
	void resizeGL( int w, int h );
	void paintGL();
	void mouseMoveEvent( QMouseEvent *e );
	void mousePressEvent( QMouseEvent *e );
	void mouseReleaseEvent( QMouseEvent *e );

	virtual GLuint    makeObject( int nr );
	virtual GLuint    makeObject2( int s );
	virtual GLuint    makeObject3();

private:

	GLuint object, object2_0, object2_1, object3, object_2nd_graph;
	GLfloat scale, xa, ya, za;
	int rows, bands, columns, fnc_rows, fnc_bands, fnc_columns;
	VImage *src, *fnc;
	prefs *pr;
	double *ca, *cp, *extent;
	int ifile_m;
};

#endif
