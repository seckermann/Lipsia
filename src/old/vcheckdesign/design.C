/****************************************************************
 *
 * vcheckdesign:
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Authors Heiko Mentzel, Karsten Mueller, 1999, <lipsia@cbs.mpg.de>
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
 * $Id: design.C 2999 2007-11-30 10:55:56Z karstenm $
 *
 *****************************************************************/

/* From the std. library: */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* my own inludes */
#include "design.h"
#include <lip.xpm>

#define ABS(x) ((x) < 0 ? -(x) : (x))

extern "C" {
  extern void getLipsiaVersion(char*, size_t);
}
double version=2;
extern QString origconfpath, idsprog, refpath;
int condanz=1, condition=0;
FILE *in_file;

static VBoolean in_found;
static VString design = NULL;
static VBoolean verbose = FALSE;
static VOptionDescRec  options[] = {
  { "verbose", VBooleanRepn, 1, &verbose, VOptionalOpt, 0, "Show verbose messages" },
};

/* Constructs a Widget. */
Design::Design( QWidget *parent, const char *name ) {

  appFont = QApplication::font();  

  VAttrList  image_list=NULL;
  VImage mask=NULL, src=NULL, src1=NULL, orth=NULL, gamma=NULL;
  VAttrListPosn posn;
  double sum=0, w=0;
  VFloat repetition_time;
  VString session_lengths=NULL, session_covariates=NULL; 
  VString modality=NULL, name1=NULL;
  char *token=NULL;
  int sessanz = 1;
  int *sesslength=NULL, *sesscov=NULL;

  /* Read the input file */
  image_list = VReadFile (in_file, NULL);
  if (!image_list) exit(1); 

  /* Load design image */
  for (VFirstAttr (image_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, &mask);
    if (VPixelRepn(mask) == VFloatRepn) {

      if (VGetAttr (VImageAttrList (mask), "modality", NULL,
		    VStringRepn, (VPointer) &modality) == VAttrFound) {
	if (strcmp("X",modality)==0) src = VTransposeImage(mask,NULL,VAllBands);
      }  

      if (strcmp("plot_gamma",VGetAttrName(&posn))==0)
	gamma = VCopyImage(mask,NULL,VAllBands);

    }     // if object is VFloat
  }       // loop through all objects 
  
  
  /* Error handling */
  if (src==NULL) VError("Can not find design structure in input file");
  if (verbose) {
    fprintf(stderr,"Timesteps: %d, Cond: %d\n",VImageNColumns(src),VImageNRows(src));
    if (gamma== NULL) fprintf(stderr,"gamma function NOT found in input file.\n");
  }
  
  /*repetition time */
  if (VGetAttr (VImageAttrList (src), "repetition_time", NULL,
		VFloatRepn, (VPointer) &repetition_time) != VAttrFound) 
    VError(" attribute 'repetition_time' missing in design.");
  if (verbose) fprintf(stderr,"repetition time: %f\n",repetition_time);

  /* number of sessions */
  if (VGetAttr (VImageAttrList (src), "nsessions", NULL,
		VLongRepn, (VPointer) &sessanz) != VAttrFound) sessanz=1;
  if (verbose) fprintf(stderr,"number of sessions: %d\n",sessanz);

  /* initialization of session lengths */
  sesslength    = (int *)malloc(sizeof(int)*sessanz);
  sesscov       = (int *)malloc(sizeof(int)*sessanz);
  sesslength[0] = VImageNColumns(src);
  sesscov[0]    = VImageNRows(src);

  /* session lengths and number of covariates */
  if (VGetAttr (VImageAttrList (src), "session_lengths", NULL,
		VStringRepn, (VPointer) &session_lengths) != VAttrFound) 
    sesslength[0]=VImageNColumns(src);
  else {
    token = strtok(session_lengths," ");
    for (int i=0; i<sessanz; i++) {
      sesslength[i]= (int)atoi(token);
      if (verbose) fprintf(stderr,"length of session %d: %d\n",i,sesslength[i]);
      if (token) token = strtok(NULL," ");
    }
  }
  if (VGetAttr (VImageAttrList (src), "session_covariates", NULL,
		VStringRepn, (VPointer) &session_covariates) != VAttrFound) 
    sesscov[0]=VImageNRows(src);
  else {
    token = strtok(session_covariates," ");
    for (int i=0; i<sessanz; i++) {
      sesscov[i]= (int)atoi(token);
      if (verbose) fprintf(stderr,"number of covariates in session %d: %d\n",i,sesscov[i]);
      if (token) token = strtok(NULL," ");
    }
  }

  /* normalization of src */
  src1 = VCopyImage(src,NULL,VAllBands);
  for (int i=0; i<VImageNRows(src); i++) {
    sum=0;
    for (int j=0; j<VImageNColumns(src); j++) 
      sum += (double)(VPixel(src1,0,i,j,VFloat) * VPixel(src1,0,i,j,VFloat));
    for (int j=0; j<VImageNColumns(src); j++) 
      VPixel(src1,0,i,j,VFloat) /= (VFloat)sqrt(sum);
  }
  
  /* ortho-matrix */
  orth  = VCreateImage(1,VImageNRows(src),VImageNColumns(src),VFloatRepn);
  for (int i=0; i<VImageNRows(src); i++) {
    for (int j=0; j<VImageNRows(src); j++) {
      sum=0;
      for (int k=0; k<VImageNColumns(src); k++) 
	sum +=  (double)(VPixel(src1,0,i,k,VFloat) * VPixel(src1,0,j,k,VFloat));
      if (ABS(sum)<0.000001) sum=0;
      VPixel(orth,0,i,j,VFloat)=(VFloat)(1.0-ABS(sum));
      VPixel(orth,0,j,i,VFloat)=(VFloat)(1.0-ABS(sum));
    }
  }
	
  //even number?
  if ((double)sesslength[0]/2.0 != rint((double)sesslength[0]/2.0)) sesslength[0]=sesslength[0]-1;

  // OPEN OF THE MAIN WIDGET
  condanz = VImageNRows(src);
  center = new designCW( this, "center", version, (double)repetition_time/1000.0, sessanz, sesslength, sesscov, src, src1, orth, gamma);
 
  setCentralWidget( center );
  
  QPopupMenu* popup = new QPopupMenu;
  CHECK_PTR( popup );
  popup->insertItem( "&Quit", qApp, SLOT(quit()), CTRL+Key_Q );
  
  QPopupMenu* select = new QPopupMenu;
  CHECK_PTR( select );
  select->setCheckable(TRUE);
  for ( int i = 0; i<sessanz; i++) {
    select->insertItem(tr("Session %1").arg(i+1), i);
  }
  connect(select, SIGNAL(activated(int)), this, SLOT(setSession(int)));

  condmenu = new QPopupMenu;
  CHECK_PTR( condmenu );
  condmenu->setCheckable(TRUE);
  for ( int i = 0; i<condanz; i++) {
    condmenu->insertItem(tr("Column %1").arg(i), i+200);
  }
  connect(condmenu, SIGNAL(activated(int)), this, SLOT(setCondition(int)));

  QToolBar *Stools = new QToolBar( "statusbar", this, QMainWindow::Bottom );
  Stools->setHorizontalStretchable ( 1 );
  
  menubar = new QMenuBar( this );
  menubar->insertItem( "&Quit", popup );
  menubar->insertItem( "&Session", select );
  //menubar->insertItem( "&Column", condmenu, 55 );

  QPixmap  lipsia( lip );
  QIconSet ( lipsia, QIconSet::Automatic );

  statusbar = new QStatusBar( Stools, "Graph" );

  wert = new QLCDNumber ( 10, statusbar, "wert" );
  wert->setSegmentStyle ( QLCDNumber::Filled );
  QWhatsThis::add( wert, "Score");
  statusbar->addWidget( wert, 2 );
  wert->display( "" );

  text = new QLabel( statusbar, "message", 0 );
  statusbar->addWidget( text, 6 );
  text->setFont( QFont( "lucida", 12, QFont::Normal, TRUE ) );
  text->setText( tr("vcheckdesign v%1: Session 1").arg(version) );

  QLabel *logo;
  logo = new QLabel(statusbar, "message", 0);
  // logo->setFrameStyle( QFrame::Panel|QFrame::Raised );
  logo->setPixmap(lipsia);
  QWhatsThis::add( logo, "This is the logo only.");
  statusbar->addWidget( logo, 1, FALSE );

  QObject::connect(center->bild1, SIGNAL(wertView(double)), this, SLOT(wertView( double )));
  QObject::connect(center->bild3, SIGNAL(wertView(double)), this, SLOT(wertView( double )));
  QObject::connect(center->bild6, SIGNAL(wertView(double)), this, SLOT(wertView( double )));
  QObject::connect(center->bild3, SIGNAL(sendMaxCut(double)), this, SLOT(writeStatus( double )));
}
    
void Design::setSession(int session)
{
  
    center->bild3->getSessionData(session);
    center->bild4->getSessionData(session);
    center->bild5->getSessionData(session);
    center->bild6->getSessionData(session);
    text->setText( tr("vcheckdesign v%1: Session %2").arg(version).arg(session+1) );

    /*
    condition=0;
    condmenu = new QPopupMenu;
    CHECK_PTR( condmenu );
    condmenu->setCheckable(TRUE);
    for ( int i = 0; i<condanz; i++) {
      condmenu->insertItem(tr("Column %1").arg(i), i+200);
    }
    connect(condmenu, SIGNAL(activated(int)), this, SLOT(setCondition(int)));

    menubar->removeItem( 55 );
    menubar->insertItem( "&Column", condmenu, 55 );
    */
}

void Design::writeStatus(double maxcut)
{
  text->setText( tr("Cutoff: %3 s").arg(maxcut) );

}

void Design::setCondition(int id)
{

  /*
    condition=id-200;
    center->bild3->getSessionData();
    center->bild4->getSessionData();
    center->bild5->getSessionData();
    center->bild6->getSessionData();
  */
}

/* close all */
Design::~Design() {

}

/* Main */
int main (int argc,char *argv[]) {

  /* Parse command line arguments and identify files: */
  VParseFilterCmd (VNumber (options), options, argc, argv,&in_file,NULL /* &out_file */);
  
  char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vcheckdesign V%s", ver);
  
  fprintf (stderr, "%s\n", prg_name);

  /* opens the window */
  QApplication app( argc, argv );
  Design fenster;
  fenster.resize( 1000, 600 );
  app.setMainWidget( &fenster );
  fenster.show();

  return app.exec();
  /* end of the program */
}


void Design::wertView( double z ) {
  QString text;
  text=tr("%1").arg(z);
  wert->display( text );
}

