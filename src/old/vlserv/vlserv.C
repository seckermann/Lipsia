/****************************************************************
 *
 * lserv:
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Karsten Mueller, 2004, <lipsia@cbs.mpg.de>
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
 * $Id: vlserv.C 2999 2007-11-30 10:55:56Z karstenm $
 *
 *****************************************************************/


/* From the std. library: */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <qapplication.h>
#include "dock.h"

extern "C" {
  extern char * getLipsiaVersion();
}

void signal_handler(int sig)
{
    qApp->quit();
}


/* the main routine */
int main (int argc,char *argv[]) {
  QApplication::setColorSpec( QApplication::ManyColor );

  /* write out the revision string */
  char prg_name[50];	
  sprintf(prg_name,"vlserv V%s", getLipsiaVersion());
  
  fprintf (stderr, "%s\n", prg_name);
  
  /* initialize open the main window */
  QApplication a( argc, argv );
  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);

  lDock ww;
  
  a.setMainWidget( &ww );
  ww.show();
  
  // do further cleanups here
  return a.exec();
}

