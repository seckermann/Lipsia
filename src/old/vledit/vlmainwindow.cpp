/***************************************************************************
 *   Copyright (C) 2005 by Hannes Niederhausen                             *
 *   niederhausen@cbs.mpg.de                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "vlmainwindow.h"
#include "vlglwidget.h"
#include "datamanager.h"
#include "uiconfig.h"
#include "aboutdialog.h"

#include <iostream>
#include <sstream>
#include <math.h>

#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qspinbox.h>
#include <qfiledialog.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <qaction.h>
#include <qcombobox.h>
#include <qwhatsthis.h>
#include <qimage.h>

using namespace std;

extern "C" {
      extern void VPixel2Tal(float [3],float [3],float [3],int,int,int,
              float *,float *,float *);
}

static unsigned char connect1_image[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x12,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x56, 0xce, 0x8e, 0x57, 0x00, 0x00, 0x01,
    0x21, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xcd, 0xd3, 0xbb, 0x8d, 0x84,
    0x30, 0x14, 0x85, 0xe1, 0xdf, 0xa3, 0x6d, 0x82, 0x0c, 0x69, 0x1a, 0x80,
    0x1c, 0x22, 0x9a, 0x70, 0x48, 0x09, 0x53, 0x02, 0x25, 0x50, 0x82, 0xc9,
    0x68, 0xc2, 0x11, 0x14, 0x40, 0x48, 0x30, 0x96, 0xc8, 0x5c, 0xc6, 0xdd,
    0x04, 0x2c, 0x58, 0x86, 0xc7, 0x6e, 0xb4, 0x57, 0xb2, 0x78, 0xd9, 0x1f,
    0xc7, 0x60, 0xc3, 0xdf, 0x4a, 0x44, 0x44, 0x8c, 0x31, 0x72, 0xab, 0xf3,
    0xaa, 0x1d, 0x21, 0x97, 0x90, 0xac, 0x6b, 0x35, 0xe0, 0xf7, 0xc8, 0xfb,
    0xfd, 0x0e, 0x6d, 0xc1, 0xce, 0x90, 0xc7, 0x87, 0xe9, 0xa0, 0x94, 0xa2,
    0xef, 0xfb, 0x70, 0xd3, 0x39, 0x87, 0x88, 0x90, 0xa6, 0x29, 0x65, 0x59,
    0x02, 0xa8, 0xcb, 0x24, 0x5d, 0xd7, 0x85, 0x6f, 0x63, 0x8c, 0xd9, 0x25,
    0x4b, 0x92, 0xe4, 0xe3, 0x94, 0xd4, 0x0a, 0xc1, 0x39, 0x07, 0x80, 0xf7,
    0x9e, 0x3c, 0xcf, 0x49, 0x92, 0x84, 0xd7, 0xeb, 0x45, 0x96, 0x65, 0x61,
    0xc0, 0xf3, 0xf9, 0x44, 0x29, 0xb5, 0x4b, 0xf5, 0x00, 0xa4, 0xaa, 0xaa,
    0xcd, 0x54, 0xa2, 0x28, 0xa2, 0xeb, 0x3a, 0x86, 0x61, 0xa0, 0x2c, 0xcb,
    0xcd, 0xb3, 0xf5, 0xf9, 0x4f, 0x88, 0x71, 0x1c, 0xb1, 0xd6, 0xe2, 0xbd,
    0xdf, 0x61, 0x00, 0x75, 0x5d, 0xd3, 0xf7, 0x3d, 0xde, 0x7b, 0xac, 0xb5,
    0xc7, 0xd0, 0x15, 0xb6, 0x24, 0xb3, 0xd6, 0x32, 0x8e, 0xe3, 0x21, 0xa4,
    0xda, 0xb6, 0x05, 0xa0, 0x28, 0x8a, 0x1d, 0x06, 0x60, 0x8c, 0x09, 0x2f,
    0x9b, 0xfb, 0xee, 0xfe, 0xda, 0x92, 0x48, 0xb5, 0x6d, 0x4b, 0x96, 0x65,
    0xc4, 0x71, 0xbc, 0xc1, 0x9c, 0x73, 0x4c, 0xd3, 0x04, 0x70, 0x88, 0x00,
    0x7c, 0xcd, 0x47, 0x11, 0x11, 0x9a, 0xa6, 0x61, 0x9a, 0xa6, 0x10, 0x3f,
    0x8e, 0xe3, 0xcd, 0xf5, 0x11, 0xb2, 0x40, 0x01, 0x99, 0x17, 0x1b, 0x5a,
    0x6b, 0x80, 0x80, 0x9c, 0x25, 0x09, 0x75, 0xb0, 0xec, 0x45, 0x6b, 0x2d,
    0x5a, 0xeb, 0x5b, 0x7b, 0x0a, 0x80, 0x93, 0x0d, 0x78, 0x1f, 0xf9, 0x97,
    0xf5, 0x0d, 0xce, 0x55, 0x01, 0xa3, 0x6f, 0x7a, 0xe6, 0x87, 0x00, 0x00,
    0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

static unsigned char connect0_image[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x12,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x56, 0xce, 0x8e, 0x57, 0x00, 0x00, 0x01,
    0x0a, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xcd, 0xd1, 0xc1, 0x8d, 0x83,
    0x30, 0x10, 0x85, 0xe1, 0xdf, 0x51, 0x9a, 0xe0, 0x46, 0x21, 0x58, 0x42,
    0xa2, 0x09, 0x1f, 0xd3, 0x49, 0x5c, 0x0a, 0xb9, 0xa5, 0x09, 0x24, 0x22,
    0x9a, 0xc8, 0x05, 0x89, 0x1b, 0x65, 0xbc, 0x3d, 0xec, 0xda, 0xd8, 0x04,
    0x12, 0x72, 0xdb, 0x77, 0xc2, 0x83, 0xf9, 0x34, 0x33, 0x18, 0xbe, 0x8c,
    0x84, 0xd2, 0xb3, 0x31, 0x18, 0x80, 0xf3, 0xb7, 0x1f, 0x3f, 0x1e, 0x4b,
    0xbd, 0xef, 0x97, 0xe7, 0x43, 0x50, 0xfa, 0x71, 0x0a, 0x78, 0xff, 0xdb,
    0xcd, 0x61, 0xa8, 0xef, 0xa1, 0xae, 0x97, 0x73, 0x0a, 0x84, 0xac, 0x0b,
    0xda, 0xa9, 0xe3, 0x3d, 0x4a, 0xb1, 0xba, 0xce, 0xef, 0x9c, 0x52, 0x44,
    0x12, 0xc3, 0x30, 0xa4, 0x60, 0xcc, 0xf5, 0xfa, 0xd2, 0x65, 0x76, 0x27,
    0x40, 0x92, 0xc4, 0x38, 0x8e, 0x14, 0x45, 0xf1, 0x82, 0x49, 0x68, 0xbd,
    0xe4, 0x74, 0xd1, 0x61, 0x04, 0x79, 0xef, 0x69, 0x9a, 0x86, 0xa2, 0x28,
    0xe2, 0x8b, 0x79, 0x9e, 0xb1, 0xd6, 0x22, 0x2d, 0xcb, 0xde, 0x5a, 0x72,
    0xc8, 0x19, 0xe0, 0xf9, 0x7c, 0x02, 0x64, 0x58, 0xe8, 0xcc, 0x18, 0x8b,
    0xf7, 0xfb, 0x40, 0x06, 0x7d, 0xc2, 0xac, 0xb5, 0xa1, 0xfb, 0xdd, 0x9c,
    0x00, 0x73, 0xbf, 0xdf, 0x23, 0xd2, 0x75, 0x1d, 0xf3, 0x3c, 0x67, 0x97,
    0xda, 0xb6, 0x85, 0x8d, 0x1f, 0xb0, 0x86, 0x08, 0x58, 0x55, 0x55, 0x94,
    0x65, 0x99, 0x61, 0xe3, 0x38, 0x32, 0x4d, 0xd3, 0x3b, 0x23, 0x1b, 0x4d,
    0x92, 0xb8, 0xdd, 0x6e, 0x4c, 0xd3, 0x14, 0xc7, 0x2c, 0xcb, 0x32, 0x3b,
    0x7f, 0x82, 0x22, 0x72, 0xb9, 0x5c, 0x00, 0x70, 0xce, 0x01, 0x44, 0xe4,
    0x6f, 0xf4, 0xb7, 0x3b, 0x42, 0x92, 0xda, 0xb6, 0x15, 0xf9, 0x0e, 0xe4,
    0x9c, 0x93, 0x73, 0x6e, 0x5d, 0xdf, 0xcf, 0x06, 0x12, 0xb1, 0xc3, 0xc8,
    0xbf, 0xcc, 0x0f, 0x8d, 0x78, 0x98, 0x1f, 0xb7, 0x37, 0x7b, 0x72, 0x00,
    0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

vlMainWindow::vlMainWindow(QWidget* parent, const char* name, WFlags fl)
 : MainWindow(parent, name, fl)
{
	m_serverConnection=new vlServerConnection();
    QImage img;
    img.loadFromData(connect0_image, sizeof(connect0_image), "PNG");
    connect0_img = img;
    img.loadFromData(connect1_image, sizeof(connect1_image), "PNG");
    connect1_img = img;
}


vlMainWindow::~vlMainWindow()
{
	delete m_serverConnection;
}

/**
 * Erstellt den mittelteil des Fensters, bestehend aus den 3 OpenGL Ansichten.
 */
void vlMainWindow::createCenterWidget()
{
	//anfragen nach centralWidget
	QWidget* widget=centralWidget();
	if (widget==NULL)
	{
		//es gibt keins, also legen wir ein neues an
		widget = new QWidget(this);
	}
	//setzen des Layouts
	QHBoxLayout* layout = new QHBoxLayout(widget, 0, 1, "MainWindLayout");

	layout->setAutoAdd(true);

	coronal = new vlGLWidget(widget, CORONAL);
	sagittal = new vlGLWidget(widget, SAGITTAL);
	axial   = new vlGLWidget(widget, AXIAL);

	//setzen der "What's this?"-Hilfe
	QWhatsThis::add(coronal, tr("This is the <b>coronal</b> view of the object."));
	QWhatsThis::add(sagittal, tr("This is the <b>sagittal</b> view of the object."));
	QWhatsThis::add(axial, tr("This is the <b>axial</b> view of the object."));

	//hinzufügen eines "What's this"-Buttons
	QWhatsThis::whatsThisButton(Toolbar);

	//Signal zur Neuerstellung der Views
	connect (coronal, SIGNAL(recreateViews(bool )), sagittal, SLOT(recreateView(bool)));
	connect (coronal, SIGNAL(recreateViews(bool )), axial, SLOT(recreateView(bool)));
	connect (sagittal, SIGNAL(recreateViews(bool )), coronal, SLOT(recreateView(bool)));
	connect (sagittal, SIGNAL(recreateViews(bool )), axial, SLOT(recreateView(bool)));
	connect (axial, SIGNAL(recreateViews(bool )), sagittal, SLOT(recreateView(bool)));
	connect (axial, SIGNAL(recreateViews(bool )), coronal, SLOT(recreateView(bool)));

	connect (UICONFIG, SIGNAL(aimChanged()), sagittal, SLOT(reaim()));
	connect (UICONFIG, SIGNAL(aimChanged()), coronal, SLOT(reaim()));
	connect (UICONFIG, SIGNAL(aimChanged()), axial, SLOT(reaim()));
	connect (UICONFIG, SIGNAL(aimChanged()), this, SLOT(aimMoved()));
	connect (UICONFIG, SIGNAL(scaleChanged()), this, SLOT(zoomChanged()));

	//Signale zur Aktualisierung der VlGLWidgets
	connect (UICONFIG, SIGNAL(recreateViews(bool)), sagittal, SLOT(recreateView(bool)));
	connect (UICONFIG, SIGNAL(recreateViews(bool)), axial, SLOT(recreateView(bool)));
	connect (UICONFIG, SIGNAL(recreateViews(bool)), coronal, SLOT(recreateView(bool)));

	connect (coronal, SIGNAL(mouseMoved(int, int, int )), this, SLOT(mouseMoved(int, int, int )));
	connect (sagittal, SIGNAL(mouseMoved(int, int, int )), this, SLOT(mouseMoved(int, int, int )));
	connect (axial, SIGNAL(mouseMoved(int, int, int )), this, SLOT(mouseMoved(int, int, int )));

	// Signal zum Aktualisieren des Segmentvolumens
	connect (coronal, SIGNAL(volumeChanged()), segWindow, SLOT(volumeChanged()));
	connect (sagittal, SIGNAL(volumeChanged()), segWindow, SLOT(volumeChanged()));
	connect (axial, SIGNAL(volumeChanged()), segWindow, SLOT(volumeChanged()));

	// callback when segment window is closed
	connect (segWindow, SIGNAL(segmentWindowClosed()), viewSegmentWindowAction, SLOT(toggle()));

    // callback when server connection lost.
    connect(m_serverConnection, SIGNAL(serverDown()), this, SLOT(disableSync()));

	// Signal zum Aktualisieren der Filter Drop-Down-Box
	connect (UICONFIG, SIGNAL(filterChanged(const QString &)), this,
            SLOT(filterChanged(const QString &)));

    // Signal zum Schliessen des Segmentfensters
    connect (this, SIGNAL(vlMainWindowClosed()), segWindow, SLOT(close()));


	setCentralWidget(widget);
	//setzen der default werte:
	greyValChanged(10);

	radiusChanged(2);

	viewAction->toggle();

	//setzen des Fokus auf die Coronale ansicht:
	coronal->setFocus();

}

void vlMainWindow::fileExit()
{
	this->close();
}

void vlMainWindow::show()
{
	//segDialog = new vlSegmentDialog((QWidget*) this);
	segWindow = new vlSegmentWindow((QWidget*) this);
	createCenterWidget();
	createStatusBar();

	//show windows according to editing mode
	MainWindow::show();

	// fill filter list with entries
	filterComboBox->insertItem(NN_FILTER, -1);
	filterComboBox->insertItem(BILIN_FILTER, -1);

    // fill display mode combo box
    coordComboBox->insertItem(ANA_MODE, -1);
    coordComboBox->insertItem(SEG_MODE, -1);
    coordComboBox->insertItem(TAL_MODE, -1);

    // set delta start value
    greyValBox->setValue(60);
    greyValChanged(60);

	// enable/disable actions according to editing mode
    // if segment mode
	if(UICONFIG->getMode() == 1) {
		fileSaveAction->setEnabled(FALSE);
		segWindow->show();
	}
	else{
		//toggle actions sind immer "on" --> ausschalten
		viewSegmentWindowAction->setOn(FALSE);
		viewSegmentWindowAction->setEnabled(FALSE);
		markSegmentsAction->setEnabled(FALSE);
    }

	// enable/disable delta checkbox and delta value box according to resolution
	if(DATAMANAGER->image()->getResolution()[0] <
		DATAMANAGER->getSegResolution()[0]) {
			delSegCheckBox->setEnabled(false);
			greyValBox->setEnabled(false);
	}

	resetView();	

    // try to connect to vlserv
	m_serverConnection->connectToServer();

    // no server found -> disable toggle server syncronization button
    if(!m_serverConnection->serverFound())
        toggleSyncAction->setEnabled(false);

}

void vlMainWindow::editDelete( bool val )
{
	if (val) {
		int tmpVal=axial->getDelete();
		if(tmpVal&USE_DELTA)
			tmpVal=DELETE+USE_DELTA;
		else
			tmpVal=DELETE;

		axial->setDelete(tmpVal);
		coronal->setDelete(tmpVal);
		sagittal->setDelete(tmpVal);
	}

	return;
}

void vlMainWindow::radiusChanged( int val )
{
	UICONFIG->setRadius(val);
	coronal->setCursorSize(val);
	axial->setCursorSize(val);
	sagittal->setCursorSize(val);
}

void vlMainWindow::fileSave( )
{
	//Falls kein Bild geladen wurde --> Abbruch.
	if(!DATAMANAGER->isValid()){
		qFatal("fileSave error: no valid core image found!");
		//TODO Sollte der Savebutton in diesem Fall nicht lieber deaktiviert sein?
		QMessageBox::warning((QWidget*)this, tr("Error!"), tr("No vista image loaded. Aborting save process."), QMessageBox::Ok,  QMessageBox::NoButton,  QMessageBox::NoButton);
		return;
	}

	QString filename = QFileDialog::getSaveFileName(
		QString::null,
		"Vista Images (*.v)",
		this,
		"save file dialog",
		"Choose a file to save to");

	if ( filename != "" )
	{
		if (QFile::exists(filename))
		{
			QString text=tr("The file ") +filename+ tr(" already exists.\nDo you want to overwrite it?");
			if(QMessageBox::question(this, "overwrite file??", text, "Ja", "Nein")==1)
				return;
		}
		DATAMANAGER->save(filename);
	}

}

void vlMainWindow::closeEvent( QCloseEvent * e )
{
	m_serverConnection->disconnectFromServer();
	e->accept();
    emit vlMainWindowClosed();
}

void vlMainWindow::greyValChanged( int x)
{
	UICONFIG->setDelta(x);
	coronal->setGreyDelta(x);
	axial->setGreyDelta(x);
	sagittal->setGreyDelta(x);
}

void vlMainWindow::delSegCheckedChanged( int val)
{
	int tmpVal=coronal->getDelete();

	if (val!=0) {
		tmpVal+=USE_DELTA;
	}
	else
		tmpVal-=USE_DELTA;

	axial->setDelete(tmpVal);
	coronal->setDelete(tmpVal);
	sagittal->setDelete(tmpVal);
}

void vlMainWindow::createStatusBar()
{
	valMouse = new QLCDNumber (5, statusBar(), "valMaus");
	valMouse->setSegmentStyle ( QLCDNumber::Flat );
	statusBar()->addWidget(valMouse , 2);

	mouseCoords = new QLabel(statusBar(), "mouseCoords");
	statusBar()->addWidget(mouseCoords, 4);

	valAim = new QLCDNumber (5, statusBar(), "valAim");
	valAim->setSegmentStyle ( QLCDNumber::Flat );
	statusBar()->addWidget(valAim, 2);

	aimCoords = new QLabel(statusBar(), "aimCoords");
	statusBar()->addWidget(aimCoords, 4);

	scale = new QLabel(statusBar(), "scale");
	statusBar()->addWidget(scale, 2);

	modeLabel = new QLabel(statusBar(), "mode");
	QString modeText = "";
	if(UICONFIG->getMode() == 0) {
			modeLabel->setPaletteForegroundColor(Qt::darkGreen);
			modeText+="Anatomie";
	}
	else {
			modeLabel->setPaletteForegroundColor(Qt::red);
			modeText+="Segment";
	}
	modeLabel->setText(modeText);
	statusBar()->addWidget(modeLabel, 0, TRUE);
}

void vlMainWindow::mouseMoved( int band, int row, int column )
{
	char tmp[256]="";

    // save mouse pos for later use.
    UICONFIG->setMousePos(band,row,column);

    if(UICONFIG->coordMode() == ANA_MODE) {
        sprintf(tmp, "%d %d %d",column, row, band);
    }
    else if(UICONFIG->coordMode() == TAL_MODE) {
        float x,y,z;
        // bestimme resolution
        float* res = DATAMANAGER->image()->getResolution();
        // bestimme ca in mm
        float* ca = new float[3];
        ca[0]=DATAMANAGER->ca()[0]*res[0];
        ca[1]=DATAMANAGER->ca()[1]*res[1];
        ca[2]=DATAMANAGER->ca()[2]*res[2];
        // bestimme extend in mm
        float* extent = new float[3];
        extent[0]=DATAMANAGER->extent()[0]*res[0];
        extent[1]=DATAMANAGER->extent()[1]*res[1];
        extent[2]=DATAMANAGER->extent()[2]*res[2];

        // Talairachkoordinaten bestimmen.
        VPixel2Tal(ca,res,extent,band,row,column,&x,&y,&z);

        x = rintf(x);
        y = rintf(y);
        z = rintf(z);

        sprintf(tmp, "%d %d %d", (int)x,(int)y,(int)z);

        delete extent;
        delete ca;

    }
    else if(UICONFIG->coordMode() == SEG_MODE) {
        // gelieferte Anatomiekoordinaten an Segmentkoordinaten anpassen
        float resFactor = DATAMANAGER->image()->getResolution()[0] /
            DATAMANAGER->getSegResolution()[0];

        int ba = (int)rintf(band * resFactor);
        int ro = (int)rintf(row * resFactor);
        int co = (int)rintf(column * resFactor);

        sprintf(tmp, "%d %d %d",co, ro, ba);

}

    mouseCoords->setText(tmp);

    sprintf(tmp, "%d", DATAMANAGER->image()->get(band, row, column));
    valMouse->display(tmp);
}

void vlMainWindow::aimMoved( )
{
	char tmp[256]="";

    if(UICONFIG->coordMode() == ANA_MODE) {
        sprintf(tmp, "%d %d %d",(int)rintf(UICONFIG->column()), (int)rintf(UICONFIG->row()),
                (int)rintf(UICONFIG->band()));
    }
    else if(UICONFIG->coordMode() == TAL_MODE) {
        float x,y,z;
        // bestimme resolution
        float* res = DATAMANAGER->image()->getResolution();
        // bestimme ca
        float* ca = new float[3];
        ca[0]=rintf(DATAMANAGER->ca()[0]);
        ca[1]=rintf(DATAMANAGER->ca()[1]);
        ca[2]=rintf(DATAMANAGER->ca()[2]);
        // bestimme extend in mm
        float* extent = new float[3];
        extent[0]=DATAMANAGER->extent()[0]*res[0];
        extent[1]=DATAMANAGER->extent()[1]*res[1];
        extent[2]=DATAMANAGER->extent()[2]*res[2];

        // Talairachkoordinaten bestimmen.
        VPixel2Tal(ca,res,extent,(int)rintf(UICONFIG->band()),(int)rintf(UICONFIG->row()),
                (int)rintf(UICONFIG->column()),&x,&y,&z);

        x = rintf(x);
        y = rintf(y);
        z = rintf(z);

        sprintf(tmp, "%d %d %d", (int)x,(int)y,(int)z);

        delete extent;
        delete ca;

    }
    else if(UICONFIG->coordMode() == SEG_MODE) {
        // gelieferte Anatomiekoordinaten an Segmentkoordinaten anpassen
        float resFactor = DATAMANAGER->image()->getResolution()[0] /
            DATAMANAGER->getSegResolution()[0];

        int ba = (int)rintf(UICONFIG->band() * resFactor);
        int ro = (int)rintf(UICONFIG->row() * resFactor);
        int co = (int)rintf(UICONFIG->column() * resFactor);

        sprintf(tmp, "%d %d %d",co, ro, ba);

    }

	aimCoords->setText(tmp);

	sprintf(tmp, "%d", DATAMANAGER->image()->get((int)UICONFIG->band(), (int)UICONFIG->row(),
                (int)UICONFIG->column()));
	valAim->display(tmp);
}

void vlMainWindow::zoomChanged( )
{
	char tmp[256]="";

	sprintf(tmp, "Zoom:%.2f", UICONFIG->scale());
	scale->setText(tmp);
}

void vlMainWindow::reset()
{
  resetView();
  UICONFIG->mouseReleased();
  
}

void vlMainWindow::resetView( )
{
	float *tmp=DATAMANAGER->ca();

	if (tmp==NULL) {
		tmp=DATAMANAGER->midPoint();
	}

	if (tmp==NULL) {
		return;
	}
	float scale=(float)sagittal->width()/(float)DATAMANAGER->image()->height();
	UICONFIG->setScale(scale);

	UICONFIG->setAim(tmp[2], tmp[1], tmp[0]);

	//reset camera position
	sagittal->reset();
	axial->reset();
	coronal->reset();
}

void vlMainWindow::markSegment(bool val)
{
	if (val) {
		int tmp=axial->getDelete();
		if (tmp&USE_DELTA)
			tmp=MARK+USE_DELTA;
		else
			tmp=MARK;

		axial->setDelete(tmp);
		coronal->setDelete(tmp);
		sagittal->setDelete(tmp);
	}
}



void vlMainWindow::zoomIn( )
{
	UICONFIG->addScale(SCALE_STEP);


	axial->recreateView(false);
	coronal->recreateView(false);
	sagittal->recreateView(false);

	return;
}

void vlMainWindow::zoomOut( )
{
	UICONFIG->addScale(-SCALE_STEP);

	axial->recreateView(false);
	coronal->recreateView(false);
	sagittal->recreateView(false);

	return;
}

void vlMainWindow::viewActionToggled( bool )
{
	int tmp=axial->getDelete();
	if (tmp&USE_DELTA)
		tmp=VIEW+USE_DELTA;
	else
		tmp=VIEW;

	axial->setDelete(tmp);
	coronal->setDelete(tmp);
	sagittal->setDelete(tmp);
}

void vlMainWindow::viewSegmentWindow(bool visible)
{
	if(visible )
		segWindow->show();
	else
		segWindow->hide();
}

void vlMainWindow::filterComboBoxChanged(const QString & newVal) {

	/* sagen wir der UI Konfiguration bescheid, dass etwas passiert ist
	 * um den Rest kümmert sich UICONFIG.
	 */
	UICONFIG->setFilter(newVal);
}

void vlMainWindow::filterChanged(const QString & newVal) {

	/* 1. Die Combobox */
	// ist eine Aktualisierung notwendig?
	if(!((filterComboBox->currentText() != QString::null) && (filterComboBox->currentText().compare(newVal) != 0)))
		filterComboBox->setCurrentText(newVal);

	/* 2. Die Widgets */
	/**Die Widgets neu zeichnen und dabei die Textur neu aufbauen,
	da sich die Filtermethode geaendert hat. */
	axial->recreateView(true);
	coronal->recreateView(true);
	sagittal->recreateView(true);

}

void vlMainWindow::coordComboBox_activated( const QString & newVal ){

    // nur in den Talairachmodus wechseln wenn ein gueltiger ca-Wert
    // zur Verfuegung steht.
    if((newVal == TAL_MODE) && (DATAMANAGER->ca() == NULL)) {
        // Liste durchlaufen und das vorherige Element auswaehlen
        for(int i=0; i<coordComboBox->count();i++) {
            if(coordComboBox->text(i) == UICONFIG->coordMode()) {
                coordComboBox->setCurrentItem(i);
                break;
            }
        }
        qWarning("Anatomie image doesn't provide a ca-Value. No Talairach mode available");
        return;
    }


    // Ab hier funktioniert alles auf gewohnte Weise.
    UICONFIG->setCoordMode(newVal);

    // Zielkreuzkoordinaten
    aimMoved();

    // Mauskoordinaten
    // bisherige Werte verwenden.
    int* mp = UICONFIG->mousePos();
    mouseMoved(mp[0],mp[1],mp[2]);

}

void vlMainWindow::toggleSyncAction_activated() {
    // from sync to no-sync
    if(UICONFIG->sync()) {
        UICONFIG->setSync(false);
        toggleSyncAction->setIconSet(QIconSet(connect0_img));
    }
    else {
        UICONFIG->setSync(true);
        toggleSyncAction->setIconSet(QIconSet(connect1_img));
    }

}

void vlMainWindow::disableSync() {
    toggleSyncAction->setEnabled(false);
}

void vlMainWindow::helpAbout() {
    AboutDialog(this).exec();
}

void vlMainWindow::leaveEvent ( QEvent *) {
     char tmp[256]="";

     sprintf(tmp, "   ");
     mouseCoords->setText(tmp);

     sprintf(tmp, " ");
     valMouse->display(tmp);
}

