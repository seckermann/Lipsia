/****************************************************************************
** $Id: preferences.C,v 1.0 2000/10/30-2002/04/25
** preferences window to change all options in vlview
**
** by Heiko Mentzel
*****************************************************************************/

/* From the Std library: */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>

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
#include <qcheckbox.h>
#include <qimage.h> 
#include <qfont.h> 

/* My own library: */
#include "preferences.h"
#include "showVImage.h"

#define NP -32768

VLShow  myshow;
float shift=0, factor=1;
float stretch = 200; /* constant for brightness */
double stretchfact = 1.02; /* constant for contrast */

TabDialog::TabDialog( QWidget *parent, const char *name, prefs *pr_, double *ca, double *cp, double *extent )
  : QTabDialog( NULL, name ), pr(pr_), ca_m(ca), cp_m(cp), extent_m(extent)
{
  pp=pr->pmax;
  nn=pr->nmax;
  setupTab1();
  setupTab3();
  setupTab5();
  setupTab2();
  setupTab4();
  

  QDialog::resize(350,380);
  setApplyButton("&Apply");

  connect( this, SIGNAL( defaultButtonPressed() ), this, SLOT( ok() ) );
  connect( this, SIGNAL( applyButtonPressed() ), this, SLOT( apply() ) );
}

void TabDialog::setupTab1()
{
  QVBox *tab1 = new QVBox( this );
  tab1->setMargin( 5 );
  
  //search radius
  QButtonGroup *sr = new QButtonGroup( 1, QGroupBox::Horizontal, "search radius", tab1 );
  QSlider *slide = new QSlider( sr, "RadiusSlider" );
  slide->setOrientation ( Horizontal );
  slide->setValue ( pr->radius );
  lcd = new QLCDNumber( 2, sr, "RadiusLCD");
  lcd->setSegmentStyle ( QLCDNumber::Flat );
  lcd->display( pr->radius );
  QObject::connect( slide, SIGNAL(valueChanged(int)), lcd, SLOT( display(int) ) );
  showRad = new QCheckBox( "show radius", sr );
  if ( pr->showradius==1 )
    showRad->setChecked( TRUE );
  
  lbrig = new QLabel( "Note: The search radius has to be specified in anatomical voxels.", sr );

  //coordinates
  QButtonGroup *bg0 = new QButtonGroup( 1, QGroupBox::Horizontal, "coordinates", tab1 );
  showCro = new QCheckBox("show cross", bg0);
  if ( pr->showcross==1 )
    showCro->setChecked( TRUE );
    
  croSize = new QCheckBox("big cross", bg0);
  if ( pr->crossize==1 )
    croSize->setChecked( TRUE );
  if (pr->graph[0]==NULL)
    croSize->setEnabled( FALSE );
  
  addTab( tab1, "Cursor" );
}

/* anatomical / zmap TAB */
void TabDialog::setupTab2()
{
  QVBox *tab2 = new QVBox( this );
  tab2->setMargin( 5 );
  
  QButtonGroup *ip = new QButtonGroup( 1, QGroupBox::Horizontal, "representation", tab2 );
  intPol = new QCheckBox( "interpolate zmap", ip );
  if ( pr->interpol==1 )
    intPol->setChecked( TRUE );
  
  traZma = new QCheckBox( "transparent zmap", ip );
  if ( pr->transzmap==1 )
    traZma->setChecked( TRUE );

  fullzmapogl = new QCheckBox( "show all zmap voxels in sulcus image", ip );
  if ( pr->oglzmapdense==1 )
    fullzmapogl->setChecked( TRUE );
  if (pr->graph[0]==NULL)
    fullzmapogl->setEnabled( FALSE );
  
  showglassbrain = new QCheckBox( "Zmap projection", ip );
  if ( pr->glassbrain==1 )
    showglassbrain->setChecked( TRUE );
  else
    showglassbrain->setChecked( FALSE );

  // KEIN GLASS BRAIN
  showglassbrain->setEnabled( TRUE );


  QButtonGroup *pn = new QButtonGroup( 3, QGroupBox::Horizontal, "zscores", tab2 );
  
  QButtonGroup *pn1 = new QButtonGroup( 1, QGroupBox::Horizontal, "minimum", pn );
  //QLabel *n0 = new QLabel( "max negative zscore:", pn1 );
  n0max = new QLineEdit( tr("%1").arg(-pr->nmax), pn1 );
  n0max->setFocus();
  
  QButtonGroup *pn05 = new QButtonGroup( 1, QGroupBox::Horizontal, "zero point", pn );
  zero = new QLineEdit( tr("%1").arg(pr->zeropoint), pn05 );
  
  QButtonGroup *pn0 = new QButtonGroup( 1, QGroupBox::Horizontal, "maximum", pn );
  //QLabel *p0 = new QLabel( "max positive zscore:", pn0 );
  p0max = new QLineEdit( tr("%1").arg(pr->pmax), pn0 );
  p0max->setFocus();
  if (!pr->sw2) {
    ip->setEnabled( FALSE );
    pn->setEnabled( FALSE );
    pn0->setEnabled( FALSE );
    p0max->setEnabled( FALSE );
    traZma->setEnabled( FALSE );
    fullzmapogl->setEnabled( FALSE );
    showglassbrain->setEnabled( FALSE );
    intPol->setEnabled( FALSE );
  }

  lockmaxz = new QCheckBox( "lock", pn );
  if ( pr->lockz==1 )
    lockmaxz->setChecked( TRUE );
  
  if (!pr->sw2 || pr->only_sulci) {
    pn1->setEnabled( FALSE );
    n0max->setEnabled( FALSE );
  }
  addTab( tab2, "Zmap" );
}

void TabDialog::setupTab3()
{
	QVBox *tab3 = new QVBox( this );
	tab3->setMargin( 5 );
	//tab3->setSpacing( 5 );
	  
	QButtonGroup *ltab = new QButtonGroup( 2, QGroupBox::Horizontal, "Line Colors", tab3 );
	
	QPushButton *kreuz = new QPushButton ( "Cross", ltab );
	connect( kreuz, SIGNAL(clicked()), SLOT(color1Select()) );
	  
	QPushButton *radiu = new QPushButton ( "Radius", ltab );
	connect( radiu, SIGNAL(clicked()), SLOT(color2Select()) );
	  //cd->show();
	  
	QButtonGroup* ectGroup = new QButtonGroup( 2, QGroupBox::Vertical, "Color Table Options", tab3 );
	
	QCheckBox* ectCheckBox = new QCheckBox("Don't split color table", ectGroup);
	ectCheckBox->setChecked(pr->equidistantColorTable == 0 ? FALSE : TRUE);
	if(pr->equidistantColorTable == TRUE) {
		ectCheckBox->setEnabled( TRUE );
	}
	connect(ectCheckBox, SIGNAL(clicked()), this, SLOT(equidistantColorTableToggled()));
	  
	QString ectExplanation("Usually the color table is split into two parts with the upper half used to\nmap the positive values and the lower half mapping the negative values.");
	QLabel* ectExplanationLabel = new QLabel(ectExplanation, ectGroup);
	ectExplanationLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	ectExplanationLabel->setAlignment(Qt::AlignJustify);
	QFont _font = ectExplanationLabel->font();
	_font.setPointSizeFloat(_font.pointSizeFloat() * 0.8);
	ectExplanationLabel->setFont(_font);
	  
	QButtonGroup *ctab = new QButtonGroup( 2, QGroupBox::Horizontal, "Z-map Color Table", tab3 );
	if (!pr->sw2 && !pr->graph[0]) {
		ctab->setEnabled( FALSE );
		ectGroup->setEnabled( FALSE );
	}
	
	QColor *rgbfarbe = (QColor *) malloc(sizeof(QColor) * 65536);
	QColor *rgbfarbeoverlay = (QColor *) malloc(sizeof(QColor) * 256);
	//  QColor *negfarbe = (QColor *) malloc(sizeof(QColor) * 23);
	ColorTableButtons = (QPushButton**)malloc(sizeof(QPushButton*) * 16);
	  
	  
	for(int j = 0; j < 14; j++) {
		ColorTableButtons[j] = new QPushButton(ctab, "colors");
	}

	QObject::connect(ColorTableButtons[0],  SIGNAL(clicked()), this, SLOT(coltabSel0()));
	QObject::connect(ColorTableButtons[1],  SIGNAL(clicked()), this, SLOT(coltabSel1()));
	QObject::connect(ColorTableButtons[2],  SIGNAL(clicked()), this, SLOT(coltabSel2()));
	QObject::connect(ColorTableButtons[3],  SIGNAL(clicked()), this, SLOT(coltabSel3()));
	QObject::connect(ColorTableButtons[4],  SIGNAL(clicked()), this, SLOT(coltabSel4()));
	QObject::connect(ColorTableButtons[5],  SIGNAL(clicked()), this, SLOT(coltabSel5()));
	QObject::connect(ColorTableButtons[6],  SIGNAL(clicked()), this, SLOT(coltabSel6()));
	QObject::connect(ColorTableButtons[7],  SIGNAL(clicked()), this, SLOT(coltabSel7()));
	QObject::connect(ColorTableButtons[8],  SIGNAL(clicked()), this, SLOT(coltabSel8()));
	QObject::connect(ColorTableButtons[9],  SIGNAL(clicked()), this, SLOT(coltabSel9()));
	QObject::connect(ColorTableButtons[10], SIGNAL(clicked()), this, SLOT(coltabSel10()));
	QObject::connect(ColorTableButtons[11], SIGNAL(clicked()), this, SLOT(coltabSel11()));
	QObject::connect(ColorTableButtons[12], SIGNAL(clicked()), this, SLOT(coltabSel12()));
	QObject::connect(ColorTableButtons[13], SIGNAL(clicked()), this, SLOT(coltabSel13()));
    
	QButtonGroup *actab = new QButtonGroup(2, QGroupBox::Horizontal, "Anatomy Color Table", tab3);
  
	ColorTableButtons[14] = new QPushButton(actab, "colors");
	QObject::connect(ColorTableButtons[14], SIGNAL(clicked()), this, SLOT(coltabSel14()));


	ColorTableButtons[15] = new QPushButton(actab, "colors");
	QObject::connect(ColorTableButtons[15], SIGNAL(clicked()), this, SLOT(coltabSel15()));

	prepareColorTableButtons();
	
	if(pr->only_sulci) {
		actab->setEnabled(FALSE);
	}

	addTab(tab3, "Colors");
}

void TabDialog::setupTab4()
{
  QVBox *tab4 = new QVBox( this );
  tab4->setMargin( 5 );
  //tab4->setSpacing( 5 );
  QButtonGroup *rawd = new QButtonGroup( 1, QGroupBox::Horizontal, "rawdata", tab4 );

  pschange = new QCheckBox( "percent signal change", rawd );
  if ( pr->persi==1 )
    pschange->setChecked( TRUE );
  else
    pschange->setChecked( FALSE );
  
  QButtonGroup *trl = new QButtonGroup( 1, QGroupBox::Horizontal, "Trial Averages", tab4 );
  lbrig = new QLabel( "length of trial in seconds", trl );
  //QLabel *tl = new QLabel( "trial length:", trl );
  triall = new QLineEdit( tr("%1").arg(pr->triallength), trl );
  //trl->setFocus();
  lbrig = new QLabel( "temporal resolution in msec", trl );
  trialreso = new QLineEdit( tr("%1").arg(pr->trialresolution), trl );

  if (!pr->raw || pr->only_sulci) {
    rawd->setEnabled( FALSE );
    trl->setEnabled( FALSE );
  }

  if (!pr->des) trl->setEnabled( FALSE );
  addTab( tab4, "RawData" );
}

void TabDialog::setupTab5()
{
  QVBox *tab5 = new QVBox( this );
  tab5->setMargin( 5 );
  //tab4->setSpacing( 5 );
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
  tablemin = new QLineEdit( tr("%1").arg(pr->minwert1), table_min );
  tablemin->setFocus();

  QButtonGroup *table_max = new QButtonGroup( 1, QGroupBox::Horizontal, "maximum", table_minmax );
  tablemax = new QLineEdit( tr("%1").arg(pr->maxwert1), table_max );
  tablemax->setFocus();

  QPushButton *Reset = new QPushButton ( "Reset", cont );
  connect( Reset, SIGNAL(clicked()), SLOT(hellreset()) );

  QButtonGroup *contcolor;
  contcolor = new QButtonGroup( 1, QGroupBox::Horizontal, "color manipulation", tab5 );
  
  dunk = new QSlider( contcolor, "color" );
  dunk->setOrientation ( Horizontal );
  dunk->setRange( 0 , 2*(pr->maxwert - pr->minwert) );
  dunk->setValue ( 0 );  
  dunk->setTickInterval ( pr->maxwert - pr->minwert );
  dunk->setTickmarks(QSlider::TickSetting(3));
  lblac = new QLabel( "color change", contcolor );
  QObject::connect( dunk, SIGNAL(valueChanged(int)), this, SLOT( contrastd(int) ) );
 
  dunk1 = new QSlider( contcolor, "color" );
  dunk1->setOrientation ( Horizontal );
  dunk1->setRange( 10 , 110 );
  dunk1->setValue ( 0 );  
  dunk1->setTickInterval ( 50 );
  dunk1->setTickmarks(QSlider::TickSetting(3));
  lblac1 = new QLabel( "color spread", contcolor );
  QObject::connect( dunk1, SIGNAL(valueChanged(int)), this, SLOT( contrastd1(int) ) );


  Reset1 = new QPushButton ( "Color", contcolor );
  connect( Reset1, SIGNAL(clicked()), SLOT(colorreset()) );

  // for Lipsia-users only
  contcolor->setEnabled( TRUE );

  if (pr->only_sulci) cont->setEnabled( FALSE );
  addTab( tab5, "Contrast" );
}

void TabDialog::color1Select()
{
  pr->crosscolor = QColorDialog::getColor ( pr->crosscolor, this, "colors" );
}

void TabDialog::color2Select()
{
  pr->radiuscolor = QColorDialog::getColor ( pr->radiuscolor, this, "colors" );
}

void TabDialog::equidistantColorTableToggled() {
	pr->equidistantColorTable = pr->equidistantColorTable == 0 ? TRUE : FALSE;
	updateColorTableButtons();
	emit newColtype();
}




void TabDialog::coltabSel0()
{
  pr->coltype=0;
  emit newColtype();
  pr->gcoltype=0;
  emit newgraphColtype();
  updateColorTableButtons();
}
void TabDialog::coltabSel1()
{
  pr->coltype=1;
  emit newColtype();
  pr->gcoltype=1;
  emit newgraphColtype();
  updateColorTableButtons();
}
void TabDialog::coltabSel2()
{
  pr->coltype=2;
  emit newColtype();
  pr->gcoltype=2;
  emit newgraphColtype();
  updateColorTableButtons();
}
void TabDialog::coltabSel3()
{
  pr->coltype=3;
  emit newColtype();
  pr->gcoltype=3;
  emit newgraphColtype();
  updateColorTableButtons();
}
void TabDialog::coltabSel4()
{
  pr->coltype=4;
  emit newColtype();
  pr->gcoltype=4;
  emit newgraphColtype();
  updateColorTableButtons();
}
void TabDialog::coltabSel5()
{
  pr->coltype=5;
  emit newColtype();
  pr->gcoltype=5;
  emit newgraphColtype();
  updateColorTableButtons();
}
void TabDialog::coltabSel6()
{
  pr->coltype=6;
  emit newColtype();
  pr->gcoltype=6;
  emit newgraphColtype();
  updateColorTableButtons();
}
void TabDialog::coltabSel7()
{
  pr->coltype=7;
  emit newColtype();
  pr->gcoltype=7;
  emit newgraphColtype();
  updateColorTableButtons();
}
void TabDialog::coltabSel8()
{
  pr->coltype=8;
  emit newColtype();
  pr->gcoltype=8;
  emit newgraphColtype();
  updateColorTableButtons();
}
void TabDialog::coltabSel9()
{
  pr->coltype=9;
  emit newColtype();
  pr->gcoltype=9;
  emit newgraphColtype();
  updateColorTableButtons();
}

void TabDialog::coltabSel10()
{
  pr->coltype=10;
  emit newColtype();
  pr->gcoltype=10;
  emit newgraphColtype();
  updateColorTableButtons();
}

void TabDialog::coltabSel11()
{
  pr->coltype=11;
  emit newColtype();
  pr->gcoltype=11;
  emit newgraphColtype();
  updateColorTableButtons();
}

void TabDialog::coltabSel12()
{
  pr->coltype=12;
  emit newColtype();
  pr->gcoltype=12;
  emit newgraphColtype();
  updateColorTableButtons();
}

void TabDialog::coltabSel13()
{
  pr->coltype=13;
  emit newColtype();
  pr->gcoltype=13;
  emit newgraphColtype();
  updateColorTableButtons();
}

void TabDialog::coltabSel14()
{
  pr->acoltype=0;
  emit newColtype();
  pr->gcoltype=0;
  emit newgraphColtype();
  updateColorTableButtons();
}

void TabDialog::coltabSel15()
{
  pr->acoltype=1;
  emit newColtype();
  updateColorTableButtons();
}


void TabDialog::helligkeit(int mean) {
  float ymax = (float)pr->maxwert;
  float ymin = (float)pr->minwert;
  float ymaxmin = (float)(ymax - ymin);
  shift = (float)mean * ymaxmin/stretch;
  pr->anaalpha = factor * 255.0 / ymaxmin;
  pr->anamean = (float)ymin - shift + (ymaxmin - 255.0/pr->anaalpha)/2.0 ;
  setMinMaxValue();
  emit newColtype();
}

void TabDialog::contrastl(int alpha) {
  float ymax = (float)pr->maxwert;
  float ymin = (float)pr->minwert;
  float ymaxmin = (float)(ymax - ymin);
  factor = (float)pow(stretchfact,(double)alpha);
  pr->anaalpha = factor * 255.0 / ymaxmin;
  pr->anamean = (float)ymin - shift + (ymaxmin - 255.0/pr->anaalpha)/2.0 ;
  setMinMaxValue();
  emit newColtype();
}

void TabDialog::hellreset() {
  brig->setValue ( 0 );
  hell->setValue ( 0 );
  dunk->setValue ( 0 );
  dunk1->setValue ( 0 );
  brig->setEnabled( TRUE );
  hell->setEnabled( TRUE );
  lbrigbr->setEnabled( TRUE );
  lwhit->setEnabled( TRUE );
  float ymax = (float)pr->maxwert;
  float ymin = (float)pr->minwert;
  float ymaxmin = (float)(ymax - ymin);
  pr->anaalpha = 255.0 / ymaxmin;
  pr->anamean  = (float)ymin; 
  pr->shift    = 0;
  pr->spread   = 10;
  pr->acoltype = 0;
  Reset1->setText("Color");
  setMinMaxValue();
  emit newColtype();
}

void TabDialog::setMinMaxValue() {
  pr->minwert1= (int)rint(pr->anamean);
  pr->maxwert1= (int)rint(255.0/pr->anaalpha + pr->anamean);
  tablemin->setText(tr("%1").arg(pr->minwert1));
  tablemax->setText(tr("%1").arg(pr->maxwert1));
}

void TabDialog::contrastd(int contrastt) {
  pr->acoltype=2;
  Reset1->setText("B/W");
  pr->shift  = contrastt;
  emit newColtype();
}

void TabDialog::contrastd1(int contrastt) {
  pr->acoltype=2;
  Reset1->setText("B/W");
  pr->spread  = contrastt;
  emit newColtype();
}

void TabDialog::colorreset() {
  if (pr->acoltype>0) {
    Reset1->setText("Color");
    pr->acoltype=0;
  } else {
    Reset1->setText("B/W");
    pr->acoltype=2; // 0= normal, 1= reg. Farbe, 2= scr. Farbe
  }
  emit newColtype();
}


void TabDialog::apply() {

  if (pr->sw2) {
    if (pr->graph[0]) {
      if ( fullzmapogl->isChecked()==TRUE ) pr->oglzmapdense = 1;
      else pr->oglzmapdense = 0;
    }

    if ( showglassbrain->isChecked()==TRUE ) pr->glassbrain = 1;
    else pr->glassbrain = 0;
    
    if (strlen(VNewString(p0max->text()))>0) {
      if (atof(strtok(VNewString(p0max->text())," ")) >= pp) {
	pr->pmax=atof(strtok(VNewString(p0max->text())," "));
      } else {
	pr->cutoutside=1;
	pr->pmax=atof(strtok(VNewString(p0max->text())," "));
      }
    } else 
      p0max->setText(tr("%1").arg(pr->pmax));

    if (strlen(VNewString(n0max->text()))>0) {
      if (atof(strtok(VNewString(n0max->text())," ")) <= -nn) {
	pr->nmax=-atof(strtok(VNewString(n0max->text())," "));
      } else {
	pr->cutoutside=1;
	pr->nmax=-atof(strtok(VNewString(n0max->text())," "));
      }
    } else
      n0max->setText(tr("%1").arg(pr->nmax));

    if (strlen(VNewString(zero->text()))>0) {
      if ( atof(strtok(VNewString(zero->text())," ")) <= pr->pmax &&
	   atof(strtok(VNewString(zero->text())," ")) >= -pr->pmax ) {
	pr->zeropoint=atof(strtok(VNewString(zero->text())," "));
      } else {
	zero->setText(tr("%1").arg(pr->zeropoint));
      } 
    } else
      zero->setText(tr("%1").arg(pr->zeropoint));

    if (pr->pmax<pr->zeropoint) {
      QMessageBox::warning( this, "outside range",
        "Your selected maximum is smaller then the zero point.\n"
        "Maximum set to zero point.\n");
      pr->pmax=pr->zeropoint;
      p0max->setText(tr("%1").arg(pr->pmax));
    }
    
    if (-pr->nmax>pr->zeropoint) {
      QMessageBox::warning( this, "outside range",
        "Your selected minimum is larger then the zero point.\n"
	"Minimum set to zero point.\n");
      pr->nmax=pr->zeropoint;
      n0max->setText(tr("%1").arg(pr->nmax));
    }
    
    if (-pr->nmax>=pr->pmax )
      QMessageBox::warning( this, "Warning",
	"Selected minimum coincides with maximum.\n"
	 "This makes no sense.\n");
   

    emit newColtype();
    emit nowsliderChangeS(); 
    if (pr->verbose>0) fprintf(stderr,"min: %f , zero: %f , max: %f\n",-pr->nmax,pr->zeropoint,pr->pmax);
  }
   
  // radius
  pr->radius = (int)lcd->value ();

  // min und max in gray value table
  if (strlen(VNewString(tablemin->text()))>0 && strlen(VNewString(tablemax->text()))>0) {
    float ymax = (float)pr->maxwert;
    float ymin = (float)pr->minwert;
    float ymaxmin = (float)(ymax - ymin);
    float min1 = atof(strtok(VNewString(tablemin->text())," "));
    float max1 = atof(strtok(VNewString(tablemax->text())," "));
    if (min1 > max1) {
      tablemin->setText(tr("%1").arg(pr->minwert1));
      tablemax->setText(tr("%1").arg(pr->maxwert1));
    } else {
      pr->anamean = min1;
      pr->anaalpha = (float)255.0/(max1 - pr->anamean);
      int newbrig = (int)rint(stretch*(ymin - pr->anamean + (ymaxmin - 255.0/pr->anaalpha)/2.0)/ymaxmin);
      int newhell = (int)rint(log(pr->anaalpha*ymaxmin/255.0)/log(stretchfact));

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
    tablemin->setText(tr("%1").arg(pr->minwert1));
    tablemax->setText(tr("%1").arg(pr->maxwert1));
  }

  if ( showCro->isChecked()==TRUE ) pr->showcross = 1;
  else pr->showcross = 0;
  if ( croSize->isChecked()==TRUE ) pr->crossize = 1;
  else pr->crossize = 0;
  if ( showRad->isChecked()==TRUE ) pr->showradius = 1;
  else pr->showradius = 0;

  if (pr->sw2) {
    if ( lockmaxz->isChecked()==TRUE ) pr->lockz = 1;
    else pr->lockz = 0;
    if ( traZma->isChecked()==TRUE ) pr->transzmap = 1;
    else pr->transzmap = 0;
    if ( intPol->isChecked()==TRUE && pr->interpol==0) {
      pr->interpol = 1;
      emit reloadFiles();
    } else if (intPol->isChecked()==FALSE && pr->interpol==1) {
      pr->interpol = 0;
      emit reloadFiles();
    }
  }

  // TRial length and resolution
  if (pr->raw) {
    if ( pschange->isChecked()==TRUE ) pr->persi = 1;
    else pr->persi = 0;

    if (atof(strtok(VNewString(triall->text())," ")) > 0 && atof(strtok(VNewString(triall->text())," ")) < 10000)
      pr->triallength=(int) atof(strtok(VNewString(triall->text())," "));
    else
      triall->setText(tr("%1").arg(pr->triallength));
    
    if (atof(strtok(VNewString(trialreso->text())," ")) > 0 && atof(strtok(VNewString(trialreso->text())," ")) < 3000)
      pr->trialresolution=(int) atof(strtok(VNewString(trialreso->text())," "));
    else
      trialreso->setText(tr("%1").arg(pr->trialresolution));
  }
}

void TabDialog::ok() {

  apply(); 
  hide();
}


void TabDialog::prepareColorTableButtons() {

	QColor *rgbfarbe = (QColor *) malloc(sizeof(QColor) * 65536);
	QColor *rgbfarbeoverlay = (QColor *) malloc(sizeof(QColor) * 256);
	//  QColor *negfarbe = (QColor *) malloc(sizeof(QColor) * 23);
	  
	  
	  
	QRgb background = ColorTableButtons[0]->paletteBackgroundColor().rgb();

	QImage colorimage[16];

   	QColor red(255, 0, 0);

   	
	for(int j = 0; j < 14; j++) {
	    
		/* create colormaps for anatomie and zmap */
	    myshow.vlhColorMap(rgbfarbe, rgbfarbeoverlay, pr->acoltype, j, src[0], fnc, pr);

	    colorimage[j].create(180, 20, 32, 1024);
	    colorimage[j].fill(background);

	    if(pr->equidistantColorTable) {
		    for(int i = 0; i < 120; i++) {
		    	colorimage[j].setPixel(30 + i,  6, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(30 + i,  7, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(30 + i,  8, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(30 + i,  9, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(30 + i, 10, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(30 + i, 11, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(30 + i, 12, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    }
	    } else {
		    for(int i = 0; i <= 60; i++) {
		    	colorimage[j].setPixel(20 + i,  6, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(20 + i,  7, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(20 + i,  8, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(20 + i,  9, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(20 + i, 10, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(20 + i, 11, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(20 + i, 12, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    }
		    for(int i = 61; i < 120; i++) {
		    	colorimage[j].setPixel(40 + i,  6, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(40 + i,  7, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(40 + i,  8, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(40 + i,  9, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(40 + i, 10, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(40 + i, 11, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    	colorimage[j].setPixel(40 + i, 12, rgbfarbeoverlay[(int)rint((double)i * 2.12)].rgb());
		    }
	    }

	    // mark the selected color table
	    if(j == pr->coltype) {
	    	for(int i = 0; i < 180; i++) {
	    		colorimage[j].setPixel(i,  0, red.rgb());
	    		colorimage[j].setPixel(i, 19, red.rgb());
	    	}
	    	for(int i = 0; i < 20; i++) {
	    		colorimage[j].setPixel(  0, i, red.rgb());
	    		colorimage[j].setPixel(179, i, red.rgb());
	    	}
	    }
	    
	    QPixmap cpm = QPixmap();
	    cpm.convertFromImage(colorimage[j], ~ColorMode_Mask | AutoColor);

	    ColorTableButtons[j]->setPixmap(cpm);
	}
	  
	for(int i = 0; i < 256; i++) {
		rgbfarbe[i].setRgb(i, i, i);
	}

	colorimage[14].create (180,20, 32, 1024);
	colorimage[14].fill(background);
	for(int i = 0; i < 120; i++) {
	    colorimage[14].setPixel(30 + i,  6, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	    colorimage[14].setPixel(30 + i,  7, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	    colorimage[14].setPixel(30 + i,  8, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	    colorimage[14].setPixel(30 + i,  9, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	    colorimage[14].setPixel(30 + i, 10, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	    colorimage[14].setPixel(30 + i, 11, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	    colorimage[14].setPixel(30 + i, 12, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	}

	if(pr->acoltype == 0) {
    	for(int i = 0; i < 180; i++) {
    		colorimage[14].setPixel(i,  0, red.rgb());
    		colorimage[14].setPixel(i, 19, red.rgb());
    	}
    	for(int i = 0; i < 20; i++) {
    		colorimage[14].setPixel(  0, i, red.rgb());
    		colorimage[14].setPixel(179, i, red.rgb());
    	}
    }
	
	QPixmap cpm = QPixmap();
	cpm.convertFromImage(colorimage[14], ~ColorMode_Mask | AutoColor);
	ColorTableButtons[14]->setPixmap(cpm);


	for(int i = 0; i < 256; i++) {
	    rgbfarbe[i].setHsv((int)((360.0 / 256.0) * i), 255, 255);
	}

	colorimage[15].create(180, 20, 32, 1024);
	colorimage[15].fill(background);
	for(int i = 0;i < 120; i++) {
	    colorimage[15].setPixel(30 + i,  6, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	    colorimage[15].setPixel(30 + i,  7, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	    colorimage[15].setPixel(30 + i,  8, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	    colorimage[15].setPixel(30 + i,  9, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	    colorimage[15].setPixel(30 + i, 10, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	    colorimage[15].setPixel(30 + i, 11, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	    colorimage[15].setPixel(30 + i, 12, rgbfarbe[(int)rint((double)i * 255.0 / 120.0)].rgb());
	}

	if(pr->acoltype == 1) {
    	for(int i = 0; i < 180; i++) {
    		colorimage[15].setPixel(i,  0, red.rgb());
    		colorimage[15].setPixel(i, 19, red.rgb());
    	}
    	for(int i = 0; i < 20; i++) {
    		colorimage[15].setPixel(  0, i, red.rgb());
    		colorimage[15].setPixel(179, i, red.rgb());
    	}
    }
	
	cpm.convertFromImage(colorimage[15], ~ColorMode_Mask | AutoColor );  
	ColorTableButtons[15]->setPixmap(cpm);
	
}


void TabDialog::updateColorTableButtons() {

	prepareColorTableButtons();
	for(int i = 0; i < 16; i++) {
		ColorTableButtons[i]->update();
	}	
}


