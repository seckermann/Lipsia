/****************************************************************
 *
 * bild3.C
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
 * $Id: bild3.C 2023 2007-03-13 15:07:58Z karstenm $
 *
 *****************************************************************/


#include <qfont.h> 
#include <qinputdialog.h> 
#include <math.h>
#include <fftw3.h>
#include "bild3.h"

#define TEST 1

extern int condanz, condition;
extern double minwert, maxwert;
double powmin, powmax;
double *powsum=NULL;
int *powthre=NULL;

extern QColor desfarbe[256];



double condmin, condmax, ortmin, ortmax;

fftw_plan p1=NULL;
fftw_complex *out;
double *in,*powspec=NULL,*powspec_g=NULL;



Bild3::Bild3( QWidget *parent, const char *name, double repetition_time, int mode, int sessanz, int *sesslength, int *sesscov, VImage src, VImage src1, VImage orth )
  : QWidget( parent, name ), TR(repetition_time), mode_m(mode), sessanz_m(sessanz), sesslength_m(sesslength), sesscov_m(sesscov), src_m(src), src1_m(src1), orth_m(orth) {

  setPalette( QPalette( QColor( 0,0,0) ) );
  QWidget::setMouseTracking ( TRUE );

  getSessionData(0);
  zoom1=1, posit1=0;
  zoom2=1, posit2=0;

  //fprintf(stderr,"starting bild3...\n");
}

void Bild3::getSessionData(int session) {

  ses_1=sesslength_m[session];
  ses_2=sesscov_m[session];   
  begin_2=0, begin_1=0;
  for (int i=0; i<session; i++) {
    begin_2 +=  sesscov_m[i];
    begin_1 +=  sesslength_m[i];
  }

  /* Normierung für Kurven */
  condmin=0.0;
  condmax=0.0;
  for (int i=0; i<ses_2-1; i++) {
    for (int j=0; j<ses_1; j++) {
      if (VPixel(src_m,0,i+begin_2,j+begin_1,VFloat)*1000<condmin) 
	condmin=VPixel(src_m,0,i+begin_2,j+begin_1,VFloat)*1000;
      if (VPixel(src_m,0,i+begin_2,j+begin_1,VFloat)*1000>condmax) 
	condmax=VPixel(src_m,0,i+begin_2,j+begin_1,VFloat)*1000;
    }
  }



  // POWERSPECTRUM

  if (out!=NULL) free(out);
  if (powspec!=NULL) free(powspec);
  if (powspec_g!=NULL) free(powspec_g);
  if (p1!=NULL) free(p1);
  if (powsum!=NULL) free(powsum);
  if (powthre!=NULL) free(powthre);

  out = (fftw_complex *) fftw_malloc (sizeof (fftw_complex ) * ses_1);
  in        = (double *) VMalloc(sizeof(double) * ses_1);
  powspec   = (double *) VMalloc(sizeof(double) * ((int)(ses_1/2) + 1));
  powspec_g = (double *) VMalloc(sizeof(double) * ses_2 * ((int)(ses_1/2) + 1));
  powsum    = (double *) VMalloc(sizeof(double) * ses_2);
  powthre   = (int    *) VMalloc(sizeof(int)    * ses_2);

  /* create plan */
  p1 = fftw_plan_dft_r2c_1d (ses_1,in,out,FFTW_ESTIMATE);


  for (int i=0;i<ses_2-1;i++) {
    for (int j=0;j<ses_1;j++) {
      in[j]=(double)VPixel(src_m,0,i+begin_2,j+begin_1,VFloat);
    }
    fftw_execute(p1);
    
    for (int k = 1; k < ((int)(ses_1/2) + 1); k++)
      powspec[k] = out[k][0] * out[k][0] + out[k][1] * out[k][1];
    powspec[0] = 0.0;

    for (int j=0;j<ses_1/2;j++) {
      powspec_g[i*ses_1/2+j]=powspec[j];
    }
  }

  powmin=0.0;
  powmax=0.0;
  for (int i=0; i<ses_2-1; i++) {
    powsum[i] = 0.0;
    for (int j=1; j<(int)(ses_1/2)-1; j++) {
      if (powspec_g[i*(ses_1/2)+j]<powmin) powmin=powspec_g[i*(ses_1/2)+j];
      if (powspec_g[i*(ses_1/2)+j]>powmax) powmax=powspec_g[i*(ses_1/2)+j];
      if (j > (int)(ses_1/60)) powsum[i] += powspec_g[i*(ses_1/2)+j];
    }
  }
  for (int i=0; i<ses_2-1; i++) {
    double powersumme=0.0;
    for (int j=(int)(ses_1/60); j<(int)(ses_1/2)-1; j++) {
      powersumme += powspec_g[i*(ses_1/2)+j];
      if (powersumme > powsum[i] * 0.05) {
	powthre[i]=j-1;
	break;
      }
    }
  }

  /* Normierung fuer orth */
  ortmin=0.0;
  ortmax=0.0;
  for (int i=0; i<ses_2; i++) {
    for (int j=0; j<ses_2; j++) {
      if (VPixel(orth_m,0,i+begin_2,j+begin_2,VFloat)<ortmin) 
	ortmin=VPixel(orth_m,0,i+begin_2,j+begin_2,VFloat);
      if (VPixel(orth_m,0,i+begin_2,j+begin_2,VFloat)>ortmax) 
	ortmax=VPixel(orth_m,0,i+begin_2,j+begin_2,VFloat);
    }
  }
  repaint();
}

void Bild3::paintEvent( QPaintEvent * ) {
  double vgroesse=width(), hgroesse=height();

  int hgfarbe[2];
  hgfarbe[0]=1;

  if (hgfarbe[0]==0) setPalette( QPalette( QColor( 0,0,0) ) );
  else setPalette( QPalette( QColor( 255,255,255) ) );
  
  QPainter paint( this );

  //double positzo  = (double)(posit*zoom);
  //double tempzo   = 0;

  paint.setFont( QFont( "courier", 8, QFont::Bold ) );
  paint.setPen(QColor(black));
  paint.drawLine(0,0,(int) vgroesse,0);
  paint.drawLine(0,0,0,(int) hgroesse);
  
  if (mode_m==1) {
    for (int i=0; i<ses_2; i++) {
      
      for (int j=0; j<ses_1; j++) {
	/* ansicht2 */
	paint.fillRect(int (130+j*(vgroesse-150)/ses_1), int (30+i*(hgroesse-50)/ses_2),int ((vgroesse-150)/ses_1+1), int ((hgroesse-50)/ses_2+1), desfarbe[ (int)((-minwert+  VPixel(src1_m,0,i+begin_2,j+begin_1,VFloat) )*(double)255/(maxwert-minwert)) ] );
	
	/* obere scale mit beschriftung für ansicht2 */
	if ( fmod(j,50)==0 ) {
	  paint.drawLine (int (130+j*(vgroesse-150)/ses_1) ,25, int (130+j*(vgroesse-150)/ses_1) ,30);
	  paint.drawLine (int (130+j*(vgroesse-150)/ses_1), int (hgroesse-20), int (130+j*(vgroesse-150)/ses_1), int (hgroesse-15));
	  paint.drawText (int (130+j*(vgroesse-150)/ses_1-3), 20, tr("%1").arg(j), 10 );
	}
      }
    }
    /* linke scale mit beschriftung für ansicht2 */
    for (int i=0; i<ses_2; i++) 
      paint.drawText ( 10, int (30+(i+0.5)*(hgroesse-50)/ses_2+5), tr("Col %1").arg(i), 16 );




  } else if (mode_m==2) {
    paint.setPen(QPen(QColor(lightGray), 1, QPen::SolidLine));
    paint.drawRect(80,30,int (vgroesse-100), int (hgroesse-50));
    
    //fprintf(stderr,"condmin: %f condmax: %f condmax-condmin: %f\n", condmin, condmax, condmax-condmin);

    paint.setPen( QPen(QColor(black), 1, QPen::SolidLine) );
    /* linke scale mit beschriftung */
    for (int i=(int)condmin; i<(int)condmax; i++ ) {
      if (i==0) paint.drawLine(70,int (30+(hgroesse-50)-(i-(int)condmin)*(hgroesse-50)/(int)(condmax-condmin)), int ((vgroesse-15)), int (30+(hgroesse-50)-(i-(int)condmin)*(hgroesse-50)/(int)(condmax-condmin)));
      if ( fmod(i,(int)((condmax-condmin)/10))==0 ) {
	paint.drawLine(70, int (30+(hgroesse-50)-(i-(int)condmin)*(hgroesse-50)/(int)(condmax-condmin)),80, int (30+(hgroesse-50)-(i-(int)condmin)*(hgroesse-50)/(int)(condmax-condmin)));
	paint.drawLine(int (vgroesse-20), int (30+(hgroesse-50)-(i-(int)condmin)*(hgroesse-50)/(int)(condmax-condmin)), int (vgroesse-15), int (30+(hgroesse-50)-(i-(int)condmin)*(hgroesse-50)/(int)(condmax-condmin)));
	paint.drawText ( 20, int (30+(hgroesse-50)-(i-(int)condmin)*(hgroesse-50)/(int)(condmax-condmin)+5), tr("%1").arg( (double)i/(double)1000 ), 8 );
      }
    }

    QColor stift;
    for (int i=0; i<ses_2-1; i++) {
      for (int j=0; j<ses_1-posit1; j++) {
	stift.setHsv( int ((360/(ses_2+2)+1) * (i)), 255,  255);
	paint.setPen( QPen(stift, 2, QPen::SolidLine) );
	/* kurven zeichnen */
	  paint.drawLine( 
			 int (80+zoom1*(j)*(vgroesse-100)/ses_1), int (30+(hgroesse-50)-( VPixel(src_m,0,i+begin_2,j+begin_1+posit1,VFloat)*1000-condmin)*(hgroesse-50)/(condmax-condmin)),
			 int (80+zoom1*(j+1)*(vgroesse-100)/ses_1), int (30+(hgroesse-50)-(  VPixel(src_m,0,i+begin_2,j+1+begin_1+posit1,VFloat)*1000-condmin)*(hgroesse-50)/(condmax-condmin))
			 );
     
	

      }
    }
    for (int j=0; j<ses_1; j++) {
      paint.setPen( QPen(QColor(black), 1, QPen::SolidLine) );
      if ( fmod(j,(int)rint((double)(ses_1/10/zoom1)))==0 ) {
	paint.drawLine (int (80+zoom1*(j-posit1)*(vgroesse-100)/ses_1),25, int (80+zoom1*(j-posit1)*(vgroesse-100)/ses_1) ,30 );
	paint.drawLine (int (80+zoom1*(j-posit1)*(vgroesse-100)/ses_1), int (hgroesse-20), int (80+zoom1*(j-posit1)*(vgroesse-100)/ses_1), int (hgroesse-15));
	paint.drawText (int (80+zoom1*(j-posit1)*(vgroesse-100)/ses_1-3), 23, tr("%1").arg((int)(j*TR)), 10 );
      }
    }     
    paint.drawText (int (80+(ses_1/2)*(vgroesse-100)/ses_1), 10, tr("%1").arg("- time in seconds -") );
    
  } else if (mode_m==3) {
    
    paint.setPen(QPen(QColor(lightGray), 1, QPen::SolidLine));
    paint.drawRect(80,30,int (vgroesse-100), int (hgroesse-50));

    //fprintf(stderr,"%f %f\n", condmin, condmax);
    //fprintf(stderr,"powmin: %f powmax: %f powmax-powmin: %f\n", powmin, powmax, powmax-powmin);

    paint.setPen( QPen(QColor(black), 1, QPen::SolidLine) );
    // linke scale mit beschriftung 
    for (int i=(int)powmin; i<(int)powmax; i++ ) {
      if (i==0) paint.drawLine(70,int (30+(hgroesse-50)-(i-(int)powmin)*(hgroesse-50)/(int)(powmax-powmin)), int (vgroesse-15), int (30+(hgroesse-50)-(i-(int)powmin)*(hgroesse-50)/(int)(powmax-powmin)));
      if ( fmod(i,(int)((powmax-powmin)/10))==0 ) {
	paint.drawLine(70, int (30+(hgroesse-50)-(i-(int)powmin)*(hgroesse-50)/(int)(powmax-powmin)),80, int (30+(hgroesse-50)-(i-(int)powmin)*(hgroesse-50)/(int)(powmax-powmin)));
	paint.drawLine(int (vgroesse-20), int (30+(hgroesse-50)-(i-(int)powmin)*(hgroesse-50)/(int)(powmax-powmin)), int (vgroesse-15), int (30+(hgroesse-50)-(i-(int)powmin)*(hgroesse-50)/(int)(powmax-powmin)));
	paint.drawText ( 20, int (30+(hgroesse-50)-(i-(int)powmin)*(hgroesse-50)/(int)(powmax-powmin)+5), tr("%1").arg( (double)i/1000 ), 5 );
      }
    }
    

    QColor stift;
    double maxcut=0.0;
    for (int i=0; i<ses_2-1; i++) {
      int sw3=1;
      for (int j=1; j<(int)(ses_1/2)-1-posit2; j++) {
	stift.setHsv( int ((double)(360/(ses_2+2)+1) * (i)), 255,  255);
	paint.setPen( QPen(stift, 2, QPen::SolidLine) );
	// kurven zeichnen
	paint.drawLine( 
		       int (80+zoom2*(j)*(vgroesse-100)/(ses_1/2)), int (30+(hgroesse-50)-(powspec_g[i*(ses_1/2)+j+posit2]-powmin)*(hgroesse-50)/(powmax-powmin)),
		       int (80+zoom2*(j+1)*(vgroesse-100)/(ses_1/2)), int (30+(hgroesse-50)-(powspec_g[i*(ses_1/2)+j+1+posit2]-powmin)*(hgroesse-50)/(powmax-powmin))
		       );
	if (j > powthre[i] && sw3>0) {
	  sw3=0;
	  paint.setPen( QPen(stift, 2, QPen::DashDotDotLine) );
	  paint.drawLine (int (80+zoom2*(int)((double)(j-posit2)*((double)vgroesse-100.0)/(double)(ses_1/2))),30, int (80+zoom2*(int)((double)(j-posit2)*((double)vgroesse-100.0)/(double)(ses_1/2))), int(30+(hgroesse-50)) );
	  paint.setPen( QPen(stift, 10, QPen::SolidLine) );
	  //paint.drawText (int (80+zoom2*(int)((double)(j-posit2)*((double)vgroesse-100.0)/(double)(ses_1/2))-3), 10 , tr("%1").arg((double)(ses_1)/j*TR), 4 );
	  paint.setFont( QFont( "courier", 20, QFont::Bold ) );
	  paint.drawText (int (80+zoom2*(int)((double)(j-posit2)*((double)vgroesse-100.0)/(double)(ses_1/2))-65), 60+25*i , tr("%1").arg((double)(ses_1)/j*TR), 4);
	  paint.setFont( QFont( "courier", 8, QFont::Bold ) );

	  if (maxcut < (double)((ses_1)/j*TR)) maxcut=(double)((ses_1)/j*TR);
	}
      }
    }
    //paint.setPen( QPen(QColor(black), 5, QPen::SolidLine) );
    //paint.drawText ( (int)(80), (int)(hgroesse-5), tr("Recommended Cutoff: %1 seconds").arg((double)maxcut), 50 );
    //emit sendMaxCut(maxcut);

      for (int j=0; j<ses_1/2; j++) {
	// obere scale mit beschriftung für ansicht2
	paint.setPen( QPen(QColor(black), 1, QPen::SolidLine) );
	if ( fmod(j,(int)rint((double)(ses_1/20/zoom2)))==0 ) {
	  paint.drawLine (int (80+zoom2*(int)((double)(j-posit2)*((double)vgroesse-100.0)/(double)(ses_1/2))),25, int (80+zoom2*(int)((double)(j-posit2)*((double)vgroesse-100.0)/(double)(ses_1/2))),30 );
	  paint.drawLine (int (80+zoom2*(int)((double)(j-posit2)*((double)vgroesse-100.0)/(double)(ses_1/2))), int (hgroesse-20), int (80+zoom2*(int)((double)(j-posit2)*((double)vgroesse-100.0)/(double)(ses_1/2))), int (hgroesse-15));
	  paint.drawText (int (80+zoom2*(int)((double)(j-posit2)*((double)vgroesse-100.0)/(double)(ses_1/2))-3), 20, tr("%1").arg((double)(ses_1)/j*TR), 4 );
	}
    }
  } else if (mode_m==4) { // +++++ ORTHOGONALITY +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


    for (int i=0; i<ses_2; i++) {
      for (int j=0; j<ses_2; j++) {
	// ansicht2
	paint.fillRect(int (130+j*(vgroesse-150)/ses_2), int (30+i*(hgroesse-50)/ses_2), int ((vgroesse-150)/ses_2+1), int ((hgroesse-50)/ses_2+1), desfarbe[ (int)((-ortmin+ VPixel(orth_m,0,i+begin_2,j+begin_2,VFloat))*(double)255/(ortmax-ortmin)) ] );
	
      }
    }   

    // linke scale mit beschriftung für ansicht2
    for (int i=0; i<ses_2; i++) {
      //  paint.drawText ( 10, 30+i*(hgroesse-50)/6+5, tr("%1").arg(sesnames[i]), 10 );
      paint.drawText ( 10, int (30+(i+0.5)*(hgroesse-50)/ses_2+5), tr("Column %1").arg(i), 16 );
    }
    
  } 
}

void Bild3::repaintf()
{
  repaint( 0,0,width(),height(),FALSE);
}

QSizePolicy Bild3::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

void Bild3::mouseMoveEvent( QMouseEvent *e ) {
  int x = e->pos().x();
  int y = e->pos().y();
  int xx, yy;
  double ausgabe=0.0;
  //  if (x>)
  if ( x >= 130 && x <= width()-20 && y >= 30 && y <= height()-20 ) {
    setCursor( crossCursor );
    if (mode_m==1) {
      xx = (int)( (double)(x-130)/(double)(width()-150)*ses_1 );
      yy = (int)( (double)(y-30)/(double)(height()-50)*ses_2 );
      if (yy+begin_2<VImageNRows(src_m) && xx+begin_1<VImageNColumns(src_m))
	ausgabe = VPixel(src_m,0,yy+begin_2,xx+begin_1,VFloat);
      else
	ausgabe = 0;
      if ( ausgabe < 0.0001 && ausgabe > -0.0001 ) ausgabe=0.0;
    } else if (mode_m==4) {
      xx = (int)( (double)(x-130)/(double)(width()-150)*ses_2 );
      yy = (int)( (double)(y-30)/(double)(height()-50)*ses_2 );
      if (yy+begin_2<VImageNRows(orth_m) && xx+begin_2<VImageNColumns(orth_m))
	ausgabe = VPixel(orth_m,0,yy+begin_2,xx+begin_2,VFloat);
      else
	ausgabe = 0;
      if ( ausgabe < 0.0001 && ausgabe > -0.0001 ) ausgabe=0.0;
      if (ausgabe >= 0.0)
	ausgabe=1.0-ausgabe;
      else 
	ausgabe=-1.0-ausgabe;
    }
  } else {
    setCursor( pointingHandCursor );
    xx = 0;
    yy = 0;
    ausgabe=0.0;
  }
  emit wertView(ausgabe);
}

void Bild3::mousePressEvent( QMouseEvent *e ) {

  if (e->button()==MidButton && zoom1<16 && mode_m==2)
    zoom1=zoom1*2;
  if (e->button()==RightButton && zoom1>1 && mode_m==2)
    zoom1=zoom1/2;
  
  if (e->button()==MidButton && zoom2<16 && mode_m==3)
    zoom2=zoom2*2;
  if (e->button()==RightButton && zoom2>1 && mode_m==3)
    zoom2=zoom2/2;

  repaint();
  if (mode_m==2) emit neuZoom1(zoom1);
  if (mode_m==3) emit neuZoom2(zoom2);
}

void Bild3::position1( int pos ) {
  posit1=pos;
  repaint();
}

void Bild3::position2( int pos ) {
  posit2=pos;
  repaint();
}
