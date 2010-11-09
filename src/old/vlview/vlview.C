/****************************************************************
 *
 * vlview:
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
 * $Id: vlview.C 3540 2009-04-16 14:59:58Z proeger $
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
#include "lLoad.h"

extern "C" {
  extern char * getLipsiaVersion();
}
extern prefs * prefsini();

/* cannot be named 'myLoad', since such symbol is already defined in libvlh */
lLoad myload_;

/* Parameter */
VArgVector in_files, zmap_files, graph_files;  
prefs *pr;
VBoolean in_found, zmap_found, graph_found;
VFloat th11=0;
int verbose=0;
float mincol = 123456, maxcol = 123456;
VShort background[2]={0,0};
VString rawfile=NULL;
VString desfile=NULL;
VString betafile=NULL;
VString graphfile=NULL;
VString colortable_filename=NULL;
static VOptionDescRec  options[] = {
  { "in", VStringRepn, 0, &in_files, &in_found, NULL, "Input file (containing anatomical data)" },
  { "zmap", VStringRepn, 0, &zmap_files, &zmap_found, NULL, "Map file (zmap, tmap, Fmap, or conimg)" },
  { "thresh", VFloatRepn, 1, &th11, VOptionalOpt, NULL, "Theshold of map" },
  { "raw", VStringRepn, 1, &rawfile, VOptionalOpt, 0, "Raw data file" },
  { "beta", VStringRepn, 1, &betafile, VOptionalOpt, 0, "Beta file (output of vcolorglm)" },
  { "des", VStringRepn, 1, &desfile, VOptionalOpt, 0, "Design file" },
  { "graph",  VStringRepn, 1, &graphfile, VOptionalOpt, 0, "Graph file" },
  { "color", VStringRepn, 1, &colortable_filename, VOptionalOpt, 0, "ASCII-File with colortable for Graph" },
  { "min",VFloatRepn,1,&mincol, VOptionalOpt,0,"fixed minimum for graph colors" },
  { "max",VFloatRepn,1,&maxcol, VOptionalOpt,0,"fixed maximum for graph colors" },
  { "background", VShortRepn, 2, &background, VOptionalOpt, NULL, "Interval for background"},
  { "verbose",VShortRepn,1,(VPointer) &verbose, VOptionalOpt,NULL,"Verbose messages" },
};

/* the main routine */
int main (int argc,char *argv[]) {
  QApplication::setColorSpec( QApplication::ManyColor );                                                                                  

  /* initialize preferences variables */
  pr = prefsini();

  /* write out the revision string */
  char prg_name[50];	
  sprintf(prg_name,"vlview V%s", getLipsiaVersion());
  
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

  /* A. Hagert: exceptions */
  if (mincol == 123456 && maxcol == 123456) { pr->col_min = 0; pr->col_max = 0; }
  else if (mincol != 123456 && maxcol == 123456) 
    { printf("ERROR: maximum not given -> ignoring given minimum.\n"); pr->col_min = 0; pr->col_max = 0; }
  else if (mincol == 123456 && maxcol != 123456)
    { printf("ERROR: minimum not given -> ignoring given maximum.\n"); pr->col_min = 0; pr->col_max = 0; }
  else if (mincol<=maxcol) { pr->col_min=mincol; pr->col_max=maxcol; }
  else { printf("ERROR: given minimum is higher than maximum -> ignoring both.\n"); 
  pr->col_min = 0; pr->col_max = 0; 
  }
  
  /* set verbose option to prefs */
  pr->prg_name=prg_name;
  pr->verbose=verbose;
  pr->raw=rawfile;
  pr->des=desfile;
  pr->beta=betafile;
  pr->colortable=colortable_filename;
  pr->graph[0]=graphfile;
  pr->files=0;
  pr->infilenum=in_files.number;
  pr->zmapfilenum=zmap_files.number;
  pr->thresh=th11;
  pr->background0=background[0];
  pr->background1=background[1];

  if (pr->infilenum > pr->zmapfilenum)
    pr->files=pr->infilenum;
  else 
    pr->files=pr->zmapfilenum;

  if (pr->infilenum==0) 
    VError("No infile specified");

  if (pr->files>1) 
    VError("Use 'vlv' for visualization of data sets > 1");

  /* get the number of graphs (pr->g_number) */
  myload_.testFiles();

  /* initialize open the main window */
  QApplication a( argc, argv );
  lView ww;
  
  a.setMainWidget( &ww );
  ww.show();
  

  return a.exec();
  /* end of the program */
}


