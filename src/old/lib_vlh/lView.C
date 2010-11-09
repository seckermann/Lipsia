/****************************************************************
 *
 * lView.C
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
 * $Id: lView.C 3369 2008-07-03 12:52:50Z karstenm $
 *
 *****************************************************************/

#include "lView.h"
#include "lLoad.h"
#include "VLTools.h"
#include "showVImage.h"
#include <iostream>

extern prefs *pr; 
extern VArgVector graph_files;

/* Parameter */
double     *farbtab=NULL;
QLabel     *cota;
VStringConst zmap_filename=NULL, in_filename=NULL;
QColor crosscolor, radiuscolor;
VImage *src=NULL, *fnc=NULL, *tmp=NULL;
double *ca, *cp, *extent, *fixpoint;
double *scalec, *scaler, *scaleb;
int counter=0, serverini=0;
char puffer[2000], buf[2000];
char fifoname[50];
int fd_fifo0;

VLTools mytools;
VCheck mycheck;
lLoad myload;


/* the first widget */
lView::lView( QWidget *parent, const char *name ) {

  /* Allocate memory for farbtab */
  farbtab = (double *) malloc(sizeof(double)*4);

  /* Allocate memory for images */
  char *programname=strdup(pr->prg_name);
  programname=strtok(programname," ");
  src=(VImage *) VMalloc(sizeof(VImage) * pr->files);
  tmp=(VImage *) VMalloc(sizeof(VImage) * pr->files);
  if (strcmp(programname,"vlcorr")) {
    fnc=(VImage *) VMalloc(sizeof(VImage) * pr->files);
  } else {
    fnc=(VImage *) VMalloc(sizeof(VImage) * 4);
    for (int i=0;i<4;i++) fnc[i]=NULL;
  }


  /* Allocate memory for CA and scaling */
  ca       = (double *) VMalloc(sizeof(double) * pr->files * 3);
  cp       = (double *) VMalloc(sizeof(double) * pr->files * 3);
  extent   = (double *) VMalloc(sizeof(double) * pr->files * 3);
  fixpoint = (double *) VMalloc(sizeof(double) * pr->files * 6);
  scaleb   = (double *) VMalloc(sizeof(double) * pr->files);
  scaler   = (double *) VMalloc(sizeof(double) * pr->files);
  scalec   = (double *) VMalloc(sizeof(double) * pr->files);

  for (int i=0;i<pr->files;i++) {
    src[i]=NULL; fnc[i]=NULL;
    scaleb[i]=1.0; scaler[i]=1.0; scalec[i]=1.0;
  }

  /* load files */
  myload.loadFiles();
  if (!strcmp(programname,"vlcorr")) myload.loadFilesForCorr();
  if (pr->verbose) {
    fprintf(stderr,"Program: %s, Inputfiles: %d\n",programname,pr->files);
    for (int i=0;i<pr->files;i++) {
      fprintf(stderr,"%d.Datei:  ",i+1);
      fprintf(stderr,"ca: %f %f %f  ",ca[0*pr->files],ca[1*pr->files],ca[2*pr->files]);
      fprintf(stderr,"cp: %f %f %f  ",cp[0*pr->files],cp[1*pr->files],cp[2*pr->files]);
      fprintf(stderr,"ex: %f %f %f  ",extent[0*pr->files],extent[1*pr->files],extent[2*pr->files]);
      fprintf(stderr,"fixpoint: %f %f %f\n",fixpoint[0*pr->files],fixpoint[1*pr->files],fixpoint[2*pr->files]);
    }
  }

  appFont = QApplication::font(); 
  /* predefine any colors */
  crosscolor = QColor(yellow);
  radiuscolor = QColor(green);

  /* if pr->raw entered */
  if (pr->verbose) fprintf(stderr,"Checking raw data file ... ");
  int raw_defekt=0;
  if (pr->raw) {
    raw_defekt=mycheck.VCheckRawDataFile(pr->raw,pr->tc_minlength);   
    if (raw_defekt>0) pr->raw = NULL;
  }
  if (pr->verbose) fprintf(stderr,"done\n");
  if (pr->raw==NULL) {
    pr->des=NULL;
    pr->beta=NULL;
  }
 
  int design_defekt=0;
  if (pr->des) {
    pr->condim=mycheck.VReadDesignFile(pr->des,pr->verbose);
    if (pr->condim==NULL) {
      pr->des = NULL;
      design_defekt=1;
    }
  } 

  int beta_defekt=0;
  if (pr->beta) {
    pr->designim=mycheck.VCheckBetaFile(pr->beta);
    if (pr->designim==NULL) {
      pr->beta = NULL;
      beta_defekt=1;
    }
  } 
  
  /* Set the window caption/title */
  VString pat=NULL;
  QString vlviewname="";
  if (VGetAttr(VImageAttrList(src[0]),"patient",NULL,
	       VStringRepn,&pat)  == VAttrFound) {
    if (zmap_filename) 
      vlviewname = tr("%1 [%2] [%3]").arg((programname)).arg(QString(pat).left(4)).arg(QString(zmap_filename).left(QString(zmap_filename).length()-2)); 
    else
      vlviewname = tr("%1 [%2] [%3]").arg((programname)).arg(QString(pat).left(4)).arg(QString(in_filename).left(QString(in_filename).length()-2));
  } else 
    vlviewname = tr("%1").arg((programname));
  
  //if (pr->g_number == 1 || graph_files.number > 1) 
  setCaption( tr("%1").arg(vlviewname) );


  /* sets the icon pixmap, i dont know how it works */
  lipsia = QPixmap( lip_xpm );
  QIconSet ( lipsia, QIconSet::Automatic );
 
  /* initialize another class for loading preferences options */
  dialog = new MyDialog(this, "Dialog", pr, programname );
  dialog->saveOptions(0);
  dialog->hide();

  /* initialize the preferences window */
  if (pr->verbose>1) fprintf(stderr,"initialize the preferences window\n");
  prefer = new TabDialog( this, "prefs", pr, ca, cp, extent );

  /* file count of the input files */
  pr->files=0;
  if (pr->infilenum > pr->zmapfilenum)
    pr->files=pr->infilenum;
  else 
    pr->files=pr->zmapfilenum;

  if (pr->verbose==1) fprintf(stderr,"Files %d ...\n", pr->files);
  

  int *scalingfaktor;
  scalingfaktor = (int *) malloc(sizeof(int)*3);
  
  for (int i=0;i<pr->files;i++) {
    scalingfaktor[0]=(int)(scaleb[i]/pr->nbands_mult);
    scalingfaktor[1]=(int)(scaler[i]/pr->nrows_mult);
    scalingfaktor[2]=(int)(scalec[i]/pr->ncols_mult);
  }

  /* INTERPOLATE ZMAP OR INTERPOLATE ANATOMICAL*/
  for (int ifile=0;ifile<pr->files;ifile++) {
    mytools.interpolate( src[ifile], fnc[ifile], pr->ncols_mult, pr->nrows_mult, pr->nbands_mult, scaleb[ifile], scaler[ifile], scalec[ifile], 0, pr->interpol );
  }

  if (pr->verbose>=1) qWarning("initialize Correlation" );
  VLCorr *correl = new VLCorr();

  if (pr->verbose>=1) qWarning("initialize RawPlot" );
  RawPlot *rawplot = new RawPlot( this, "Rawdata Plot", pr, programname, pr->files, vlviewname );
  rawplot->hide();
  if (pr->verbose>=1) qWarning("initialize BilderCW" );
  centralw = new BilderCW( this, "Central", pr, pat, ca, cp, extent, fixpoint, programname, scalingfaktor );
  setCentralWidget( centralw );

  /* Talairach message */
  if (pr->sw2 && pr->extent_match==0) 
    QMessageBox::warning( this, "I'm sorry...","Header attribite 'extent' does not coincide\nin anatomical and functional data.\nWorking without Talairach coordinates." );
  
  /* =================== CREATING TOOLBARS ====================== */
   if (pr->verbose>1) fprintf(stderr,"Creating toolbars...\n");
  // create a toolbar
  /* menu and statusbar I put also in a toolbar, then you can move the bar */
  /* Mtools = new QToolBar( this, "menu" ); */
  tools = new QToolBar( this, "toolbar" );
  Stools = new QToolBar( "statusbar", this, QMainWindow::Bottom );
  //Ctools = new QToolBar( "clockbar", this, QMainWindow::Top );
  Stools->setHorizontalStretchable ( 1 );

  pr->ogl=0;
  if (pr->graph[0]) {
    if ( QGLFormat::hasOpenGL() ) {
      pr->ogl=1;
      for (int i=0;i<pr->files;i++) /* added by A. Hagert: give the name of the colortable to OGL (->opengl.c) */
	{ 
	  //centralw->ogl[i]->graphtype = 0;
	  centralw->ogl[i]->graphtype = pr->graphtype;
	  if (pr->colortable) centralw->ogl[i]->col_tab_file = pr->colortable;
	}
    } else {
      pr->graph[0]=NULL;
      pr->ogl=0;
      QMessageBox::about( this, "OpenGL",
			  "This system has no OpenGL support.\n");    
    }
  }
  
  if (pr->ogl) {
    oGLtools = new QToolBar( "OpenGL-toolbar", this, QMainWindow::Top );
    //oGLtools->setOrientation();
  }

  /* =================== first TOOLBAR ====================== */

  /* button to load a file 
     QPixmap fileIcon(fileopen);
     toolb = new QToolButton( fileIcon, "load vistafile",
     QString::null, dialog, SLOT(open()),
     tools, "open file" );
     QWhatsThis::add( toolb, "Here you can <b>load</b> a Vista file."
     "It should have a colortable." );
  */

  /* print button 
     QPixmap  printIcon( fileprint );
     toolb = new QToolButton( printIcon, "print images", QString::null,
     centralw, SLOT(print()),
     tools, "print images" );
     QWhatsThis::add( toolb, "The <b>print</b> button. "
     "Click to print the images!" );
     tools->addSeparator ();
  */

  /* only a line in the menu */
  QPixmap  prefIcon( settings );
  toolb = new QToolButton( prefIcon, "Preferences", QString::null, this, SLOT(prefs()), tools, "Preferences" );
  QWhatsThis::add( toolb, "The <b>preferences</b> button. " 
		   "Click to change the preferences." );
  tools->addSeparator ();

  /* Zoom */
  QPixmap  zoominIcon( zoomin );
  QPixmap  zoomoutIcon( zoomout );
  if (pr->only_sulci == 0) {   /* added by A. Hagert: no need for that without anatomical picture */
    toolb = new QToolButton( zoominIcon, "zoom in", QString::null, centralw, SLOT(zoomplusdouble()), tools, "zoom in" );
    QWhatsThis::add( toolb, "The <b>zoom in</b> button." );
    tools->addSeparator ();
    toolb = new QToolButton( zoomoutIcon, "zoom out", QString::null, centralw, SLOT(zoomminusdouble()), tools, "zoom out" );
    QWhatsThis::add( toolb, "The <b>zoom out</b> button." );
    tools->addSeparator ();
  }
 
  /* button to find the maximum zscore */
  QPixmap  findIcon( findminz );
  QPixmap  findmaxIcon( findmaxz );
  if (pr->only_sulci == 0) {
    toolb = new QToolButton( findIcon, "find min", QString::null,
			     centralw, SLOT(findMinZ()),tools, "find min" );
    QWhatsThis::add( toolb, "Search for local minimum" );
    tools->addSeparator ();
    toolb = new QToolButton( findmaxIcon, "find max", QString::null,
			     centralw, SLOT(findMaxZ()),tools, "find max" );
    QWhatsThis::add( toolb, "Search for local maximum" );
    tools->addSeparator ();
  }
  
  /* button to open a rawdata window */
  QPixmap  rawIcon( rawdat );
  if (pr->raw && pr->only_sulci == 0) {
    toolb = new QToolButton( rawIcon, "show rawdata", QString::null,
			     rawplot, SLOT(ausgabe()),
			     tools, "show rawdata" );
    toolb->setOn(FALSE);
    
    QWhatsThis::add( toolb, "This is the <b>Rawdata Button</b>. "
		     "Click to open a new window with rawdata." );
    
    tools->addSeparator ();
  }

  /* Set coordindates */
  QPixmap  setcoordIcon( setcoord );

  if (pr->only_sulci == 0)    
    toolb = new QToolButton( setcoordIcon, "Set coordinates", QString::null, centralw, SLOT(coordIN()), tools, "Set coordinates" );
  else
    toolb = new QToolButton( setcoordIcon, "Set coordinates", QString::null, centralw->ogl[0], SLOT(coordIN()), tools, "Set coordinates" );
  QWhatsThis::add( toolb, "The button to set coordinates. " 
		   "Click to specify coordinates." );
  tools->addSeparator (); 
   
  /* reset */
  QPixmap  blobsIcon( blobs );
  QPixmap  corrIcon( corr );
  QPixmap  reloadIcon( reload );
  if (pr->only_sulci == 0) {  
    toolb = new QToolButton( reloadIcon, "Reset", QString::null,
			     centralw, SLOT(reset()),tools, "Reset" );
    QWhatsThis::add( toolb, "Reset view" );
    tools->addSeparator ();
  }
  
  /* buttons to enable / disable views */
  toolb1 = new QToolButton ( QPixmap(coronal), "coronar", QString::null, this, SLOT(toolb1Switch()), tools, "coronar" );
  if (pr->only_sulci == 0) {  
    toolb1->setToggleButton ( TRUE );
    toolb1->setTextLabel ( "coronal", TRUE );
  } else {
    toolb1->setToggleButton ( FALSE );
    toolb1->setTextLabel ( "coronal", FALSE );
    toolb1Switch();
    toolb1->hide();
  }
  QWhatsThis::add( toolb1, "Here You can switch on/off the <b>coronar</b> view." );
  if (pr->picture[0]) toolb1->toggle();
  toolb2 = new QToolButton ( QPixmap(sagittal), "sagittal", QString::null, this, SLOT(toolb2Switch()), tools, "sagittal" ); 
  if (pr->only_sulci == 0) {  
    toolb2->setToggleButton ( TRUE );
    toolb2->setTextLabel ( "sagittal", TRUE );
  } else {
    toolb2->setToggleButton ( FALSE );
    toolb2->setTextLabel ( "sagittal", FALSE );
    toolb2Switch();
    toolb2->hide();
  }
  QWhatsThis::add( toolb2, "Here You can switch on/off the <b>sagittal</b> view." );
  if (pr->picture[1]) toolb2->toggle();
  toolb3 = new QToolButton ( QPixmap(axial), "axial", QString::null, this, SLOT(toolb3Switch()), tools, "axial" );
  if (pr->only_sulci == 0) {  
    toolb3->setToggleButton ( TRUE );
    toolb3->setTextLabel ( "axial", TRUE );
  } else {
    toolb3->setToggleButton ( FALSE );
    toolb3->setTextLabel ( "axial", FALSE );
    toolb3Switch();
    toolb3->hide();
  }
  QWhatsThis::add( toolb3, "Here You can switch on/off the <b>axial</b> view." );
  if (pr->picture[2]) toolb3->toggle();
  if (pr->only_sulci == 0) tools->addSeparator();
  
  /* button to switch on/off zmap */ 
  if (pr->sw2) {
    toolb7 = new QToolButton( blobsIcon, "zmap", QString::null, this, SLOT(toolbZmapSwitch()),
			      tools, "zmap" );
    toolb7->setToggleButton ( TRUE );
    toolb7->setTextLabel ( "zmap", TRUE );
    toolb7->toggle();
    QWhatsThis::add( toolb7, "Here You can switch on/off the <b>zmap</b> in anatomical view." );
    tools->addSeparator();
  }
  
  /* correlation */
  if (pr->raw) {
    if (!strcmp(programname,"vlcorr")) {
      toolb8 = new QToolButton( corrIcon, "Compute Correlation Map", QString::null, 
				correl, SLOT(correlation()), tools, "Compute Correlation Map" );
      QWhatsThis::add( toolb, "This is the <b>Correlation Button</b>. "
		       "Click to compute the correlation map." );
      tools->addSeparator ();
    }
  }

  /* Button for syncronization */
  QPixmap conn0Icon(connect0);
  QPixmap conn1Icon(connect1);
  QPixmap  emptyIcon(20, 20, -1, QPixmap::DefaultOptim);
  emptyIcon.fill( QColor (210,210,210) );
  if (pr->ipc>0) {
    toolbsyn = new QToolButton ( conn1Icon, "sync", QString::null, this, SLOT(synchronize_vlviews()), tools, "sync" );
    toolbsyn->setToggleButton ( TRUE );
    QWhatsThis::add( toolbsyn, "Syncronization via IPC on/off" );
    if (pr->ipc==2) toolbsyn->toggle();
    tools->addSeparator();
  }

  /* button to switch background color */
  QPixmap hgIcon(hgpixmap);
  hgswitch = new QToolButton( hgIcon, "black/white", QString::null, this, SLOT(hgsw()), tools, "black/white" );
  QWhatsThis::add( hgswitch, "This button switches the background color "
		   "between black and white.");
  tools->addSeparator ();

  /*interpolation buttons */
  if (pr->only_sulci == 0) {
    combobox = new QComboBox( FALSE, tools, "Interpolation" );
    if (pr->sw2) {
      static const char* itemcombo[] = { "NN", "Bilin", 0 };
      combobox->insertStrList( itemcombo );
      if (pr->interpoltype==0) combobox->setCurrentItem(0);
      if (pr->interpoltype==1) combobox->setCurrentItem(1);
      if (pr->interpoltype>1) {
	pr->interpoltype=0;
	combobox->setCurrentItem(0);
      }
    } else {
      static const char* itemcombo[] = { "NN", "Bilin", "Bicub", "Bicub6", "B-Spline", 0 };
      combobox->insertStrList( itemcombo );
      if (pr->interpoltype==0) combobox->setCurrentItem(0);
      if (pr->interpoltype==1) combobox->setCurrentItem(1);
      if (pr->interpoltype==2) combobox->setCurrentItem(2);
      if (pr->interpoltype==3) combobox->setCurrentItem(3);
      if (pr->interpoltype==4) combobox->setCurrentItem(4);
    }
    connect(combobox, SIGNAL(activated(int)), this, SLOT(setInterpolation(int)) );
    combobox->setEditable(FALSE);
    combobox->setFocusPolicy(NoFocus);
    tools->addSeparator ();
  }
  
  /* voxel system */
  if (pr->pixelco==2 && pr->sw2<0.5) pr->pixelco=1;
  if (pr->talairachoff==1) pr->talairach=0;
  combobox2 = new QComboBox( FALSE, tools, "Voxelsystem" );
  static const char* itemcombo2[] = { "Ana", "mm", "Zmap", "Tal",  0 };
  combobox2->insertStrList( itemcombo2 );
  if ( pr->pixelco==1 && pr->talairach==0 ) combobox2->setCurrentItem(0);
  if ( pr->pixelco==0 && pr->talairach==0 ) combobox2->setCurrentItem(1);
  if ( pr->pixelco==2 && pr->talairach==0 ) combobox2->setCurrentItem(2);
  if ( pr->talairach==1) combobox2->setCurrentItem(3);  
  connect(combobox2, SIGNAL(activated(int)), centralw, SLOT(setVoxelSystem(int)) );
  combobox2->setEditable(FALSE);
  combobox2->setFocusPolicy(NoFocus);
  tools->addSeparator ();

 
  toolb = QWhatsThis::whatsThisButton( tools );
  QWhatsThis::add( toolb, "This is a <b>What's This</b> button "
		   "It enables the user to ask for help "
		   "about widgets on the screen.");
  
  /* =================== OpenGL TOOLBAR ====================== */
  if (pr->ogl) {
    QPixmap  oglIcon( opengl );
    toolb4 = new QToolButton( oglIcon, "openGL", QString::null, this, SLOT(toolb4Switch()),
			      oGLtools, "openGL" );
    toolb4->setToggleButton ( TRUE );
    toolb4->setTextLabel ( "openGL", TRUE );
    if (pr->picture[3]) toolb4->toggle();
    QWhatsThis::add( toolb4, "Here You can switch on/off the <b>openGL window</b>." );
    
    oGLtools->addSeparator ();

    /* button to enable / disable activations in sulci window */
    if (pr->sw2) {
      toolb5 = new QToolButton( blobsIcon, "zmap", QString::null, this, SLOT(toolbBlobsSwitch()),oGLtools, "zmap" );
      toolb5->setToggleButton ( TRUE );
      toolb5->setTextLabel ( "zmap", TRUE );
      QWhatsThis::add( toolb5, "Here You can switch on/off the <b>zmap</b> in openGL window." );
      oGLtools->addSeparator ();
    }
      
    /* buttons for predefined standard positions in sulci window */
    toolb6 = new QToolButton( emptyIcon, "top", QString::null, this, SLOT(jumpTop()),
			      oGLtools, "top" );
    toolb6->setText( "T" );
    toolb6->setTextLabel ( "top", TRUE );
    QWhatsThis::add( toolb6, "Here You can jump into the std position: top." );

    toolb6 = new QToolButton( emptyIcon, "bottom", QString::null, this, SLOT(jumpButtom()),
			      oGLtools, "bottom" );
    toolb6->setText( "B" );
    toolb6->setTextLabel ( "bottom", TRUE );
    QWhatsThis::add( toolb6, "Here You can jump into the std position: bottom." );

    toolb6 = new QToolButton( emptyIcon, "left", QString::null, this, SLOT(jumpLeft()),
			      oGLtools, "left" );
    toolb6->setText( "L" );
    toolb6->setTextLabel ( "left", TRUE );
    QWhatsThis::add( toolb6, "Here You can jump into the std position: left." );

    toolb6 = new QToolButton( emptyIcon, "right", QString::null, this, SLOT(jumpRight()),
			      oGLtools, "right" );
    toolb6->setText( "R" );
    toolb6->setTextLabel ( "right", TRUE );
    QWhatsThis::add( toolb6, "Here You can jump into the std position: right." );

    toolb6 = new QToolButton( emptyIcon, "anterior", QString::null, this, SLOT(jumpFront()),
			      oGLtools, "anterior" );
    toolb6->setText( "A" );
    toolb6->setTextLabel ( "anterior", TRUE );
    QWhatsThis::add( toolb6, "Here You can jump into the std position: anterior." );

    toolb6 = new QToolButton( emptyIcon, "posterior", QString::null, this, SLOT(jumpBack()),
			      oGLtools, "posterior" );
    toolb6->setText( "P" );
    toolb6->setTextLabel ( "posterior", TRUE );
    QWhatsThis::add( toolb6, "Here You can jump into the std position: posterior." );

    /* additions by A. Hagert (12 Buttons): */
    oGLtools->addSeparator ();
    toolb6 = new QToolButton( emptyIcon, "color on/off", QString::null, this, SLOT(switchColor()),
			      oGLtools, "color on/off" );
    toolb6->setToggleButton ( TRUE );
    toolb6->setText( "C" );
    toolb6->setTextLabel ( "color on/off", TRUE );
    QWhatsThis::add( toolb6, "Here You can toggle between colors on and off: color." );
    
    toolb6 = new QToolButton( emptyIcon, "color on/off", QString::null, this, SLOT(switchLines()),
			      oGLtools, "color on/off" );
    toolb6->setToggleButton ( TRUE );
    toolb6->setText( "L/P" );
    toolb6->setTextLabel ( "lines on/off", TRUE );
    QWhatsThis::add( toolb6, "Here You can toggle between lined and pointed representation." );
    
    toolb6 = new QToolButton( emptyIcon, "polygons on/off", QString::null, this, SLOT(switchPolygons()),
			      oGLtools, "poylgons on/off" );
    toolb6->setToggleButton ( TRUE );
    toolb6->setText( "P" );
    toolb6->setTextLabel ( "polygons on/off", TRUE );
    QWhatsThis::add( toolb6, "Here You can turn on or off the view of polygons (if they are prensent)." );
    toolb6->toggle();
    if (!pr->polygons) toolb6->hide();
    
    oGLtools->addSeparator ();
    
    toolb6 = new QToolButton( emptyIcon, "zoom in", QString::null, this, SLOT(graph_zoom_in()),
			      oGLtools, "zoom in" );
    toolb6->setText( "Z+" );
    toolb6->setTextLabel ( "zoom_in", TRUE );
    QWhatsThis::add( toolb6, "Here You zoom into the graph." );
    
    toolb6 = new QToolButton( emptyIcon, "zoom out", QString::null, this, SLOT(graph_zoom_out()),
			      oGLtools, "zoom out" );
    toolb6->setText( "Z-" );
    toolb6->setTextLabel ( "zoom_out", TRUE );
    QWhatsThis::add( toolb6, "Here You zoom out of the graph." );
    
    toolb6 = new QToolButton( emptyIcon, "zoom norm", QString::null, this, SLOT(graph_zoom_norm()),
			      oGLtools, "zoom norm" );
    toolb6->setText( "Z0" );
    toolb6->setTextLabel ( "zoom_norm", TRUE );
    QWhatsThis::add( toolb6, "Here You set back the graph-zoom." );
    oGLtools->addSeparator ();
    
    toolb6 = new QToolButton( emptyIcon, "fog on/off", QString::null, this, SLOT(switchFog()),
			      oGLtools, "fog on/off" );
    toolb6->setToggleButton ( TRUE );
    toolb6->setText( "fog" );
    toolb6->setTextLabel ( "fog on/off", TRUE );
    if (pr->fog == 1) toolb6->toggle();
    QWhatsThis::add( toolb6, "Here You switch the fog on or off." );
    
    toolb6 = new QToolButton( emptyIcon, "exact clicking on/off", QString::null, this, SLOT(clicking()),
			      oGLtools, "exact clicking on/off" );
    toolb6->setToggleButton ( TRUE );
    toolb6->setText( "E" );
    toolb6->setTextLabel ( "exact clicking on/off", TRUE );
    if (pr->exact == 1) toolb6->toggle();
    QWhatsThis::add( toolb6, "If it is 'on' - mouse clicking is more exact - but slower." );
    oGLtools->addSeparator ();    
  
    /*
      toolb6 = new QToolButton( emptyIcon, "single points are crosses on/off", QString::null, this, SLOT(switchCrosses()),
      oGLtools, "crosses on/off" );
      toolb6->setToggleButton ( TRUE );
      toolb6->setText( "X" );
      toolb6->setTextLabel ( "single points are crosses on/off", TRUE );
      QWhatsThis::add( toolb6, "If it is 'on' - single points are crosses." );
      toolb6->toggle();
      oGLtools->addSeparator ();    
    */

      
    toolb6 = new QToolButton( emptyIcon, "find local minimum", QString::null, this, SLOT(findminZ_graph()),
			      oGLtools, "local minimum" );
    toolb6->setToggleButton ( FALSE );
    toolb6->setText( "l_Min" );
    toolb6->setTextLabel ( "local minimum", TRUE );
    QWhatsThis::add( toolb6, "finds local minimum" );
    //toolb6->toggle();
    oGLtools->addSeparator ();    

    toolb6 = new QToolButton( emptyIcon, "find local maximum", QString::null, this, SLOT(findmaxZ_graph()),
			      oGLtools, "local maximum" );
    toolb6->setToggleButton ( FALSE );
    toolb6->setText( "l_Max" );
    toolb6->setTextLabel ( "local maximum", TRUE );
    QWhatsThis::add( toolb6, "finds local maximum" );
    //toolb6->toggle();
    oGLtools->addSeparator ();    

    /*
      if (pr->ipc>0) {
      oGLtools->addSeparator();    
      toolb6 = new QToolButton ( emptyIcon, "synchronize rotation", QString::null, this, SLOT(synchronize_rotation()),oGLtools, "axial" );
      toolb6->setToggleButton ( TRUE );
      toolb6->setText( "S/R" );
      toolb6->setTextLabel ( "synchronize rotation", TRUE );
      if (pr->synchronize == 1) toolb6->toggle();
      }
    */

    combobox3 = new QComboBox( FALSE, oGLtools, "openvis" );
    static const char* itemcombo3[] = { "Dot", "Cro", "Sph", 0 };
    combobox3->insertStrList( itemcombo3 );
    if (pr->openvis==0) combobox3->setCurrentItem(0);
    if (pr->openvis==1) combobox3->setCurrentItem(1);
    if (pr->openvis==2) combobox3->setCurrentItem(2);
    connect(combobox3, SIGNAL(activated(int)), this, SLOT(setOpenvis(int)) );
    combobox3->setEditable(FALSE);
    setOpenvis(pr->openvis);

  }

  /* =================== MENUBAR ====================== */
  if (pr->verbose>1) fprintf(stderr,"Creating menubar...\n");
  QMenuBar *menubar = new QMenuBar( /* Mtools */ this );
  
  QPopupMenu* popup = new QPopupMenu;
  CHECK_PTR( popup );

  /* opens a filedialog 
     popup->insertItem( fileIcon, "&Open",  dialog, SLOT(open()), CTRL+Key_O );
     QPixmap  saveIcon( filesave );
     popup->insertSeparator();
  */

  /* print button 
     int printid = popup->insertItem( "&Print", centralw, SLOT(print()), CTRL+Key_P );
     popup->insertSeparator ();
  */

  /* Quit */
  QPixmap fileIcon(fileopen);
  if (pr->files == 1) {
    popup->insertItem( fileIcon, "&Open File",  dialog, SLOT(open()), CTRL+Key_O );
    //popup->insertItem( fileIcon, "&Open Zmap",  dialog, SLOT(openZ()), CTRL+Key_O );
  }
  popup->insertItem( "&Export", this, SLOT(imageexport()), CTRL+Key_E);
  popup->insertItem( "&Quit", qApp, SLOT(quit()), CTRL+Key_Q );
 
  QPopupMenu* view = new QPopupMenu( this );
  CHECK_PTR( view );
  view->insertItem( prefIcon, "&Preferences", this, SLOT(prefs()), CTRL+Key_P );
  if (pr->only_sulci == 0) {  
    view->insertItem( setcoordIcon, "Set &Coordinates", centralw, SLOT(coordIN()), CTRL+Key_C );
    view->insertItem( zoominIcon,"Zoom &In", centralw, SLOT(zoomplus()), CTRL+Key_I );
    view->insertItem( zoomoutIcon,"Zoom &Out", centralw, SLOT(zoomminus()), CTRL+Key_O );
    view->insertItem( findIcon,"&Find Minimum", centralw, SLOT(findMinZ()),CTRL+Key_F );
    view->insertItem( findmaxIcon,"Find &Maximum", centralw, SLOT(findMaxZ()),CTRL+Key_M );
    if (pr->raw) {
      view->insertItem( rawIcon, "Show &RawData", rawplot, SLOT(ausgabe()), CTRL+Key_S );
      if (!strcmp(programname,"vlcorr"))
	view->insertItem( corrIcon, "Co&rrelation", correl, SLOT(correlation()), CTRL+Key_R );
    }
  }

  QPopupMenu* help = new QPopupMenu( this );
  CHECK_PTR( help );

  /* button to open a about dialog */
  if (pr->verbose>1) fprintf(stderr,"Creating menu...\n");
  help->insertItem( "&About", dialog, SLOT(about()), CTRL+Key_A ); 
  menubar->insertItem( "&File", popup );
  menubar->insertItem( "&Options", view );
  menubar->insertItem( "&Help", help );
  
  count = 0;
  down = FALSE;

  /* generate 256 gray colors */
  colors = new QColor[256];
  for (int i=0; i<256; i++ )	
    colors[i] = QColor( i, i, i );
  
  /* Statusbar on the button of the window */
  QStatusBar *statusbar = new QStatusBar( Stools, "Graph" );

  zzwert = new QLCDNumber ( pr->digits, statusbar, "zwert" );
  zzwert->setSegmentStyle ( QLCDNumber::Filled );
  QWhatsThis::add( zzwert, "Actual zscore of the voxel.");
  statusbar->addWidget( zzwert, 2 );
  zzwert->display( "" );

  echt = new QLabel( statusbar, "message", 0 );
  echt->setFont( QFont( "arial", 10, QFont::Normal, FALSE ) );
  QWhatsThis::add( echt, "This widget displays the actual mouse position. "
		   "The menu let you switch to voxel, mm or talairach mode.");
  statusbar->addWidget( echt, 6 );
  echt->setText( tr("%1").arg(programname) );

  if (pr->verbose>1) fprintf(stderr,"Initializing slots...\n");
  z2wert = new QLCDNumber ( pr->digits, statusbar, "zwert" );
  z2wert->setSegmentStyle ( QLCDNumber::Filled );
  QWhatsThis::add( z2wert, "Selected zscore of the voxel.");
  statusbar->addWidget( z2wert, 2 );
  z2wert->display( "" );
  
  if (pr->only_sulci == 0) {   
    for (int i=0;i<pr->files;i++) {
      QObject::connect(centralw->bild1[i], SIGNAL(echtPosit(float, float, float, QString)), this, SLOT(echtXYZ(float,float,float,QString )));
      QObject::connect(centralw->bild2[i], SIGNAL(echtPosit(float, float, float, QString)), this, SLOT(echtXYZ(float,float,float,QString )));
      QObject::connect(centralw->bild3[i], SIGNAL(echtPosit(float, float, float, QString)), this, SLOT(echtXYZ(float,float,float,QString )));
    }
  }
  else 
    QObject::connect(centralw->ogl[0], SIGNAL(got_color_min_max(float, float)), this, SLOT(print_color_min_max(float, float)));
  
  
  xyz = new QLabel( statusbar, "message", 0 );
  xyz->setFont( QFont( "arial", 10, QFont::Normal, FALSE ) );
  QWhatsThis::add( xyz, "This widget displays the selected coordinates. "
		   "The menu let you switch to voxel, mm or talairach mode.");
  statusbar->addWidget( xyz, 6 );
  
  xyz->setText( "" );
  
  for (int i=0;i<pr->files;i++) {
    if (pr->only_sulci == 0) {  
      QObject::connect(centralw->bild1[i], SIGNAL(crossPosit(float, float, float, QString)), this, SLOT(statXYZ(float, float, float, QString )));
      QObject::connect(centralw->bild2[i], SIGNAL(crossPosit(float, float, float, QString)), this, SLOT(statXYZ(float, float, float, QString )));
      QObject::connect(centralw->bild3[i], SIGNAL(crossPosit(float, float, float, QString)), this, SLOT(statXYZ(float, float, float, QString )));
    }
    if (pr->ogl)
      QObject::connect(centralw->ogl[i], SIGNAL(crossPosit(float, float, float, QString)), this, SLOT(statXYZ(float, float, float, QString )));
    if (pr->only_sulci == 0) {  
      QObject::connect(centralw->bild1[i], SIGNAL(zWert(double)), this, SLOT(zWert( double )));
      QObject::connect(centralw->bild2[i], SIGNAL(zWert(double)), this, SLOT(zWert( double )));
      QObject::connect(centralw->bild3[i], SIGNAL(zWert(double)), this, SLOT(zWert( double )));
      QObject::connect(centralw->bild1[i], SIGNAL(z2Wert(double)), this, SLOT(z2aWert( double )));
      QObject::connect(centralw->bild2[i], SIGNAL(z2Wert(double)), this, SLOT(z2aWert( double )));
      QObject::connect(centralw->bild3[i], SIGNAL(z2Wert(double)), this, SLOT(z2aWert( double )));
    }
    if (pr->ogl)
      QObject::connect(centralw->ogl[i], SIGNAL(crossLabel(double)), this, SLOT(z2aWert_sulci( double )));
  }
  
  if (pr->only_sulci == 0) {   
    QObject::connect(this, SIGNAL(crossPosit(float, float, float, QString)), this, SLOT(statXYZ(float, float, float, QString )));
    QObject::connect(centralw, SIGNAL(crossPosit(float, float, float, QString)), this, SLOT(statXYZ(float, float, float, QString )));
    QObject::connect(centralw, SIGNAL(echtPosit(float, float, float, QString)), this, SLOT(echtXYZ(float, float, float, QString )));
    QObject::connect(centralw, SIGNAL(z2Wert(double)), this, SLOT(z2aWert( double )));
    QObject::connect(centralw, SIGNAL(zWert(double)), this, SLOT(zWert( double )));
    
    cota = new QLabel(statusbar, "message", 0);
    cota->setPixmap(centralw->bild1[0]->cpm);
    statusbar->addWidget( cota, 1 );
    QWhatsThis::add( cota, "Here you see the colortable of the pictures.");
    QObject::connect(centralw->bild3[0], SIGNAL(colbarRepaint()), this, SLOT(colbarRepaint()));
  }
  QObject::connect(centralw, SIGNAL(setVoxelBox()), this, SLOT(setVoxelBox()));
  

  QLabel *logo;
  logo = new QLabel(statusbar, "message", 0);
  logo->setPixmap(lipsia);
  QWhatsThis::add( logo, "This is the logo only.");
  statusbar->addWidget( logo, 1, FALSE );

  for (int i=0;i<pr->files;i++) {
    if (pr->only_sulci == 0) {   
      if (!strcmp(programname,"vlcorr")) {
	connect( correl, SIGNAL(viewChanged()), centralw->bild1[i], SLOT(repaintf()) );
	connect( correl, SIGNAL(viewChanged()), centralw->bild2[i], SLOT(repaintf()) );
	connect( correl, SIGNAL(viewChanged()), centralw->bild3[i], SLOT(repaintf()) );
      }
      connect( this, SIGNAL(viewChanged()), centralw->bild1[i], SLOT(repaintf()) );
      connect( this, SIGNAL(viewChanged()), centralw->bild2[i], SLOT(repaintf()) );
      connect( this, SIGNAL(viewChanged()), centralw->bild3[i], SLOT(repaintf()) );
      connect( prefer, SIGNAL( applyButtonPressed() ), centralw->bild1[i], SLOT( repaintf() ) );
      connect( prefer, SIGNAL( applyButtonPressed() ), centralw->bild2[i], SLOT( repaintf() ) );
      connect( prefer, SIGNAL( applyButtonPressed() ), centralw->bild3[i], SLOT( repaintf() ) );
      QObject::connect( prefer, SIGNAL(newColtype()), centralw->bild1[i], SLOT(colorMap()));
      QObject::connect( prefer, SIGNAL(newColtype()), centralw->bild2[i], SLOT(colorMap()));
      QObject::connect( prefer, SIGNAL(newColtype()), centralw->bild3[i], SLOT(colorMap()));
      QObject::connect( prefer, SIGNAL(newColtype()), this, SLOT(colbarRepaint()));
      if (pr->graph[0]) QObject::connect( prefer, SIGNAL(newColtype()), centralw->ogl[i], SLOT(initializeGL()));
      if (pr->graph[0]) QObject::connect( prefer, SIGNAL(newColtype()), centralw->ogl[i], SLOT(updateGL()));
    }
    if (pr->graph[0]) QObject::connect( prefer, SIGNAL(newgraphColtype()), this, SLOT(change_coltab()));
  }
  if (pr->only_sulci == 0) {  
    QObject::connect( prefer, SIGNAL( applyButtonPressed() ), this, SLOT(colbarRepaint()));
    QObject::connect( prefer, SIGNAL( reloadFiles() ), this, SLOT(reloadFiles()));
    QObject::connect( this, SIGNAL(z2Wert(double)), this, SLOT(z2aWert( double )));
    QObject::connect( prefer, SIGNAL( nowsliderChangeS() ), centralw, SLOT(nowsliderChange()));
    QObject::connect( centralw, SIGNAL( SlideReleasedForVLRender() ), this, SLOT(sendtoserver()));
  }
  QObject::connect( prefer, SIGNAL( talOnOff() ), this, SLOT(talOnOff()));
  QObject::connect( prefer, SIGNAL( setVoxelBox() ), this, SLOT(setVoxelBox()));
  //QObject::connect( this, SIGNAL(mouseMoveEvent (QMouseEvent*)), this, SLOT(mouseMoved(QMouseEvent*)));
  

  /***********************************************************************************************
   *                                                                                             *
   *                     P I P E   -   S Y N C H R O N I Z A T I O N                             *
   *                                                                                             *
   ***********************************************************************************************/                           

  /* Erstelle Name fr Pipe */
  char zeile[100], zeile2[100]; FILE *progpipe; char pipename[100]; char *zeile1;
  progpipe = popen("whoami","r");
  if (fgets(zeile,100,progpipe) != NULL) {
    zeile1 = strtok(zeile,"\n");
    sprintf(pipename,"/tmp/lipsia.%s",zeile1);
  } else 
    sprintf(pipename,"/tmp/lipsia.unknown");
  pclose(progpipe);
  if (pr->verbose>0) fprintf(stderr,"Name of pipe is: %s\n",pipename);
  
  /* check if vlserv is running */
  bool sw_vlserv=FALSE;
  int vlservpid=0;
  sprintf(zeile2,"ps -u %s | grep vlserv",zeile1);
  progpipe = popen(zeile2,"r");
  if (fgets(zeile,100,progpipe) != NULL) {
    zeile1 = strtok(strtok(zeile,"\n")," ");
    vlservpid=atoi(zeile1);
    while (zeile1) {
      zeile1 = strtok(NULL," ");
      if (zeile1) {
	if (!strcmp(zeile1,"vlserv")) sw_vlserv=TRUE;
      }
    }
  }
  pclose(progpipe);
  //free(zeile); free(zeile2);
  
  /* open own pipe for obtaining from server */
  if (pr->ipc>0) {
    if (sw_vlserv==TRUE) {
      serverini=1;
      if ((fd_fifo0=open(pipename, O_WRONLY)) == - 1) {
	sprintf(zeile2,"kill -9 %d\0",vlservpid);
	system(zeile2);
	sw_vlserv=FALSE;
      }
    }
    if (sw_vlserv==FALSE) {
      serverini=0;
      if (pr->verbose>0) fprintf(stderr, "Do not find Lipsia Server. Restarting.\n");
      FILE *ftest;
      ftest = fopen ("/usr/bin/vlserv", "r");
      if (! ftest) {
	if (pr->verbose>0) fprintf(stderr,"vlserv daemon not installed. Giving up.\n");
	pr->ipc=0;
	toolbsyn->setDisabled(TRUE);
      } else {
	fclose(ftest);
	system("/usr/bin/vlserv &");
	sleep(1);
	if((fd_fifo0=open(pipename, O_WRONLY)) == - 1) {
	  if (pr->verbose>0) fprintf(stderr, "No contact with Lipsia Server. Giving up.\n");
	  pr->ipc=0;
	  toolbsyn->setDisabled(TRUE);
	} else {
	  sw_vlserv==TRUE;
	}
      }
    }
  }
  
  /* make own pipe for IPC */
  if (pr->ipc>0) {
    sprintf(fifoname,"/tmp/lipsia.%d\0",getpid());
    if(mknod(fifoname, S_IFIFO | S_IRUSR | S_IWUSR, 0) == - 1) {
      fprintf(stderr, "Can't creat a fifo.........\n");
      pr->ipc=0;
    } else {
      if((fd_fifo=open(fifoname, O_RDWR)) == - 1) {
	fprintf(stderr, "Can't open the fifo socket %s.\n",fifoname);
	pr->ipc=0;
      } else {
	notif_m = new QSocketNotifier (fd_fifo, QSocketNotifier::Read);
	connect (notif_m, SIGNAL(activated(int)), SLOT(syncronize(int)));
	for (int i=0;i<pr->files;i++) {
	  if (pr->only_sulci == 0) {  
	    connect(centralw->bild1[i], SIGNAL(sendtoserver()), this, SLOT(sendtoserver()));
	    connect(centralw->bild2[i], SIGNAL(sendtoserver()), this, SLOT(sendtoserver()));
	    connect(centralw->bild3[i], SIGNAL(sendtoserver()), this, SLOT(sendtoserver()));
	    connect(centralw, SIGNAL(sendtoserver()), this, SLOT(sendtoserver()));
	  }
	  if (pr->ogl) {
	    connect(centralw->ogl[i], SIGNAL(sendtoserver()), this, SLOT(sendtoserver()));
	    connect(centralw->rc1, SIGNAL(sliderReleased()), this, SLOT(sendtoserver()));
	    connect(centralw->rc2, SIGNAL(sliderReleased()), this, SLOT(sendtoserver()));
	    connect(centralw->rc3, SIGNAL(sliderReleased()), this, SLOT(sendtoserver()));
	  }
	}
      }
    }
    sendtoserver();
    serverini=0;
  } 
  
  /***********************************************************************************************
   *                                                                                             *
   *            E N D         --         P I P E   -   S Y N C H R O N I Z A T I O N             *
   *                                                                                             *
   ***********************************************************************************************/           

  hideitems(1);
  StartResize();
  
  
  // Error boxes
  if (raw_defekt) QMessageBox::warning( this, "Error",
 				    "Error with raw data file\nWorking without raw data");
  if (design_defekt) QMessageBox::warning( this, "Error",
				    "Error with design file\nWorking without design file");
  if (beta_defekt) QMessageBox::warning( this, "Error",
				    "Error with beta file\nWorking without beta file");
  if (pr->verbose>0) fprintf(stderr,"End of lView.C\n");

}

/* cleanup memory */
lView::~lView() {
  if (pr->ipc) unlink(fifoname);
  if (pr->only_sulci == 1) { pr->picture[0] = 1; pr->picture[1] = 1; pr->picture[2] = 1; pr->picture[3] = 1; }
  if (pr->fog>5) do {pr->fog-=10;} while (pr->fog>5);
  dialog->saveOptions(2);
  delete[] colors;
  VFree(ca);
  VFree(extent);
} 

/* slot for reloading input-files */
void lView::reloadFiles( ) {
  char *programname=strdup(pr->prg_name);
  programname=strtok(programname,":");

  /* load files */
  myload.loadFiles();
  if (!strcmp(programname,"vlcorr")) myload.loadFilesForCorr();

  /* INTERPOLATE ZMAP OR INTERPOLATE ANATOMICAL*/
  for (int ifile=0;ifile<pr->files;ifile++) {
    mytools.interpolate( src[ifile], fnc[ifile], pr->ncols_mult, pr->nrows_mult, pr->nbands_mult, scaleb[ifile], scaler[ifile], scalec[ifile], 0, pr->interpol );
  }

}

void lView::imageexport() {
	QFileDialog dlg(this, "Export Image", true);
	dlg.setMode( QFileDialog::AnyFile );
	dlg.setFilter("Image files (*.png)");
	
	
    if ( dlg.exec() == QDialog::Accepted ) {
        QString filename = dlg.selectedFile();
        if (!(filename.endsWith(".png")))
        		filename+=".png";
        fprintf(stderr, "%s; files: %d\n", filename.latin1(), centralw->files);
        QPixmap map(centralw->width(), centralw->height()+centralw->bild1[0]->cpm.height()+2);
        QPainter painter(&map);
        
        QColor fg;
        QColor bg; 
                
        if (pr->hgfarbe[0]==0) {
        	fg = QColor(255, 255, 255);
        	bg = QColor (0, 0, 0);
        } else {
        	fg = QColor (0, 0, 0);
        	bg = QColor(255, 255, 255);
        }
        
        painter.setBrush(bg);
        painter.setPen(bg);
        painter.drawRect(0, 0, map.width(), map.height());
        
        for (int i=0; i<centralw->files; i++) {
        	if (pr->picture[0]) {
	        	QRect rec = centralw->bild1[i]->geometry();
	        	painter.resetXForm();
	        	painter.translate(rec.x(), rec.y());
	        	centralw->bild1[i]->paint(&painter);
        	}
        	if (pr->picture[1]) {
        		QRect rec = centralw->bild2[i]->geometry();
	        	painter.resetXForm();
	        	painter.translate(rec.x(), rec.y());
	        	centralw->bild2[i]->paint(&painter);
        	}
        	if (pr->picture[2]) {
        		QRect rec = centralw->bild3[i]->geometry();
	        	painter.resetXForm();
	        	painter.translate(rec.x(), rec.y());
	        	centralw->bild3[i]->paint(&painter);
        	}
        	if (pr->ogl) {
        		QRect rec = centralw->ogl[i]->geometry();
        		painter.resetXForm();
        		painter.translate(rec.x(), rec.y());
        		QImage img = centralw->ogl[i]->grabFrameBuffer();
        		painter.drawImage(0, 0, img);
        	}
        }
        
        // drawing the colormap:
        painter.resetXForm();
        VLShow myshow;
        QPixmap cpm;
        
        
        /* create legend if zmap is activated*/
        if ( (pr->sw2) && (pr->zmapview) ) {
	      	myshow.vlhCreateLegend( cpm, 
	      			centralw->bild1[0]->rgbfarbeoverlay, 
	      			centralw->bild1[0]->ppmax, pr->pmax, 
	      			centralw->bild1[0]->nnmax, pr->nmax, 
	      			pr->equidistantColorTable,
	      			bg, fg);
	      		
	        int y = map.height()-cpm.height();
	        
	        painter.drawPixmap(0, y, cpm);
        }
        map.save(filename, "PNG");
    }
}

/* handle all key press events */
void lView::keyPressEvent( QKeyEvent *k ) {
  int step = 10;

  if (pr->ogl) {
    if (k->ascii() == 'a' && pr->ogl)
      for (int i=0;i<pr->files;i++) {
	centralw->ogl[i]->move((float)-step/100.0, 0.0, 0.0);
	//centralw->ogl[i]->move_total_x += -step;
	centralw->ogl[i]->updateGL();
      }
    if (k->ascii() == 'd' && pr->ogl)
      for (int i=0;i<pr->files;i++) {
	centralw->ogl[i]->move((float)step/100.0, 0.0, 0.0);
	//centralw->ogl[i]->move_total_x += step;
	centralw->ogl[i]->updateGL();
      }
    if (k->ascii() == 's' && pr->ogl)
      for (int i=0;i<pr->files;i++) {
	centralw->ogl[i]->move(0.0, (float)-step/100.0, 0.0);
	//centralw->ogl[i]->move_total_y += -step;
	centralw->ogl[i]->updateGL();
      }
    if (k->ascii() == 'w' && pr->ogl)
      for (int i=0;i<pr->files;i++) {
	centralw->ogl[i]->move(0.0, (float)step/100.0, 0.0);
	//centralw->ogl[i]->move_total_y += step;
	centralw->ogl[i]->updateGL();
      }
    if (k->ascii() == 'q' && pr->ogl)
      for (int i=0;i<pr->files;i++) {
	//centralw->ogl[i]->move((float)-centralw->ogl[i]->move_total_x/100.0, (float)-centralw->ogl[i]->move_total_y/100.0, 0.0);
	//centralw->ogl[i]->move_total_x = 0;
	//centralw->ogl[i]->move_total_y = 0;
	centralw->ogl[i]->updateGL();
      }
  }
  if (k->key() == Key_Left && pr->cursorp[0]>0) pr->cursorp[0]--;
  if (k->key() == Key_Up && pr->cursorp[2]>0) pr->cursorp[2]--;
  if (k->key() == Key_Right && pr->cursorp[0]<VImageNColumns (src[0])-1 ) pr->cursorp[0]++;
  if (k->key() == Key_Down && pr->cursorp[2]<VImageNFrames (src[0])-1 ) pr->cursorp[2]++;
  if (k->key() == Key_Prior && pr->cursorp[1]>0) pr->cursorp[1]--;
  if (k->key() == Key_Next && pr->cursorp[1]<VImageNRows (src[0])-1 ) pr->cursorp[1]++;
  
  if (k->key() == Key_Left || k->key() == Key_Up || k->key() == Key_Right || k->key() == Key_Down || k->key() == Key_Prior || k->key() == Key_Next) {
    centralw->bild1[0]->talCross(int(pr->cursorp[0]),int(pr->cursorp[1]),int(pr->cursorp[2]));
    
    if (fnc[0] && pr->cursorp[2]<VImageNFrames(fnc[0]) && pr->cursorp[1]<VImageNRows(fnc[0]) && pr->cursorp[0]<VImageNColumns(fnc[0]))
      emit z2Wert(VPixel(fnc[pr->active],(int)rint(pr->cursorp[2]),(int)rint(pr->cursorp[1]),(int)rint(pr->cursorp[0]),VFloat)); 
    else if (pr->cursorp[2]<VImageNFrames(src[0]) && pr->cursorp[1]<VImageNRows(src[0]) && pr->cursorp[0]<VImageNColumns(src[0]))
      emit z2Wert(VGetPixel(src[pr->active],(int)rint(pr->cursorp[2]),(int)rint(pr->cursorp[1]),(int)rint(pr->cursorp[0])));
  }
  
  emit viewChanged();
  sendtoserver();
  
  if (k->key() == Key_F1) talOnOff();
  if (k->ascii() == 't')  talOnOff();
  if (k->key() == Key_F12) hideitems(0); 
}

/* write out the clicked coordinates */
void lView::statXYZ( float x, float y, float z, QString mode ) {
  if (strcmp(pr->prg_name,"vgview")) {
    xyz->setText( tr(" %1 %2 %3").arg((int)rint((double)x)).arg((int)rint((double)y)).arg((int)rint((double)z)) );
  } else {
    xyz->setText( tr(" %1 %2 %3").arg((float)x).arg((float)y).arg((float)z) );
  }
}



/* write out the mouse over coordinates */
void lView::echtXYZ( float x, float y, float z, QString mode ) {
  if (centralw->hasMouse()) {
    
    if (strcmp(pr->prg_name,"vgview")) {
      echt->setText( tr(" %1 %2 %3").arg((int)rint((double)x)).arg((int)rint((double)y)).arg((int)rint((double)z)) );
    } else {
      echt->setText( tr(" %1 %2 %3").arg((float)x).arg((float)y).arg((float)z) );
    }
  } else {
    echt->setText( tr("  "));
  }
}

/* display the zmap value */
void lView::zWert( double z ) {
  QString text;

	if(centralw->hasMouse()) {
		if(fnc[0]) {
			if(z!=0) {
				text.sprintf("%f\0", z);
			} else {
				text = "0";
		    }
			  
			if((int)text.length() > pr->digits) {
				text = text.left(pr->digits);
			}
			  
			if(pr->equidistantColorTable) {
				// NOTE: [TS]
				// pr->nmax and centralw->bild1[0]->ppmax have a _positive_ 
				// value and therefore the signs here look a wee bit funny
				if(z != 0.0 && z < (pr->pmax - centralw->bild1[0]->ppmax) && z > -(pr->nmax - centralw->bild1[0]->nnmax)) {
					int colorIndex = (int)rint(255.0 * (z - -pr->nmax) / (pr->pmax - -pr->nmax));
					zzwert->setBackgroundColor(centralw->bild1[0]->rgbfarbeoverlay[colorIndex]);
				} else {
		    		zzwert->setBackgroundMode(PaletteBackground);
				}
			} else {
				int farbeunten = (int)rint(127.0 / (pr->pmax - centralw->bild1[0]->ppmax) * (z - centralw->bild1[0]->ppmax)) + 128;
				if(farbeunten < 0) {
					farbeunten = 0;
				}
				if(farbeunten > 255) {
					farbeunten = 255;
				}
		
				int farbeoben = 127 - (int)rint(127.0 / (pr->nmax - centralw->bild1[0]->nnmax) * (-z - centralw->bild1[0]->nnmax));
			    if(farbeoben < 0) {
			    	farbeoben = 0;
			    }
			    if(farbeoben > 255) {
			    	farbeoben = 255;
			    }
			      
			    if(z > centralw->bild1[0]->ppmax) {
			    	zzwert->setBackgroundColor(centralw->bild1[0]->rgbfarbeoverlay[farbeunten]);
			    } else {
			    	if(z < -centralw->bild1[0]->nnmax) {
			    		zzwert->setBackgroundColor(centralw->bild1[0]->rgbfarbeoverlay[farbeoben]);
			    	} else {			
			    		zzwert->setBackgroundMode(PaletteBackground);
			    	}
			    }
			}
			zzwert->display( text );
		} else { 
		    /* show voxelvalue of anatomy */
		    zzwert->setBackgroundMode(PaletteBackground);
		    text=tr("%1").arg(z);
		    zzwert->display( text );
		}
	} else {
	    text=tr("  ");
	    zzwert->setBackgroundMode(PaletteBackground);
	    zzwert->display(text);
	}
}

/* display the zmap value */
void lView::z2aWert( double z ) {

	QString text;
	FILE *f;
  
	if(fnc[0]) {
		if(z!=0) {
			text.sprintf("%f\0", z);
		} else {
			text = "0";
	    }
		  
		if((int)text.length() > pr->digits) {
			text = text.left(pr->digits);
		}
		  
		if(pr->equidistantColorTable) {
			// NOTE: [TS]
			// pr->nmax and centralw->bild1[0]->ppmax have a _positive_ 
			// value and therefore the signs here look a wee bit funny
			if(z != 0.0 && z < (pr->pmax - centralw->bild1[0]->ppmax) && z > -(pr->nmax - centralw->bild1[0]->nnmax)) {
				int colorIndex = (int)rint(255.0 * (z - -pr->nmax) / (pr->pmax - -pr->nmax));
				z2wert->setBackgroundColor(centralw->bild1[0]->rgbfarbeoverlay[colorIndex]);
			} else {
	    		z2wert->setBackgroundMode(PaletteBackground);
			}
		} else {
			int farbeunten = (int)rint(127.0 / (pr->pmax - centralw->bild1[0]->ppmax) * (z - centralw->bild1[0]->ppmax)) + 128;
			if(farbeunten < 0) {
				farbeunten = 0;
			}
			if(farbeunten > 255) {
				farbeunten = 255;
			}
	
			int farbeoben = 127 - (int)rint(127.0 / (pr->nmax - centralw->bild1[0]->nnmax) * (-z - centralw->bild1[0]->nnmax));
		    if(farbeoben < 0) {
		    	farbeoben = 0;
		    }
		    if(farbeoben > 255) {
		    	farbeoben = 255;
		    }
		      
		    if(z > centralw->bild1[0]->ppmax) {
		    	z2wert->setBackgroundColor(centralw->bild1[0]->rgbfarbeoverlay[farbeunten]);
		    } else {
		    	if(z < -centralw->bild1[0]->nnmax) {
		    		z2wert->setBackgroundColor(centralw->bild1[0]->rgbfarbeoverlay[farbeoben]);
		    	} else {			
		    		z2wert->setBackgroundMode(PaletteBackground);
		    	}
		    }
		}
		z2wert->display(text);
	} else { 
	    /* show voxelvalue of anatomy */
	    z2wert->setBackgroundMode(PaletteBackground);
	    text = tr("%1").arg(z);
	    z2wert->display(text);
	}

  /* A Hagert */
  if (pr->fog > 5) pr->fog -= 10;
}

/* Addition by A. Hagert */
void lView::z2aWert_sulci( double x )   
{
  int rgb[3];
  FILE *f;
  QString text;
  
  /* show voxelvalue of graphpoint */
  text=tr("%1").arg(x);
  z2wert->display( /* x */text );
  for (int i=0;i<pr->files;i++) 
  {
    rgb[0] = centralw->ogl[i]->lastcolor[0];
    rgb[1] = centralw->ogl[i]->lastcolor[1];
    rgb[2] = centralw->ogl[i]->lastcolor[2];
  }
  if (rgb[0]<0 || rgb[0]>255) rgb[0]=127;
  if (rgb[1]<0 || rgb[1]>255) rgb[1]=127;
  if (rgb[2]<0 || rgb[2]>255) rgb[2]=127;
  if (rgb[0]!=127) z2wert->setBackgroundColor(QColor(rgb[0], rgb[1], rgb[2]));  
  else z2wert->setBackgroundMode(PaletteBackground);   

  /* A Hagert */
  if (pr->fog > 5) pr->fog -= 10;
  
  for (int i=0;i<pr->files;i++) 
  {
    centralw->ogl[i]->lastcolor[0]=-1;
    centralw->ogl[i]->lastcolor[1]=-1;
    centralw->ogl[i]->lastcolor[2]=-1;
  }
}

/* switch hide mode on/off */
void lView::hideitems(int z)
{
  if (z==1) {
    if (pr->hideitems==0) { 
      //Stools->show(); 
      tools->show();
      if (pr->ogl) oGLtools->show(); 
    } else { 
      //Stools->hide(); 
      tools->hide(); 
      if (pr->ogl) oGLtools->hide();
    }
  } else {
    if (pr->hideitems==0) {
      //Stools->hide(); 
      tools->hide();
      if (pr->ogl) oGLtools->hide();
      pr->hideitems=1;
    } else {
      //Stools->show(); 
      tools->show();
      if (pr->ogl) oGLtools->show();
      pr->hideitems=0;
    }
  }
  resizePicture();
}

/* handle button to switch talairach coordinates on/off */
void lView::talOnOff()
{
  if (pr->talairach==0) pr->talairach=1;
  else pr->talairach=0;
  emit viewChanged();
  if (pr->only_sulci==1)
    centralw->ogl[0]->talCross(float(pr->cursorp[0]),float(pr->cursorp[1]),float(pr->cursorp[2]));
  else {
    centralw->talCross(int(pr->cursorp[0]),int(pr->cursorp[1]),int(pr->cursorp[2]));
    centralw->talEcht(int(pr->cursorp[0]),int(pr->cursorp[1]),int(pr->cursorp[2]));
  }
  setVoxelBox();
}

/* repaint only the colorbar */
void lView::colbarRepaint()
{
  if (pr->picture[0])
    cota->setPixmap(centralw->bild1[0]->cpm);
  else if (pr->picture[1])
    cota->setPixmap(lipsia);
  else if (pr->picture[2])
    cota->setPixmap(lipsia);
}

/* opens the preferences window */
void lView::prefs()
{
  prefer->show();
}

void lView::toolb1Switch(){
  pr->picture[0]=toolb1->isOn();
  centralw->hideLayout(this);
  resizePicture();
}

void lView::toolb2Switch() {
  pr->picture[1]=toolb2->isOn();
  centralw->hideLayout(this);
  resizePicture();
}

void lView::toolb3Switch() {
  pr->picture[2]=toolb3->isOn();
  centralw->hideLayout(this);
  resizePicture();
}

void lView::toolb4Switch() {
  pr->picture[3]=toolb4->isOn();
  centralw->hideLayout(this);
  resizePicture();
}

/* turn on/off zmap in opengl window */
void lView::toolbBlobsSwitch() {
  pr->picture[4]=toolb5->isOn();
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->optionsOnOff(this);
  }
}

/* turn on/off zmap in picture-window */
void lView::toolbZmapSwitch() {
  pr->zmapview=toolb7->isOn();

  //  picture[4]=toolb5->isOn();
  for (int i=0;i<pr->files;i++) {
    centralw->bild1[i]->repaintf();
    centralw->bild2[i]->repaintf();
    centralw->bild3[i]->repaintf();
  }
}

/* switchs the backgroundcolor betwen black and white */
void lView::hgsw() {
  if (pr->hgfarbe[0]==1) {
    pr->hgfarbe[0]=0;
    centralw->setPalette( QPalette( QColor( 0,0,0) ) );
  } else {
    pr->hgfarbe[0]=1;
    centralw->setPalette( QPalette( QColor( 255,255,255) ) );
  }
  for (int i=0;i<pr->files;i++) {
    if (pr->graph[i]) { 
      centralw->ogl[i]->initializeGL(); 
      if (pr->fog == 0) centralw->ogl[i]->Disable_fog();
      else centralw->ogl[i]->Enable_fog();
      centralw->ogl[i]->updateGL();
    }
    centralw->bild1[i]->repaintf();
    centralw->bild2[i]->repaintf();
    centralw->bild3[i]->repaintf();
  }
}

/* default object rotation */
void lView::jumpTop() {
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->xRot = 0.0;
    centralw->ogl[i]->yRot = 180.0;
    centralw->ogl[i]->zRot = 180.0;
    centralw->rc1->setValue(0);     // x - scrollbar
    centralw->rc2->setValue(180);   // y - scrollbar
    centralw->rc3->setValue(180);   // z - scrollbar
    centralw->ogl[i]->updateGL();
  }
  centralw->nowsliderChange();
}

/* default object rotation */
void lView::jumpButtom() {
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->xRot = 180.0;
    centralw->ogl[i]->yRot = 180.0;
    centralw->ogl[i]->zRot = 0.0;    //xyz->setText( tr(" %1 %2 %3").arg((int)x).arg((int)y).arg((int)z) );
    centralw->rc1->setValue(180);   // x - scrollbar
    centralw->rc2->setValue(180);   // y - scrollbar
    centralw->rc3->setValue(0);     // z - scrollbar
    centralw->ogl[i]->updateGL();
  }
  centralw->nowsliderChange();
}

/* default object rotation */
void lView::jumpFront() {
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->xRot = -90.0;
    centralw->ogl[i]->yRot = 180.0;
    centralw->ogl[i]->zRot = 0.0;
    centralw->rc1->setValue(-90);   // x - scrollbar
    centralw->rc2->setValue(180);   // y - scrollbar
    centralw->rc3->setValue(0);     // z - scrollbar
    centralw->ogl[i]->updateGL();
  }
  centralw->nowsliderChange();
}

/* default object rotation */
void lView::jumpBack()
{
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->xRot = -90.0;
    centralw->ogl[i]->yRot = 180.0;
    centralw->ogl[i]->zRot = -180.0;
    centralw->rc1->setValue(-90);   // x - scrollbar
    centralw->rc2->setValue(180);   // y - scrollbar
    centralw->rc3->setValue(-180);     // z - scrollbar
    centralw->ogl[i]->updateGL();
  }
  centralw->nowsliderChange();
}

/* default object rotation */
void lView::jumpRight() {
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->xRot = -90.0;
    centralw->ogl[i]->yRot = 180.0;
    centralw->ogl[i]->zRot = -90.0;
    centralw->rc1->setValue(-90);   // x - scrollbar
    centralw->rc2->setValue(180);   // y - scrollbar
    centralw->rc3->setValue(-90);     // z - scrollbar
    centralw->ogl[i]->updateGL();
  }
  centralw->nowsliderChange();
}

/* default object rotation */
void lView::jumpLeft() {
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->xRot = -90.0;
    centralw->ogl[i]->yRot = 180.0;
    centralw->ogl[i]->zRot = 90.0;
    centralw->rc1->setValue(-90);   // x - scrollbar
    centralw->rc2->setValue(180);   // y - scrollbar
    centralw->rc3->setValue(90);     // z - scrollbar
    centralw->ogl[i]->updateGL();
  }
  centralw->nowsliderChange();
}

// ===============================================================================

// ================================ RESIZE =======================================

// ===============================================================================

/* set size on start */
void lView::StartResize() {

  int fensterbreite;
  if (pr->graph[0])
    fensterbreite=pr->picture[0]+pr->picture[1]+pr->picture[2]+pr->picture[3];
  else
    fensterbreite=pr->picture[0]+pr->picture[1]+pr->picture[2];

  if (pr->verbose>1) fprintf(stderr,"fensterbreite: %d\n",fensterbreite);
  
  if (pr->files<=1) {
    if (fensterbreite==4) {
      resize( 100+2*200, 100+2*200 );
    } else {
      resize( 100+fensterbreite*200, 100+pr->files*200 );
    }
    resizePicture();
  } else if (pr->files < 8) {
    resize( 100+fensterbreite*200, 100+pr->files*200 );
    if (pr->files<=1) {
      resizePicture();
    }
  } else {
    resize( 2*(100+fensterbreite*200), 100+pr->files*200 );
    resizePicture();
  }
}

/* resizeevent */
void lView::resizeEvent ( QResizeEvent *rs ) {
  resizePicture();
  pr->mousemove=0;
}

/* resize all pictures on resizeevent */
void lView::resizePicture () {
  double fb, fbh, fbv, hgesamt, vgesamt, hrund, vrund, rund;
  int nbands, ncols, nrows;

  if (src[0]) {
    //  nbands, nrows, ncols;
    nrows  = (int)VImageNRows (src[0]);
    ncols  = (int)VImageNColumns (src[0]);
    nbands = (int)VImageNFrames (src[0]);

    if (pr->graph[0]) {
      hgesamt=(double)(ncols+nrows+ncols+nrows);
      vgesamt=(double)(nbands+nbands+nrows+nrows);
      hrund=hgesamt/(double)4.0;
      vrund=vgesamt/(double)4.0;
    } else {
      hgesamt=(double)(ncols+nrows+ncols);
      vgesamt=(double)(nbands+nbands+nrows);
      hrund=hgesamt/(double)3.0;
      vrund=vgesamt/(double)3.0;
    }
    rund=(hrund+vrund)/(double)2.0;
   
    if (pr->graph[0]) {
      fb =(double)(pr->picture[0]+pr->picture[1]+pr->picture[2]+pr->picture[3]);
      fbh=(double)(pr->picture[0]*ncols)/rund  + (double)(pr->picture[1]*nrows)/rund  + (double)(pr->picture[2]*ncols)/rund + (double)(pr->picture[3]*nrows)/rund;
      fbv=(double)(pr->picture[0]*nbands)/rund + (double)(pr->picture[1]*nbands)/rund + (double)(pr->picture[2]*nrows)/rund + (double)(pr->picture[3]*nrows)/rund;
    } else {
      fb =(double)(pr->picture[0]+pr->picture[1]+pr->picture[2]);
      fbh=(double)(pr->picture[0]*ncols)/rund  + (double)(pr->picture[1]*nrows)/rund  + (double)(pr->picture[2]*ncols)/rund;
      fbv=(double)(pr->picture[0]*nbands)/rund + (double)(pr->picture[1]*nbands)/rund + (double)(pr->picture[2]*nrows)/rund;
    }


    /* flowlayout only on single files */

    if ( fb >= 1.0 ) {
      float breite = centralw->width()-100.0;
      float hoehe  = centralw->height()/pr->files;
      float p_ho2;

      if (breite>1600) breite=1600;
      if (hoehe>1200) hoehe=1200;
	
      if (nbands >= nrows)
	p_ho2 = nbands + nbands;
      else 
	p_ho2 = nrows + nrows;


      
      for ( int i=0;i<pr->files;i++) {
	if (breite/fb <= hoehe/2.0 && breite >= hoehe) {
	  if (pr->graph[0]) {
	    centralw->ogl[i]->setMaximumSize ( int(hoehe*(nrows/rund)/2.0), int(hoehe*(nrows/rund)/2.0) );
	    centralw->ogl[i]->setMinimumSize ( int(hoehe*(nrows/rund)/2.0), int(hoehe*(nrows/rund)/2.0) );
	  }
	  //fprintf(stderr,"LAYOUT: quadratisch (beachte hoehe)\n");
	  centralw->bild1[i]->setMaximumSize ( int(hoehe*(ncols/rund)/2.0), int(hoehe*(nbands/rund)/2.0) );
	  centralw->bild1[i]->setMinimumSize ( int(hoehe*(ncols/rund)/2.0), int(hoehe*(nbands/rund)/2.0) );
	  centralw->bild2[i]->setMaximumSize ( int(hoehe*(nrows/rund)/2.0), int(hoehe*(nbands/rund)/2.0) );
	  centralw->bild2[i]->setMinimumSize ( int(hoehe*(nrows/rund)/2.0), int(hoehe*(nbands/rund)/2.0) );
	  centralw->bild3[i]->setMaximumSize ( int(hoehe*(ncols/rund)/2.0), int(hoehe*(nrows/rund)/2.0) );
	  centralw->bild3[i]->setMinimumSize ( int(hoehe*(ncols/rund)/2.0), int(hoehe*(nrows/rund)/2.0) );
	} else if (breite/fb <= hoehe/2.0) {
	  if (pr->graph[0]) {
	    centralw->ogl[i]->setMaximumSize ( int(breite*(nrows/rund)/2.0), int(breite*(nrows/rund)/2.0) );
	    centralw->ogl[i]->setMinimumSize ( int(breite*(nrows/rund)/2.0), int(breite*(nrows/rund)/2.0) );
	  }
	  //fprintf(stderr,"LAYOUT: quadratisch (beachte breite)\n");
	  centralw->bild1[i]->setMaximumSize ( int(breite*(ncols/rund)/2.0), int(breite*(nbands/rund)/2.0) );
	  centralw->bild1[i]->setMinimumSize ( int(breite*(ncols/rund)/2.0), int(breite*(nbands/rund)/2.0) );
	  centralw->bild2[i]->setMaximumSize ( int(breite*(nrows/rund)/2.0), int(breite*(nbands/rund)/2.0) );
	  centralw->bild2[i]->setMinimumSize ( int(breite*(nrows/rund)/2.0), int(breite*(nbands/rund)/2.0) );
	  centralw->bild3[i]->setMaximumSize ( int(breite*(ncols/rund)/2.0), int(breite*(nrows/rund)/2.0) );
	  centralw->bild3[i]->setMinimumSize ( int(breite*(ncols/rund)/2.0), int(breite*(nrows/rund)/2.0) );
	} else {
	  if (hoehe/fb <= breite/fb && fb<=1.0) {
	    if (pr->graph[0]) {
	      centralw->ogl[i]->setMaximumSize ( int(hoehe*(nrows/rund)/fbv), int(hoehe*(nrows/rund)/fbv) );
	      centralw->ogl[i]->setMinimumSize ( int(hoehe*(nrows/rund)/fbv), int(hoehe*(nrows/rund)/fbv) );
	    }
	    //fprintf(stderr,"LAYOUT: ein pic (beachte hoehe)\n");
	    centralw->bild1[i]->setMaximumSize ( int(hoehe*(ncols/rund)/fbv), int(hoehe*(nbands/rund)/fbv) );
	    centralw->bild1[i]->setMinimumSize ( int(hoehe*(ncols/rund)/fbv), int(hoehe*(nbands/rund)/fbv) );
	    centralw->bild2[i]->setMaximumSize ( int(hoehe*(nrows/rund)/fbv), int(hoehe*(nbands/rund)/fbv) );
	    centralw->bild2[i]->setMinimumSize ( int(hoehe*(nrows/rund)/fbv), int(hoehe*(nbands/rund)/fbv) );
	    centralw->bild3[i]->setMaximumSize ( int(hoehe*(ncols/rund)/fbv), int(hoehe*(nrows/rund)/fbv) );
	    centralw->bild3[i]->setMinimumSize ( int(hoehe*(ncols/rund)/fbv), int(hoehe*(nrows/rund)/fbv) );
	  } else {
	    if (pr->graph[0]) {
	      centralw->ogl[i]->setMaximumSize ( int(breite*(nrows/rund)/fbh), int(breite*(nrows/rund)/fbh) );
	      centralw->ogl[i]->setMinimumSize ( int(breite*(nrows/rund)/fbh), int(breite*(nrows/rund)/fbh) );
	    }
	    //fprintf(stderr,"LAYOUT: in reihe\n");
	    centralw->bild1[i]->setMaximumSize ( int(breite*(ncols/rund)/fbh), int(breite*(nbands/rund)/fbh) );
	    centralw->bild1[i]->setMinimumSize ( int(breite*(ncols/rund)/fbh), int(breite*(nbands/rund)/fbh) );
	    centralw->bild2[i]->setMaximumSize ( int(breite*(nrows/rund)/fbh), int(breite*(nbands/rund)/fbh) );
	    centralw->bild2[i]->setMinimumSize ( int(breite*(nrows/rund)/fbh), int(breite*(nbands/rund)/fbh) );
	    centralw->bild3[i]->setMaximumSize ( int(breite*(ncols/rund)/fbh), int(breite*(nrows/rund)/fbh) );
	    centralw->bild3[i]->setMinimumSize ( int(breite*(ncols/rund)/fbh), int(breite*(nrows/rund)/fbh) );
	  }
	} 
      } 
    } else {
      
    }
  }
  for (int i=0;i<pr->files;i++) {
    centralw->bild1[i]->recreate=1;
    centralw->bild2[i]->recreate=1;
    centralw->bild3[i]->recreate=1;
  }
}

// ===============================================================================

// ================================ RESIZE === ENDE ==============================

// ===============================================================================


void lView::setInterpolation(int i) {
  if (i==0) {
    pr->interpoltype=0;
    centralw->setInterpolNN();
  }
  if (i==1) {
    pr->interpoltype=1;
    centralw->setInterpolBilin();
  }
  if (i==2) {
    pr->interpoltype=2;
    centralw->setInterpolBicub();
  }
  if (i==3) {
    pr->interpoltype=3;
    centralw->setInterpolBicub6();
  }
  if (i==4) {
    pr->interpoltype=4;
    centralw->setInterpolBSpline();
  }
}

void lView::setVoxelBox() {
  if ( pr->pixelco==1 && pr->talairach==0 ) combobox2->setCurrentItem(0);
  if ( pr->pixelco==0 && pr->talairach==0 ) combobox2->setCurrentItem(1);
  if ( pr->sw2>0.5 && pr->pixelco==2 && pr->talairach==0 ) combobox2->setCurrentItem(2);
  if ( pr->talairachoff==0 && pr->talairach==1 ) combobox2->setCurrentItem(3);
}

void lView::setOpenvis(int x) {
  if (x==0) {
    combobox3->setCurrentItem(0);
    pr->openvis=0;
    for (int i=0;i<pr->files;i++) centralw->ogl[i]->crosses_onoff = 0;
  }
  if (x==1) {
    combobox3->setCurrentItem(1);
    pr->openvis=1;
    for (int i=0;i<pr->files;i++) centralw->ogl[i]->crosses_onoff = 1;
  }
  if (x==2) {
    combobox3->setCurrentItem(2);
    pr->openvis=2;
    for (int i=0;i<pr->files;i++) centralw->ogl[i]->crosses_onoff = 0;
  }
  switchCrosses();
}

void QComboBox::keyPressEvent ( QKeyEvent * e ) {
  e->ignore();
}

/* added by A. Hagert : synchronizes vgview-windows (rotation...) - IPC */
void lView::synchronize_vlviews() {
  QPixmap conn0Icon(connect0);
  QPixmap conn1Icon(connect1);
  if (toolbsyn->isOn()== TRUE) {
    pr->ipc=2;
    toolbsyn->setIconSet(conn1Icon);
    serverini=1;
    sendtoserver();
    serverini=0;
  } else {
    pr->ipc=1;
    toolbsyn->setIconSet(conn0Icon);
  }
}

/* added by A. Hagert : synchronizes vgview-windows (rotation...) - IPC 
   void lView::synchronize_rotation() {
   if (pr->synchronize == 0) pr->synchronize = 1;
   else pr->synchronize = 0;
   }
*/

/* added by A. Hagert */
void lView::print_color_min_max(float mi, float ma)
{
  echt->setText( tr("min: %1 max: %2").arg(mi).arg(ma) );   
}

/* added by A. Hagert : find local minimum */
void lView::findmaxZ_graph() {
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->findMaxZ();
    //centralw->ogl[i]->updateGL();
  }
}

/* added by A. Hagert : find local maximum */
void lView::findminZ_graph() {
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->findMinZ();
    //centralw->ogl[i]->updateGL();
  }
}

/* added by A. Hagert : single points can be shown as (3d) crosses ... */
void lView::switchCrosses() {
  for (int i=0;i<pr->files;i++) {
    /*
      if (centralw->ogl[i]->crosses_onoff == 0) centralw->ogl[i]->crosses_onoff = 1;
      else centralw->ogl[i]->crosses_onoff = 0;
    */
    centralw->ogl[i]->clean();
    centralw->ogl[i]->initializeGL();
    if (pr->fog == 0) centralw->ogl[i]->Disable_fog();
    else centralw->ogl[i]->Enable_fog();
    centralw->ogl[i]->updateGL();
  }
}

/* added by A. Hagert : repaint with changed colortable */
void lView::change_coltab() {
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->clean();
    centralw->ogl[i]->initializeGL();
    if (pr->fog == 0) centralw->ogl[i]->Disable_fog();
    else centralw->ogl[i]->Enable_fog();
    centralw->ogl[i]->updateGL();
  }
}

/* added by A. Hagert : turns on/off the fog */
void lView::switchFog() {
  if (pr->fog == 0) pr->fog = 1; 
  else pr->fog = 0; 
  for (int i=0;i<pr->files;i++) {
    if (pr->fog == 0) centralw->ogl[i]->Disable_fog();
    else centralw->ogl[i]->Enable_fog();
    centralw->ogl[i]->fog_onoff = pr->fog;
    centralw->ogl[i]->updateGL();
  }
}

/* added by A. Hagert : enables/disables the more exact clicking routine (slower) */
void lView::clicking() {
  for (int i=0;i<pr->files;i++) {
    if (pr->exact == 0) pr->exact = 1;
    else pr->exact = 0;
  }
}

/* added by A. Hagert : zoom in */
void lView::graph_zoom_in() {
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->zoom (1);
    centralw->ogl[i]->updateGL();
  }
}

/* added by A. Hagert : zoom out */
void lView::graph_zoom_out() {
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->zoom (-1);
    centralw->ogl[i]->updateGL();
  }
}

/* added by A. Hagert : set the zoom back to normal */
void lView::graph_zoom_norm() {
  for (int i=0;i<pr->files;i++) {
    centralw->ogl[i]->zoom (0);
    centralw->ogl[i]->updateGL();
  }
}

/* added by A. Hagert : switch between visualization with or without links */
void lView::switchLines() {
  for (int i=0;i<pr->files;i++) {
    if (centralw->ogl[i]->lines_onoff == 0) centralw->ogl[i]->lines_onoff = 1;
    else centralw->ogl[i]->lines_onoff = 0;
    centralw->ogl[i]->clean();           /* clean the memory */
    centralw->ogl[i]->initializeGL();    /* read again the whole graph */
    if (pr->fog == 0) centralw->ogl[i]->Disable_fog();
    else centralw->ogl[i]->Enable_fog();
    centralw->ogl[i]->updateGL();
  }
}

/* added by A. Hagert : turn on/off polygons (of present) */
void lView::switchPolygons() {
  for (int i=0;i<pr->files;i++) {
    if (centralw->ogl[i]->polygons_onoff == 0) centralw->ogl[i]->polygons_onoff = 1;
    else centralw->ogl[i]->polygons_onoff = 0;
    centralw->ogl[i]->clean();           /* clean the memory */
    centralw->ogl[i]->initializeGL();    /* read again the whole graph */
    if (pr->fog == 0) centralw->ogl[i]->Disable_fog();
    else centralw->ogl[i]->Enable_fog();
    centralw->ogl[i]->updateGL();
  }
}

/* added by A. Hagert : color on/off */
void lView::switchColor() {
  for (int i=0;i<pr->files;i++) {
    if (centralw->ogl[i]->color_onoff == 0) centralw->ogl[i]->color_onoff = 1;
    else centralw->ogl[i]->color_onoff = 0;
    centralw->ogl[i]->clean();           /* clean the memory */
    centralw->ogl[i]->initializeGL();    /* read again the whole graph */
    if (pr->fog == 0) centralw->ogl[i]->Disable_fog();
    else centralw->ogl[i]->Enable_fog();
    centralw->ogl[i]->updateGL();
  }
}

void lView::sendtoserver() {
  /* IPC by KM */
  QPixmap conn0Icon(connect0);
  int x,y,z;

  if (pr->ipc>1) {

    if (serverini==0) {
    
      if (pr->talairach==1) {
	double XXt = (double)pr->cursorp[0]; 
	double YYt = (double)pr->cursorp[1]; 
	double ZZt = (double)pr->cursorp[2]; 
	mytools.VPixel3Tal(XXt,YYt,ZZt, extent, ca, cp, pr->files, pr->pixelmult);
	x=(int)rint(XXt);
	y=(int)rint(YYt);
	z=(int)rint(ZZt);
      } else {
	x=(int)pr->cursorp[0];
	y=(int)pr->cursorp[1];
	z=(int)pr->cursorp[2];
      }
      
      if (pr->ogl) 
	sprintf (puffer, "%d %d %d %d %d %d %d %d %d %f %f \0", getpid(), (int)centralw->ogl[0]->xRot, (int)centralw->ogl[0]->yRot, (int)centralw->ogl[0]->zRot, (int)1, x, y, z, (int)pr->talairach, pr->tpos, pr->tneg);
      else 
	sprintf (puffer, "%d %d %d %d %d %d %d %d %d %f %f \0", getpid(), (int)0, (int)0, (int)0, (int)0, x, y, z, (int)pr->talairach, pr->tpos, pr->tneg);
      

    } else 
      sprintf (puffer, "%d %d %d %d %d %d %d %d %d %f %f \0", getpid(), (int)0, (int)0, (int)0, (int)0, (int)32000, (int)32000, (int)32000, (int)pr->talairach, pr->tpos, pr->tneg);
      

    // test if vlserv is available
    bool sw_vlserv=FALSE; char zeile[100], zeile2[100]; FILE *progpipe; char *zeile1;
    int vlservpid=0;
    progpipe = popen("whoami","r");
    if (fgets(zeile,100,progpipe) != NULL) zeile1 = strtok(zeile,"\n");
    pclose(progpipe);
    sprintf(zeile2,"ps -u %s | grep vlserv",zeile1);
    progpipe = popen(zeile2,"r");
    if (fgets(zeile,100,progpipe) != NULL) {
      zeile1 = strtok(strtok(zeile,"\n")," ");
      vlservpid=atoi(zeile1);
      while (zeile1) {
	zeile1 = strtok(NULL," ");
	if (zeile1) {
	  if (!strcmp(zeile1,"vlserv")) sw_vlserv=TRUE;
	}
      }
    }
    pclose(progpipe);
    if (sw_vlserv==FALSE) {
      pr->ipc=0;
      toolbsyn->setIconSet(conn0Icon);
      toolbsyn->setDisabled(TRUE);
      QMessageBox::warning( this, "Warning",
			    "vlserv: Connection lost!");
    } else {
      if (write(fd_fifo0,puffer,strlen(puffer))  == -1) {
	pr->ipc=0;
	toolbsyn->setIconSet(conn0Icon);
	toolbsyn->setDisabled(TRUE);
	QMessageBox::warning( this, "Warning",
			      "vlserv: Can not write into pipe!");
      }
    }
  }
}


void lView::syncronize(int i) {
  int x, y, z, type, a, b, c, modus;
  int checkpid=0;
  float tpos, tneg;

  if (pr->ipc>1) {
    if(read(fd_fifo, &buf, sizeof(buf)) == -1)
      fprintf(stderr, "Error! can't read from FIFO.......\n");
    else 
      sscanf (buf, "%d %d %d %d %d %d %d %d %d %f %f\n", &checkpid, &a, &b, &c, &modus, &x, &y, &z, &type, &tpos, &tneg);
    
    if (checkpid!=16) {
      unlink(fifoname);
      pr->ipc=0;
    } else {
      if (type==1 && pr->atlas==0) return;
      if (type==1) mytools.VTal3Pixel(x,y,z, pr->voxel, extent, ca, pr->files, pr->pixelmult );
      if (y < VImageNRows(src[0]) && x < VImageNColumns(src[0]) && z< VImageNFrames(src[0])) {
	pr->cursorp[0]=(float)x;
	pr->cursorp[1]=(float)y;
	pr->cursorp[2]=(float)z;
	emit viewChanged();  
	centralw->bild1[0]->talCross (int(pr->cursorp[0]), int(pr->cursorp[1]), int(pr->cursorp[2]));

	if (fnc[0] && pr->cursorp[2]<VImageNFrames(fnc[0]) && pr->cursorp[1]<VImageNRows(fnc[0]) && pr->cursorp[0]<VImageNColumns(fnc[0]))
	  emit z2Wert(VPixel(fnc[pr->active],(int)rint(pr->cursorp[2]),(int)rint(pr->cursorp[1]),(int)rint(pr->cursorp[0]),VFloat)); 
	else if (pr->cursorp[2]<VImageNFrames(src[0]) && pr->cursorp[1]<VImageNRows(src[0]) && pr->cursorp[0]<VImageNColumns(src[0]))
	  emit z2Wert(VGetPixel(src[pr->active],(int)rint(pr->cursorp[2]),(int)rint(pr->cursorp[1]),(int)rint(pr->cursorp[0])));

	if (pr->ogl) {
	  if (pr->synchronize && modus>0) {
	    centralw->ogl[0]->xRot=(float)a;
	    centralw->ogl[0]->yRot=(float)b;
	    centralw->ogl[0]->zRot=(float)c;
	    centralw->SetXYZScrollbars( a, b, c);
	  }
	  centralw->ogl[0]->move_cross ();
	  centralw->ogl[0]->updateGL ();
	}
      }
    }
  }
}

