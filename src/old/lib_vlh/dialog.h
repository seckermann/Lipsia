/****************************************************************
**
** Definition of Bild1 class
**
****************************************************************/

#ifndef DIALOG_H
#define DIALOG_H

/* Vista - Libraries */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* Std. - Libraries */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Qt - Libraries */
#include <qpainter.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qdir.h>
#include <qtextstream.h>
#include <qwidget.h>

#include "prefs.h"

class MyDialog : public QWidget
{
	Q_OBJECT
public:
	MyDialog( QWidget *parent = 0, const char *name = 0, prefs *pr_ = 0, const char *program_id = 0 );
	int  lesePref( int var = 0, const char *s = 0, const char *name = 0 );

public slots:
	void open();
	void save();
	void about();
	void saveOptions( int rw = 0 );

signals:
	void neuGeladen();

private:
	prefs *pr;
	const char *program_id_m;
	char *program_name;
};

#endif // DIALOG_H

