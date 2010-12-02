/****************************************************************************
** $Id: CW.h 3181 2008-04-01 15:19:44Z karstenm $
**
*****************************************************************************/

#ifndef SETUPCW_H
#define SETUPCW_H

#include <viaio/Vlib.h>

/* Qt - Libraries */
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qwidget.h>
#include <qevent.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qstringlist.h>
#include <qlistbox.h>
#include <qwidget.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <qscrollbar.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qvaluelist.h>

#include "bild1.h"
#include "preferences.h"

class CW : public QWidget
{
	Q_OBJECT

public:
	CW( QWidget *parent, const char *name = 0, QValueList<ImageData *> data = QValueList<ImageData *>(), QStringList namelist = 0, QString cp = 0 );
	~CW();
	QLabel *label21, *label22, *label23, *label24, *label25, *label31, *label32, *label400, *label401, *label41, *label42, *label43, *label44;
	int  lesePref( int var = 0, const char *s = 0, const char *name = 0 );


signals:
	void sliderwechsel();


public slots:
	void ZZchanged( int );
	void ImageSelect();
	void Wert( int, int, double );
	void showAttributes();
	void showPrefs();

protected:

private:
	QLineEdit *d0, *d1, *d01;
	QCheckBox  *useDB;
	Bild *bild;
	//QScrollBar *slider;
	QSlider *slider;
	QListBox *listbox;
	QString cp_m;
	TabDialog *prefs;
	QValueList<ImageData *> data_m;
};

#endif
