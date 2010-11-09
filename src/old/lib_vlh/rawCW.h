/****************************************************************************
** $Id: rawCW.h 522 2003-06-26 09:31:01Z karstenm $
**
*****************************************************************************/

#ifndef RAWCW_H
#define RAWCW_H

#include <stdio.h>
#include <stdlib.h>

#include <qwidget.h>
#include <qlayout.h>
#include <qprinter.h> 
#include <qpaintdevicemetrics.h> 
#include <qtoolbar.h>
#include <qscrollbar.h>

#include "rawpaint.h"

class RawCW : public QWidget
{
    Q_OBJECT

public:
    RawCW( QWidget *parent, const char *name=0, prefs *pr=0, int i=0, float *wert=0, int pi=0, float *powerwert=0, float *fitres=0, float *se=0, double trpre=0, int *length=0, float *model=0, const char *version=0, QStringList fname=0, float *conditions=0, float *Sess=0, int bline=0, float **evres=0, float **evresSE=0, int length_ER=0, int width_ER=0, int cnd=0 );
    ~RawCW();
    RawPaint *rpaint;
    QScrollBar *sb;
    QBoxLayout *topLayout, *secondLayout;
    //QToolBar   *tools;

public slots:
    void changeScroll( int zoom );
    void print();
    void saveTimelineTC();
    void saveTimelineFR();
    void saveTimeline( float *wert=NULL );
    void savePowerspectrumTC();
    void savePowerspectrumFR();
    void savePowerspectrum( float *powerwert=NULL );

private:
    int i_m;
    const char *version_m;
    float *wertneu;
    float *powerwert_m;
    int pi_m;
    double trpre_m;
    float *model_m;
    float *fitresneu, *Sess_m;
    QPrinter *printer;
};

#endif
