#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))

extern "C" {
  extern double sqrt(double);
} 

/************************************************************
 *                                                          *
 *                    timecourse                            *
 *                                                          *
 ************************************************************/

float **
timecourse(VImage condimage,float *wert,int ntimesteps,int len,int resolution,double tr,int psc)
{
  float *mean_course, *sigma_course;
  float **evresSE=NULL;
  int i=0, j=0, k=0, kk=0, ntrials=0;
  int k1,k2,lenold,onsindex=0;
  double reso=0, u1=0, u2=0;
  double sum,sum1,sum2,nx,nxx,ave,sigma;
  double faktor=1;
  double x,s,w1,w2,u,norm;
  double min = 1000000.0;
  double max = -1000000.0;
    

  // Number of conditions
  int cnd = VImageNRows(condimage);
  if (resolution > 10) reso = (double)resolution/(double)1000.0;
  else { 
    reso = (double)resolution;
    VWarning("Temporal resolution should be specified in milliseconds");
  }
  if (tr > 20) tr = tr/(double)1000.0;
  else VWarning("Repetition time should be specified in milliseconds");
  lenold = len;

 
  len = (int)rint((double)len/reso);
  //fprintf(stderr,"reso: %1.3f, tr: %1.3f, len: %d\n",reso,tr,len);

  // Allocate memory
  evresSE = (float **) malloc(sizeof(float *) * (2*cnd + 1));
  
  // LOOP over all conditions
  for (kk=0; kk<cnd; kk++) {

    // Allocate memory
    mean_course  = (float *) malloc(sizeof(float) * len);
    sigma_course = (float *) malloc(sizeof(float) * len);
    ntrials = (int)rint(VPixel(condimage,0,kk,0,VFloat)); 
    if (ntrials < 5) VWarning(" too few trials");
    for (j=0; j<len; j++) {
      mean_course[j] = 0;
      sigma_course[j] = 0;
    }    
    
    // Calculation
    for (j=0; j<len; j++) {
      sum1 = 0;
      sum2 = 0;
      nx = 0;   

      //fprintf(stderr,"num_trials: %d\n",ntrials);
      // LOOP over each trial
      for (i=0; i<ntrials; i++) {

	//Check if the remaining timesteps are enough
	onsindex = (int)rint((double)VPixel(condimage,0,kk,i+1,VFloat)*tr/reso);
	if (onsindex+len > (int)rint((double)ntimesteps*tr/reso)) continue;

	ave = 0;
	if (psc) {
	  sum = nxx = 0;
	  if (ntimesteps < 13) VWarning(" too few timesteps");
	  for (k=10; k<ntimesteps; k++) {
	    sum += (double)wert[k];
	    nxx++;
	  }
	  ave = sum / nxx;
	}
	
	// event start + len  (measured in seconds)
	x = (double)VPixel(condimage,0,kk,i+1,VFloat);
	s = x*tr + ((double)j)*reso;
	
	k1 = (int)rint(floor(s/tr));
	k2 = k1+1;
	if (k1 >= ntimesteps || k2 >= ntimesteps) continue;
	if (k1 < 0 || k2 < 0) continue;
	
	w1 = ABS(s/tr - (double)k1);
	w2 = ABS(s/tr - (double)k2);
	
	norm = w1 + w2;
	w1 = 1.0 - w1 / norm;
	w2 = 1.0 - w2 / norm;
	
	u1 = (double)wert[k1];
	u2 = (double)wert[k2];
	
	//u1 = VPixel(src,k1,r,c,VShort);
	//u2 = VPixel(src,k2,r,c,VShort);
	
	if (psc) {
	  u1 = 100.0f * (u1 - ave) / ave;
	  u2 = 100.0f * (u2 - ave) / ave;
	}
	u = u1*w1 + u2*w2;
	
	sum1 += u;
	sum2 += u*u;
	nx++;
      }
	
      ave = sum1/nx;
      sigma = sqrt((double)ABS(sum2 - nx*ave*ave) / (nx - 1.0));
      //if (j<5 && kk==0) fprintf(stderr,"j: %d, ave: %f, sigma: %f, nx: %f\n",j,ave,sigma,nx);
      if (psc) faktor=1000.0;
      mean_course[j]  = (float)(faktor*ave);
      sigma_course[j] = (float)(faktor*sigma/sqrt((double)ntrials));
      if (mean_course[j]<min) min=mean_course[j];
      if (mean_course[j]>max) max=mean_course[j];
    }

    evresSE[2*kk+1]=mean_course;
    evresSE[2*kk+2]=sigma_course;
  }
  
  /* triallength, trialresolution, maximum, minimum */
  mean_course  = (float *) malloc(sizeof(float) * 5);
  mean_course[0] = (float)len;
  mean_course[1] = (float)resolution;
  mean_course[2] = (float)min;
  mean_course[3] = (float)max;
  mean_course[4] = (float)lenold;
  evresSE[0]=mean_course;


  return evresSE;
}
