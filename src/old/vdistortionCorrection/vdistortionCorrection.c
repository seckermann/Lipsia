/****************************************************************
 *
 * Program: vdistortionCorrection
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
 * $Id: vdistortionCorrection.c 3181 2008-04-01 15:19:44Z karstenm $
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


extern void VDistortionCorrection(VAttrList,VAttrList,VDouble,VDouble);
extern char * getLipsiaVersion();

int
main(int argc, char *argv[])
{
  static VDouble sigma = 0.7;
  static VDouble edist = 3;
  static VString filename = "";
  static VOptionDescRec  options[] = {
    {"sigma", VDoubleRepn,1,(VPointer) &sigma,VOptionalOpt,NULL,"sigma for Gauss filter"},
    {"edist", VDoubleRepn,1,(VPointer) &edist,VOptionalOpt,NULL,"edist"},
    {"shift", VStringRepn,1,(VPointer) &filename,VRequiredOpt,NULL,"Shift image"}
  };
  FILE *in_file=NULL,*out_file=NULL,*fp=NULL;  
  VAttrList list=NULL,list_shift=NULL;
  char prg[50];	
  sprintf(prg,"vdistortionCorrection V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg);

  VParseFilterCmd(VNumber(options),options,argc,argv,&in_file,&out_file);

  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);


  fp = VOpenInputFile (filename, TRUE);
  list_shift = VReadFile (fp, NULL);
  if (! list_shift)  VError("Error reading shift image");
  fclose(fp);


  VDistortionCorrection(list,list_shift,sigma,edist);


  /*
  ** output 
  */
  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  exit(0);
}
