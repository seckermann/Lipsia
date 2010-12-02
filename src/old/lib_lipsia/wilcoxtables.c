#include <viaio/Vlib.h>
#include <viaio/mu.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "wilcoxtab18.c"
#include "wilcoxtab19.c"
#include "wilcoxtab20.c"
#include "wilcoxtab21.c"
#include "wilcoxtab22.c"
#include "wilcoxtab23.c"
#include "wilcoxtab24.c"
#include "wilcoxtab25.c"
#include "wilcoxtab26.c"
#include "wilcoxtab27.c"
#include "wilcoxtab28.c"
#include "wilcoxtab29.c"
#include "wilcoxtab30.c"


float *
getTable( int id )
{
	float *table = NULL;

	switch ( id ) {
	case 18:
		table = table18();
		break;

	case 19:
		table = table19();
		break;

	case 20:
		table = table20();
		break;

	case 21:
		table = table21();
		break;

	case 22:
		table = table22();
		break;

	case 23:
		table = table23();
		break;

	case 24:
		table = table24();
		break;

	case 25:
		table = table25();
		break;

	case 26:
		table = table26();
		break;

	case 27:
		table = table27();
		break;

	case 28:
		table = table28();
		break;

	case 29:
		table = table29();
		break;

	case 30:
		table = table30();
		break;

	default:
		VError( " not yet implemented for n >= %d images", id );
	}

	return table;
}
