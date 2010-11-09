/****************************************************************
**
** Implementation Bild class
** (c) 1999 by Heiko Mentzel
**
****************************************************************/


#include <qfont.h> 
#include <qinputdialog.h> 
#include "bild1.h"
#include "imagedata.h"

#define DEBUGING 0
#define NP -32768
#define MFT 65535


int XXplus=0;
int YYplus=0;
int ZZplus=0;

extern int atlas, rows, columns, bands;
int colortable=NULL;
extern ImageData* selImage;

Bild::Bild( QWidget *parent, const char *name )
        : QWidget( parent, name ) {
 
  setPalette( parent->palette() );   
  setCursor( crossCursor );
  
  QWidget::setMouseTracking ( TRUE );
  
  rgbfarbe = (QColor *) malloc(sizeof(QColor) * (MFT + 1));

  int mycol=0;
  for (int i=0; i<MFT; i++ ) {
    mycol=(int)rint((double)((float)(i+NP - selImage->anamean)*selImage->anaalpha));
    if (mycol>255) mycol=255;
    if (mycol<0) mycol=0;
    rgbfarbe[i].setRgb( mycol, mycol, mycol );
  }
}

void Bild::paintEvent( QPaintEvent * )
{
  double vgroesse=width(), hgroesse=height();
  int horplus=0, verplus=0;
  QPainter paint( this );
  
  //double farbe1=0;  // unused
  int pixelvalue = 0;
  
  VImage tmp = selImage->image;
  VRepnKind pixel_repn = VPixelRepn(tmp);
  double fact = 255.0/(selImage->maxwert-selImage->minwert);
  
  image.create ( columns, rows, 32);
    
  if (ZZ<bands)
		for (int i=0; i<rows; i++) {
			for (int j=0; j<columns; j++) {
				if (pixel_repn == VUByteRepn)
					pixelvalue = (int)VPixel(tmp,ZZ,i,j,VUByte);
				if (pixel_repn == VShortRepn)
					pixelvalue = (int)VPixel(tmp,ZZ,i,j,VShort);
				if (pixel_repn == VBitRepn)
					pixelvalue = (int)(fact*VPixel(tmp,ZZ,i,j,VBit));
				if (pixel_repn == VFloatRepn)
					pixelvalue = (int)(fact*VPixel(tmp,ZZ,i,j,VFloat));
				if (pixel_repn == VDoubleRepn)
					pixelvalue = (int)(fact*VPixel(tmp,ZZ,i,j,VDouble));
				image.setPixel(j, i, rgbfarbe[pixelvalue-NP].rgb());
				/*
				  if (pixelvalue!=0)
				  image.setPixel(j, i, rgbfarbe[pixelvalue-NP].rgb());
				  else
				  image.setPixel(j, i, rgbfarbe[0].rgb() );
				*/
			}
		}
	sc1=vgroesse/columns;
	sc2=hgroesse/rows;
  
  if (sc1<sc2)
    paint.scale(sc1,sc1);
  else 
    paint.scale(sc2,sc2);

  paint.drawImage(0, 0, image, -verplus, -horplus );
}

void Bild::mouseMoveEvent( QMouseEvent *e )
{
  int x, y, z;
  z=ZZ;
  if (sc1 >= sc2) {
    y=(int)(e->pos().y()/(sc2));
    x=(int)(e->pos().x()/(sc2));
  } else {
    y=(int)(e->pos().y()/(sc1));
    x=(int)(e->pos().x()/(sc1));
  }
  if (x>=columns-1) x=columns-1;
  if (y>=rows-1) y=rows-1;

  if (y>=0 && y<=rows && x>=0 && x<=columns)
    emit Wert( x, y, (double)VGetPixel( selImage->image,z,y,x ) );
}

void Bild::colorMap() {
  int mycol=0;
  for (int i=0; i<MFT; i++ ) {
    mycol=(int)rint((double)((float)(i+NP - selImage->anamean)*selImage->anaalpha));
    if (mycol>255) mycol=255;
    if (mycol<0) mycol=0;
    rgbfarbe[i].setRgb( mycol, mycol, mycol );
  }

  repaintf();
}

void Bild::repaintf() {
  recreate=1;
  repaint( 0,0,width(),height(),FALSE);
}

void Bild::leaveEvent ( QEvent *)
{
  emit Wert( 0, 0, 0.0 );
}
