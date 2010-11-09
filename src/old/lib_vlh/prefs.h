#ifndef PREFS_H
#define PREFS_H

/* From the std. library: */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>   /* Interprozesskommunikation */
#include <signal.h>

/* From the QT library: */
#include <qcolor.h>

/* all preferences variables */
struct prefs {
  int active;         //0
  int acoltype;       //0
  int gcoltype;       //0   /* added by A. Hagert */
  int crossize;       //1   /* added by A. Hagert */
  int fog;            //0   /* added by A. Hagert */
  int exact;          //0   /* added by A. Hagert */
  int synchronize;    //0   /* added by A. Hagert */
  int openvis;
  int atlas;          //0
  int baco;           //0
  int bolres;         //0
  float bildzoom;       //1
  int coltype;        //0
  int cutoutside;
  int digits;
  int firesp;         //1
  int fitevobool;
  int files;
  int *hgfarbe;
  int interpol;     //1
  int glassbrain;
  int lockz;
  int lipstyle;
  int midclick;
  int ogl;
  int oglzmapdense;
  int pixelco;
  int *picture;
  int persi;        //1
  int radius;
  int showcross;
  int showradius;
  int stdera;       //1
  int sw2;
  int tiline;       //1
  int triallength;   //20
  int trialresolution;
  int transzmap;
  int talairach;
  int talairachoff;
  int verbose;
  int zmapview;     //1
  float cursorp[3];  // was "int" before - changed by A. Hagert for float-graphs
  int infilenum;
  int infilenum_graph;	/* added by A. Hagert - number of graphs */
  float col_min;	/* added by A. Hagert - number of graphs */
  float col_max;	/* added by A. Hagert - number of graphs */
  int spheresize;       /* added by K. Muller -- size of sphere in graphs */
  float tpos;           // positive threshold of zmap
  float tneg;           // negative threshold of zmap
  float anamean;       
  float anaalpha;
  int minwert;      
  int maxwert;
  int minwert1;      
  int maxwert1;
  int background0;
  int background1;
  int shift;
  int spread;
  int zmapfilenum;
  int interpoltype;
  int mousemove;
  int nba;         // for correlation
  int nro;         // for correlation
  int nco;         // for correlation 
  int hideitems;
  int tc_minlength;
  double ncols_mult;
  double nrows_mult;
  double nbands_mult;
  double thresh;
  double nmax;
  double pmax;
  double zeropoint;
  double pixelmult[3];
  double voxel[3];
  double pixelm2[3];
  double slidefaktor[2];
  QColor crosscolor;
  QColor radiuscolor;
  VString raw;
  VString beta;
  VString des;
  VString graph[5];  /* added by A. Hagert -> more than one graph */
  VString colortable;
  VBoolean equidistantColorTable;
  VImage condim;
  VImage designim;
  VImage *sr;
  VImage *fn;
  VImage *gr;
  short only_sulci;
  short graphtype;
  short polygons;
  short g_number;
  int   ipc;
  int   extent_match;
  char *prg_name;
};

                                                                            
#endif // PREFS
