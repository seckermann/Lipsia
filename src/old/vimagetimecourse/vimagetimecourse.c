/****************************************************************
 *
 * vimagetimecourse:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Jane Neumann, 2004, <lipsia@cns.mpg.de>
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
 * $Id: vimagetimecourse.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>
#include <via/via.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define ABS(x) ((x) > 0 ? (x) : -(x))
#define NTRIALS 1024
#define MAXSLICE 100
#define LEN 10000

extern char * getLipsiaVersion();

int
test_ascii(int val)
{
  if (val >= 'a' && val <= 'z') return 1;
  if (val >= 'A' && val <= 'Z') return 1;
  if (val >= '0' && val <= '9') return 1;
  if (val ==  ' ') return 1;
  if (val == '\0') return 1;
  if (val == '\n') return 1;
  if (val == '\r') return 1;
  if (val == '\t') return 1;
  if (val == '\v') return 1;
  return 0;
}



int main (int argc,char *argv[]) 
{
  /* Command line options: */
  static VShort cond;
  static VShort len=15;
  static VFloat reso=0.2;
  static VFloat tr=0;
  static VFloat minval=0;
  static VString desfile="";
  static VString reportfile="report.txt";

  static VOptionDescRec options[] = {
    {"length",VShortRepn,1,(VPointer) &len,
     VOptionalOpt,NULL,"Length of average time course (in sec)"},
    {"cond",VShortRepn,1,(VPointer) &cond,
     VRequiredOpt,NULL,"Condition"},
    {"reso",VFloatRepn,1,(VPointer) &reso,
     VOptionalOpt,NULL,"Temporal resolution (in sec)"},
    {"tr",VFloatRepn,1,(VPointer) &tr,
     VOptionalOpt,NULL,"TR (in sec)"},
    {"minval",VFloatRepn,1,(VPointer) &minval,
     VOptionalOpt,NULL,"Signal threshold"},
    {"des",VStringRepn,1,(VPointer) &desfile,
     VRequiredOpt,NULL,"Design file"},
    {"report",VStringRepn,1,(VPointer) &reportfile,
     VOptionalOpt,NULL,"Report file"} };
  

  FILE *in_file, *out_file, *des_file, *rep_file; 
  VAttrList in_list, out_list;
  VAttrListPosn posn;
  VImage src=NULL, out[MAXSLICE]={NULL};
  VString str;
  int i=0,j=0,k=0;
  int rr=0,cc=0, bb=0, nrows=0, ncols=0, slice=0, nslice=0, fullslice=0; 
  int ntrials=0, ntimesteps=0; 
  int k1,k2;

  /* for determining minval */
  double mean, grandmean, rowmean, colmean, imagemean;
  VFloat temp;

  char line[4096],*token;
  size_t n=4096;
  VBoolean old=FALSE;
  double x[NTRIALS]; 
  float onset,duration,height;
  int id=0;

  double s;  

  double sum1,nx,ave;
  double *mean_course=NULL;

  double norm,u,u1,u2,w1,w2;
  
  char prg_name[50];	
  sprintf(prg_name,"vimagetimecourse V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg_name);

  /* Parse command line arguments and identify files: */
  VParseFilterCmd(VNumber (options), options, argc, argv, &in_file, &out_file);

  if (reso > 100) 
    VError(" reso parameter must be in seconds - not milliseconds");


  /* Read the input file, return as attribute list */
  in_list = VReadFile (in_file, NULL);
  if (!in_list) exit(1);
  fclose(in_file);

  /* Create attribute list for output image and r-value image */
  out_list = VCreateAttrList();

  /* Make History */
  VHistory(VNumber(options),options,prg_name,&in_list,&out_list);

  /* write reportfile */
  rep_file = NULL;
  if (strlen(reportfile) > 2) {
    rep_file = fopen(reportfile,"w");
    if (rep_file == NULL) VError("Error opening report file %s",reportfile);

    fprintf(rep_file,"Design file: %s\n\n",desfile);
    fprintf(rep_file,"Length of average time course (len):    %d\n",len);
    fprintf(rep_file,"Temporal resolution (reso):             %.2f\n",reso); 
  }
  else
    VError("Report filename not valid");


  if (minval==0) fprintf(stderr,"Determining minval...\n");
  nslice    = 0;
  fullslice = 0; 
  grandmean = 0;
  for (VFirstAttr (in_list, & posn); VAttrExists(& posn); VNextAttr(& posn)) {    
     if (VGetAttrRepn (& posn) != VImageRepn) continue;
     VGetAttrValue (& posn, NULL, VImageRepn, & src);

     /* copy anatomy to output files */
     if (VPixelRepn(src)==VUByteRepn) {
       VAppendAttr(out_list,"image",NULL,VImageRepn,src);  
    }

    if (VPixelRepn(src)==VShortRepn) {
 
      /* if tr unknown, set from current object */
      if (tr == 0) {
	if (VGetAttr(VImageAttrList(src), "MPIL_vista_0", 0, VStringRepn, &str)
          == VAttrFound) {
	  sscanf(str," repetition_time=%d ",&k);
	  tr = k;
	  if (tr > 20) tr = tr / 1000.0; 
          fprintf(rep_file,"TR:                                     %.2f\n",tr); 
        }
        else VError("TR in functional data missing");
      }

      nslice++; 
  
      if ((int)VImageNRows(src)<2) continue; 

      fullslice++;

     /* compare rows, colums, timesteps of current slice with previous slice */
      if (ntimesteps==0){ 
        ntimesteps = (int)VImageNBands(src);
        nrows  = (int)VImageNRows(src);
        ncols  = (int)VImageNColumns(src);
      }
      else {
        if (ntimesteps != VImageNBands(src)) VError("Different number of timesteps in slices");
        if (nrows  != VImageNRows(src))      VError("Different number of rows in slices");
        if (ncols  != VImageNColumns(src))   VError("Different number of columns in slices");
      }

      /* calculate grand mean */ 
      if (minval == 0){
	mean = 0; colmean = 0; rowmean = 0; imagemean = 0;
        for (rr=0; rr<nrows; rr++) {
          for (cc=0; cc<ncols; cc++) { 
            for (bb=0; bb<ntimesteps; bb++) { 
	      mean = mean + VPixel(src,bb,rr,cc,VShort); 
	    }
            mean = mean/ntimesteps;
            colmean = colmean + mean;
          }
          colmean = colmean/ncols;  
          rowmean = rowmean + colmean;
        }
        imagemean = rowmean/nrows; 
      }
    }
    if (minval == 0) grandmean += imagemean;
  }
  /*  if (minval == 0) minval = grandmean/(fullslice*8); */
  if (minval == 0) minval = grandmean/fullslice; 
  fprintf(rep_file,"Automatic minval:                       %.2f\n",minval);

  /* Check if there are functional slices */ 
  if (nslice==0) VError("No functional slices");

  /* read design file */
  fprintf(stderr,"Reading design file...\n");
  des_file = fopen(desfile,"r");
  if (des_file == NULL) VError("Error opening design file");

  /* check if old or new design */
  old=FALSE;
  while (fgets(line,n,des_file)) {
    if (strlen(line) < 3) continue;
    if ((line[0] == '#') || (line[0] == '%')) continue;
    token = strtok(line," ");
    if (strcmp(token,"onset:") == 0) {
      old=TRUE;
      fprintf(stderr,"Attention: using old design file\n");
      goto next;
    }
  }
  next:
  fclose(des_file);
  des_file = fopen(desfile,"r");
  if (des_file == NULL) VError("Error opening design file");


  /* get onsets from design file */

  /* old design */
  if (old) { 
    i=0;
    while (fgets(line,n,des_file)) {
      if (strlen(line) < 3) continue;
      if (line[0] == '#') continue;
      token = strtok(line," ");
      if (strcmp(token,"onset:") == 0) {
        i++;
        if (i != cond) continue;
      
        j=0; 
        token = strtok(NULL," ");
        while (token != NULL) {
          x[j] = atof(token); 
          j++; 
          if (j >= NTRIALS) VError("Too many trials");
          token = strtok(NULL," ");
        }
      }
    }
    ntrials = j; 
    if (ntrials == 0) VError("No onsets for condition found");
  }

  else {

    j=i=0;
    while (!feof(des_file)) {
      fgets(line,LEN,des_file);
      if (strlen(line) < 2) continue;
      if (line[0] == '%') continue;
      if (! test_ascii((int)line[0])) VError(" input file must be a text file");

      if (sscanf(line,"%d %f %f %f",&id,&onset,&duration,&height) != 4)
        VError(" line %d: illegal input format",i+1);
      i++;

      if (id != cond) continue;
      x[j] = onset / tr;
      j++;
    }
    ntrials = j;
    if (ntrials < 1) VError(" no trials in condition %d",cond);

  }

  ntrials--;
  fclose(des_file);



  /* write condition and onsets in report file */    
  fprintf(rep_file,"Condition:                              %d\n\n",cond);
  fprintf(rep_file,"onsets:");
  for (j=0; j<ntrials; j++) if (old) fprintf(rep_file," %.1f",x[j]); 
                            else fprintf(rep_file," %.1f",x[j]*tr);
  fprintf(rep_file,"\n\n");


  /* prepare calculations */

  temp = (VFloat) len;
  temp = temp/reso;
  len = (VShort) temp;
  if (mean_course == NULL) 
    mean_course  = (double *) VMalloc(sizeof(double) * (len+2));
  for (i=0; i<len+2; i++) mean_course[i] = 0;

  slice=-1;


  /* loop through input images */

  fprintf(stderr,"Processing...\n ");
  for (VFirstAttr (in_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & src);
      if (VPixelRepn(src) != VShortRepn) continue;
      
      slice++;
           

     /* for data sets with empty slices */
     if ((int)VImageNRows(src)<2) {
       out[slice] = VCreateImage(1,1,1,VShortRepn);
       goto next1;
     }

      /* create new output slice */
      out[slice] = VCreateImage(len,nrows,ncols,VShortRepn);
      VFillImage(out[slice],VAllBands,0);

     /* loop through all pixels */

     /* print progress */
     fprintf(stderr,"...%.f%% \r",(float)(slice*100)/(float) nslice);

     for (rr=0; rr<nrows; rr++) {
       for (cc=0; cc<ncols; cc++) { 
	 
	 if (VPixel(src,0,rr,cc,VShort)< minval) continue;

            for (j=0; j<len; j++) { 

            sum1 = nx = 0;   

	    for (i=0; i<ntrials; i++) { 

	      ave = 0;
	    	  
	      /* event start + len  (measured in seconds) */
	      s = x[i]*tr + ((double)j)*reso;
             
	      k1 = floor(s/tr);
	      k2 = k1+1;
	      
              if (k1 >= ntimesteps || k2 >= ntimesteps) continue;
	      if (k1 < 0 || k2 < 0) continue;

	      w1 = ABS(s/tr - (double)k1);
	      w2 = ABS(s/tr - (double)k2);

	      norm = w1 + w2;
	      w1 = 1.0 - w1 / norm;
	      w2 = 1.0 - w2 / norm;

	      u1 = VPixel(src,k1,rr,cc,VShort);
	      u2 = VPixel(src,k2,rr,cc,VShort);

	      u = u1*w1 + u2*w2;

	      sum1 += u;
	      nx++;

	    }
            ave = sum1/nx;

            mean_course[j]  = ave;
            
	    VPixel(out[slice],j,rr,cc,VShort)=  (VShort) mean_course[j];

	  } 
	}
      }  

 
     /* add output slice to output image */
     next1:

        VCopyImageAttrs(src, out[slice]);
        VAppendAttr(out_list,"image",NULL,VImageRepn,out[slice]); 
     
     }  

   if (! VWriteFile (out_file, out_list)) exit (1);

  fprintf(stderr,"...100%% \n");
  fprintf(stderr,"done \n");

  fclose(rep_file);
  fclose(out_file);

  return 0;
}


