
//Vista - Libraries 
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>
#include <viaio/Vlib.h>
#include <viaio/headerinfo.h>

#include "VCheck.h"

#define LEN     10000
#define NTRIALS  2000

typedef struct TrialStruct{
  int   id;
  float onset;
  float duration;
  float height;
} Trial;


Trial trial[NTRIALS];
int ntrials=0;

extern "C" {
  extern void VImageInfoIni(VImageInfo *);
  extern VBoolean  ReadHeader(FILE *);
  extern VBoolean  VGetImageInfo(FILE *,VAttrList,int,VImageInfo *);
  extern VBoolean  VReadBlockData(FILE *,VImageInfo *,int,int,VImage *);
  extern VAttrList ReadAttrList(FILE *);
}


int test_ascii(int val)
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



int VCheck::VCheckRawDataFile(VString rawfile, int tc_minlength) {
  int raw_defekt=0;
  int nobjects=0;
  int funcobjs=0;
  VAttrListPosn posn;
  VAttrList raw_list=NULL;
  VString voxel_raw=NULL;
  VLong repetition_time=0;

  if (strlen(rawfile) > 2) {
    FILE *rawfp;
    rawfp = VOpenInputFile (rawfile, TRUE);
    if (! ReadHeader (rawfp)) return 1; // VError(" error reading raw data file");
    if (! (raw_list = ReadAttrList (rawfp))) return 1; //VError(" error reading raw data attribute list");      

    /* count the number of objects in vista file */
    int hist_items=0;
    for (VFirstAttr (raw_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if ( strncmp(VGetAttrName(&posn),"history",7) == 0 ) {
	hist_items++;
      }
      nobjects++;
    }   

    /* check raw data file */
    VImageInfo tempInfo;
    int bands_raw=0, rows_raw=0, cols_raw=0;
    for (int rawobjects=0+hist_items; rawobjects<nobjects; rawobjects++) {
      VImageInfoIni(&tempInfo);
      if (! VGetImageInfo(rawfp,NULL,rawobjects,&tempInfo)) return 1;
      if (tempInfo.repn==VShortRepn) {
	funcobjs++;
	if (tempInfo.nbands>2) {
	  if ( repetition_time<2 )  repetition_time = (VLong) tempInfo.repetition_time;
	  else {
	    if ( tempInfo.repetition_time != repetition_time) raw_defekt=1;
	  }
	  if (voxel_raw==NULL)   voxel_raw  = (VString) strdup(tempInfo.voxel);
	  else {
	    if (strcmp(tempInfo.voxel,voxel_raw)) raw_defekt=1;
	  }
	  if (strlen(voxel_raw)<3) raw_defekt=1;
	  if (bands_raw<2) bands_raw = tempInfo.nbands;
	  else {
	    if (bands_raw != tempInfo.nbands) raw_defekt=1;
	  }
	  if (rows_raw<2)  rows_raw  = tempInfo.nrows;
	  else {
	    if (rows_raw  != tempInfo.nrows) raw_defekt=1;
	  }
	  if (cols_raw<2)  cols_raw  = tempInfo.ncolumns;
	  else {
	    if (cols_raw  != tempInfo.ncolumns) raw_defekt=1;
	  }
	}
      }
    }
    //fprintf(stderr,"repetition_time: %d\n",repetition_time);
    //if (bands_raw < tc_minlength) raw_defekt=1;
    //fprintf(stderr,"bands_raw: %d, tc_minlength: %d\n",bands_raw,tc_minlength);
    fclose(rawfp);
  } else raw_defekt=1; 

  if (nobjects==0 || voxel_raw==NULL) raw_defekt=1;
  if (voxel_raw==NULL) { 
    raw_defekt=1;
  } else {
    if (strlen(voxel_raw)<3) raw_defekt=1;
  }
  if (repetition_time<2 || funcobjs==0) raw_defekt=1;
  
  return raw_defekt;
}


  
VImage VCheck::VCheckBetaFile(VString betafile) {
  VAttrListPosn posn;
  VAttrList beta_list=NULL;
  VImage designmatrix=NULL;
  VString voxel_beta=NULL; 
  int nobjects=0;
  int betaobjs=0;
  VLong repetition_time=0;

  if (strlen(betafile) > 2) {
    FILE *betafp;
    betafp = VOpenInputFile (betafile, TRUE);
    if (! ReadHeader (betafp)) return NULL; 
    if (! (beta_list = ReadAttrList (betafp))) return NULL;
    
    /* count the number of objects in vista file */
    int hist_items=0;
    for (VFirstAttr (beta_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
      if ( strncmp(VGetAttrName(&posn),"history",7) == 0 ) {
	hist_items++;
      }
      nobjects++;
    }   
  
    /* check beta file */
    VImageInfo tempInfo;
    int bands_beta=0, rows_beta=0, cols_beta=0;
    for (int betaobjects=0+hist_items; betaobjects<nobjects; betaobjects++) {
      VImageInfoIni(&tempInfo);
      if (! VGetImageInfo(betafp,NULL,betaobjects,&tempInfo)) return NULL;
      if (tempInfo.repn==VFloatRepn) {
	if (strcmp(tempInfo.modality,"RES/trRV") == 0 || strcmp(tempInfo.modality,"BETA") == 0) {
	  betaobjs++;
	  if ( repetition_time<2 ) repetition_time = (VLong) tempInfo.repetition_time;
	  else {
	    if ( tempInfo.repetition_time != repetition_time ) return NULL;
	  }
	  if (voxel_beta==NULL)   voxel_beta  = (VString) strdup(tempInfo.voxel);
	  else {
	    if (strcmp(tempInfo.voxel,voxel_beta)) return NULL;
	  }
	  if (bands_beta<2) bands_beta = tempInfo.nbands;
	  else {
	    if (bands_beta != tempInfo.nbands) return NULL;
	  }
	  if (rows_beta<2)  rows_beta  = tempInfo.nrows;
	  else {
	    if (rows_beta  != tempInfo.nrows) return NULL;
	  }
	  if (cols_beta<2)  cols_beta  = tempInfo.ncolumns;
	  else {
	    if (cols_beta  != tempInfo.ncolumns) return NULL;
	  }
	}
	if (strcmp(tempInfo.modality,"X") == 0) {
	  designmatrix = VCreateImage(1,tempInfo.nrows,tempInfo.ncolumns,VFloatRepn);
	  if (!VReadBlockData (betafp,&tempInfo,0,tempInfo.nrows,&designmatrix)) {
	    fprintf(stderr,"Can not read X matrix from beta file\n");
	    return NULL;
	  }
	}
      }
    }
    fclose(betafp);
  } else return NULL;
 

  // test ob designmatrix und betaobjekte zusammenpassen
  if (designmatrix) {
    if (VImageNColumns(designmatrix)+1!=betaobjs || strlen(voxel_beta)<3 || repetition_time<2) {
      VDestroyImage(designmatrix);
      designmatrix=NULL;
    }
  }
  
  return designmatrix;
}



VImage VCheck::VReadDesignFile(VString desfile,int verbose) {
  int *numtrials, maxnumtrials=0;
  int i=0, j=0, k, m, numcond=0, id;
  size_t n=4096;
  FILE *des_file=NULL;
  char line[4096],*token;
  VImage condimage=NULL;
  char buf[LEN];
  float onset,duration,height;
  VFloat *floatptr=NULL;

  /* count number of conditions for old design */
  if (strlen(desfile) > 2) {
    des_file = fopen(desfile,"r");
    if (des_file == NULL) return 0; //VError("error opening design file");
    while (fgets(line,n,des_file)) {
      if (strlen(line) < 3) continue;
      token = strtok(line," ");
      if (strcmp(token,"onset:") == 0) {
	numcond++;
      }
    }
    fclose(des_file);
  } else return 0;
  
  /* event-related design for *OLD* design file */
  if (numcond) {
    if (verbose>0) fprintf(stderr,"reading as *OLD* design file type.\n");
    numtrials = (int *) malloc(sizeof(int) * numcond);
    i=0;
    des_file = fopen(desfile,"r");
    while (fgets(line,n,des_file)) {
      if (strlen(line) < 3) continue;
      token = strtok(line," ");
      if (strcmp(token,"onset:") == 0) {
	j=0;
	token = strtok(NULL," ");
	while (token && strcmp(token,"\n")) {
	  //x[j] = atof(token);
	  j++;
	  token = strtok(NULL," ");
	}
	numtrials[i]=j;
	if (j>maxnumtrials) maxnumtrials=j;
	i++;
      }
    }
    fclose(des_file);
      
    // get the trials for each condition
    condimage=VCreateImage(2,numcond,maxnumtrials+1,VFloatRepn);
    VSetAttr(VImageAttrList (condimage),"numcond",NULL,VShortRepn,(VShort)numcond);
    VSetAttr(VImageAttrList (condimage),"scale",NULL,VStringRepn,"timesteps");
    floatptr  = (VFloat *) VPixelPtr(condimage,0,0,0);
    memset (floatptr,0,sizeof(VFloat) * numcond * (maxnumtrials+1) * 2);
    i=0;
    des_file = fopen(desfile,"r");
    while (fgets(line,n,des_file)) {
      if (strlen(line) < 3) continue;
      token = strtok(line," ");
      if (strcmp(token,"onset:") == 0) {
	j=0;
	VPixel(condimage,0,i,0,VFloat)=(VFloat)numtrials[i];
	VPixel(condimage,1,i,0,VFloat)=(VFloat)-1.0;
	token = strtok(NULL," ");
	while (token && strcmp(token,"\n")) {
	  j++;
	  VPixel(condimage,0,i,j,VFloat)=(VFloat)atof(token);
	  token = strtok(NULL," ");
	}
	i++;
      }
    }
    fclose(des_file);
  } else {

    /* count number of conditions for *NEW* design */ 
    if (verbose>0) fprintf(stderr,"reading as new design file type.\n");
    if (strlen(desfile) > 3) {
      des_file = fopen(desfile,"r");
      if (des_file == NULL) return 0; 
      //VError("error opening design file");
      
      /*
      ** parse input file, get number of timesteps, TR, number of events
      */
      i=0;
      while (!feof(des_file)) {  
	fgets(buf,LEN,des_file);
	if (strlen(buf) < 2) continue;
	if (buf[0] == '%') continue;
	if (! test_ascii((int)buf[0])) return 0;
	//VError(" input file must be a text file");
	
	
	/* remove non-alphanumeric characters */
	for (j=0; j<strlen(buf); j++) {
	  k = (int)buf[j];
	  if (!isgraph(k) && buf[j] != '\n' && buf[j] != '\r' && buf[j] != '\0') {
	    buf[j] = ' ';
	  }
	  if (buf[j] == '\v') buf[j] = ' '; /* remove tabs */
	  if (buf[j] == '\t') buf[j] = ' ';
	}
	
	if (sscanf(buf,"%d %f %f %f",&id,&onset,&duration,&height) != 4) return 0;
	//VError(" line %d: illegal input format",i+1);
	
	if (duration < 0.5 && duration >= -0.0001) duration = 0.5;
	trial[i].id       = id-1;
	trial[i].onset    = onset;
	trial[i].duration = duration;
	trial[i].height   = height;
	i++;
	if (i > NTRIALS) return 0;
	//VError(" too many trials %d",i);
	
	if (id > numcond) numcond++;
      }
      
      fclose(des_file);
    } else return 0; 
    
    // finish if no conditions found 
    if (numcond==0) return 0;

    // get the number of trials per condition
    numtrials = (int *) malloc(sizeof(int) * numcond);
    for (j=0; j<numcond; j++) numtrials[j]=0;
    for (j=0; j<i; j++) numtrials[trial[j].id] += (int)1;
    for (j=0; j<numcond; j++) {
      if (maxnumtrials<numtrials[j]) maxnumtrials=numtrials[j];
    }

    // get the trials for each condition
    condimage=VCreateImage(2,numcond,maxnumtrials+1,VFloatRepn);
    VSetAttr(VImageAttrList (condimage),"numcond",NULL,VShortRepn,(VShort)numcond);
    VSetAttr(VImageAttrList (condimage),"scale",NULL,VStringRepn,"seconds");
    floatptr  = (VFloat *) VPixelPtr(condimage,0,0,0);
    memset (floatptr,0,sizeof(VFloat) * numcond * (maxnumtrials+1) * 2);
    for (j=0; j<numcond; j++) {
      VPixel(condimage,0,j,0,VFloat)=(VFloat)numtrials[j];
      VPixel(condimage,1,j,0,VFloat)=(VFloat)-1.0;
      m=1;
      for (k=0; k<i; k++) {
	if (trial[k].id==j) {
	  VPixel(condimage,0,j,m,VFloat)=(VFloat)(trial[k].onset);
	  VPixel(condimage,1,j,m,VFloat)=(VFloat)(trial[k].duration);
	  m++;
	}
      }
    }
  }
   
  // finish if no conditions found 
  if (numcond==0) return 0;
 
  // Anzeigemodul
  if (verbose>0) {
    VShort numcondshort=0;
    VGetAttr(VImageAttrList (condimage),"numcond",NULL,VShortRepn,(VPointer) & numcondshort);
    numcond=(int)rint(numcondshort);
    fprintf(stderr,"The design has %d conditions.\n",numcond);
    for (i=0; i<numcond; i++) {
      fprintf(stderr,"Onset%d: %d items: ",i,numtrials[i]);
      for (j=0; j<(int)rint(VPixel(condimage,0,i,0,VFloat)); j++)
	fprintf(stderr,"%1.1f ",VPixel(condimage,0,i,j+1,VFloat));
      fprintf(stderr,"\n");  
    }
  }

  return condimage;
}
