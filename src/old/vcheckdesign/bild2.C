/****************************************************************
 *
 * bild2.C
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
 * $Id: bild2.C 2022 2007-03-13 14:57:03Z karstenm $
 *
 *****************************************************************/


#include <qfont.h> 
#include <qinputdialog.h> 

#include "bild2.h"

#define DEBUGING 0

double maxwert2=0.0, minwert2=0.0;
int laenge, breite, laenge2, breite2;

Bild2::Bild2( QWidget *parent, const char *name, double repetition_time, VImage gamma)
  : QWidget( parent, name ),  TR(repetition_time), gamma_m(gamma)
{
  //  if (DEBUGING) qWarning( tr("initialize view %1").arg(typ) );
  
  setPalette( QPalette( QColor( 0,0,0) ) );
  QWidget::setMouseTracking ( TRUE );

  getSessionData(0);

}

void Bild2::getSessionData(int session) {

  /* Minimum und Maximum der Kurven */
  minwert2=100000000;
  maxwert2=-100000000;
  for (int i=1; i<VImageNRows(gamma_m); i++ ) {
    for (int j=0; j<VImageNColumns(gamma_m); j++ ) {
      if (VPixel(gamma_m,0,i,j,VFloat)*10000>maxwert2) maxwert2=VPixel(gamma_m,0,i,j,VFloat)*10000;
      if (VPixel(gamma_m,0,i,j,VFloat)*10000<minwert2) minwert2=VPixel(gamma_m,0,i,j,VFloat)*10000;
    }
  }
  //fprintf(stderr,"min: %f max: %f\n",minwert2,maxwert2);
  //fprintf(stderr,"Rows: %d, Cols: %d\n",VImageNRows(gamma_m),VImageNColumns(gamma_m));

  laenge=VImageNColumns(gamma_m);
  breite=VImageNRows(gamma_m)-1;

  repaint();
}

void Bild2::paintEvent( QPaintEvent * ) {
  double vgroesse=width(), hgroesse=height();
  
  // int horplus=0, verplus=0;

  int hgfarbe[2];
  hgfarbe[0]=1;

  if (hgfarbe[0]==0) setPalette( QPalette( QColor( 0,0,0) ) );
  else setPalette( QPalette( QColor( 255,255,255) ) );
  
  QPainter paint( this );
  
  paint.setFont( QFont( "courier", 8, QFont::Bold ) );

  paint.setPen(QPen(QColor(lightGray), 1, QPen::SolidLine));
  paint.drawRect(80,30,int(vgroesse-100),int(hgroesse-50));


  double geswert2=maxwert2-minwert2;

  paint.setPen( QPen(QColor(black), 1, QPen::SolidLine) );
  paint.drawLine(0,0,int(vgroesse),0);
  paint.drawLine(0,0,0,int(hgroesse));

  /* linke scale mit beschriftung */
  for (int i=(int)minwert2; i<(int)maxwert2; i++ ) {
    if (i==0) paint.drawLine(70,int(30+(hgroesse-50)-(i-minwert2)*(hgroesse-50)/geswert2),int((vgroesse-15)),int(30+(hgroesse-50)-(i-minwert2)*(hgroesse-50)/geswert2));
    if ( fmod(i,200)==0 ) {
      paint.drawLine(70,int(30+(hgroesse-50)-(i-minwert2)*(hgroesse-50)/geswert2),80,int(30+(hgroesse-50)-(i-minwert2)*(hgroesse-50)/geswert2));
      paint.drawLine(int((vgroesse-20)),int(30+(hgroesse-50)-(i-minwert2)*(hgroesse-50)/geswert2),int((vgroesse-15)),int(30+(hgroesse-50)-(i-minwert2)*(hgroesse-50)/geswert2));
      paint.drawText ( 20, int(30+(hgroesse-50)-(i-minwert2)*(hgroesse-50)/geswert2+5), tr("%1").arg( (double)i/10000), 5 );
    }
  }
  

  QColor stift;
  for (int i=1; i<VImageNRows(gamma_m); i++ ) {
    stift.setHsv(int((360/(breite+2)+1) * (i)), 255,  255);
    paint.setPen( QPen(stift, 2, QPen::SolidLine) );
    for (int j=0; j<VImageNColumns(gamma_m)-1; j++ ) {
      /* kurven zeichnen */
      paint.drawLine(int(80+(j)*(vgroesse-100)/laenge),int(30+(hgroesse-50)-( (int)(VPixel(gamma_m,0,i,j,VFloat)*10000)-minwert2)*(hgroesse-50)/geswert2),int(80+(j+1)*(vgroesse-100)/laenge),int(30+(hgroesse-50)-( (int)(VPixel(gamma_m,0,i,j+1,VFloat)*10000) -minwert2)*(hgroesse-50)/geswert2));  
    }
  }

  for (int j=0; j<(int)laenge; j++ ) {
    // x-achse beschriftung 
    paint.setPen( QPen(QColor(black), 1, QPen::SolidLine) );
    paint.drawLine(int(80+(0)*(vgroesse-100)/laenge),(30),int(80+(0)*(vgroesse-100)/laenge),int(hgroesse-20));
    if ( fmod(j,10)==0 ) {
      paint.drawLine(int(80+(j)*(vgroesse-100)/laenge),(25),int(80+(j)*(vgroesse-100)/laenge),(30));
      paint.drawLine(int(80+(j)*(vgroesse-100)/laenge),int((hgroesse-20)), int(80+(j)*(vgroesse-100)/laenge),int(hgroesse-15));
      paint.drawText (int(77+(j)*(vgroesse-100)/laenge), 20, tr("%1").arg( VPixel(gamma_m,0,0,j,VFloat)  ), 4);
    }
  }
  
  stift.setHsv(int((360/(breite+2)+1) * (breite)), 255,  255);
  paint.setPen( QPen(stift, 2, QPen::SolidLine) );
  

 
  

}

void Bild2::repaintf()
{
  repaint( 0,0,width(),height(),FALSE);
}

QSizePolicy Bild2::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

