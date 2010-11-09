/****************************************************************************
** $Id: CW.C 3181 2008-04-01 15:19:44Z karstenm $
**
*****************************************************************************/

/* Std. - Libraries */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Vista - Libraries */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include "CW.h"
#include "attrWin.h"

int rows=0, bands=0, columns=0;
int XX=1, YY=1, ZZ=1, imagesel=0;
ImageData *selImage;

CW::CW( QWidget *parent, const char *name, QValueList<ImageData*> data, QStringList namelist, QString cp )
  : QWidget( parent, name ), cp_m(cp), data_m(data)
{
  selImage = (ImageData*) data.first();
  prefs = new TabDialog ( this, "preferences" );

  rows=VImageNRows(selImage->image);
  columns=VImageNColumns(selImage->image);
  bands=VImageNBands(selImage->image);

  YY=(int)(rows/2);
  XX=(int)(columns/2);
  ZZ=(int)(bands/2);
  
  QBoxLayout *hlayout = new QHBoxLayout( this, 4, 6, "hlayout" );
  QBoxLayout *vlayout = new QVBoxLayout( hlayout, 1 );

  //--------------------------- linke seite

  QBoxLayout *h1layout = new QHBoxLayout( vlayout, 0 );

  QLabel *label1 = new QLabel ( this, "Object:" );
  label1->setText ( "Object:" );
  listbox = new QListBox ( this, "listbox" );
  listbox->insertStringList ( namelist, -1 ); 
  listbox->setCurrentItem ( 0 );
  listbox->setFocusPolicy ( QWidget::NoFocus);
  h1layout->add ( label1 ); 
  h1layout->add ( listbox );

  QBoxLayout *h2layout = new QHBoxLayout( vlayout, 0 );

  QLabel *label2 = new QLabel ( this, "Image:" );
  label2->setText ( "Image:  " );
  label21 = new QLabel ( this, "21" );
  label22 = new QLabel ( this, "22" );
  label23 = new QLabel ( this, "23" );
  label24 = new QLabel ( this, "24" );
  label25 = new QLabel ( this, "repn" );
  label21->setNum( rows );
  label22->setText ( "rows, " );
  label23->setNum( columns );
  label24->setText ( "columns, " );
  label25->setText( data_m.first()->rep );
  h2layout->add ( label2 ); 
  h2layout->add ( label21 ); 
  h2layout->add ( label22 ); 
  h2layout->add ( label23 ); 
  h2layout->add ( label24 ); 
  h2layout->add ( label25 ); 

  QBoxLayout *h3layout = new QHBoxLayout( vlayout, 0 );

  QLabel *label3 = new QLabel ( this, "Band:" );
  label3->setText ( "Band:" );
  label31 = new QLabel ( this, "wert" );
  label31->setNum( (int)(bands/2) );
  h3layout->add ( label3 );
  h3layout->add ( label31 );

  QBoxLayout *h30layout = new QHBoxLayout( vlayout, 0 );

  QLabel *label30 = new QLabel ( this, "0" );
  label30->setText ( "0" );
  label32 = new QLabel ( this, "max" );
  slider = new QSlider( Horizontal, this, "band" );
  label32->setNum( bands-1 );
  h30layout->add ( label30 ); 
  h30layout->add ( slider ); 
  h30layout->add ( label32 );

  QBoxLayout *h4layout = new QHBoxLayout( vlayout, 0 );

  QLabel *label4 = new QLabel ( this, "Pixel:  " );
  label400 = new QLabel ( this, "[" );
  label400->setText ( "[" );
  label401 = new QLabel ( this, "x" );
  label401->setText ( "0" );
  label41 = new QLabel ( this, "," );
  label41->setText ( "," );
  label42 = new QLabel ( this, "y" );
  label42->setText ( "0" );
  label43 = new QLabel ( this, "]:" );
  label43->setText ( "]: " );
  label44 = new QLabel ( this, "pixwert" );
  label4->setText ( "Pixel:" );
  label44->setText ( "0" );
  h4layout->add ( label4 );
  h4layout->add ( label400 );
  h4layout->add ( label401 );
  h4layout->add ( label41 );
  h4layout->add ( label42 );
  h4layout->add ( label43 );
  h4layout->add ( label44 );

  //--------------------------- rechte seite

  bild = new Bild( this, "bild" );
  hlayout->addWidget ( bild, 1 );
  
  slider->setRange( 0, bands-1 );
  slider->setValue( (int)(bands/2) );
 
  connect( slider, SIGNAL(valueChanged(int)), this, SLOT( ZZchanged(int) ) );
  connect( listbox, SIGNAL( selectionChanged () ), this, SLOT( ImageSelect() ) );
  connect( bild, SIGNAL( Wert (int, int, double) ), this, SLOT( Wert(int, int, double) ) );
  connect( prefs, SIGNAL(newColtype()), bild, SLOT(colorMap()));
  connect( this, SIGNAL(sliderwechsel()), prefs, SLOT(sliderwechsel()));

}

/* close all */
CW::~CW()
{
  //something TODO
}

void CW::ImageSelect() {
  imagesel = listbox->currentItem();
  
  selImage = data_m[imagesel];

  rows=VImageNRows(selImage->image);
  columns=VImageNColumns(selImage->image);
  bands=VImageNBands(selImage->image);

  YY=(int)(rows/2);
  XX=(int)(columns/2);
  ZZ=(int)(bands/2);
  bild->colorMap();
  label32->setNum( bands-1 );
  label21->setNum( rows );
  label23->setNum( columns );
  label25->setText( selImage->rep );
  slider->setRange( 0, bands-1 );
  slider->setValue( (int)(bands/2) );
  bild->repaint();
  
  emit sliderwechsel();
}

void CW::ZZchanged(int z) {
  ZZ=z;
  label31->setNum( z );
  bild->repaint(false);
}

void CW::Wert(int x, int y, double wert) {
  if (bild->hasMouse()) {
    label42->setNum( y );
    label401->setNum( x );
    label44->setNum( wert ); 
  } else {
    label42->setText( tr(" "));
    label401->setText( tr(" "));
    label44->setText( tr(" "));
  }
}

void CW::showAttributes() {
  BildWin *attr = new BildWin ( this, "attributes", cp_m );
  attr->resize(600,600);
  attr->show();
}

void CW::showPrefs() {
  prefs->show();
}

