/****************************************************************************
** $Id: bilderCW.h 3181 2008-04-01 15:19:44Z karstenm $
**
*****************************************************************************/

#ifndef BILDERCW_H
#define BILDERCW_H

#include <viaio/Vlib.h>

#include <qwidget.h>
#include <qprinter.h> 
#include <qpaintdevicemetrics.h> 
#include <qslider.h>
#include <qlayout.h>
#include <qscrollbar.h> 


#include "pictureView.h"
#include "openGL.h"
#include "flow.h"

class BilderCW : public QWidget
{
    Q_OBJECT

public:
    BilderCW( QWidget *parent, const char *name=0, prefs *pr_=0, VString pat=0, double *ca=0, double *cp=0, double *extent=0, double *fixpoint=0, const char *version=0, int *scalingfaktor=0 );
    pictureView **bild1, **bild2, **bild3;
    MyGLDrawer **ogl;
    QBoxLayout *hlayout;
    QGridLayout *layout;
    QScrollBar *rc1, *rc2, *rc3;
    QLabel *la1, *la2, *la3;
    SimpleFlow *flowLayout;
    int files;

signals:
    void z2Wert( double );
    void zWert( double );
    void crossPosit( /*int*/float, /*int*/float, /*int*/float, QString );  /* changed by A.Hagert - for float-graphs */
    void echtPosit( /*int*/float, /*int*/float, /*int*/float, QString );
    void setVoxelBox();
    void SlideReleasedForVLRender();
    void sendtoserver();

public slots:
    void print();
    void gLayout(QWidget *parent); 
    void hideLayout(QWidget *parent); 
    void nowsliderChange();
    void setInterpolNN();
    void setInterpolBilin();
    void setInterpolBicub();
    void setInterpolBicub6();
    void setInterpolBSpline();
    void setVoxelSystem(int);
    void zoomplus();
    void zoomminus();
    void zoomplusdouble();
    void zoomminusdouble();
    void coordIN();
    void talCross( int, int, int );
    void talEcht( int, int, int );
    void findMaxZ();
    void findMinZ();
    void findMinMaxZ( int );
    void reset();
    void SetXYZScrollbars( int, int, int ); // added by A. Hagert
    void SlideReleased();

protected:
    void wheelEvent( QWheelEvent* e);
    void leaveEvent ( QEvent *e );

private:
    int rows, bands, columns, fnc_rows, fnc_bands, fnc_columns;
    prefs *pr;
    VString pat_m;
    QPrinter *printer;
    const char *version_m;
    double *ca_m, *cp_m, *extent_m, *fixpoint_m;
    int *scalingfaktor_m;
    QSlider *posslide, *negslide;
};

#endif
