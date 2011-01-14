/****************************************************************
 *
 * vsview:
 *  Program to display vista VImage files
 *
 * MPI of Cognitive Neuroscience, Leipzig
 *
 * Authors Heiko Mentzel, Karsten Mueller, Aug. 2001, <mentzel@cns.mpg.de>
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
 *****************************************************************
 *
 * History:
 * ========
 *  Heiko Mentzel -- 08/09/2001 -- first qt vxview version finished
 *  Karsten Mueller -- 08/30/2007 -- further developments
 *  Hannes Niederhausen -- ....
 *  Karsten Müller -- 10/2007 -- further contrast enhancement
 *
 *****************************************************************/

/* From the std. library: */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* QT - Librarys */
#include <qiconset.h> 
#include <qframe.h>
#include <qimage.h> 
#include <qkeycode.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlayout.h>
#include <qtoolbar.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qstatusbar.h> 
#include <qpicture.h> 
#include <qfont.h> 
#include <qstring.h>
#include <qvaluelist.h>

/* my own inludes */
#include "xv.h"
#include "imagedata.h"

#define ABS(x) ((x) > 0 ? (x) : -(x))

extern "C" {
  extern void getLipsiaVersion(char*,size_t);
}

QValueList<ImageData*> imageDataList;

QStringList namelist;

QString attributeString;

int globalmin=32766, globalmax=-32766; 
static VDouble black=0.01;
static VDouble white=0.01;
static VShort background[2]={0,0};

/* Constructs a Widget. */
XView::XView(QWidget *parent, const char *name) : QMainWindow(parent, name){
	appFont = QApplication::font();

	center = new CW( this, "center", imageDataList, namelist, attributeString );
	setCentralWidget(center );

	QPopupMenu* popup = new QPopupMenu( this );
	CHECK_PTR(popup );
	popup->insertItem("&Quit", qApp, SLOT(quit()), CTRL+Key_Q );

	QPopupMenu* viewmenu = new QPopupMenu( this );
	CHECK_PTR(viewmenu );
	viewmenu->insertItem("&Attributes", center, SLOT(showAttributes()), CTRL
			+Key_A );
	viewmenu->insertItem("Display &Prefs", center, SLOT(showPrefs()), CTRL
			+Key_P );

	QMenuBar *menubar = new QMenuBar( this, "menubar" );
	menubar->insertItem("&File", popup );
	menubar->insertItem("&View", viewmenu );

}

/* close all */
XView::~XView() {
}

void getini(ImageData *data) {

  // VUByte
  if (data->pixel_repn == VUByteRepn) {
    data->minwert  = (float)0.0;
    data->maxwert  = (float)255.0;
    data->anaalpha = (float)1.0;
    data->anamean  = (float)0.0;
  }

  // VBit
  if (data->pixel_repn == VBitRepn) {
    data->minwert  = (float)0.0;
    data->maxwert  = (float)1.0;
    data->anaalpha = (float)1.0;
    data->anamean  = (float)0.0;
  }

  // VShort
  if (data->pixel_repn == VShortRepn) {
    int nbands = VImageNBands(data->image);
    int nrows  = VImageNRows(data->image);
    int ncols  = VImageNColumns(data->image);
    int npixels = nbands * nrows * ncols;
    int smin = (int)VRepnMinValue(VShortRepn);
    int smax = (int)VRepnMaxValue(VShortRepn);    
    int i=0, j=0;
    int dim = 2*smax + 1;

    float *histo = (float *) VMalloc(sizeof(float)*dim);
    for (j=0; j<dim; j++) histo[j] = 0;
    
    VShort *src_pp = (VShort *) VImageData(data->image);
    for (i=0; i<(int)(npixels/4.0); i++) {
      j = *src_pp;
      src_pp += 4;
      if (j<background[0] || j>background[1]) {
	j -= smin;
	histo[j]++;
      }
    }
    
    float sum = 0;
    for (j=0; j<dim; j++) sum += histo[j];
    for (j=0; j<dim; j++) histo[j] /= sum;
    
    sum  = 0;
    for (j=0; j<dim; j++) {
      sum += histo[j];
      if (sum > black) break;
    }
    int xmin = j+smin;
    
    sum = 0;
    for (j=dim-1; j>=0; j--) {
      sum += histo[j];
      if (sum > white) break;
    }
    int xmax = j+smin;
    
    data->minwert  = (float)xmin;
    data->maxwert  = (float)xmax;
    data->anaalpha = (float)255.0 / (float)(xmax - xmin);
    data->anamean  = (float)xmin;
    //fprintf(stderr,"xmin: %f, xmax: %f, alpha: %f\n",data->minwert, data->maxwert,  data->anaalpha);

    if (!data->isanatomy) {
      if (xmin < globalmin) globalmin = xmin;
      if (xmax > globalmax) globalmax = xmax;
    }
  }
}

/**
 * All functional data should have the same min- and maxvalues which
 * have is the maximum/minimum values.
 **/
void calculateColors() {
	QValueList<ImageData*>::iterator it = imageDataList.begin();
 
	for (it = imageDataList.begin(); it != imageDataList.end(); it++) {
		ImageData* data = *it;
		getini(data);
	}
	
	for (it = imageDataList.begin(); it != imageDataList.end(); it++) {
		ImageData* data = *it;
		if (!data->isanatomy) {
		  data->minwert = (float)globalmin;
		  data->maxwert = (float)globalmax;
		  data->anaalpha = (float)(255.0 / (globalmax - globalmin));
		  data->anamean = (float)globalmin;
		}
	}
	
	
}

/* Main */
int main(int argc, char *argv[]) {
	VAttrList in_list;
	VString str, newstr;
	VAttrListPosn posn;
	static VBoolean in_found;
	VArgVector in_files;
	VStringConst in_filename=NULL;
	static VOptionDescRec options[] = { 
	  {"in", VStringRepn, 0, &in_files,&in_found, NULL, "Anatomical image or functional volumes" },
	  {"black",VDoubleRepn,1,(VPointer)&black,VOptionalOpt,NULL,"Lower histogram cut-off for contrast scaling in %"},
	  {"white",VDoubleRepn,1,(VPointer)&white,VOptionalOpt,NULL,"Upper histogram cut-off for contrast scaling in %"},
	  {"background", VShortRepn, 2, &background, VOptionalOpt, NULL, "Interval for background"},
	};
	FILE *in_file, *f;

        char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vsview V%s", ver);
  
        fprintf (stderr, "%s\n", prg_name);


	/* Parse command line arguments and identify files: */
	if (!VParseCommand (VNumber (options), options, & argc, argv) ||!VIdentifyFiles (VNumber (options), options, "in", & argc, argv, 0))
		goto Usage;
	if (argc > 1) {
		VReportBadArgs (argc, argv);
		Usage: VReportUsage (argv[0], VNumber (options), options, NULL);
		exit (EXIT_FAILURE);
	}

	in_filename = ((VStringConst *) in_files.vector)[0];

	/* Read its contents: */
	if (strcmp (in_filename, "-") == 0)
		in_file = stdin;
	else {
		in_file = fopen (in_filename, "r");
		if (!in_file)
			VError ("Failed to open input file %s", in_filename);
	}

	/* Read the input file: */
	in_list = VReadFile (in_file, NULL);
	if (!in_list)
		exit (EXIT_FAILURE);
	fclose(in_file);

	int imagenr = 0;
	ImageData *id = new ImageData;
	for (VFirstAttr (in_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {

		if (VGetAttrRepn (& posn) != VImageRepn)
			continue;
		
		if (id==NULL)
			id = new ImageData();
		
		VGetAttrValue (&posn, NULL, VImageRepn, &id->image);

		if ( ((VImageNColumns(id->image)==1)&&(VImageNRows(id->image)==1)) ||
				(VPixelRepn(id->image)==VFloatRepn) || (VPixelRepn(id->image)==VDoubleRepn)){
			continue;
		}
		
		id->pixel_repn = VPixelRepn(id->image);

		// checking if we have anatomy data of functional
		if (id->pixel_repn==VShortRepn && VGetAttr (VImageAttrList (id->image), 
			"MPIL_vista_0", NULL, VStringRepn, (VPointer) & str) == VAttrFound) {
		  id->isanatomy = false;
		} else if (id->pixel_repn==VShortRepn && VGetAttr (VImageAttrList (id->image), 
		        "repetition_time", NULL, VStringRepn, (VPointer) & str) == VAttrFound) {
		  id->isanatomy = false;
		} else {
		  id->isanatomy = true;
		}
		
		if (VGetAttr (VImageAttrList (id->image), "name", NULL,
		VStringRepn, (VPointer) & str) == VAttrFound) {
			newstr = VNewString(str);
			QString qnewstr = QString(newstr);
			if (qnewstr.length()<=0)
				qnewstr = "Image"; //tr("Image %1").arg(imagenr);
			namelist.append(qnewstr );
		} else {
			namelist.append(QObject::tr("Image %1").arg(imagenr ) );
		}

		id->rep = (char *)VPixelRepnName (id->image);
		id->brightness = 0;
		id->contrast = 0;

		imageDataList.append(id);
		id = NULL;
		imagenr++;
	}

	f = fopen (in_filename, "r");
	if (!f)
		VWarning ("Unable to open temporary file using tmpfile(3)");

	attributeString = QString("File: ") + in_filename + "\n";

	char c = fgetc(f);
	while (! (c == VFileDelimiter[0]&&(c=fgetc(f)) == VFileDelimiter[1])) {
		attributeString += c;
		c = fgetc(f);
	}

	fclose (f);

	if (imageDataList.size()==0) {
		VError("No renderable data found.");
		exit(0);
	}
	
	calculateColors();
	
	/* opens the window */
	QApplication app(argc, argv );
	XView fenster(0, "fenster");
	fenster.resize( 500, 300);
	app.setMainWidget( &fenster );
	fenster.show();

	return app.exec();
	/* end of the program */
}

