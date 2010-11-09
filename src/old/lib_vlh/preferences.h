/****************************************************************************
** $Id: preferences.h,v 1.0 1999/11/11 10:08:56
**
** Copyright (C) 1992-1999 Heiko Mentzel.  All rights reserved.
*****************************************************************************/

#ifndef TABDIALOG_H
#define TABDIALOG_H

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <qtabdialog.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qslider.h> 
#include <qlcdnumber.h> 
#include <qradiobutton.h>
#include <qcolor.h>

//#include "bild1.h"
#include "../lib_vlh/pictureView.h"
//#include "prefs.h"

class TabDialog : public QTabDialog
{
    Q_OBJECT

public:
    TabDialog( QWidget *parent, const char *name, prefs *pr, double *ca, double *cp, double *extent );

signals:
    void talOnOff();
    void setVoxelBox();
    void newColtype();
    void newgraphColtype();   /* added by A. Hagert */	
    void nowsliderChangeS();
    void reloadFiles();

public slots:
    void ok();
    void apply();
    void hellreset();
    void colorreset();
    void color1Select();
    void color2Select();
    void equidistantColorTableToggled();
    void coltabSel0();
    void coltabSel1();
    void coltabSel2();
    void coltabSel3();
    void coltabSel4();
    void coltabSel5();
    void coltabSel6();
    void coltabSel7();
    void coltabSel8();
    void coltabSel9();
    void coltabSel10();
    void coltabSel11();
    void coltabSel12(); 	
    void coltabSel13();
    void coltabSel14(); 	
    void coltabSel15();
    void helligkeit( int );
    void contrastl( int );
    void contrastd( int );
    void contrastd1( int );
    void setMinMaxValue();


protected:
    QString filename;
    QFileInfo fileinfo;

    void setupTab1();
    void setupTab2();
    void setupTab3();
    void setupTab4();
    void setupTab5();
    void prepareColorTableButtons();
    void updateColorTableButtons();

private:
    QLineEdit  *p0max, *zero, *n0max, *triall, *trialreso, *tablemin, *tablemax;
    QCheckBox  *showRad, *showCro, *intPol, *traZma, *fullzmapogl;
    QCheckBox  *showglassbrain; 
    QCheckBox *pschange, *lockmaxz;
    QCheckBox *croSize; /* added by A. Hagert */
    QPushButton *Reset1, *Apply1;
    //QCheckBox  *triala, *bcorr;
    QRadioButton *talai, *mm, *voxzmap, *voxanatomisch; 
    struct prefs *pr;
    double nn, pp, *ca_m, *cp_m, *extent_m;
    QLCDNumber *lcd;
    pictureView *bild1;
    QSlider *brig, *hell, *dunk, *dunk1;  
    QLabel *lwhit, *lbrig, *lbrigbr, *lblac, *lblac1;
    QPushButton **ColorTableButtons;
};

#endif
