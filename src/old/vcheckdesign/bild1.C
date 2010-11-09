/****************************************************************
 *
 * bild1.C
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
 * $Id: bild1.C 1166 2005-09-08 09:29:32Z karstenm $
 *
 *****************************************************************/


#include <qfont.h> 
#include <qinputdialog.h> 

#include "bild1.h"

#define DEBUGING 0

double maxwert=0.0, minwert=0.0;
QColor desfarbe[256];

Bild::Bild( QWidget *parent, const char *name, VImage src, VImage src1)
  : QWidget( parent, name ), src_m(src), src1_m(src1)


{
  //  if (DEBUGING) qWarning( tr("initialize view %1").arg(typ) );
  
  setPalette( QPalette( QColor( 0,0,0) ) );
  QWidget::setMouseTracking ( TRUE );

  effect_m=VImageNRows(src_m);
  scan_m=VImageNColumns(src_m);

  for (int i=0; i<effect_m; i++) {
    for (int j=0; j<scan_m; j++) {
      if (VPixel(src1_m,0,i,j,VFloat) > maxwert) maxwert=VPixel(src1_m,0,i,j,VFloat);
      else if (VPixel(src1_m,0,i,j,VFloat) < minwert) minwert=VPixel(src1_m,0,i,j,VFloat);
    }
  }  

  colorMap(); 
}

void Bild::colorMap() {
  for (int i=0; i<256; i++) {
    desfarbe[i]=QColor(i,i,i);
  }
}

void Bild::paintEvent( QPaintEvent * )
{
  double vgroesse=width(), hgroesse=height();
  
  // int horplus=0, verplus=0;

  int hgfarbe[2];
  hgfarbe[0]=1;

  if (hgfarbe[0]==0) setPalette( QPalette( QColor( 0,0,0) ) );
  else setPalette( QPalette( QColor( 255,255,255) ) );
  
  QPainter paint( this );
  
  paint.setFont( QFont( "courier", 8, QFont::Bold ) );
  paint.setPen(QColor(black));
  paint.drawLine(0, 0, (int) vgroesse,0);
  paint.drawLine(0, 0, 0, (int) hgroesse);

  paint.drawRect(80, 30, int (vgroesse-100), int (hgroesse-50));
  
  //  fprintf(stderr,"test0");
  for (int i=0; i<effect_m; i++) {

    /* obere beschriftung */
    paint.drawText(int (80+i*(vgroesse-100)/effect_m), 20, tr("Col %1").arg(i), 20);

    for (int j=0; j<scan_m; j++) {
      paint.fillRect(int (80+i*(vgroesse-100)/effect_m), int (30+j*(hgroesse-50)/scan_m), int ((vgroesse-100)/effect_m+1), int ((hgroesse-50)/scan_m+1), desfarbe[ (int)((-minwert+  VPixel(src1_m,0,i,j,VFloat) )*255/(maxwert-minwert)) ] );

      /* linke scale mit beschriftung */
      if ( fmod(j,50)==0 ) {
	paint.drawLine (70, int (30+j*(hgroesse-50)/scan_m), 80, int(30+j*(hgroesse-50)/scan_m));
	paint.drawLine (int (vgroesse-20), int (30+j*(hgroesse-50)/scan_m), int (vgroesse-15), int(30+j*(hgroesse-50)/scan_m));
	paint.drawText (20, int (30+j*(hgroesse-50)/scan_m+5), tr("%1").arg(j), 10 );
      }
    }
  }

}

void Bild::repaintf()
{
  repaint( 0,0,width(),height(),FALSE);
}

QSizePolicy Bild::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

void Bild::mouseMoveEvent( QMouseEvent *e ) {
  int x = e->pos().x();
  int y = e->pos().y();
  int xx, yy;
  double ausgabe=0;
  //  if (x>)
  if ( x >= 80 && x <= width()-20 && y >= 30 && y <= height()-20 ) {
    setCursor( crossCursor );
    xx = (int)( (double)(x-80)/(double)(width()-100)*effect_m );
    yy = (int)( (double)(y-30)/(double)(height()-50)*scan_m );
    if (xx<VImageNRows(src_m) && yy<VImageNColumns(src_m))
      ausgabe = VPixel(src_m,0,xx,yy,VFloat);
    else
      ausgabe=0;
    if ( ausgabe < 0.0001 && ausgabe > -0.0001 ) ausgabe=0.0;
  } else {
    setCursor( pointingHandCursor );
    xx = 0;
    yy = 0;
    ausgabe=0.0;
  }
  emit wertView(ausgabe);
}
