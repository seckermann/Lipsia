/****************************************************************
 *
 * VLCorr.h
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
 * $Id: VLCorr.h 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#ifndef VLCORR_H
#define VLCORR_H

/* Std. Include Files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Vista Include Files */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* Qt Include Files */
#include <qwidget.h>
#include "prefs.h"

class VLCorr : public QWidget
{
	Q_OBJECT

public:
	VLCorr();

signals:
	void viewChanged( );

public slots:
	void correlation();
	void CreateMap( int, int, int );


};

#endif // VLCORR_H
