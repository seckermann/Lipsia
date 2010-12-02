/****************************************************************
 *
 * Program: vqview
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Hannes Niederhausen, 2007, <lipsia@cbs.mpg.de>
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
 * $Id: main.cpp 2999 2007-11-30 10:55:56Z karstenm $
 *
 *****************************************************************/

#include <iostream>

#include <qapplication.h>

#include "MainWindow.h"

extern "C" {
	extern char *getLipsiaVersion();
}

int main( int argc, char **argv )
{

	char prg_name[50];
	sprintf( prg_name, "vqview V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg_name );


	/* opens the window */
	QApplication app( argc, argv );

	MainWindow *wnd = new MainWindow( new Configurator( &argc, argv ),
									  NULL, "vtview mainwindow", &app );
	wnd->resize( 600, 400 );
	app.setMainWidget( wnd );
	wnd->show();

	return app.exec();
}
