#ifndef MODAL_H
#define MODAL_H

/* From the C library: */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* From the Vista library: */
#include <viaio/Vlib.h>

/* QT - Librarys */
#include <qapplication.h>
#include <qwidget.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qdialog.h>
#include <qpainter.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qstatusbar.h> 
#include <qwhatsthis.h>
#include <qtoolbar.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qtoolbutton.h> 

#include "rawCW.h"
#include "prefs.h"
#include "dialog.h"

class Modal : public QMainWindow
{
    Q_OBJECT
public:
    Modal( QWidget *parent=0, const char *name=0, prefs *pr_=0, int i=0, float *wert=0, int pi=0, float *powerwert=0, float *fitres=0, float *se=0, double trpre=0, int *length=0, float *model=0, const char *version=0, float *conditions=0, float *Sess=0, int bline=0, float **evres=0, float **evresSE=0, int length_ER=0, int width_ER=0, int cnd=0, QString vlviewname=0 );
    ~Modal();
    RawCW *centralw;
    QStringList fname_m;

public slots:
    void orderOO( int );
    void orderOO();
    void mdg();
    void mtc();
    void tln();
    void tae();
    void psm();
    void rast();
    void hgsw();

private:
    prefs *pr;
    int i_m, *order_m, *maxorder_m, maxfarben_m;
    float *wert_m;
    int *length_m,  cnd_m;
    float *model_m;
    MyDialog   *dialog;
    int basecorr, persigch, boldresp, *ordermenu;
    QPopupMenu *opti, *zeige;
    QToolButton *rb, *rb1, *rb2, *rb3, **rb0, *mod, *tc, *raster, *toolb, *hgswitch, *std;
    char *nameptr;
    float *TYPE;
};

#endif // MODAL_H
