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

#include "bild1.h"
#include "imagedata.h"

extern ImageData* selImage;


class TabDialog : public QTabDialog
{
    Q_OBJECT

public:
    TabDialog( QWidget *parent, const char *name );

signals:
    void newColtype();

public slots:
    void ok();
    void apply();
    void hellreset();
    void helligkeit( int );
    void contrastl( int );
    void sliderwechsel();
    void setMinMaxValue();

protected:
    QString filename;
    QFileInfo fileinfo;
    void setupTab5();

private:
    Bild *bild1;
    QCheckBox *ccont;
    QSlider *brig, *hell; 
    QPushButton *Reset;
    QLineEdit *tablemin, *tablemax;
    QLabel *lwhit, *lblac, *lbrig, *lbrigbr;
};

#endif
