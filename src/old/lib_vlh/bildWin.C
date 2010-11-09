/*
 * Ausgabe des gewaehlten Bildes in einem extra Fenster
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <viaio/Vlib.h>
#include <stdlib.h>
#include <stdio.h>

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

#include "bildWin.h"

BildWin::BildWin( QWidget *parent, const char *name, prefs *pr, double *ca, double *cp, double *extent, double *fixpoint, int i, double ppmax, double nnmax, int ifile, int files )
  : QMainWindow( 0, "bold view", WDestructiveClose )
{
  bild1 = new pictureView( this, "bold", pr, i, ifile, files, ca, cp, extent, fixpoint, 1, ppmax, nnmax );
  setCentralWidget( bild1 );
}
