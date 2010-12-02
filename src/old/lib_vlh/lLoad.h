/****************************************************************
 *
 * lLoad.h
 *
 * Copyright (C) 2002 MPI of Cognitive Neuroscience, Leipzig
 *
 * Author Heiko Mentzel, Jan. 2002, <mentzel@cns.mpg.de>
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
 *****************************************************************/

#ifndef LLOAD_H
#define LLOAD_H

/* From the std. library: */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>   /* Interprozesskommunikation */
#include <signal.h>

/* Vista Include Files */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/VGraph.h>
#include <viaio/option.h>
#include "prefs.h"

class lLoad
{
	//    Q_OBJECT
public:
	//public slots:
	void ScanGraph( int *, int *, int *, int *, int *, int * );
	void testFiles();
	void loadFiles();
	void loadFilesForCorr();

};

#endif // LLOAD_H
