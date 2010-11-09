/****************************************************************
 *
 * rawpaint:
 *
 * Copyright (C) 1998 MPI of Cognitive Neuroscience, Leipzig
 *
 * Author Heiko Mentzel, 1998, <lipsia@cns.mpg.de>
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
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Vista - Librarys */
#include <viaio/Vlib.h>

/* Qt - Librarys */
#include <qpushbutton.h>
#include <qstring.h>
#include <qimage.h> 
#include <qfile.h> 
#include <qfiledialog.h> 

#include "rawpaint.h"

#define OLIVERGRUBER 0

extern int allefarben;

RawPaint::RawPaint( QWidget *parent, const char *name, prefs *pr_, int i, float *wert1, int pi, float *powerwert, float *fitres, float *se, double trpre, int *length, float *model, QStringList fname, float *conditions, float *Sess, int bline, float **evres, float **evresSE, int length_ER, int width_ER, int cnd )
  : QWidget( parent, name ), fname_m(fname), bline_m(bline), pr(pr_), i_m(i), wert_m(wert1), pi_m(pi), powerwert_m(powerwert), fitres_m(fitres), se_m(se), trpre_m(trpre), length_m(length), model_m(model), conditions1m(conditions), Sess_m(Sess), evres_m(evres), evresSE_m(evresSE), lER(length_ER), wER(width_ER) , cnd_m(cnd) {
  session=1;

  //if (pr->des) maxfarben = cnd_m;
  //else maxfarben = 1;

  maxfarben=cnd_m;
  if (cnd_m) {
    onslengths = (int *) malloc(sizeof(int) * maxfarben );
    for (int i1=0;i1<maxfarben;i1++) onslengths[i1]=(int)rint(VPixel(pr->condim,0,i1,0,VFloat));
  }
  
  trpre_m /= (double)1000.0;
  persip=pr->persi;
  bolresp=pr->bolres;
  firespp=pr->firesp;
  tilinep=pr->tiline;
  fitevoboolp=pr->fitevobool;
  //setPalette( QPalette( QColor( 255,255,255 ) ) );
  if (pr->hgfarbe[1]==1) setPalette( QPalette( QColor( 255,255,255 ) ) );
  else setPalette( QPalette( QColor( 0,0,0 ) ) );

  raster=1; //fittedresponse=1;

  wertneu  = (float *) malloc(sizeof(float) * i_m );
  if (pr->beta) fitresneu   = (float *) malloc(sizeof(float) * i_m );

  /* cline = parameter fuer farbe ein/aus */
  if ( pr->des || pr->beta) {
    if (pr->verbose) fprintf(stderr,"define colors\n");
    if (maxfarben>cnd_m) allefarben = maxfarben; else allefarben = cnd;
    cline = (int *) malloc(sizeof(int) * allefarben+1 );
    for (int i0=0;i0<=allefarben;i0++) {
      cline[i0]=0;
    }

    bgfarbemidd = (QColor *) malloc(sizeof(QColor)*(allefarben+1) );
    bgfarbemidd[0]=QColor( black );
    for (i=1;i<=allefarben;i++) {
      if (i==bline)
	bgfarbemidd[i]=QColor( black );
      else
	bgfarbemidd[i].setHsv( int((double)(360/allefarben+1) * (i-1)), 255,  255);
    }    
    bgfarbedark = (QColor *) malloc(sizeof(QColor)*(allefarben+1) );
    bgfarbedark[0]=QColor( black );
    for (i=1;i<=allefarben;i++) {
      //bgfarbedark[i]=bgfarbemidd[i].dark(140);
      bgfarbedark[i].setHsv( int((double)(360/allefarben+1) * (i-1)), 255,  190);
    }
    
    bgfarbe = (QColor *) malloc(sizeof(QColor)*(allefarben+1) );
    bgfarbe[0]=QColor( black );
    for (i=1;i<=allefarben;i++) {
      //bgfarbe[i]=bgfarbemidd[i].light(140);
      bgfarbe[i].setHsv( int((double)(360/allefarben+1) * (i-1)), 170,  255);
    }
  }
  
  obabst=10;
  zoom=1, posit=0;
  mittelwert      = (double *) malloc(sizeof(double) * 101 );
  qmittelwert     = (double *) malloc(sizeof(double) * 101 );
  if (pr->beta) {
    mittelfitt    = (double *) malloc(sizeof(double) * 101 );
  }
  //if (pr->verbose) fprintf(stderr,"end destructor\n");
}

void RawPaint::paintEvent( QPaintEvent * ) {
  //int minmod=0, maxmod=0;
 
  //Einlesen der daten und skalieren der fitted response
  min_m=1000000; max_m=-1000000;
  for (int i=0;i<i_m;i++) {
    if (persip==1) 
      wertneu[i]=(float)1000.0*((float)100.0*(wert_m[i]-wert_m[i_m+2])/wert_m[i_m+2]);
    else 
      wertneu[i]=wert_m[i];
    if (pr->beta) {
      if (persip==1)
	fitresneu[i]=(float)1000.0*((float)100.0*(fitres_m[i]-wert_m[i_m+2])/wert_m[i_m+2]  ); 
      else 
	fitresneu[i]=fitres_m[i];
    }
  }

  // minimum und maximum berechnen
  if (persip==1) {
    min_m = (float)1000.0*((float)100.0*(wert_m[i_m]-wert_m[i_m+2])/wert_m[i_m+2]);
    max_m = (float)1000.0*((float)100.0*(wert_m[i_m+1]-wert_m[i_m+2])/wert_m[i_m+2]);
  } else {
    min_m=wert_m[i_m];
    max_m=wert_m[i_m+1];
  }

  double vgroesse=width();
  double hgroesse=height();
  double size = max_m-min_m;

  //if (pr->verbose) fprintf(stderr,"create picture\n");

  pic.create ( int(vgroesse), int(hgroesse), 32, 1024);
  //pic.fill( qRgb(255,255,255) );
  //if (hgfarbe[1]==1) setBackgoundColor( QColor( 255,255,255 ) );
  //else setBackgoundColor( QColor( 0,0,0 ) );
  if (pr->hgfarbe[1]==1) pic.fill( qRgb(255,255,255) );
  else pic.fill( qRgb(0,0,0) );

  QPainter paint( this );

  cpm = QPixmap();
  cpm.convertFromImage(pic,~ColorMode_Mask | AutoColor );
  QPainter pcpm( &cpm );
  pcpm.setFont( QFont( "arial", 8, QFont::Normal, FALSE ) );


  // timecourse
  //#############################################################################
  if ( bolresp == 0 ) {
    if (zoom>8) zoom=8;    
    if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(gray), 1, QPen::SolidLine));
    else pcpm.setPen( QPen( QColor(darkGray), 1, QPen::SolidLine));
    pcpm.drawLine(40-posit*zoom,10,40-posit*zoom,int(hgroesse-25));        // senkrechte Linie    
    if (pr->baco==1) 
      pcpm.drawLine(35-posit*zoom-posit*zoom,int(10+hgroesse-((int)1000*mean-min_m)*hgroesse/size),
		    int(vgroesse-10-posit*zoom),int(10+hgroesse-((int)1000*mean-min_m)*hgroesse/size));

    if ( fitevoboolp == 1 ) 
      if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(gray), 1, QPen::SolidLine));
      else pcpm.setPen( QPen( QColor(darkGray), 1, QPen::SolidLine));
    if (raster) {
      pcpm.drawLine(35-posit*zoom,obabst,int(vgroesse-10),obabst); // waagerechte Linien
    }
    pcpm.drawLine(35-posit*zoom,int(hgroesse-30),int(vgroesse-10),int(hgroesse-30)); // waagerechte Linien
    if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(black), 1, QPen::SolidLine));
    else pcpm.setPen( QPen( QColor(white), 1, QPen::SolidLine));
    pcpm.drawLine(35-posit*zoom,10,40-posit*zoom,10);
    
    /* scalen zeichen */
    //vgroesse/50
    int abstand = i_m/15;
    abstand=abstand/10;
    abstand=abstand*10;
    if (abstand < 10) abstand=10;
    double hilfstemp = 0.0;
    double hilfsvar1 = (double)(zoom*(vgroesse-50)/i_m);
    double hilfsvar2 = (double)(posit*zoom);
    int    hilfsvar3 = 0;
    double abstzo = (double)(abstand/zoom);

    /* if designfile - zeichnen des Designs*/
    if (pr->des) {
      double onswert;
      double onsdur;
      for (int i1=0;i1<maxfarben;i1++) {
	for (int k=0; k<onslengths[i1]; k++) {
	  //	      fprintf(stderr,"%f \n",onsptr[k]);
	  if ( cline[i1+1] != 0) {
	    onswert = (double)VPixel(pr->condim,0,i1,k+1,VFloat);
	    onsdur  = (double)VPixel(pr->condim,1,i1,k+1,VFloat);
	    if (onsdur>0) {
	      hilfsvar3 = (int)(onsdur*hilfsvar1);
	      pcpm.setPen( QPen( bgfarbe[i1+1], hilfsvar3, QPen::SolidLine) );
	      pcpm.drawLine( int(40+hilfsvar1*onswert-hilfsvar2  + onsdur*hilfsvar1/2), 10, 
			     int(40+hilfsvar1*onswert-hilfsvar2  + onsdur*hilfsvar1/2), int(hgroesse-30) );
	    } else {
	      pcpm.setPen( QPen( bgfarbe[i1+1], 2, QPen::SolidLine) );
	      pcpm.drawLine( int(40+hilfsvar1*onswert-hilfsvar2), 10, 
			     int(40+hilfsvar1*onswert-hilfsvar2), int(hgroesse-30) );
	    }
	  }
	}
      }
    }
   
    if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(black), 1, QPen::SolidLine));
      else pcpm.setPen( QPen( QColor(white), 1, QPen::SolidLine));
      pcpm.setFont( QFont( "arial", 7, QFont::Normal, FALSE ) );
      if ( vgroesse < 700 ) pcpm.drawText( int(vgroesse-90),int(hgroesse-15), "x10 timesteps", -1 );
      else pcpm.drawText( int(vgroesse-60),int(hgroesse-15), "timesteps", -1 );
      pcpm.setFont( QFont( "arial", 8, QFont::Normal, FALSE ) );
      
      for ( int i=0;i<i_m;i++) {
	/* print scales */
	if (i<i_m/abstzo) {
	  hilfstemp = (double)(40+zoom*i*abstzo*(vgroesse-50)/i_m-hilfsvar2);
	  if (raster) {
	    if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(gray), 1, QPen::SolidLine));
	    else pcpm.setPen( QPen( QColor(darkGray), 1, QPen::SolidLine));
	    if (i>0) pcpm.drawLine(int(hilfstemp),10,int(hilfstemp),int(hgroesse-30));
	  }
	  if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(black), 1, QPen::SolidLine));
	  else pcpm.setPen( QPen( QColor(white), 1, QPen::SolidLine));
	  pcpm.drawLine(int(hilfstemp),int(hgroesse-25),int(hilfstemp),int(hgroesse-30));
	  hilfstemp = (double)(i*abstzo*(vgroesse-50)/i_m);
	  if ( vgroesse < 700 ) {
	    pcpm.drawText( int(36+hilfstemp-hilfsvar2),int(hgroesse-2), tr("%1").arg((int)i*abstzo/10), -1 );
	  } else {
	    pcpm.drawText( int(36+zoom*hilfstemp-hilfsvar2),int(hgroesse-2), tr("%1").arg((float)i*abstzo), -1 );
	  }
	}
      }
      
      if (persip==1) {
	pcpm.drawText ( int(3-hilfsvar2), 15, tr("%1").arg((double)max_m/1000), 5 );
	pcpm.drawText ( int(40-hilfsvar2), 15, "%", 5 );
	pcpm.drawText ( int(3-hilfsvar2), int(hgroesse-25), tr("%1").arg((double)min_m/1000), 5 );
      } else {
	pcpm.drawText ( int(3-hilfsvar2), 15, tr("%1").arg((int)max_m), 5 );
	pcpm.drawText ( int(3-hilfsvar2), int(hgroesse-25), tr("%1").arg((int)min_m), 5 );
      }
      
      pcpm.setPen(QColor( 0,150,0));
      hgroesse=hgroesse-40;
      
      /* draw raster */
      if (raster && persip==0) 
	for ( int i=1;i<=max_m;i++) {
	if ( i==(int)((max_m-min_m)/5+min_m) || i==(int)((max_m-min_m)/5*2+min_m) || i==(int)((max_m-min_m)/5*3+min_m) || i==(int)((max_m-min_m)/5*4+min_m) || i==(int)((max_m-min_m)/5*5+min_m) ) {
	  if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor( gray ), 1, QPen::SolidLine) );
	  else pcpm.setPen( QPen( QColor( darkGray ), 1, QPen::SolidLine) );
	  /* horizontal line */
	  pcpm.drawLine(int(35-hilfsvar2),int(10+hgroesse-((int)i-min_m)*(hgroesse)/size),
			int(vgroesse-10),int(10+hgroesse-((int)i-min_m)*hgroesse/size));
	  
	  if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(black), 1, QPen::SolidLine));
	  else pcpm.setPen( QPen( QColor(white), 1, QPen::SolidLine));
	  pcpm.drawLine(int(35-hilfsvar2),int(10+(hgroesse)-((int)i-min_m)*hgroesse/size),
			int(40-hilfsvar2),int(10+hgroesse-((int)i-min_m)*hgroesse/size));
	  
	  if (i<max_m-10) 
		  pcpm.drawText ( int(3-hilfsvar2), int(15+hgroesse-((int)i-min_m)*hgroesse/size), tr("%1").arg(i), 5 );
	}
      }
      for (int i=0;i<i_m-1;i++) {

	/* FITTED RESPONSE */
	if (pr->beta && firespp) {
	  pcpm.setPen( QPen( QColor( gray ), 2, QPen::SolidLine) );
	  pcpm.drawLine(int(40+zoom*i*(vgroesse-50)/i_m-hilfsvar2),
			int(10+hgroesse-((int)fitresneu[i]-min_m)*hgroesse/size),
			int(40+zoom*(vgroesse-50)/i_m+zoom*(i)*(vgroesse-50)/i_m-hilfsvar2),
			int(10+hgroesse-((int)fitresneu[i+1]-min_m)*hgroesse/size));
	}
	if (tilinep) {
	  if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(black), 1, QPen::SolidLine));
	  else pcpm.setPen( QPen( QColor(white), 1, QPen::SolidLine));
	  pcpm.drawLine(int(40+zoom*(i)*(vgroesse-50)/i_m-hilfsvar2),
			int(10+hgroesse-((int)wertneu[i]-min_m)*hgroesse/size),
			int(40+zoom*(vgroesse-50)/i_m+zoom*(i)*(vgroesse-50)/i_m-hilfsvar2),
			int(10+hgroesse-((int)wertneu[i+1]-min_m)*hgroesse/size));
	}
      }
  } else if ( bolresp == 2 ) {

    /* POWER SPECTRUM */
    //#############################################################################
    if (zoom>8) zoom=8;
    int powermin_m=0, powermax_m=0;
    double positzo  = (double)(posit*zoom);

    if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(black), 1, QPen::SolidLine));
    else pcpm.setPen( QPen( QColor(white), 1, QPen::SolidLine));
    /* vertical line */
    pcpm.drawLine(int(40-positzo),10,int(40-positzo),int(hgroesse-25));
    pcpm.setFont( QFont( "arial", 7, QFont::Normal, FALSE ) );
    pcpm.drawText( int(vgroesse-40),int(hgroesse-15), "1/Hz", -1 );
    pcpm.setFont( QFont( "arial", 8, QFont::Normal, FALSE ) );

    /* horizontal lines */
    pcpm.drawLine(int(35-positzo),int(hgroesse-30),int(vgroesse-10),int(hgroesse-30));
    pcpm.drawLine(int(35-positzo),10,int(40-positzo),10);
    powermin_m=(int)powerwert_m[pi_m];
    powermax_m=(int)powerwert_m[pi_m+1];
    int powsize=powermax_m-powermin_m;
    int abstand = pi_m/15;
    abstand=abstand/10;
    abstand=abstand*10;
    if (abstand < 10) abstand=10;

    /* draw scales */
    double abstand1 = (double)(abstand/zoom);
    double abstvgr  = (double)(abstand1*(vgroesse-50)/pi_m);
    double tempzo   = 0;
 
    /* draw scales */
    for (int i=0;i<pi_m;i++) {
      if (i<pi_m/(abstand/zoom)) {
	if (raster) {
	  if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(gray), 1, QPen::SolidLine));
	  else pcpm.setPen( QPen( QColor(darkGray), 1, QPen::SolidLine));
	  if (i>0) pcpm.drawLine(int(40+zoom*i*abstvgr-positzo),10,int(40+zoom*i*abstvgr-positzo),int(hgroesse-30));
	}
	if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(black), 1, QPen::SolidLine));
	else pcpm.setPen( QPen( QColor(white), 1, QPen::SolidLine));
	pcpm.drawLine(int(40+zoom*i*abstvgr-positzo),int(hgroesse-25),
		      int(40+zoom*i*abstvgr-positzo),int(hgroesse-30));
	pcpm.drawText( int(36+zoom*i*abstvgr-positzo),int(hgroesse-2), tr("%1").arg((double) pi_m/(i*abstand1)*trpre_m*2 ), 4 );
      }
    }
    
    int skalierung=1000;
    if (powermax_m<500000) skalierung=50000;
    else if (powermax_m<100000) skalierung=10000;
    else if (powermax_m<50000) skalierung=5000;
    else if (powermax_m<10000) skalierung=1000;
    else if (powermax_m<5000) skalierung=500;
    else if (powermax_m<1000) skalierung=100;
    else if (powermax_m<500) skalierung=50;
    pcpm.drawText ( int(3-positzo), 15, tr("%1").arg((double)powermax_m), 5 );
    pcpm.drawText ( int(3-positzo), int(hgroesse-25), tr("%1").arg((int)powermin_m), 5 );    
    pcpm.setPen(QColor( 0,150,0));
    hgroesse=hgroesse-40;

    /* raster zeichnen */
    double hgrneu = (double)(hgroesse/powsize);
    if (raster) {
      for (int i=1;i<=powermax_m;i++) {
	tempzo = hgroesse-(i-powermin_m)*hgrneu;
	/* senkrechte Skale */
	if ( fmod(i,skalierung)==0 ) {
	  if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor( gray ), 1, QPen::SolidLine) );
	  else pcpm.setPen( QPen( QColor( darkGray ), 1, QPen::SolidLine) );
	  /* horizontal line */
	  pcpm.drawLine(int(35-positzo),int(10+tempzo),int(vgroesse-10),int(10+tempzo));  
	  if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(black), 1, QPen::SolidLine));
	  else pcpm.setPen( QPen( QColor(white), 1, QPen::SolidLine));
	  pcpm.drawLine(int(35-positzo),int(10+tempzo),int(40-positzo),int(10+tempzo));
	  if (i<powermax_m-10) pcpm.drawText ( int(3-positzo), int(15+tempzo), tr("%1").arg(i), 5 );
	}
      }
    }

    /* Powerspectrum Kurve zeichnen */
    if (pr->beta && firespp) {
      pcpm.setPen( QPen( QColor( gray ), 2, QPen::SolidLine) );
      for (int i=1;i<pi_m;i++) {
	tempzo = zoom*i*(vgroesse-50)/pi_m-positzo;
	pcpm.drawLine(int(40+tempzo),int(10+hgroesse-((int)Sess_m[i]-powermin_m)*hgrneu),
		      int(40+zoom*(vgroesse-50)/pi_m+tempzo),int(10+hgroesse-((int)Sess_m[i+1]-powermin_m)*hgrneu));
      }
    }
    if (tilinep) {
      pcpm.setPen( QPen( QColor( red ), 2, QPen::SolidLine) );
      for (int i=1;i<pi_m;i++) {
	tempzo = zoom*i*(vgroesse-50)/pi_m-positzo;
	pcpm.drawLine(int(40+tempzo),int(10+hgroesse-((int)powerwert_m[i]-powermin_m)*hgrneu),
		      int(40+zoom*(vgroesse-50)/pi_m+tempzo),int(10+hgroesse-((int)powerwert_m[i+1]-powermin_m)*hgrneu));
      }
    }

  } else if (bolresp == 1) {


    if (pr->des) {

      /* TRIAL AVERAGE */
      //#########################################################################################
      
      
      float *ave = evresSE_m[0];
      float *sigma;
      int j=0;
      int triallength     = (int)rint(ave[0]);
      int trialresolution = (int)rint(ave[1]);
      float minimum       = (int)rint(ave[2]);
      float maximum       = (int)rint(ave[3]);
      double size=(double)(maximum-minimum);

      //zoom=1;
      if (zoom>8) zoom=8;
      double positzo  = (double)(posit*zoom);

      if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(gray), 1, QPen::SolidLine));
      else pcpm.setPen( QPen( QColor(darkGray), 1, QPen::SolidLine));
      pcpm.drawLine(int(40-positzo),10,int(40-positzo),int(hgroesse-25));        // senkrechte Linie
      pcpm.drawLine(int(35-positzo),int(hgroesse-30),int(vgroesse-10),int(hgroesse-30)); // waagerechte Linien
      if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(black), 1, QPen::SolidLine));
      else pcpm.setPen( QPen( QColor(white), 1, QPen::SolidLine));
      pcpm.drawLine(int(35-positzo),10,int(40-positzo),10);
      
      /* scalen zeichen */
      //vgroesse/50
      int abstand = (int)rint((double)triallength*(double)ave[4]/(double)1000/(double)trialresolution);
      if (abstand < 1000/trialresolution) abstand=1000/trialresolution;
      double vtriall  = (double)((vgroesse-50)/triallength);
      double abstand1 = (double)(abstand/zoom);
      double abstand2 = (double)(zoom*abstand1*vtriall);
      
      
      if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(black), 1, QPen::SolidLine));
      else pcpm.setPen( QPen( QColor(white), 1, QPen::SolidLine));
      pcpm.setFont( QFont( "arial", 7, QFont::Normal, FALSE ) );
      if ( vgroesse < 700 ) pcpm.drawText( int(vgroesse-90),int(hgroesse-15), "x10 seconds", -1 );
      else pcpm.drawText( int(vgroesse-60),int(hgroesse-15), "seconds", -1 );
      pcpm.setFont( QFont( "arial", 8, QFont::Normal, FALSE ) );
      
      for ( int i=0;i<(int)triallength;i++) {
	/* print scales */
	if (i<(int)triallength/abstand1) {
	  if (raster) {
	    if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(gray), 1, QPen::SolidLine));
	    else pcpm.setPen( QPen( QColor(darkGray), 1, QPen::SolidLine));
	    if (i>0) pcpm.drawLine(int(40+i*abstand2-positzo),10,int(40+i*abstand2-positzo),int(hgroesse-30));
	  }
	  if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(black), 1, QPen::SolidLine));
	  else pcpm.setPen( QPen( QColor(white), 1, QPen::SolidLine));
	  pcpm.drawLine(int(40+i*abstand2-positzo),int(hgroesse-25),int(40+i*abstand2-positzo),int(hgroesse-30));
	  if ( vgroesse < 700 ) {
	    pcpm.drawText( int(36+i*abstand2/zoom-positzo),int(hgroesse-2), tr("%1").arg((int)trialresolution/1000 *i*abstand1/10), -1 );
	  } else {
	    pcpm.drawText( int(36+i*abstand2-positzo),int(hgroesse-2), tr("%1").arg((float)trialresolution/1000 *i*abstand1), -1 );
	  }
	}
      }
      
      if (persip==1) {
	pcpm.drawText ( int(3-positzo), 15, tr("%1").arg((double)maximum/1000), 5 );
	//pcpm.drawText ( 40-positzo, 15, "%", 5 );
	pcpm.drawText ( int(3-positzo), int(hgroesse-25), tr("%1").arg((double)minimum/1000), 5 );
      } else {
	pcpm.drawText ( int(3-positzo), 15, tr("%1").arg((int)maximum), 5 );
	pcpm.drawText ( int(3-positzo), int(hgroesse-25), tr("%1").arg((int)minimum), 5 );
      }
      
      pcpm.setPen(QColor( 0,150,0));
      hgroesse=hgroesse-40;
      
      /* draw raster */
      if (raster && persip==0) 
	for ( int i=1;i<=maximum;i++) {
	  if ( i==(int)(size/5+minimum) || i==(int)(size/5*2+minimum) || i==(int)(size/5*3+minimum) || i==(int)(size/5*4+minimum) || i==(int)(size/5*5+minimum) ) {
	    int iminhgr=(int)rint((i-minimum)*hgroesse/size);
	    if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor( gray ), 1, QPen::SolidLine) );
	    else pcpm.setPen( QPen( QColor( darkGray ), 1, QPen::SolidLine) );
	    /* horizontal line */
	    pcpm.drawLine(int(35-positzo),int(10+hgroesse-iminhgr),int(vgroesse-10),int(10+hgroesse-iminhgr));
	    if (pr->hgfarbe[1]==1) pcpm.setPen( QPen( QColor(black), 1, QPen::SolidLine));
	    else pcpm.setPen( QPen( QColor(white), 1, QPen::SolidLine));
	    pcpm.drawLine(int(35-positzo),int(10+(hgroesse)-iminhgr),int(40-positzo),int(10+hgroesse-iminhgr));
	    
	    if (i<maximum-10) pcpm.drawText ( int(3-positzo), int(15+hgroesse-iminhgr), tr("%1").arg(i), 5 );
	  }
	}

      double hgrneu   = (double)(hgroesse/size);
      double vtriall1 = (double)(zoom*vtriall);
      for (j=1;j<=maxfarben;j++) {
	ave = evresSE_m[2*(j-1)+1];
	sigma = evresSE_m[2*(j-1)+2];
	if (cline[j]!=0) {
	  for (int i=0;i<(int)triallength-1;i++) {
	    if (tilinep) {
	      pcpm.setPen( QPen( bgfarbemidd[j], 2, QPen::SolidLine) );
	      pcpm.drawLine(int(40+i*vtriall1-positzo),int(10+hgroesse-((int)ave[i]-minimum)*hgrneu),
			    int(40+vtriall1+i*vtriall1-positzo),int(10+hgroesse-((int)ave[i+1]-minimum)*hgrneu));
	      pcpm.setPen( QPen( bgfarbemidd[j], 1, QPen::SolidLine) );
	      pcpm.drawLine(int(40+i*vtriall1-positzo),int(10+hgroesse-((int)ave[i]+sigma[i]-minimum)*hgrneu),
			    int(40+vtriall1+i*vtriall1-positzo),int(10+hgroesse-((int)ave[i+1]+sigma[i+1]-minimum)*hgrneu));
	      pcpm.drawLine(int(40+i*vtriall1-positzo),int(10+hgroesse-((int)ave[i]-sigma[i]-minimum)*hgrneu),
			    int(40+vtriall1+i*vtriall1-positzo),int(10+hgroesse-((int)ave[i+1]-sigma[i+1]-minimum)*hgrneu));
	    }
	  }
	}
      } 
      if (pr->beta) {
	for (j=1;j<=maxfarben;j++) {
	  ave = evres_m[2*(j-1)+1];
	  sigma = evres_m[2*(j-1)+2];
	  if (cline[j]!=0) {
	    for (int i=0;i<(int)triallength-1;i++) {
	      if (firespp) {
		pcpm.setPen( QPen( bgfarbemidd[j], 2, QPen::SolidLine) );
		pcpm.drawLine(int(40+i*vtriall1-positzo),int(10+hgroesse-((int)ave[i]-minimum)*hgrneu),
			      int(40+vtriall1+i*vtriall1-positzo),int(10+hgroesse-((int)ave[i+1]-minimum)*hgrneu));
		pcpm.setPen( QPen( bgfarbemidd[j], 1, QPen::SolidLine) );
		pcpm.drawLine(int(40+i*vtriall1-positzo),int(10+hgroesse-((int)ave[i]+sigma[i]-minimum)*hgrneu),
			      int(40+vtriall1+i*vtriall1-positzo),int(10+hgroesse-((int)ave[i+1]+sigma[i+1]-minimum)*hgrneu));
		pcpm.drawLine(int(40+i*vtriall1-positzo),int(10+hgroesse-((int)ave[i]-sigma[i]-minimum)*hgrneu),
			      int(40+vtriall1+i*vtriall1-positzo),int(10+hgroesse-((int)ave[i+1]-sigma[i+1]-minimum)*hgrneu));
	      }
	    }
	  }
	}   
      }



    }   
  }
  paint.drawPixmap(0, 0, cpm);
  paint.end();                                    // painting done

}

void RawPaint::mousePressEvent( QMouseEvent *e ) {
  if (e->button()==MidButton && zoom<16) {
    zoom=zoom*2;
  }
  else if (e->button()==RightButton && zoom>1) {
    zoom=zoom/2;
  }
  repaint();
  emit neuZoom(zoom);
}

void RawPaint::position( int pos ) {
  posit=pos;
  repaint();
}

void RawPaint::saveTrialAverageTC()
{
  saveTrialAverage(evresSE_m);
}

void RawPaint::saveTrialAverageFR()
{
  saveTrialAverage(evres_m);
}

void RawPaint::saveTrialAverage(float **evres11)
{
  double faktor=1;
  int len=0;
  float *wertta=NULL, *sigmata=NULL;
  FILE *fp1=NULL;
  QString fn = QFileDialog::getSaveFileName( QString::null, "*.txt", this );
  if ( !fn.isEmpty() ) {
    fp1 = fopen(fn,"w");
    if (fp1 != NULL) {
      fprintf(fp1,"# Trial average generated by Lipsia\n");
      fprintf(fp1,"# Raw data voxel: %1.0f %1.0f %1.0f\n",model_m[3],model_m[4],model_m[5]);
      fprintf(fp1,"# Anatomical voxel: %1.0f %1.0f %1.0f\n",model_m[0],model_m[1],model_m[2]);
      fprintf(fp1,"#\n");
      len = (int)rint((double)1000.0*(double)pr->triallength/(double)pr->trialresolution);
      if (pr->persi) faktor=1000.0;
      for (int l=0;l<cnd_m;l++) {
	fprintf(fp1,"#\n# Condition %d\n#\n",l+1);
	wertta = evres11[2*l+1];
	sigmata = evres11[2*l+2];
	for (int k=0;k<len;k++) fprintf(fp1,"   %5.2f   %10.5f   %10.5f\n",(double)k*(double)pr->trialresolution/1000.0,wertta[k]/faktor,sigmata[k]/faktor);
      }
    } 
    fclose(fp1);
  } else {
    // the user cancelled the dialog
  }
}

