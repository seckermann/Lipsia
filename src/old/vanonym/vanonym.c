/****************************************************************
 *
 * vanonym:
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2001, <lipsia@cbs.mpg.de>
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
 * $Id: vanonym.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

/* From the Vista library: */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>

/* For portability: */
#include <X11/Xos.h>

extern char *getLipsiaVersion();

int main ( int argc, char **argv )
{
	static VString id = "xxx";
	static VOptionDescRec options[] = {
		{
			"id", VStringRepn, 1, ( VPointer ) &id,
			VOptionalOpt, NULL, "Patient id code"
		}
	};
	FILE *in_file, *out_file;
	VAttrList list;
	VAttrListPosn posn;
	VImage src = NULL;
	VString str = NULL;
	int i;
	// int day=0,month=0,year=0;
	char prg_name[50];
	sprintf( prg_name, "vanonym V%s", getLipsiaVersion() );

	fprintf ( stderr, "%s\n", prg_name );

	/* Parse command line arguments: */
	VParseFilterCmd ( VNumber ( options ), options, argc, argv, &in_file, &out_file );

	/* Read the input file: */
	list = VReadFile ( in_file, NULL );

	if ( ! list ) exit ( 1 );

	for ( VFirstAttr ( list, & posn ); VAttrExists ( & posn ); VNextAttr ( & posn ) ) {
		if ( VGetAttrRepn ( & posn ) != VImageRepn ) continue;

		VGetAttrValue ( & posn, NULL, VImageRepn, & src );

		VExtractAttr ( VImageAttrList( src ), "condition", NULL, VStringRepn, &str, FALSE );
		VExtractAttr ( VImageAttrList( src ), "date", NULL, VStringRepn, &str, FALSE );
		VExtractAttr ( VImageAttrList( src ), "origin", NULL, VStringRepn, &str, FALSE );
		VExtractAttr ( VImageAttrList( src ), "location", NULL, VStringRepn, &str, FALSE );
		VExtractAttr ( VImageAttrList( src ), "birth", NULL, VStringRepn, &str, FALSE );

		VSetAttr( VImageAttrList( src ), "patient", NULL, VStringRepn, id );

		/*
		if (str1==NULL) {
		  if (VGetAttr (VImageAttrList (src), "birth", NULL,
		        VStringRepn, (VPointer) & str1) == VAttrFound) {
		sscanf(str1,"%d.%d.%d",&day,&month,&year);
		if (year < 1800 && year > 15) year += 1900;
		VSetAttr(VImageAttrList(src),"birth",NULL,VLongRepn,(VLong)year);
		fprintf(stderr," %s:  year of birth= %d\n",id,year);
		  }
		}
		else
		  VSetAttr(VImageAttrList(src),"birth",NULL,VLongRepn,(VLong)year);
		*/

		VSetAttrValue ( & posn, NULL, VImageRepn, src );
	}

	i = 0;

	while ( VExtractAttr ( list, "history", NULL, VStringRepn, & str, FALSE ) && i < 25 ) {
		i++;
	}

	/* Write the results to the output file: */
	if ( ! VWriteFile ( out_file, list ) ) exit( 1 );

	fprintf ( stderr, "%s: done.\n", argv[0] );
	return 0;
}
