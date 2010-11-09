/****************************************************************
 *
 * vwhiteglm:
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
 * $Id: vwhiteglm.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/headerinfo.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "BlockIO.h"


VDictEntry TYPDict[] = {
  { "conimg", 0 },
  { "tmap",   1 },
  { "zmap",   2 },
  { "Fmap",   3 },
  { NULL }
};


extern VAttrList* VWhiteRegression(ListInfo *,int,VImage,VShort,VShort,VShort,VArgVector,VString);
extern char * getLipsiaVersion();

int main (int argc, char *argv[])
{
  static VArgVector contrast;
  static VArgVector in_files;
  static VString out_filename;
  static VBoolean in_found, out_found;
  static VString filename=NULL;
  static VString confile=NULL;
  static VShort numcon = 1;
  static VShort numlags = 1;
  static VShort type = 0;
  static VOptionDescRec options[] = {
    {"in", VStringRepn, 0, & in_files, & in_found, NULL,"Input files" },
    {"out", VStringRepn, 1, & out_filename, & out_found, NULL,"Output file" },
    {"design",VStringRepn,1,(VPointer) &filename,VRequiredOpt,NULL,"Design file"},
    {"type",VShortRepn,1,(VPointer) &type,VOptionalOpt,TYPDict,"Type of output"},
    {"numcon",VShortRepn,1,(VPointer) &numcon,VOptionalOpt, NULL,"Number of contrasts"},
    {"contrast",VFloatRepn,0,(VPointer) &contrast,VOptionalOpt,NULL,"Contrast vector"},
    {"confile",VStringRepn,1,(VPointer) &confile, VOptionalOpt,NULL,"Contrast file"},
    {"order", VShortRepn, 1, &numlags, VOptionalOpt, NULL,"Order of AR model" },
  };
  VStringConst in_filename;
  VString ifilename;
  FILE *fp=NULL, *fptr=NULL, *f=NULL;
  VAttrList list=NULL, list1=NULL;
  VAttrList *out_list=NULL, history_list=NULL;
  VAttrListPosn posn;
  VImage design=NULL;
  VString str=NULL, str1=NULL;
  ListInfo *linfo;
  int i=0, n=0, numsub=0;

  char prg[50];	
  sprintf(prg,"vwhiteglm V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg);
  
  /* Parse command line arguments and identify files: */
  if (! VParseCommand (VNumber (options), options, & argc, argv) ||
      ! VIdentifyFiles (VNumber (options), options, "in", & argc, argv, 0) ||
      ! VIdentifyFiles (VNumber (options), options, "out", & argc, argv, -1))
    goto Usage;
  if (argc > 1) {
    VReportBadArgs (argc, argv);
  Usage:        VReportUsage (argv[0], VNumber (options), options, NULL);
  exit (EXIT_FAILURE);
  }

  /* Check plausibility */
  if (numlags < 1 || numlags > 8) VError("Choose numlags between 1 and 8");
  if (numcon < 1 || numcon > NCON) VError("Specify correct number of contrasts");
  if (confile==NULL && contrast.number==0) VError("Specify contrast vector or contrast file");
  if (confile!=NULL && contrast.number>0) VError("Specify either contrast vector or file");
  str = VMalloc(100); str1 = VMalloc(100);
  if (numcon == 1) 
    fptr = VOpenOutputFile (out_filename, TRUE);
  else {
    sprintf(str,"1.v"); strcpy(str1,(const char *)out_filename);
    fptr = VOpenOutputFile (strcat((char *)str1,(const char *)str), TRUE);
  }
  if (!fptr) VError("Error writing output file");
  else fclose(fptr);


  /* read design matrix */
  
  fp = VOpenInputFile (filename, TRUE);
  list1 = VReadFile (fp, NULL);
  if (! list1) VError("Error reading design file");
  fclose(fp);
  n=0;
  for (VFirstAttr (list1, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if (VGetAttrRepn (& posn) != VImageRepn) continue;
    VGetAttrValue (& posn, NULL, VImageRepn, & design);
    if (VPixelRepn(design) != VFloatRepn) continue;
    n++;
    break;
  }
  if (n==0) VError(" design matrix not found ");


  /* Read each input file */
  numsub = in_files.number;
  /* ListInfo array with ListInfo for every subimage */
  linfo = (ListInfo *) VMalloc(sizeof(ListInfo) * numsub);
  for (i=0; i<numsub; i++) {
    in_filename = ((VStringConst *) in_files.vector)[i];
    ifilename = VNewString(in_filename);
    fprintf(stderr," file:  %s\n",ifilename);
    list = GetListInfo(ifilename,&linfo[i]);
    /* Create history */
    if (i==0) {
      history_list = VReadHistory(&list);
      if (history_list==NULL) history_list=VCreateAttrList();
      VPrependHistory(VNumber(options),options,prg,&history_list);
    }
  }

  /* GLM */
  out_list = VWhiteRegression(linfo,numsub,design,numlags,type,numcon,contrast,confile);


  /* Output: */
  for (i=0; i<numcon; i++) {
    VPrependAttr(out_list[i],"history",NULL,VAttrListRepn,history_list);
    if (numcon == 1) 
      f = VOpenOutputFile (out_filename, TRUE);
    else {
      sprintf(str,"%d.v",i+1); strcpy(str1,(const char *)out_filename);
      f = VOpenOutputFile (strcat((char *)str1,(const char *)str), TRUE);
    }
    if (!f) VError(" error opening out file %s",out_filename);
    if (! VWriteFile (f, out_list[i])) exit (1);
  }


  printf ("%s: done.\n", argv[0]);
  return 0;
}


