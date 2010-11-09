/****************************************************************
 *
 * lView.h
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Heiko Mentzel, 1999, <lipsia@cbs.mpg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * $Id: lView.h 2946 2007-11-28 09:24:42Z karstenm $
 *
 *****************************************************************/

#ifndef MENU_H
#define MENU_H

/* From the std. library: */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>   /* Interprozesskommunikation */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

/* QT - Librarys */
#include <qiconset.h>
#include <qframe.h>
#include <qimage.h> 
#include <qkeycode.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qtoolbar.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qstatusbar.h> 
#include <qpicture.h> 
#include <qfont.h> 
#include <qdir.h> 
#include <qfile.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qcombobox.h> 
#include <qtoolbar.h> 
#include <qlabel.h>
#include <qpixmap.h>
#include <qcolor.h> 
#include <qlcdnumber.h> 
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qmainwindow.h>
#include <qsocketnotifier.h>

/* my own inludes */
#include "pictureView.h"
#include "rawplot.h"
#include "VLTools.h"
#include "VLCorr.h"
#include "VCheck.h"
#include "dialog.h"
#include "bilderCW.h"
#include "preferences.h"
#include "prefs.h"

/* images */
#include "fileopen.xpm"
//#include "filesave.xpm"
//#include "fileprint.xpm"
#include "rawdat.xpm"
#include "findmaxz.xpm"
#include "findminz.xpm"
#include "lip.xpm"
//#include "colortab.xpm"
#include "coronal.xpm"
#include "axial.xpm"
#include "sagittal.xpm"
#include "opengl.xpm"
#include "blobs.xpm"
#include "hgpixmap.xpm"
#include "preferences.xpm"
#include "settings.xpm"
#include "connect0.xpm"
#include "connect1.xpm"
#include "zoomin.xpm"
#include "zoomout.xpm"
#include "setcoord.xpm"
#include "reload.xpm"
#include "corr.xpm"

class lView : public QMainWindow
{
    Q_OBJECT
public:
    lView( QWidget *parent=0, const char *name=0);
    ~lView();

signals:
    void viewChanged( );
    void crossPosit( /*int*/float, /*int*/float, /*int*/float, QString );   /* added by A. Hagert: int->float for the float-graphs */
    void echtPosit( /*int*/float, /*int*/float, /*int*/float, QString ); 
    void z2Wert( double );

public slots:
    void statXYZ( float /*int*/, float /*int*/, float /*int*/, QString );   /* added by A. Hagert: int->float for the float-graphs */
    void echtXYZ( float /*int*/, float /*int*/, float /*int*/, QString );   /* added by A. Hagert: int->float for the float-graphs */

    void talOnOff();
    void hideitems( int );
    void colbarRepaint();
    void zWert( double );
    void z2aWert( double );
    void z2aWert_sulci( double );  /* added by A. Hagert: shows tha label of the clicked point */ 
    void prefs();
    void toolb1Switch();
    void toolb2Switch();
    void toolb3Switch();
    void toolb4Switch();
    void toolbBlobsSwitch();
    void toolbZmapSwitch();
    void StartResize();
    void hgsw();
    void jumpRight();
    void jumpLeft();
    void jumpTop();
    void jumpButtom();
    void jumpFront();
    void jumpBack();
    void resizePicture();
    void reloadFiles();
    void setInterpolation(int);
    void setVoxelBox();
    void setOpenvis(int);
    void synchronize_vlviews();
    //void synchronize_rotation();            /* added by A. Hagert : synchronizes rotation */
    void print_color_min_max(float, float); /* added by A. Hagert - prints max/min of graph-labels */
    void findmaxZ_graph();  /* added by A. Hagert - find local minimum */
    void findminZ_graph();  /* added by A. Hagert - find local maximum */
    void switchCrosses();   /* added by A. Hagert: graph: single points are crosses/points */ 
    void change_coltab();   /* added by A. Hagert - changing of colortable */
    void switchFog();       /* added by A. Hagert: graph: turns on/off the fog */ 
    void clicking();        /* added by A. Hagert: graph: turns on/off the more exact clicking routine */ 
    void graph_zoom_in();   /* added by A. Hagert: graph: zoom in */ 
    void graph_zoom_out();  /* added by A. Hagert: graph: zoom out */ 
    void graph_zoom_norm(); /* added by A. Hagert: graph: sets the zoom back */ 
    void switchLines();     /* added by A. Hagert: graph: links between the nodes shown or not */
    void switchPolygons();  /* added by A. Hagert: graph: turns on/off polygons (if present) */ 
    void switchColor();     /* added by A. Hagert: graph: colors on/pff */ 
    void syncronize(int );  /* IPC by KM */
    void sendtoserver();    /* IPC by KM */
    void imageexport();		/* added by H. Niederhausen */

private:
    BilderCW   *centralw;
    QMenuBar   *menubar;
    VLCorr     *correl;
    //    QPoint     *points;				// point array
    QColor     *colors;				// color array
    int		count;				// count = number of points
    int         fd_fifo;
    bool	down;				// TRUE if mouse down
    QToolBar   *tools, *Stools, *oGLtools, *Ctools;
    MyDialog   *dialog;
    QLabel     *xyz, *echt;
    QLCDNumber *zzwert, *z2wert;
    QPixmap     coltab;
    TabDialog  *prefer;
    QToolButton *toolb, *toolb1, *toolb2, *toolb3, *toolb4, *toolb5, *toolb6, *toolb7, *toolb8, *hgswitch, *toolbsyn;
    QComboBox  *combobox, *combobox2, *combobox3;
    QPixmap     lipsia;
    QSocketNotifier *notif_m;

    void keyPressEvent( QKeyEvent *k );
    void resizeEvent ( QResizeEvent *rs );

    QFont appFont;

};

#endif // MENU_H
