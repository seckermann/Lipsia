#ifndef OF_H
#define OF_H

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <qtoolbar.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qcolor.h>
#include <qlcdnumber.h>
#include <qtoolbutton.h>
#include <qmainwindow.h>

#include "CW.h"

class XView : public QMainWindow
{
	Q_OBJECT
public:
	XView( QWidget *parent = 0, const char *name = 0 );
	~XView();

public slots:


private:
	CW   *center;
	QMenuBar   *menubar;
	QToolButton *toolb;
	QToolBar   *Mtools;

	QFont appFont;

	void selectStyleMenu( int );
};

#endif // OF_H
