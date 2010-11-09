/****************************************************************
 *
 * VLCorr.C
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
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
 * $Id: VLCorr.C 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#include "VLCorr.h"
#include "VLTools.h"
#include <viaio/headerinfo.h>

extern "C" {
  extern VBoolean  VReadBlockData(FILE *,VImageInfo *,int,int,VImage *);
}

extern prefs *pr;
extern double *scalec, *scaler, *scaleb;
extern VImage *src, *fnc, *tmp;
extern int firstfuncobj, nobjects, hist_items;
extern VImage rawobjektbild;
extern VImageInfo *tempInfo;

VLTools mytools1;

VLCorr::VLCorr() {
  // empty

}

void VLCorr::correlation() {
  double X5 = (double)pr->cursorp[0]*pr->pixelmult[0]/pr->pixelm2[0];
  double Y5 = (double)pr->cursorp[1]*pr->pixelmult[1]/pr->pixelm2[1];
  double Z5 = (double)pr->cursorp[2]*pr->pixelmult[2]/pr->pixelm2[2];
  CreateMap((int)X5,(int)Y5,(int)Z5);
  emit viewChanged();
}

void VLCorr::CreateMap(int X, int Y, int Z) {
  double eps=0.00000000001;
  double w=0, r=0; 
  double sum=0, quadsum=0, varsum=0, mean1=0;
  double sum1=0, mixsum1=0, varsum1=0;
  int rr=0, cc=0, bb=0, sw99=1;
  double *tc=NULL;

  // Wartezeiger
  //emit wartezeiger(1);
  if (pr->verbose>1) fprintf(stderr,"Koordinaten fuer Korrelation: %d %d %d ... ",X,Y,Z);
 
  /* Read the input file */
  FILE *rawfp;
  rawfp = VOpenInputFile (pr->raw, TRUE);

  //-----------------------------------
  //IF PRE-COMPUTATIONS ARE NOT READY
  //-----------------------------------									 
  if (fnc[2]==NULL || fnc[3]==NULL) {
    fnc[2]=VCreateImage(pr->nba,pr->nro,pr->nco,VFloatRepn); //Sum-image
    fnc[3]=VCreateImage(pr->nba,pr->nro,pr->nco,VFloatRepn); //Var-image
    int funcobj=0;
    for (int rawobjects=firstfuncobj; rawobjects<nobjects-hist_items; rawobjects++) {
      if (tempInfo[rawobjects].repn==VShortRepn) {
	if (tempInfo[rawobjects].nbands>1) {
	  if (pr->nro!=tempInfo[rawobjects].nrows || pr->nco !=tempInfo[rawobjects].ncolumns)
	    VError("Problem with rows/columns 234");
	  if (tc==NULL)
	    tc=(double *) VMalloc(sizeof(double) * tempInfo[rawobjects].nbands);
	  if (!VReadBlockData (rawfp,&tempInfo[rawobjects],(int)0,pr->nro, & rawobjektbild)) 
	    VError("Error reading functional data set 234");
	}
	for (rr=0;rr<pr->nro;rr++) {
	  for (cc=0;cc<pr->nco;cc++) {	    
	    if (tempInfo[rawobjects].nbands>1) {
	      sum=0; quadsum=0;
	      for (bb=0;bb<tempInfo[rawobjects].nbands;bb++) {
		w = (double)VPixel(rawobjektbild,bb,rr,cc,VShort); 
		sum += w;
		quadsum += w*w;
	      }
	      VPixel(fnc[2],funcobj,rr,cc,VFloat)=(VFloat)sum;
	      VPixel(fnc[3],funcobj,rr,cc,VFloat)=(VFloat)sqrt(quadsum-sum*sum/(double)tempInfo[rawobjects].nbands);
	    } else {
	      VPixel(fnc[2],funcobj,rr,cc,VFloat)=(VFloat)0.0;
	      VPixel(fnc[3],funcobj,rr,cc,VFloat)=(VFloat)0.0;
	    }
	  }
	}
	funcobj++;
      }
    }
  }
  //---------------------------------------
  //END IF PRE-COMPUTATIONS ARE NOT READY
  //---------------------------------------	

  // Detect sum and varsum of the pixel 
  sum    = (double)VPixel(fnc[2],Z,Y,X,VFloat);
  varsum = (double)VPixel(fnc[3],Z,Y,X,VFloat);
  if (varsum<eps && varsum>-eps) {
    if (pr->verbose>1) fprintf(stderr,"No functional data in this voxel\n");
    sw99=0;
  }

 
  // If a functional timecourse is present
  if (sw99==1) {
    
    // Do something for slice Z  
    if (tempInfo[Z+firstfuncobj].repn!=VShortRepn) 
      VError(" error with raw slice Zuordnung 567");
    if (tempInfo[Z+firstfuncobj].nbands>1) {
      if (pr->nro!=tempInfo[Z+firstfuncobj].nrows || pr->nco !=tempInfo[Z+firstfuncobj].ncolumns)
	VError("Problem with rows/columns 567");
      if (tc==NULL)
	tc=(double *) VMalloc(sizeof(double) * tempInfo[Z+firstfuncobj].nbands);
      if (!VReadBlockData (rawfp,&tempInfo[Z+firstfuncobj],(int)0,pr->nro, & rawobjektbild)) 
	VError("Error reading functional data set 567");
      for (bb=0;bb<tempInfo[Z+firstfuncobj].nbands;bb++) tc[bb] = (double)VPixel(rawobjektbild,bb,Y,X,VShort);
    } else {
      for (bb=0;bb<tempInfo[Z+firstfuncobj].nbands;bb++) tc[bb] = (double)0.0;
    }
    
    // handle destination image 
    VDestroyImage(fnc[0]);
    fnc[0]=VCreateImage(pr->nba,pr->nro,pr->nco,VFloatRepn);
   
    // Compute something for all FUNCTIONAL images 
    int funcobj=0;
    for (int rawobjects=firstfuncobj; rawobjects<nobjects-hist_items; rawobjects++) {
      if (tempInfo[rawobjects].repn==VShortRepn) {
	if (tempInfo[rawobjects].nbands>1) {
	  if (pr->nro!=tempInfo[rawobjects].nrows || pr->nco !=tempInfo[rawobjects].ncolumns)
	    VError("Problem with rows/columns 99");
	  if (!VReadBlockData (rawfp,&tempInfo[rawobjects],(int)0,tempInfo[rawobjects].nrows, & rawobjektbild)) 
	    VError("Error reading functional data set 99"); 
	} 
	for (rr=0;rr<pr->nro;rr++) {
	  for (cc=0;cc<pr->nco;cc++) {
	    if (tempInfo[rawobjects].nbands>1) {
	      mean1 = (double)VPixel(fnc[2],funcobj,rr,cc,VFloat)/(double)tempInfo[rawobjects].nbands;
	      if (mean1>eps || mean1<-eps) {
		mixsum1=0;
		for (bb=0;bb<tempInfo[rawobjects].nbands;bb++) 
		  mixsum1 += tc[bb]*(double)VPixel(rawobjektbild,bb,rr,cc,VShort);
		sum1     = (double)VPixel(fnc[2],funcobj,rr,cc,VFloat);
		varsum1  = (double)VPixel(fnc[3],funcobj,rr,cc,VFloat);
		if (varsum1>eps || varsum1<-eps) {
		  r = (double)100.0*(mixsum1-sum*sum1/(double)tempInfo[rawobjects].nbands)/(varsum*varsum1);
		} else 
		  r=0;
	      } else
		r=0;
	    } else 
	      r=0;
	    
	    // Make image 
	    if (r>100) r=100;
	    if (r<-100) r=-100;
	    VPixel(fnc[0],funcobj,rr,cc,VFloat)=(VFloat)r;
	  }
	}
	funcobj++;	    
      }
    }

    mytools1.interpolate( src[0], fnc[0], pr->ncols_mult, pr->nrows_mult, pr->nbands_mult, scaleb[0], scaler[0], scalec[0], 0, pr->interpol );
   
  }

  // close file 
  fclose(rawfp);

  // Free unused memory
  if (tc) VFree(tc);
  if (pr->verbose>1) fprintf(stderr,"ausgefuehrt\n");
  //emit wartezeiger(0);
}

