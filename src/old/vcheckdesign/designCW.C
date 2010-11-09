/****************************************************************
 *
 * designCW.C
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
 * $Id: designCW.C 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

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

#include "designCW.h"

int lipstyle=1, udb=1;

QString origconfpath="", idsprog="", refpath="";

designCW::designCW( QWidget *parent, const char *name, double version, double TR, int sessanz, int *sesslength, int *sesscov, VImage src, VImage src1, VImage orth, VImage gamma)
  : QTabWidget( parent, name ), version_m(version), TR_m(TR), sessanz_m(sessanz), sesslength_m(sesslength), sesscov_m(sesscov), src_m(src), src1_m(src1), orth_m(orth), gamma_m(gamma)
{

  setTabPosition ( Bottom );
  setupTab1();
  setupTab3();
  setupTab6();
  setupTab4();
  setupTab5();
  if (gamma) setupTab2();
}

/* close all */
designCW::~designCW()
{

}

/* Design */
void designCW::setupTab1()
{
  QVBox *tab1 = new QVBox( this );
  tab1->setMargin( 5 );
  
  //  QButtonGroup *bil = new QButtonGroup( 1, QGroupBox::Horizontal, "Des", tab1 );
  bild1 = new Bild(tab1,"design", src_m, src1_m);

  addTab( tab1, "Design Matrix" );
}

/* Basis Set */
void designCW::setupTab2()
{
  QVBox *tab2 = new QVBox( this );
  tab2->setMargin( 5 );
  
  //  QButtonGroup *bil = new QButtonGroup( 1, QGroupBox::Horizontal, "Des", tab1 );
  bild2 = new Bild2(tab2,"design", TR_m, gamma_m );

  addTab( tab2, "Basis Set" );
}

void designCW::setupTab3()
{
  QVBox *tab3 = new QVBox( this );
  tab3->setMargin( 5 );
  
  //  QButtonGroup *bil = new QButtonGroup( 1, QGroupBox::Horizontal, "Des", tab1 );
  bild3 = new Bild3(tab3,"sm", TR_m, 1 , sessanz_m, sesslength_m, sesscov_m, src_m, src1_m, orth_m);

  addTab( tab3, "Session Matrix" );
}

/* Regressors */
void designCW::setupTab4()
{
  QVBox *tab4 = new QVBox( this );

  tab4->setMargin( 5 );
  
  bild4 = new Bild3(tab4,"reg", TR_m, 2 , sessanz_m, sesslength_m, sesscov_m, src_m, src1_m, orth_m);

  sb1 = new QScrollBar ( 0, 0, 0, 0, 0, Horizontal, tab4, "scroll" );

  addTab( tab4, "Regressors" );
  connect( sb1, SIGNAL(valueChanged(int)), bild4, SLOT(position1(int)) );
  connect( bild4, SIGNAL(neuZoom1(int)), this, SLOT(changeScroll1(int)) );
}

/* Power Spectrum */
void designCW::setupTab5()
{
  QVBox *tab5 = new QVBox( this );
  tab5->setMargin( 5 );
  
  bild5 = new Bild3(tab5,"power", TR_m, 3 , sessanz_m, sesslength_m, sesscov_m, src_m, src1_m, orth_m);

  sb2 = new QScrollBar ( 0, 0, 0, 0, 0, Horizontal, tab5, "scroll" );

  addTab( tab5, "Power Spectrum" );
  connect( sb2, SIGNAL(valueChanged(int)), bild5, SLOT(position2(int)) );
  connect( bild5, SIGNAL(neuZoom2(int)), this, SLOT(changeScroll2(int)) );
}

/* Orthogonality */
void designCW::setupTab6()
{
  QVBox *tab6 = new QVBox( this );
  tab6->setMargin( 5 );
  
  bild6 = new Bild3(tab6,"ortho", TR_m, 4 , sessanz_m, sesslength_m, sesscov_m, src_m, src1_m, orth_m);

  addTab( tab6, "Orthogonality" );
}

void designCW::changeScroll1( int zoom ) 
{
  int zo=0;
  if (zoom==1) {
    sb1->setValue( 0 );
    sb1->setSteps ( 0, 0 );
    sb1->setRange ( 0, 0 );
  }
  else {
    zo = zoom;
    sb1->setSteps( 100, 100 );
    sb1->setRange ( 0, (int)(0.3*sqrt((double)zo)* VImageNColumns(src_m) ));
  }
}

void designCW::changeScroll2( int zoom ) 
{
  int zo=0;
  if (zoom==1) {
    sb2->setValue( 0 );
    sb2->setSteps ( 0, 0 );
    sb2->setRange ( 0, 0 );
  }
  else {
    zo = zoom;
    sb2->setSteps( 100, 100 );
    sb2->setRange ( 0, (int)(0.3*sqrt((double)zo)*VImageNColumns(src_m)  ));
  }
}
