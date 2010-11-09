/****************************************************************
 *
 * Program: vmovcorrection2d
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2007, <lipsia@cbs.mpg.de>
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
 * $Id: vmovcorrection2d.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

extern VImage VMotionCorrection2d(VAttrList,VShort,VShort,VShort);
extern void VFreqFilter(VAttrList,VFloat,VFloat,VBoolean,VFloat);
extern void VApplyMotionCorrection2d(VAttrList,VImage,VString);
extern char * getLipsiaVersion();

int
main(int argc, char *argv[])
{
  static VShort  minval  = 0;
  static VShort  i0      = 50;
  static VShort  maxiter = 100;
  static VString filename = "";
  static VOptionDescRec  options[] = {
    {"tref",VShortRepn,1,(VPointer) &i0,VOptionalOpt,NULL,"reference time step"},
    {"report",VStringRepn,1,(VPointer) &filename,VOptionalOpt,NULL,"report file"},
    {"iterations",VShortRepn,1,(VPointer) &maxiter,VOptionalOpt,NULL,"Max number of iterations"},
    {"minval",VShortRepn,1,(VPointer) &minval,VOptionalOpt,NULL,"Signal threshold"}
  };
  FILE *in_file=NULL,*out_file=NULL;
  VImage motion=NULL;
  VAttrList list=NULL;
  VFloat low,high;
  char prg[50];	
  sprintf(prg,"vmovcorrection2d V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg);


  /* parse command line */
  VParseFilterCmd(VNumber(options),options,argc,argv,&in_file,&out_file);


  /* read data */
  if (! (list = VReadFile (in_file, NULL))) exit (1);


  /* remove baseline drift */
  high = 60;
  low  = 0;
  VFreqFilter(list,high,low,FALSE,0.8f);


  /* estimate motion parameters using smoothed data */
  motion = VMotionCorrection2d(list,minval,i0,maxiter);


  /* re-read original data from file */
  VDestroyAttrList(list);
  list = NULL;
  rewind(in_file);
  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);
  

  /* apply motion correction */
  VApplyMotionCorrection2d(list,motion,filename);


  /* write output */
 ende:
  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  exit(0);
}
