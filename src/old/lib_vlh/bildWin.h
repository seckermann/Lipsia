#ifndef BILDWIN_H
#define BILDWIN_H

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qdialog.h>
#include <qwidget.h>
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qlayout.h>

#include "pictureView.h"

class BildWin : public QMainWindow
{
    Q_OBJECT
public:
    BildWin( QWidget *parent=0, const char *name=0, prefs *pr=0, double *ca=0, double *cp=0, double *extent=0, double *fixpoint=0, int i=1, double ppmax=0, double nnmax=0, int ifile=0, int files=0 );
    pictureView *bild1;
    
};

#endif // BILDWIN_H
