/****************************************************************************
** Form interface generated from reading ui file 'mainwindow.ui'
**
** Created: Wed Sep 12 15:53:57 2007
**      by: The User Interface Compiler ($Id: mainwindow.h 2400 2007-09-12 13:54:51Z karstenm $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qmainwindow.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QAction;
class QActionGroup;
class QToolBar;
class QPopupMenu;
class QComboBox;
class QLabel;
class QSpinBox;
class Spacer;
class QCheckBox;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow( QWidget *parent = 0, const char *name = 0, WFlags fl = WType_TopLevel );
	~MainWindow();

	QComboBox *filterComboBox;
	QComboBox *coordComboBox;
	QLabel *textLabel1;
	QSpinBox *boxSize;
	QLabel *textLabel3;
	QSpinBox *greyValBox;
	QSpacerItem *spacer1;
	QCheckBox *delSegCheckBox;
	QMenuBar *menubar;
	QPopupMenu *fileMenu;
	QPopupMenu *View;
	QPopupMenu *Edit;
	QPopupMenu *helpMenu;
	QToolBar *Toolbar;
	QAction *fileSaveAction;
	QAction *fileExitAction;
	QAction *helpContentsAction;
	QAction *helpIndexAction;
	QAction *helpAboutAction;
	QAction *resetViewAction;
	QAction *zoomInAction;
	QAction *zoomOutAction;
	QActionGroup *editGroup;
	QAction *deleteSegmentsAction;
	QAction *markSegmentsAction;
	QAction *viewAction;
	QAction *viewSegmentWindowAction;
	QAction *toggleSyncAction;

public slots:
	virtual void fileNew();
	virtual void fileSave();
	virtual void fileSaveSegments();
	virtual void filePrint();
	virtual void fileExit();
	virtual void helpIndex();
	virtual void helpContents();
	virtual void helpAbout();
	virtual void editDelete( bool val );
	virtual void greyValChanged( int );
	virtual void delSegCheckedChanged( int );
	virtual void reset();
	virtual void markSegment( bool );
	virtual void zoomIn();
	virtual void zoomOut();
	virtual void radiusChanged( int );
	virtual void viewActionToggled( bool );
	virtual void viewSegmentWindow( bool );
	virtual void calcVolume();
	virtual void filterComboBoxChanged( const QString & );
	virtual void coordComboBox_activated( const QString & );
	virtual void toggleSyncAction_activated();

protected:

protected slots:
	virtual void languageChange();

private:
	QPixmap image0;
	QPixmap image1;
	QPixmap image2;
	QPixmap image3;
	QPixmap image4;
	QPixmap image5;
	QPixmap image6;
	QPixmap image7;

};

#endif // MAINWINDOW_H
