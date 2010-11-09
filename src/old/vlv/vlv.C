/****************************************************************
 *
 * vlv:
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Heiko Mentzel, 2000, <lipsia@cbs.mpg.de>
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
 * $Id: vlv.C 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


/* From the std. library: */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Vista - Libraries */
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/headerinfo.h>

/* QT - Librarys */
#include <qapplication.h>
#include "prefs.h"
#include "lView.h"

extern "C" {
  extern char * getLipsiaVersion();
}
extern prefs * prefsini();

/* Parameter */
VArgVector in_files, zmap_files, graph_files;  
prefs *pr;
VBoolean in_found, zmap_found, graph_found;
VFloat th11=0;
int verbose=0;
VShort background[2]={0,0};
VString rawfile=NULL;
VString desfile=NULL;
VString betafile=NULL;
VString graphfile=NULL;
static VOptionDescRec  options[] = {
  { "in", VStringRepn, 0, &in_files, &in_found, NULL, "Input file(s) containing anatomical data" },
  { "zmap", VStringRepn, 0, &zmap_files, &zmap_found, NULL, "Zmap(s), tmap(s), Fmap(s), or contrast image(s)" },
  { "thresh", VFloatRepn, 1, &th11, VOptionalOpt, NULL, "Theshold of Zmap" },
  { "background", VShortRepn, 2, &background, VOptionalOpt, NULL, "Interval for background"},
  { "verbose",VShortRepn,1,(VPointer) &verbose, VOptionalOpt,NULL,"Verbose messages" }
};


/* the main routine */
int main (int argc,char *argv[]) {
  QApplication::setColorSpec( QApplication::ManyColor );                                                                                  

  /* initialize preferences variables */
  //pr= new prefs();
  pr = prefsini();

  /* write out the revision string */
  char prg_name[50];	
  sprintf(prg_name,"vlv V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg_name);
  
  /* Parse command line arguments and identify files: */
  if (! VParseCommand (VNumber (options), options, & argc, argv) ||
      ! VIdentifyFiles (VNumber (options), options, "in", & argc, argv, 0))
    goto Usage;
  if (argc > 1) {
    VReportBadArgs (argc, argv);
  Usage:	VReportUsage (argv[0], VNumber (options), options, NULL);
    exit (EXIT_FAILURE);
  }

  /* set verbose option to prefs */
  pr->prg_name=prg_name;
  pr->verbose=verbose;
  pr->files=0;
  pr->infilenum=in_files.number;
  pr->zmapfilenum=zmap_files.number;
  pr->thresh=th11;
  pr->background0=background[0];
  pr->background1=background[1];

  if (pr->infilenum==0) VError("No infile specified");
  

  if (pr->infilenum > pr->zmapfilenum)
    pr->files=pr->infilenum;
  else 
    pr->files=pr->zmapfilenum; 

  /* initialize open the main window */
  QApplication a( argc, argv );
  lView ww;
  
  a.setMainWidget( &ww );
  ww.show();
  

  return a.exec();
  /* end of the program */
}

