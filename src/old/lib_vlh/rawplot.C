/****************************************************************
 *
 * rawplot.C
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
 * $Id: rawplot.C 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/
//Vista - Libraries 
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/headerinfo.h>

#include "rawplot.h"
#include "fenster1.h"

#define DEBUGING 0

#define ABS(x) ((x) < 0 ? -(x) : (x))

int *mylength=NULL;
QStringList des_pname;

extern "C" {
  extern void VImageInfoIni(VImageInfo *);
  extern VBoolean  ReadHeader(FILE *);
  extern VBoolean  VGetImageInfo(FILE *,VAttrList,int,VImageInfo *);
  extern VBoolean  VReadBlockData(FILE *,VImageInfo *,int,int,VImage *);
  extern VAttrList ReadAttrList(FILE *);
}

extern float * VPowerSpectrum(float *, int);
extern float ** timecourse(VImage,float *,int,int,int,double,int);


RawPlot::RawPlot( QWidget *parent, const char *name, prefs *pr_, const char *version, int files, QString vlviewname )
  : QWidget( parent, name ), pr(pr_), version_m(version), files_m(files), vlviewname_m(vlviewname) {
  //setPalette( QPalette( QColor( 255,255,255) ) );
  bline=0;
  maxfarben=0;
  beta_file=NULL;
  beta=NULL;
}

void RawPlot::ausgabe() { 
  int lX, lY, lZ;
  double ncols_mult=3, nrows_mult=3, nbands_mult=3;
  float *se=NULL, **evres=NULL, **evresSE=NULL;
  int length_ER=0, width_ER=1;
  int cnd=0, firstfuncobj=-1, nBeta=0;
  float *conditions=NULL, *Sess=NULL;

  /* Set a file pointer to the raw data */
  if (pr->raw) {
    VAttrList rawlist; 
    FILE *rawfp;
    rawfp = VOpenInputFile (pr->raw, TRUE);
    if (! ReadHeader (rawfp)) {
      QMessageBox::warning( this, "Error","Error reading raw data file\n");
      fclose(rawfp); return;
    }
    if (! (rawlist = ReadAttrList (rawfp))) {
      QMessageBox::warning( this, "Error","Error reading raw data attribute list\n");
      fclose(rawfp); return;
    }
 
    /* count the number of objects */
    int hist_items=0, nobjects=0;
    VAttrListPosn posn;
    for (VFirstAttr (rawlist, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if ( strncmp(VGetAttrName(&posn),"history",7) == 0 ) {
	hist_items++;
      }
      nobjects++;
    }   

    /* get image info */ 
    VString voxel_raw=NULL, newstr=NULL;
    VImageInfo tempInfo;
    int bands_raw=0, rows_raw=0, cols_raw=0;
    int ntimesteps=0, powersteps=0, funcobjs=0;
    int trpre_int =0, trpre_intB=0;
    double mean=0.0, sig=0.0;
    double eps=0.000000001;
    firstfuncobj=-1;
    for (int rawobjects=0+hist_items; rawobjects<nobjects; rawobjects++) {
      VImageInfoIni(&tempInfo);
      if (! VGetImageInfo(rawfp,NULL,rawobjects,&tempInfo)) {
	QMessageBox::warning( this, "Error","Error reading image info\n");
	fclose(rawfp); return;
      }
      if (tempInfo.repn==VShortRepn) {
	if (firstfuncobj==-1)  firstfuncobj=rawobjects;
	if (voxel_raw==NULL)   voxel_raw  = (VString) strdup(tempInfo.voxel);
	if (bands_raw<2) bands_raw = tempInfo.nbands;
	if (rows_raw<2)  rows_raw  = tempInfo.nrows;
	if (cols_raw<2)  cols_raw  = tempInfo.ncolumns;
	if (trpre_int<2) trpre_int = tempInfo.repetition_time;
	if (ABS(sig)<eps) {
	  mean = tempInfo.norm_mean;
	  sig  = tempInfo.norm_sig;
	}
	funcobjs++;
      }
    }
    if (ABS(sig)<eps) {
      mean = 10000.0;
      sig  = 1000.0;
    }


    //voxel
    char *token=NULL, *tmptoken=NULL;
    newstr = VNewString(voxel_raw);
    ncols_mult  = atof(strtok(newstr," "));
    nrows_mult  = atof(strtok(NULL," "));
    nbands_mult = atof(strtok(NULL," "));
    VFree(newstr);
 
    //TR and ntimesteps
    trpre = (double)trpre_int;
    ntimesteps=bands_raw;
    powersteps=(int)(ntimesteps/2 + 1);
    if (ntimesteps<pr->tc_minlength) {
      QMessageBox::warning( this, "Warning","Not enough timesteps\n");
      fclose(rawfp); return;
    }
    
    //Verbose messages
    if (pr->verbose==1) {
      fprintf(stderr,"RawFile: rows/cols/slices: %d/%d/%d\n",rows_raw,cols_raw,funcobjs);
      fprintf(stderr,"RawFile: timesteps: %d\n",ntimesteps);
      fprintf(stderr,"RawFile: voxel: %f %f %f\n", ncols_mult,nrows_mult,nbands_mult);
      fprintf(stderr,"RawFile: TR: %f ms\n",trpre);
      fprintf(stderr,"RawFile: norm_mean: %f, norm_sig: %f\n", mean, sig);
      fprintf(stderr,"Ana: pixelmult: %f %f %f .... \n",pr->pixelmult[0],pr->pixelmult[1],pr->pixelmult[2]);      
    }
    
    // Umrechnen der Koordinaten
    lX = (int)((double)(pr->cursorp[0])*pr->pixelmult[0]/nrows_mult  );
    lY = (int)((double)(pr->cursorp[1])*pr->pixelmult[1]/ncols_mult  );
    lZ = (int)((double)(pr->cursorp[2])*pr->pixelmult[2]/nbands_mult );
    if (pr->verbose==1) fprintf(stderr,"Zeitreihe: %f, %f, %f\n",pr->cursorp[1],pr->cursorp[0],pr->cursorp[2]);
    if (pr->verbose==1) fprintf(stderr,"Z Y X : %d %d %d\n", lZ, lY, lX);
    model = (float *)  VMalloc(sizeof(float) * 6 );
    model[0]=(float)pr->cursorp[0];
    model[1]=(float)pr->cursorp[1];
    model[2]=(float)pr->cursorp[2];
    model[3]=(float)lX;
    model[4]=(float)lY;
    model[5]=(float)lZ;
 
    // Einlesen der Rohdaten
    VImage raw_image=NULL;
    VImageInfo imageInfo;
    VImageInfoIni(&imageInfo);
    if (! VGetImageInfo(rawfp,NULL,lZ+firstfuncobj,&imageInfo)) {
      QMessageBox::warning( this, "Error","Error reading image info\n");
      fclose(rawfp); return;
    }
    if (imageInfo.repn!=VShortRepn) {
      QMessageBox::warning( this, "Error","Functional objects must be short\n");
      fclose(rawfp); return;
    }
    if (imageInfo.nbands<2 || imageInfo.nrows<2 || imageInfo.ncolumns<2) {
      QMessageBox::warning( this, "NO RAWDATA","No raw data available for this voxel\n");
      if (pr->verbose==1) fprintf(stderr,"Reason: bands || rows || cols < 2\n");
      fclose(rawfp); return;
    }
    raw_image=VCreateImage(ntimesteps,1,cols_raw,VShortRepn);
    if (!VReadBlockData (rawfp,&imageInfo,lY,(int)1,&raw_image)) {
      QMessageBox::warning( this, "NO RAWDATA","No raw data available for this voxel\n");
      if (pr->verbose==1) fprintf(stderr,"Reason: !VReadBlockData\n");
      fclose(rawfp); return;
    }
    fclose(rawfp);

    /* Erstellen der zeitreihe */
    double wert99=0;
    float *wert=NULL, mittelwert=0;
    int minwert=100000, maxwert=-100000;
    wert = (float *) VMalloc(sizeof(float) * (ntimesteps+3) );
    for (int i=0; i<ntimesteps; i++) {
      wert99 = (double)VPixel(raw_image,i,0,lX,VShort);
      wert99 = (wert99 - 10000.0)/1000.0;
      wert[i]= (float)(wert99*sig + mean);
      if (minwert > wert[i]) minwert=(int)wert[i];
      if (maxwert < wert[i]) maxwert=(int)wert[i];
      mittelwert += wert[i];
    }

    // Destroy raw data image
    VDestroyImage(raw_image);
    if (maxwert<=minwert) {
      QMessageBox::warning( this, "NO RAWDATA","No raw data available for this voxel\n");
      if (pr->verbose==1) fprintf(stderr,"Reason: maxwert<=minwert\n");
      return;
    } else {
      wert[ntimesteps]  =(float)minwert;
      wert[ntimesteps+1]=(float)maxwert;
      wert[ntimesteps+2]=(float)(mittelwert/ntimesteps);
    }

    // Power Spectrum 
    float *powerwert=NULL;
    powerwert=VPowerSpectrum(wert,ntimesteps);

    /***********************************************************
     *                     BETA - FILE                         *
     ***********************************************************/
    FILE *betafp=NULL;
    VImageInfo betaInfo;
    if (pr->designim) nBeta=VImageNColumns(pr->designim);
    else pr->beta=NULL;
    int bands_beta=0, rows_beta=0, cols_beta=0;
    if (pr->beta && VImageNRows(pr->designim) != ntimesteps) {
      QMessageBox::warning( this, "Warning",
      "Timesteps in beta file and raw file do NOT coincide.\nIgnoring beta file.");
      pr->beta=NULL;
    }

    if (pr->beta) {
      VAttrList beta_list; 
      betafp = VOpenInputFile (pr->beta, TRUE);
      if (! ReadHeader (betafp)) VError("Error reading beta file");
      if (! (beta_list = ReadAttrList (betafp))) VError("Error reading beta attribute");

      //number of objects
      int hist_items=0, nobjects=0;
      VAttrListPosn posn;
      for (VFirstAttr (beta_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
	if ( strncmp(VGetAttrName(&posn),"history",7) == 0 ) {
	  hist_items++;
	}
	nobjects++;
      }   

      /* get beta image info */ 
      VString voxel_beta=NULL;
      int betaobjs=0;
      firstfuncobj=-1;
      for (int betaobjects=0+hist_items; betaobjects<nobjects; betaobjects++) {
	VImageInfoIni(&betaInfo);
	if (! VGetImageInfo(betafp,NULL,betaobjects,&betaInfo)) 
	  VError("Error reading beta image info");
	if (betaInfo.repn==VFloatRepn) {
	  if (strcmp(betaInfo.modality,"RES/trRV") == 0 || strcmp(betaInfo.modality,"BETA") == 0) {
	    if (firstfuncobj==-1)  firstfuncobj=betaobjects;
	    if (voxel_beta==NULL) voxel_beta = (VString) strdup(betaInfo.voxel);
	    if (bands_beta<2) bands_beta = betaInfo.nbands;
	    if (rows_beta<2)  rows_beta  = betaInfo.nrows;
	    if (cols_beta<2)  cols_beta  = betaInfo.ncolumns;
	    if (trpre_intB<2) trpre_intB = betaInfo.repetition_time;
	    betaobjs++;
	  }
	}
      }

      //passen beta und raw file zusammen?
      if (strcmp(voxel_beta,voxel_raw) || trpre_int!=trpre_intB || rows_beta!=rows_raw || cols_beta!=cols_raw || bands_beta!=funcobjs || nBeta+1 != betaobjs || betaobjs<2) {


	fprintf(stderr,"nBeta+1/betabojs: %d/%d\n", nBeta+1 ,betaobjs);
	QMessageBox::warning( this, "Warning",
	"Rawfile / Betafile mismatch.\nIgnoring beta file.");
	pr->beta=NULL;
	fclose(betafp);
      }
    }
 
    float *fitres=NULL;
    if (pr->beta) {
      //lade betaimages
      float wert100=0;
      VImage betaimagetemp=NULL, *betaimage=NULL;
      betaimage = (VImage *) malloc(sizeof(VImage) * nBeta );
      for (int nb=0; nb<nBeta; nb++) {
	betaimage[nb]=VCreateImage(bands_beta,1,cols_beta,VFloatRepn);
	betaimagetemp = betaimage[nb];
	if (! VGetImageInfo(betafp,NULL,nb+firstfuncobj+1,&betaInfo))
	  VError("Error reading beta image info");
	if (!VReadBlockData (betafp,&betaInfo,lY,(int)1,&betaimagetemp))
	  VError("Error reading beta block data");
      }
      //generate fitres sequence
      float mittelwert=0, minwert=100000, maxwert=-100000;
      if (fitres==NULL) fitres = (float *) VMalloc(sizeof(float) * (ntimesteps+3) );
      for (int i=0; i<ntimesteps; i++) {
	wert100=0;
	for (int nb=0; nb<nBeta; nb++)
	  wert100 += (float)((double)VPixel(pr->designim,0,i,nb,VFloat)*(double)VPixel(betaimage[nb],lZ,0,lX,VFloat));
	fitres[i] = wert100;
	if (wert100<minwert) minwert=wert100;
	if (wert100>maxwert) maxwert=wert100;
	mittelwert += wert100;
      }
      fitres[ntimesteps]  =(float)minwert;
      fitres[ntimesteps+1]=(float)maxwert;
      fitres[ntimesteps+2]=(float)(mittelwert/ntimesteps);
      //destroy unused memory 
      for (int nb=0; nb<nBeta; nb++) VDestroyImage(betaimage[nb]);
      fclose(betafp);
      // normalize
      for (int i=0;i<ntimesteps;i++) 
	fitres[i]=(fitres[i]-fitres[ntimesteps+2])/fitres[ntimesteps+2]*wert[ntimesteps+2]+wert[ntimesteps+2];
      // Power Spectrum 
      Sess=VPowerSpectrum(fitres,ntimesteps);
    }
   
    /***********************************************************
     *                    Designfile                           *
     ***********************************************************/
    if (pr->des) {
      if (pr->condim) {
	VShort numcondshort=0;
	VString scaleshort=NULL;
	VGetAttr(VImageAttrList (pr->condim),"numcond",NULL,VShortRepn,(VPointer) & numcondshort);
	VGetAttr(VImageAttrList (pr->condim),"scale",NULL,VStringRepn,(VPointer) &scaleshort);
	cnd=(int)numcondshort;
	if (cnd) {
	  if (cnd != VImageNRows(pr->condim)) VError("An error occured with the number of conditions");
	  if (strcmp(scaleshort,"seconds")==0) {
	    VSetAttr(VImageAttrList (pr->condim),"scale",NULL,VStringRepn,"timesteps");
	    for (int i=0; i<VImageNRows(pr->condim); i++) {
	      for (int j=1; j<VImageNColumns(pr->condim); j++) {
		VPixel(pr->condim,0,i,j,VFloat) /= (VFloat)((double)trpre/(double)1000.0);
		VPixel(pr->condim,1,i,j,VFloat) /= (VFloat)((double)trpre/(double)1000.0);
	      }
	    }
	  }
	  evresSE = timecourse(pr->condim,wert,ntimesteps,pr->triallength,pr->trialresolution,trpre,pr->persi);
	  if (pr->beta) evres = timecourse(pr->condim,fitres,ntimesteps,pr->triallength,pr->trialresolution,trpre,pr->persi);
	} else {
	  QMessageBox::warning( this, "Warning",
				"No conditions found.\n",
				"Ignoring specified design file.");
	  pr->des=NULL;
	}
      } else {
	QMessageBox::warning( this, "Warning",
			      "No conditions found.\n",
			      "Ignoring specified design file.");
	pr->des=NULL;
      }
    }
    

 
    // START !!!!!
    Modal *m = new Modal( this, "Ausgabe", pr, ntimesteps, wert, powersteps, powerwert, fitres, se, trpre, mylength, model, version_m, conditions, Sess, bline, evres, evresSE, length_ER, width_ER, cnd, vlviewname_m );
    
    m->resize( 768, 256 );
    m->show();
  }
  else
    QMessageBox::warning( this, "Sorry",
			  "No raw or beta file loaded!\n" 
			  "If you want to use it " 
			  "please restart vlview with option: -rawbeta <raw or beta file>");
}

