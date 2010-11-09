#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <qmainwindow.h>
#include <qaction.h>
#include <qapplication.h>
#include <qlcdnumber.h>
#include <qwhatsthis.h>
#include <qlabel.h>
#include <qslider.h>
#include <qpushbutton.h>

#include "Configurator.h"
#include "CentralWidget.h"
#include "PreferenceDialog.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(Configurator *conf, QWidget *parent, QString name, QApplication *app);
	virtual ~MainWindow();
	
	const Configurator* configurator() {return m_conf;};
	
public slots:
	void closeEvent(QCloseEvent * e);
	void mousePosChanged();
	void crossPosChanged();
	void timeValueChanged();
	void playButtonPressed();
	void colorsChanged();
	void showPreferences();
	void setCoordinates();
	void openPlotDialog();
	
protected:
	void keyReleaseEvent(QKeyEvent*);
	void timerEvent( QTimerEvent * );
private:
	
	CentralWidget* m_imageWidget;
	
	// widget creation methods for better encapsulation
	
	// creates the menu
	void createMenu();
	
	// creates the widgets of the status bar
	void createStatusBar();
	
	// creates the toolbar
	void createToolbar();
	
	void createActions();
	
	void createCentralWidget();
	
	void updateAimValue();
	
	void updateMouseValue();
	
	QApplication *m_qapp;
	Configurator* m_conf;
	
	// Action List
	QAction*	m_quitAction;
	QAction*	m_preferenceAction;
	QAction*	m_plotDialogAction;
	
	QLCDNumber *m_rtValue;
	QLabel* m_rtCoords;
	QLCDNumber* m_aimValue;
	QLabel* m_aimCoords;
	QLabel* m_timeValue;
	QSlider* m_timeSlider;
	QPushButton* m_playButton;
	
	// the pref dlg
	PreferenceDialog* m_prefDlg;
};

#endif /*MAINWINDOW_H_*/
