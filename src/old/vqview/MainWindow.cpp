#include "MainWindow.h"

#include <iostream>

#include <lip.xpm>
#include <setcoord.xpm>
#include <valuedetail.xpm>

#include <qmenubar.h>
#include <qstatusbar.h>
#include <qlayout.h>
#include <qsizepolicy.h>
#include <qcolor.h>
#include <qtoolbar.h> 
#include <qtoolbutton.h> 
#include <qstringlist.h>
#include <qinputdialog.h> 
#include <qpixmap.h>
#include <qmessagebox.h> 

#include "CentralWidget.h"
#include "plotdialog.h"

using namespace std;

MainWindow::MainWindow(Configurator *conf, QWidget *parent, QString name, QApplication *qapp) :
	QMainWindow(parent, name) {
	m_conf = conf;
	m_qapp = qapp;
	m_prefDlg = NULL;
	createActions();
	createMenu();
	createToolbar();
	createCentralWidget();
	createStatusBar();

	connect(m_conf, SIGNAL(timeValueChanged()),
		this, SLOT(timeValueChanged()));
	connect(m_conf, SIGNAL(colorsChanged()),
			this, SLOT(colorsChanged()));
}

MainWindow::~MainWindow() {
}

void MainWindow::createCentralWidget() {
	QWidget* container = new QWidget(this);
	QVBoxLayout* cl = new QVBoxLayout(container);

	m_imageWidget = new CentralWidget(m_conf, container);
	cl->addWidget(m_imageWidget);

	// -------------- upper widget with time widgets
	QWidget* upperWidget = new QWidget(container);
	upperWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	cl->addWidget(upperWidget);
	QHBoxLayout* ul = new QHBoxLayout(upperWidget);
	ul->setAutoAdd(true);

	m_playButton = new QPushButton("Play", upperWidget);
	QWhatsThis::add( m_playButton, "The button to start an animation. If the animation reaches the last timeline"
				   "it starts at the beginning. The animationspeed can be set in the preference dialog");
	m_playButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(m_playButton, SIGNAL(pressed()),
			this, SLOT(playButtonPressed()));
	
	
	m_timeSlider = new QSlider(Qt::Horizontal, upperWidget);
	m_timeSlider->setMinValue(0);
	m_timeSlider->setMaxValue(m_conf->numOfTimeValues()-1);
	m_timeSlider->setValue(0);
	m_timeSlider->setFocusPolicy(QWidget::NoFocus);
	QWhatsThis::add(m_timeSlider, "The slider to set the time visible timestep");
	connect(m_timeSlider, SIGNAL(sliderMoved (int)),
			m_conf, SLOT(changeTimeValue(int)));

	
	m_timeValue = new QLabel(upperWidget);
	m_timeValue->setText("000");


	setCentralWidget(container);
}

void MainWindow::timerEvent(QTimerEvent*) {
	m_conf->setTime(m_conf->time()+1);
	m_timeSlider->setValue(m_conf->time());
}

void MainWindow::playButtonPressed() {
	static int id=-1;
	if (id != -1) {
		killTimer(id);
		id = -1;
		m_playButton->setText("Start");
	} else {
		m_playButton->setText("Stop");
		id = startTimer(1000/m_conf->fps());
	}
}

void MainWindow::timeValueChanged() {
	int newVal = m_conf->time();
	QString tmp = QString::number(newVal);
	
	m_timeValue->setText(tmp.fill('0', 3-tmp.length())+QString::number(newVal));
	m_imageWidget->updateImageData();
	updateAimValue();
	updateMouseValue();
}

void MainWindow::colorsChanged() {
	m_imageWidget->updateImageData();
}

void MainWindow::setCoordinates() {
	bool ok = FALSE;

	QString s;
	s = QInputDialog::getText( tr( "Coordinates [ X Y Z ]" ), tr( "Please enter coordinates (separated with spaces)" ), 
								   QLineEdit::Normal, QString::null, &ok, this );
	
	if ( ok && !s.isEmpty() ) {
    // user entered something and pressed ok
		QStringList list = QStringList::split(" ", s);
		if (list.count() != 3) {
			QMessageBox::warning( this, "Warning",
								  "Please specify 3 coordinates\nseparated with spaces");
			return; 
		}
		
		int pos[3];
		for (int i=0; i<3; i++) {
			QString tmp = *list.at(i);
			pos[i] = tmp.toInt();
		}
		
		int maxPos[3];
		
		maxPos[0] = m_conf->imageManager().cols();
		maxPos[1] = m_conf->imageManager().rows();
		maxPos[2] = m_conf->imageManager().bands();
		
		for (int i=0; i<3; i++) {
			if (pos[i]>=maxPos[i]) {
				QMessageBox::warning( this, "Warning",
							"Illegal coordinates specified");
				return;
			}
		}
		m_conf->setPosition( pos[2], pos[1], pos[0]);
	}
}

void MainWindow::updateAimValue() {
	int x = m_conf->imageManager().getValue(
			m_conf->band(),
			m_conf->row(),
			m_conf->col(),
			m_conf->time());
	m_aimValue->display(x);
}

void MainWindow::updateMouseValue() {
	if (m_conf->mouseValid()) {
		int x = m_conf->imageManager().getValue(
				m_conf->mouseBand(),
				m_conf->mouseRow(),
				m_conf->mouseCol(),
				m_conf->time());
		m_rtValue->display(x);
		m_rtValue->repaint(); // repaint necessary because of render errors of qt
	} else {
		m_rtValue->erase();
	}
}

void MainWindow::openPlotDialog()
{
	PlotDialog *dlg = new PlotDialog(m_conf, this, "PltDlg");
	dlg->show();
}

void MainWindow::crossPosChanged() {
	QString result="";
	result += QString::number(m_conf->col());
	result += " ";
	result += QString::number(m_conf->row());
	result += " ";
	result += QString::number(m_conf->band());

	m_aimCoords->setText(result);
	updateAimValue();
}

void MainWindow::mousePosChanged() {
	if (m_conf->mouseValid()) {
		QString result="";
		result += QString::number(m_conf->mouseCol());
		result += " ";
		result += QString::number(m_conf->mouseRow());
		result += " ";
		result += QString::number(m_conf->mouseBand());
	
		m_rtCoords->setText(result);
	} else {
		m_rtCoords->setText("");
	}
	updateMouseValue();
}

void MainWindow::keyReleaseEvent(QKeyEvent* event) {
	if (event->key()==Qt::Key_Down) {
		m_conf->setPosition(m_conf->band()+1, -1, -1);
	} else if (event->key()==Qt::Key_Up) {
		m_conf->setPosition(m_conf->band()-1, -1, -1);
	} else if (event->key()==Qt::Key_Left) {
		m_conf->setPosition(-1, m_conf->row()-1, -1);
	} else if (event->key()==Qt::Key_Right) {
		m_conf->setPosition(-1, m_conf->row()+1, -1);
	} else if (event->key()==Qt::Key_Plus) {
		int time = m_conf->time()+1;
		if (time>m_conf->numOfTimeValues())
			time = 0;
		m_timeSlider->setValue(time);
		m_conf->setTime(time);
	} else if (event->key()==Qt::Key_Minus) {
		int time = m_conf->time()-1;
		if (time<0)
			time=m_conf->numOfTimeValues()-1;
			
		m_timeSlider->setValue(time);
		m_conf->setTime(time);
	} else {
		event->ignore();
		return;
	}
	m_imageWidget->updateImageData();
}

void MainWindow::createMenu() {
	QMenuBar* menubar = new QMenuBar(this);

	QPopupMenu* filePopup = new QPopupMenu(this);
	m_quitAction->addTo(filePopup);

	QPopupMenu* optionPopup = new QPopupMenu(this);
	m_preferenceAction->addTo(optionPopup);
	
	
	menubar->insertItem("&File", filePopup);
	menubar->insertItem("&Options", optionPopup);
}

void MainWindow::createStatusBar() {

	QStatusBar* sBar = statusBar();
	
	m_rtValue = new QLCDNumber ( 6, sBar, "mouse value" );
	m_rtValue->setSegmentStyle(QLCDNumber::Filled );
	m_rtValue->setMinimumWidth(90);
	QWhatsThis::add(m_rtValue, "Actual zscore of the voxel.");
	sBar->addWidget(m_rtValue, 2);
	m_rtValue->display("");

	m_rtCoords = new QLabel(sBar);
	m_rtCoords->setFont(QFont("arial", 10, QFont::Normal, FALSE) );
	QWhatsThis::add(m_rtCoords, "This widget displays the actual mouse position. ");
	sBar->addWidget(m_rtCoords, 6);

	m_aimValue = new QLCDNumber ( 6, sBar, "mouse value" );
	m_aimValue->setSegmentStyle(QLCDNumber::Filled );
	m_aimValue->setMinimumWidth(90);
	QWhatsThis::add(m_aimValue, "zscore of the aimed voxel.");
	sBar->addWidget(m_aimValue, 2);
	m_aimValue->display("");

	m_aimCoords = new QLabel(sBar);
	m_aimCoords->setFont(QFont("arial", 10, QFont::Normal, FALSE) );
	QWhatsThis::add(m_aimCoords, "This widget displays the aiming cross position. ");
	sBar->addWidget(m_aimCoords, 6);

	QLabel *logo = new QLabel(sBar, "message", 0);
	logo->setPixmap(lip);
	QWhatsThis::add(logo, "This is the logo only.");
	sBar->addWidget(logo, 1, FALSE);

	connect(m_conf, SIGNAL(mousePositionChanged()), this, SLOT(mousePosChanged()));
	connect(m_conf, SIGNAL(crossPositionChanged()), this, SLOT(crossPosChanged()));

	crossPosChanged();

}

void MainWindow::createToolbar() {
	QToolBar* toolBar = new QToolBar(this, "toolbar");
	/* PlotDialog*/
	m_plotDialogAction->addTo(toolBar);
	
	/* Set coordindates */
	QPixmap  setcoordIcon( setcoord_xpm );
	
	QToolButton* toolb = new QToolButton( setcoordIcon, "Set coordinates", QString::null, this, SLOT(setCoordinates()), toolBar, "Set coordinates" );
	QWhatsThis::add( toolb, "The button to set coordinates. " 
			"Click to specify coordinates." );
	
	QWhatsThis::whatsThisButton(toolBar);
	
	
}

void MainWindow::createActions() {
	m_quitAction = new QAction("&Quit", CTRL+Key_Q , this, "quit");
	connect(m_quitAction, SIGNAL(activated()), m_qapp, SLOT(quit()));
	
	m_preferenceAction = new QAction("&Preferences", CTRL+Key_P, this, "prefs");
	connect(m_preferenceAction, SIGNAL(activated()), this, SLOT(showPreferences()));
	
	m_plotDialogAction = new QAction("&Show Voxel Details", CTRL+Key_D, this, "details");
	m_plotDialogAction->setIconSet(QPixmap(valuedetail_xpm));
	connect(m_plotDialogAction, SIGNAL(activated()), this, SLOT(openPlotDialog()));
}

//****************** The Slots of this QObject *************************

void MainWindow::showPreferences() {
	if (m_prefDlg==NULL)
		m_prefDlg = new PreferenceDialog(m_conf);
		
	if (m_prefDlg->isVisible()) {
		m_prefDlg->setFocus();
	} else
		m_prefDlg->show();
}

void MainWindow::closeEvent(QCloseEvent*) {
	m_qapp->quit();
}
