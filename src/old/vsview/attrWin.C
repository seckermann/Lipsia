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
#include <qtextview.h>
#include <qpainter.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qstatusbar.h> 
#include <qwhatsthis.h>

#include "attrWin.h"

BildWin::BildWin( QWidget *parent, const char *name, QString cp )
  : QMainWindow( parent, name, WType_TopLevel|WDestructiveClose )
{
  QTextView *tv = new QTextView ( this, "text" );
  tv->setText ( cp );
  setCentralWidget( tv );
}
