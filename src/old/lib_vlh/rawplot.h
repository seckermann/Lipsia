/****************************************************************
 *
 * rawplot.h
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Heiko Mentzel, 1999, <lipsia@cbs.mpg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * $Id: rawplot.h 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#ifndef RAWPLOT_H
#define RAWPLOT_H

/* Vista - Libraries */
#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/VImage.h>
#include <viaio/Vlib.h>

/* Qt - Libraries */
#include <qwidget.h>
#include <qstringlist.h> 
#include <qmessagebox.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qdir.h>

/* Std. - Libraries */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "prefs.h"

class RawPlot : public QWidget
{
    Q_OBJECT
public:
    RawPlot( QWidget *parent=0, const char *name=0, prefs *pr_=0, const char *version=0, int files=0, QString vlviewname=0 );

public slots:
    void ausgabe();

private: 
    float *model1, *model2, *model3, *model4, *model5, *model6, *model7, *model8;
    FILE *beta_file;
    VAttrList beta_list;
    VImage beta;
    prefs *pr;
    const char *version_m;
    int files_m;
    QString vlviewname_m;
    double trpre;
    int bline, maxfarben;
    float *model, *conditions, *Sess;
};

#endif // RAWPLOT_H
