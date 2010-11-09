/****************************************************************************
** $Id: preferences.C,v 1.0 2000/10/30
**
** by Heiko Mentzel
*****************************************************************************/

/* From the Std library: */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* From the Qt library: */
#include <qvbox.h>
#include <qlabel.h>
#include <qdatetime.h>
#include <qbuttongroup.h>
#include <qlistbox.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qpalette.h>
#include <qcolordialog.h>
#include <qpushbutton.h>
#include <qimage.h> 

/* My own library: */
#include "preferences.h"

float shift=0, factor=1;
float stretch = 200; /* constant for brightness */
double stretchfact = 1.02; /* constant for contrast */

TabDialog::TabDialog( QWidget *parent, const char *name )
    : QTabDialog( parent, name )
{
  setupTab5();
  
  QDialog::resize(350,380);
  setApplyButton("&Apply");

  connect( this, SIGNAL( defaultButtonPressed() ), this, SLOT( ok() ) );
  connect( this, SIGNAL( applyButtonPressed() ), this, SLOT( apply() ) );
}

void TabDialog::setupTab5()
{
  QVBox *tab5 = new QVBox( this );
  tab5->setMargin( 5 );

  QButtonGroup *cont;
  cont = new QButtonGroup( 1, QGroupBox::Horizontal, "contrast / brightness", tab5 );

  brig = new QSlider( cont, "mean" );
  brig->setOrientation ( Horizontal );
  brig->setValue ( 0 );
  brig->setRange( -100, 100 );
  brig->setTickInterval ( 100 );
  brig->setTickmarks(QSlider::TickSetting(3));
  QObject::connect( brig, SIGNAL(valueChanged(int)), this, SLOT( helligkeit(int) ) );
  lbrigbr = new QLabel( "brightness", cont );

  hell = new QSlider( cont, "alpha" );
  hell->setOrientation ( Horizontal );
  hell->setValue ( 0 );
  hell->setRange( -100, 100 );
  hell->setTickInterval ( 100 );
  hell->setTickmarks(QSlider::TickSetting(3));
  lwhit = new QLabel( "contrast", cont );

  QObject::connect( hell, SIGNAL(valueChanged(int)), this, SLOT( contrastl(int) ) );

  QButtonGroup *table_minmax = new QButtonGroup( 2, QGroupBox::Horizontal, "Gray value table", cont );

  QButtonGroup *table_min = new QButtonGroup( 1, QGroupBox::Horizontal, "minimum", table_minmax );
  tablemin = new QLineEdit( tr("%1").arg(selImage->minwert1), table_min );
  tablemin->setFocus();

  QButtonGroup *table_max = new QButtonGroup( 1, QGroupBox::Horizontal, "maximum", table_minmax );
  tablemax = new QLineEdit( tr("%1").arg(selImage->maxwert1), table_max );
  tablemax->setFocus();

  setMinMaxValue();
  Reset = new QPushButton ( "Reset", cont );
  
  if (selImage->pixel_repn == VBitRepn) {
    Reset->setEnabled( FALSE );
    lbrigbr->setEnabled( FALSE );
    lwhit->setEnabled( FALSE );
    brig->setEnabled( FALSE );
    hell->setEnabled( FALSE );
  }


  connect( Reset, SIGNAL(clicked()), SLOT(hellreset()) );
  addTab( tab5, "Contrast" );
}

void TabDialog::helligkeit(int mean) {
  selImage->brightness=mean;
  float ymax = (float)selImage->maxwert;
  float ymin = (float)selImage->minwert;
  float ymaxmin = (float)(ymax - ymin);
  shift = (float)mean * ymaxmin/stretch;
  selImage->anaalpha = factor * 255.0 / ymaxmin;
  selImage->anamean  = (float)ymin - shift + (ymaxmin - 255.0/selImage->anaalpha)/2.0 ;
  setMinMaxValue();
  emit newColtype();
}

void TabDialog::contrastl(int alpha) {
  selImage->contrast=alpha;
  float ymax = (float)selImage->maxwert;
  float ymin = (float)selImage->minwert;
  float ymaxmin = (float)(ymax - ymin);
  factor = (float)pow(stretchfact,(double)alpha);
  selImage->anaalpha = factor * 255.0 / ymaxmin;
  selImage->anamean  = (float)ymin - shift + (ymaxmin - 255.0/selImage->anaalpha)/2.0 ;
  setMinMaxValue();
  emit newColtype();
}

void TabDialog::apply() {
  // min und max in gray value table
  if (strlen(VNewString(tablemin->text()))>0 && strlen(VNewString(tablemax->text()))>0) {
    float ymax = (float)selImage->maxwert;
    float ymin = (float)selImage->minwert;
    float ymaxmin = (float)(ymax - ymin);
    float min1 = atof(strtok(VNewString(tablemin->text())," "));
    float max1 = atof(strtok(VNewString(tablemax->text())," "));
    if (min1 > max1) {
      tablemin->setText(tr("%1").arg(selImage->minwert1));
      tablemax->setText(tr("%1").arg(selImage->maxwert1));
    } else {
      selImage->anamean = min1;
      selImage->anaalpha = (float)255.0/(max1 - selImage->anamean);
      int newbrig = (int)rint(stretch*(ymin - selImage->anamean + (ymaxmin - 255.0/selImage->anaalpha)/2.0)/ymaxmin);
      int newhell = (int)rint(log(selImage->anaalpha*ymaxmin/255.0)/log(stretchfact));

      if ((newbrig>=-100 && newbrig<=100) && (newhell>=-100 && newhell<=100)) {
        brig->setValue(newbrig);
	hell->setValue(newhell);
      } else {
	brig->setEnabled( FALSE );
	hell->setEnabled( FALSE );
	lbrigbr->setEnabled( FALSE );
	lwhit->setEnabled( FALSE );
      }
      setMinMaxValue();
      emit newColtype();
    }
  } else {
    tablemin->setText(tr("%1").arg(selImage->minwert1));
    tablemax->setText(tr("%1").arg(selImage->maxwert1));
  }
  emit newColtype();
}

void TabDialog::ok() {
  apply();
  hide();
}

void TabDialog::hellreset() {
  selImage->brightness=0;
  selImage->contrast=0;
  sliderwechsel();
  setMinMaxValue();
  emit newColtype();
}

void TabDialog::sliderwechsel() {
  if (selImage->pixel_repn == VBitRepn) {
    Reset->setEnabled( FALSE );
    lbrigbr->setEnabled( FALSE );
    lwhit->setEnabled( FALSE );
    brig->setEnabled( FALSE );
    hell->setEnabled( FALSE );
  } else {
    Reset->setEnabled( TRUE );
    lbrigbr->setEnabled( TRUE );
    lwhit->setEnabled( TRUE );
    brig->setEnabled( TRUE );
    hell->setEnabled( TRUE );
  } 
  brig->setValue ((int)selImage->brightness);
  hell->setValue ((int)selImage->contrast); 
  helligkeit((int)selImage->brightness);
  contrastl((int)selImage->contrast);
}

void TabDialog::setMinMaxValue() {
  selImage->minwert1= (int)rint(selImage->anamean);
  selImage->maxwert1= (int)rint(255.0/selImage->anaalpha + selImage->anamean);
  tablemin->setText(tr("%1").arg(selImage->minwert1));
  tablemax->setText(tr("%1").arg(selImage->maxwert1));
}
