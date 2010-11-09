/****************************************************************
 *
 * vwhiteglm:
 * Author: G. Lohmann
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
 * $Id: vzmax.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <via.h>

extern VImage VZMax(VImage,VImage,VDouble,VDouble,VDouble,VShort,VFloat,VShort,VString);
extern char * getLipsiaVersion();

VDictEntry TALDict[] = {
  { "voxel", 0 },
  { "mm", 1 },
  { "talairach", 2 },
  { NULL }
};



int 
main (int argc,char *argv[])
{  
  static VDouble pos      = 3.0;
  static VDouble neg      = -1000;
  static VDouble threshold = 0;
  static VShort  minsize  = 81;
  static VFloat  radius   = 12;
  static VShort  type    =  0;
  static VString filename = "";

  static VOptionDescRec  options[] = {
    {"pos",VDoubleRepn,1,(VPointer) &pos,VOptionalOpt,NULL,"Positive threshold on zmap"},
    {"neg",VDoubleRepn,1,(VPointer) &neg,VOptionalOpt,NULL,"Negative threshold on zmap"},
    {"threshold",VDoubleRepn,1,(VPointer) &threshold,VOptionalOpt,NULL,
     "Height threshold for local maxima"},
    {"system",VShortRepn,1,(VPointer) &type,VRequiredOpt,TALDict,
     "Coordinate system to be used in output (voxel,mm,talairach)"},
    {"radius",VFloatRepn,1,(VPointer) &radius,VOptionalOpt,NULL,
     "Radius (in mm) that defines local maxima"},
    {"minsize",VShortRepn,1,(VPointer) &minsize,VOptionalOpt,NULL,"Minimal size per area (in mm^3)"},
    { "report", VStringRepn, 1, & filename, VOptionalOpt, NULL,
      "File containing output report" },
  };
  FILE *in_file,*out_file;
  VAttrList list=NULL;
  VAttrListPosn posn;
  VImage src=NULL,dest=NULL;

  char prg[50];	
  sprintf(prg,"vzmax V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg);
  

  VParseFilterCmd (VNumber (options),options,argc,argv,&in_file,&out_file);
  if (neg > 0) VError(" parameter 'neg' must be negative");
  if (pos < 0) VError(" parameter 'pos' must be positive");


  if (! (list = VReadFile (in_file, NULL))) exit (1);
  fclose(in_file);

  for (VFirstAttr (list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL,VImageRepn, & src);
    if (VPixelRepn(src) != VFloatRepn) continue;

    dest = VZMax(src,NULL,pos,neg,threshold,minsize,radius,type,filename);
    VSetAttrValue (& posn, NULL,VImageRepn,dest);
  }
  if (src == NULL) VError(" no input image found");


  VHistory(VNumber(options),options,prg,&list,&list);
  if (! VWriteFile (out_file, list)) exit (1);
  fprintf (stderr, "%s: done.\n", argv[0]);
  return 0;
}
