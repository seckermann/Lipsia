/***************************************************************************
 *   Copyright (C) 2005 by Hannes Niederhausen                             *
 *   niederhausen@cbs.mpg.de                                               *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "vlmainwindow.h"
#include "datamanager.h"
#include "uiconfig.h"

#include <qapplication.h>

#include <viaio/mu.h>
#include <viaio/option.h>
#include <viaio/Vlib.h>

extern "C" {
	extern void getLipsiaVersion(char*,size_t);
}

int main( int argc, char **argv )
{
	QApplication a( argc, argv );

	//<vista-zeuch>
	FILE *in_file = NULL;

	/*
	 *  vledit can be run in two modes
	 *  "ana" anatomie (editing) mode
	 *  "seg" segment (editing) mode (default)
	 */
	VDictEntry mode_dict[] = {
		{"ana", 0},
		{"seg", 1},
		{NULL}
	};
	/* the segments can be saved in two data formats: ubyte and bit */
	VDictEntry type_dict[] = {
		{ "ubyte" , 0 },
		{ "bit" , 1 },
		{ NULL }
	};

	VLong mode = 1;
	VLong type = 0;
	VFloat resolution = 0.0;
	static VOptionDescRec  options[] = {
		{
			"mode", VLongRepn, 1, &mode, VOptionalOpt, mode_dict,
			"Selects mode, accepted values are \"ana\" and \"seg\""
		},
		{
			"type", VLongRepn, 1, &type, VOptionalOpt, type_dict,
			"Selects segment's output type, can be \"ubyte\" or \"bit\""
		},
		{
			"resolution", VFloatRepn, 1, ( VPointer ) &resolution, VOptionalOpt,
			NULL, "Selects segment resolution"
		}
	};

	char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vledit V%s", ver);

	fprintf ( stderr, "%s\n", prg_name );

	VParseFilterCmd ( VNumber ( options ), options, argc, argv, &in_file, NULL );

	//no check needed VParseFilterCmd stops program if no file is given
	DATAMANAGER->loadVistaImage( in_file );
	// set the segment resolution
	DATAMANAGER->setSegResolution( resolution );
	// set the editing mode
	UICONFIG->setMode( mode );
	// set the output data format
	DATAMANAGER->setOutputType( type );

	//</vista-zeuch>

	vlMainWindow *window = new vlMainWindow();
	window->show();

	//quit application if no window is open
	a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
	return a.exec();
}

