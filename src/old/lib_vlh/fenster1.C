/* my own inludes */
#include "fenster1.h"

/* images */
#include "xpm/color01.xpm"
#include "xpm/fileprint.xpm"
#include "xpm/filesave.xpm"
#include "xpm/power.xpm"
#include "xpm/timeline.xpm"
#include "xpm/tav.xpm"
#include "xpm/hgpixmap.xpm"
#include "xpm/rastermap.xpm"

/* global variables */
int allefarben;

Modal::Modal( QWidget *parent, const char *name, prefs *pr_, int i, float *wert1, int pi, float *powerwert, float *fitres, float *se, double trpre, int *length, float *model, const char *version, float *conditions, float *Sess, int bline, float **evres, float **evresSE, int length_ER, int width_ER, int cnd, QString vlviewname )
  : QMainWindow( 0, "rawdata window", WDestructiveClose ), pr(pr_), i_m(i), wert_m(wert1), length_m(length), cnd_m(cnd) , model_m(model)
{
  int maxfarben;
  maxfarben=cnd;
  allefarben=cnd;
  if (pr->verbose) {
    fprintf(stderr,"maxfarben: %d, cnd: %d\n",maxfarben,cnd);
    fprintf(stderr,"try to open rawCW\n");
  }

  setCaption( tr("%1").arg(vlviewname) );

  centralw = new RawCW( this, "central", pr, i, wert1, pi, powerwert, fitres, se, trpre, length, model, version, fname_m, conditions, Sess, bline, evres, evresSE, length_ER, width_ER, cnd );
  setCentralWidget( centralw );
  
  QToolBar *tools2;
  tools2 = new QToolBar( this, "power toolbar" );
  rb0 = (QToolButton **) malloc(sizeof(QToolButton*)*(allefarben+1));

  //icons
  QPixmap timeIcon( timelinexpm );
  QPixmap tavIcon( tavxpm );
  QPixmap saveIcon( filesave );

  //time course
  rb1 = new QToolButton( timeIcon, "time course", QString::null, this, SLOT(tln()), tools2, "time course" );
  QWhatsThis::add( rb1, "time course.");
  rb1->setToggleButton( TRUE );
  if (pr->bolres==0) {
    rb1->toggle();
  }
  tools2->addSeparator();

  if (cnd) {
    rb2 = new QToolButton( tavIcon, "Trial Average", QString::null, this, SLOT(tae()), tools2, "trial average" );
    QWhatsThis::add( rb2, "trial average.");
    rb2->setToggleButton( TRUE );
    if (pr->bolres==1) {
      rb2->toggle();
    }
    tools2->addSeparator();
  }

  QPixmap  powerIcon( powerspec );
  rb3 = new QToolButton( powerIcon, "power spectrum", QString::null, this, SLOT(psm()), tools2, "power spectrum" );
  QWhatsThis::add( rb3, "power spectrum.");
  rb3->setToggleButton( TRUE );
  if (pr->bolres==2) {
    rb3->toggle();
  }
  tools2->addSeparator();

  if (pr->beta) {
    tc = new QToolButton( QPixmap(), "time course", QString::null, this, SLOT(mtc()), tools2, "time course" );
    QWhatsThis::add( tc, "time course"); 
    tc->setText("TC");
    tc->setToggleButton ( TRUE ); 
    tc->toggle(); 
    tools2->addSeparator();

    mod = new QToolButton( QPixmap(), "fitted response", QString::null, this, SLOT(mdg()), tools2, "fitted response" );
    QWhatsThis::add( mod, "fitted response"); 
    mod->setText("FR");
    mod->setToggleButton ( TRUE ); 
    mod->toggle(); 
    tools2->addSeparator();
  }

  // Create a menubar...
  QMenuBar *menubar = new QMenuBar( this );  
  QPopupMenu* popup = new QPopupMenu;
  CHECK_PTR( popup );
  QPixmap  printIcon( fileprint );
  //popup->insertItem( printIcon, "&Print", centralw, SLOT(print()), CTRL+Key_P );
  //popup->insertSeparator();
  popup->insertItem( "&Close", this, SLOT(hide()), CTRL+Key_C );
  popup->insertSeparator();
  popup->insertItem( "&Quit", qApp, SLOT(quit()), CTRL+Key_Q );

  if (cnd) 
    ordermenu = (int *) malloc(sizeof(int) * (allefarben+1) );

  // Show something
  QPopupMenu* show = new QPopupMenu;
  CHECK_PTR( show );
  show->insertItem( timeIcon, "&Time Course", this, SLOT(tln()), CTRL+Key_T );
  if (cnd) show->insertItem( tavIcon, "Trial &Average", this, SLOT(tae()), CTRL+Key_A );
  show->insertItem( powerIcon, "&PowerSpectrum", this, SLOT(psm()), CTRL+Key_P );

  // Save something
  QPopupMenu* save = new QPopupMenu;
  CHECK_PTR( save );
  save->insertItem( saveIcon, "TC: &Timeline", centralw, SLOT(saveTimelineTC()), CTRL+Key_T );
  save->insertItem( saveIcon, "TC: PowerSpectr&um", centralw, SLOT(savePowerspectrumTC()), CTRL+Key_U );
  if (pr->des) 
    save->insertItem( saveIcon, "TC: Trial &Average", centralw->rpaint, SLOT(saveTrialAverageTC()), CTRL+Key_A );
  if (pr->beta) {
    save->insertItem( saveIcon, "FR: Time&line", centralw, SLOT(saveTimelineFR()), CTRL+Key_L );
    save->insertItem( saveIcon, "FR: Po&werSpectrum", centralw, SLOT(savePowerspectrumFR()), CTRL+Key_W );
    if (pr->des) 
      save->insertItem( saveIcon, "FR: Trial A&verage", centralw->rpaint, SLOT(saveTrialAverageFR()), CTRL+Key_V );
  }

  if (cnd) {
    zeige = new QPopupMenu;
    CHECK_PTR( zeige );
    zeige->setCheckable(TRUE);
    
    for (int j=1;j<=allefarben;j++) {
      QPixmap colorIcon(color01);
      colorIcon.fill(centralw->rpaint->bgfarbemidd[j]);
      if (fname_m.begin() != fname_m.end()) {
	ordermenu[j] = zeige->insertItem( colorIcon, tr("%1").arg(fname_m[j-1]), j+20 );
      rb0[j] = new QToolButton( colorIcon, tr("%1").arg(fname_m[j-1]), QString::null, this, SLOT(orderOO()), tools2, "coronal" );
      }
      else {
	ordermenu[j] = zeige->insertItem( colorIcon, tr("&Condition %1").arg(j), j+20 );
      rb0[j] = new QToolButton( colorIcon, tr("condition %1").arg(j), QString::null, this, SLOT(orderOO()), tools2, "coronal" );
      QWhatsThis::add( rb0[j], "Here you can switch on/off "
		       "the selected condition.");
      }
      zeige->setItemChecked(ordermenu[j], FALSE);
      rb0[j]->setToggleButton ( TRUE );
    }
    zeige->setItemChecked(ordermenu[1], FALSE);

    tools2->addSeparator();
    

    QObject::connect(zeige, SIGNAL(activated(int)), this, SLOT(orderOO(int)));
  }  

  // Raster
  QPixmap rasterIcon(rastermap);
  raster = new QToolButton( rasterIcon, "raster", QString::null, this, SLOT(rast()), tools2, "raster" );
  QWhatsThis::add( raster, "This button switches the raster on/off.");
  raster->setToggleButton ( TRUE );
  raster->toggle();
  tools2->addSeparator();

  // S/W-Umschaltung
  QPixmap hgIcon(hgpixmap);
  hgswitch = new QToolButton( hgIcon, "black/white", QString::null, this, SLOT(hgsw()), tools2, "black/white" );
  QWhatsThis::add( hgswitch, "This button switches the background color"
		   "between black and white.");
  
  tools2->addSeparator();
  toolb = QWhatsThis::whatsThisButton( tools2 );
  QWhatsThis::add( toolb, "This is a <b>What's This</b> button "
		   "It enables the user to ask for help "
		   "about widgets on the screen.");
  
  

  //About
  //dialog = new MyDialog(this, "Dialog", pr, NULL );
  //QPopupMenu* help = new QPopupMenu( this );
  //CHECK_PTR( help );
  //help->insertItem( "&About", dialog, SLOT(about()), CTRL+Key_A );
 

  menubar->insertItem( "&File", popup );
  menubar->insertItem( "&Show", show );
  menubar->insertItem( "&Save", save );
  if (cnd) menubar->insertItem( "&Condition", zeige );
  //menubar->insertItem( "&Help", help );
 
  //free(bgfarbemidd);
  
  QToolBar *Stools = new QToolBar( "statusbar", this, QMainWindow::Bottom );
  Stools->setHorizontalStretchable ( 1 );
  
  QStatusBar *statusbar = new QStatusBar( Stools, "Graph" );

  // Create an label and a message in the status bar
  QLabel *msg = new QLabel( statusbar, "message" );

  statusbar->addWidget( msg, 4 );

  msg->setText( tr("Ana X:%1 Y:%2 Z:%3  /  Raw X:%4 Y:%5 Z:%6").arg(model_m[0]).arg(model_m[1]).arg(model_m[2]).arg(model_m[3]).arg(model_m[4]).arg(model_m[5]) );
  msg->setMinimumHeight( msg->sizeHint().height() );

}

/* free all mem on exit */
Modal::~Modal()
{
  /* 
     free(rb1);
     rb2->~QToolButton ();
  */
}

/* conditions menu */
void Modal::orderOO( int fe )
{
  //  fe=fabs(fe)-16;
  //while (fe>12) fe=fe-12;
  fe=fe-20;

  if (centralw->rpaint->cline[fe]==0) {
    centralw->rpaint->cline[fe]=1;
    zeige->setItemChecked(ordermenu[fe], TRUE);
    rb0[fe]->setOn( TRUE );
  } else {
    centralw->rpaint->cline[fe]=0;
    zeige->setItemChecked(ordermenu[fe], FALSE);
    rb0[fe]->setOn( FALSE );
  }    
  centralw->rpaint->repaint();
}

/* condition toolbar buttons */
void Modal::orderOO()
{
  if (cnd_m) {
    // actualize conditions menu
    for (int i=1;i<=allefarben;i++) {
      zeige->setItemChecked( ordermenu[i], rb0[i]->isOn() );
      centralw->rpaint->cline[i]=rb0[i]->isOn();
    }
  }
  centralw->rpaint->repaint();
}

/* model On/Off */
void Modal::tln() // TIMELINE
{
  if ( !rb1->isOn() ) rb1->setOn( TRUE );
  rb3->setOn( FALSE );               //power
  if (cnd_m) rb2->setOn( FALSE );    //trialav
  pr->bolres=0;
  centralw->rpaint->bolresp=0;
  //pr->tiline=1;
  //centralw->rpaint->tilinep=1;
  centralw->rpaint->repaint();
}

void Modal::tae() // TRIAL AVERAGE
{
  if ( !rb2->isOn() ) rb2->setOn( TRUE );
  rb3->setOn( FALSE );               //power
  rb1->setOn( FALSE );               //timeline
  pr->bolres=1;
  centralw->rpaint->bolresp=1;
  //pr->tiline=1;
  //centralw->rpaint->tilinep=1;
  centralw->rpaint->repaint();
}

void Modal::mdg() // MODELLIERUNG
{
  if ( mod->isOn() ) {
    pr->firesp=1;
    centralw->rpaint->firespp=1;
  } else {
    pr->firesp=0;
    centralw->rpaint->firespp=0;
  }
  centralw->rpaint->repaint();
}

void Modal::mtc() 
{
  if ( tc->isOn() ) {
    pr->tiline=1;
    centralw->rpaint->tilinep=1;
  } else {
    pr->tiline=0;
    centralw->rpaint->tilinep=0;
  }
  centralw->rpaint->repaint();
}

void Modal::psm() // POWER SPECTRUM
{
  if ( !rb3->isOn() ) rb3->setOn( TRUE );
  rb1->setOn( FALSE );               //timeline
  if (cnd_m) rb2->setOn( FALSE );    //trialav
  pr->bolres=2;
  centralw->rpaint->bolresp=2;
  //pr->tiline=0;
  //centralw->rpaint->tilinep=0;
  centralw->rpaint->repaint();
}

void Modal::rast()
{
  if ( raster->isOn() ) centralw->rpaint->raster=1;
  else centralw->rpaint->raster=0;
  centralw->rpaint->repaint();
}

void Modal::hgsw()
{
  if (pr->hgfarbe[1]==1) {
    pr->hgfarbe[1]=0;
  } else {
    pr->hgfarbe[1]=1;
  }
  centralw->rpaint->repaint();
}


