/****************************************************************
 *
 * Implementation dialog class
 *
 * Copyright (C) 1999 MPI of Cognitive Neuroscience, Leipzig
 *
 * Author Heiko Mentzel, 1999, <mentzel@cns.mpg.de>
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
****************************************************************/


#include <qmessagebox.h>
#include "dialog.h"
//#include "history.h"
#include "xpm/vlview.xpm"

#define DEBUGING 0

extern VImage *src, *fnc;
QString origconfpath="", idsprog="", refpath="";

MyDialog::MyDialog( QWidget *parent, const char *name, prefs *pr_, const char *program_id )
        : QWidget( parent, name ), pr(pr_), program_id_m(program_id) {
  pr->picture=(int *) VMalloc(sizeof(int)*4);
  pr->picture[0]=1;
  pr->picture[1]=1;
  pr->picture[2]=1;
  pr->picture[3]=0;
  pr->hgfarbe=(int *) VMalloc(sizeof(int)*3);
  pr->hgfarbe[0]=0;
  pr->hgfarbe[1]=1;
}

void MyDialog::about() {
  char program_name_temp[20];
  strncpy(program_name_temp,program_id_m,20);
  program_name = strtok(program_name_temp, ":"); 

  QString license="This program is free software; you can redistribute it and/or\n"
    "modify it under the terms of the GNU General Public License\n"
    "as published by the Free Software Foundation; either version 2\n"
    "of the License, or (at your option) any later version.\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n"
    "\n"
    "You should have received a copy of the GNU General Public License\n"
    "along with this program; if not, write to the Free Software\n"
    "Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.\n";

  QMessageBox about( tr("%1").arg(program_name), tr("%1\n\n%2 is a program for vizualisation of evaluated fMRI data.\nIf you find bugs please contact lipsia@cns.mpg.de\n\nCopyright (C) 1999-2002 MPI of Cognitive Neuroscience, Leipzig\n%3\n").arg(program_id_m).arg(program_name).arg(license), QMessageBox::NoIcon,0,0,0,this, "about",TRUE,0);
  about.setIconPixmap ( QPixmap(vlview_xpm) );
  about.exec();
}

void MyDialog::open() {
  FILE *in_file;
  VAttrList in_list;
  VAttrListPosn posn;
  VImage tmpimg=NULL;

    QString f = QFileDialog::getOpenFileName( QString::null, "*.v", this );
    const char* filename = f.data();

    // the user selected a valid existing file
    if ( !f.isEmpty() ) {        
      in_file = VOpenInputFile (filename, TRUE);

      /* Read the input file */
      in_list = VReadFile (in_file, NULL);
      if (!in_list) exit(1);   
      
      /* For each object in vista file */
      for (VFirstAttr (in_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
	if (VGetAttrRepn (& posn) != VImageRepn) continue;
	VGetAttrValue (& posn, NULL, VImageRepn, & tmpimg);
	if (VPixelRepn(tmpimg)==VUByteRepn) {
	  if (VImageNFrames(tmpimg)==VImageNFrames(src[0]) && VImageNRows(tmpimg)==VImageNRows(src[0]) && VImageNColumns(tmpimg)==VImageNColumns(src[0]))
	    src[0]=tmpimg;
	}
      }
      emit neuGeladen();
    } else {
      // the user cancelled the dialog
      return;
    }
}

void MyDialog::save()
{
    QString f = QFileDialog::getSaveFileName( QString::null, "*.v", this );
    //    fd->setFilter( "All vista files (*.v)" );
    //    fd->setFilter( "*.v" );

    if ( !f.isEmpty() ) {        // the user selected a valid existing file

      //      save( s ); // save() being your function to write the file
    } else {
        // the user cancelled the dialog
      return;
    }
  //emit explain( "File/Save selected" );
}

void MyDialog::saveOptions(int rw)
{
  char program_name_temp[20];
  strncpy(program_name_temp,program_id_m,20);
  program_name = strtok(program_name_temp, ":"); 
  char *filename = strcat(program_name,".cfg");
  char *token=NULL;
  double vers=0;
  QDir d = QDir::home();  // now points to home directory
  if (rw==0) {
    if ( !d.cd(".lipsia") ) {   // now points to ".lipsia" under home directory if OK
      QFileInfo fi( d, ".lipsia" );
      if ( fi.exists() ) {
	if ( fi.isDir() )
	  qWarning( "Cannot cd into \"%s\".", (const char*)d.absFilePath(".lipsia") );
	else
	  qWarning( "Cannot create directory \"%s\"\n"
		    "A file named \".lipsia\" already exists in \"%s\"",
		    (const char *)d.absFilePath(".lipsia"),
		    (const char *)d.path() );
	return;
      } else {
	qWarning( "Creating directory \"%s\"",
		  (const char *) d.absFilePath(".lipsia") );
	if ( !d.mkdir( ".lipsia" ) ) {
	  qWarning("Could not create directory \"%s\"",
		   (const char *)d.absFilePath(".lipsia") );
	  return;
	} else {
	  d.cd(".lipsia");
	}
      }
    }
  } else {
    d.cd(".lipsia");
  }
  QFile f;
  f.setName( d.filePath(filename /*"lipsia.cfg"*/) );
  if ( f.exists() ) {    // file opened successfully
    if ( f.open(IO_ReadOnly) ) {    // file opened successfully
      QTextStream t( &f );        // use a text stream
      QString s;;
      while ( !t.eof() ) {        // until end of file...
	s = t.readLine();       // line of text excluding '\n'
	if ( s.contains ( "origconfpath ", TRUE ) == 1) {
	  s = s.right( s.length()-13 );
	  char *ons = strdup(s);
	  token = strtok(ons," ");
	  origconfpath=(QString)token;
	}
	if ( s.contains ( "style ", TRUE ) == 1) {
	  s = s.right( s.length()-6 );	    
	  char *ons = strdup(s);
	  token = strtok(ons," ");
	  pr->lipstyle=atoi(token);
	}
	if ( s.contains ( "idsprog ", TRUE ) == 1) {
	  s = s.right( s.length()-8 );	    
	  char *ons = strdup(s);
	  token = strtok(ons," ");
	  idsprog=(QString)token;
	}
	if ( s.contains ( "refpath ", TRUE ) == 1) {
	  s = s.right( s.length()-8 );	    
	  char *ons = strdup(s);
	  token = strtok(ons," ");
	  refpath=(QString)token;
	}
      }
      f.close();
    }    
  } else {
    //QMessageBox::warning( this, "lipsia",
    //"No Lipsia config file found\nUsing standard configuration" );
    //qFatal("\nSORRY, NO LIPSIA STD CONFIG FOUND!!!\nplease run vlsetup\n");
  } 
  f.setName( d.filePath(filename /*"vlview.cfg"*/) );
  if (rw==1 || rw==0) {
    if ( f.exists() ) {              // file exists
      if ( f.open(IO_ReadOnly) ) {   // file opened successfully
        QTextStream t( &f );         // use a text stream
        QString s;
        while ( !t.eof() ) {         // until end of file...
	  s = t.readLine();          // line of text excluding '\n'
	  if ( s.contains ( "version", TRUE ) == 1) {
	    s = s.right( s.length()-8 );
	    char *ons = (char*)strdup(s);
	    token = strtok(ons," ");
	    if (token != NULL)
	      vers=(double)atof(token);
	    else {
	      QString errorDetails= "error while reading configuration file (version)";
	      QMessageBox::critical( 0, "Application name here",
				     QString("An internal error occured. Please contact ") +
				     "technical support.\n" +
				     "lipsia@cns.mpg.de and report these\n"+
				     "numbers:\n\n" + errorDetails +
				     "\n\nThe Program will now exit." );      
	    }
	  }
	  pr->talairach=lesePref( pr->talairach, s, "talairach" );
	  pr->pixelco=lesePref( pr->pixelco, s, "pixelcoordinates" );
	  pr->baco=lesePref( pr->baco, s, "baselinecorrection" );
	  pr->persi=lesePref( pr->persi, s, "percentsignalchange" );
	  pr->fitevobool=lesePref( pr->fitevobool, s, "fitted/evoked" );
	  pr->stdera=lesePref( pr->stdera, s, "standarderror" );
	  pr->bolres=0;
	  pr->triallength=lesePref( pr->triallength, s, "triallength" );
	  pr->trialresolution=lesePref( pr->trialresolution, s, "trialresolution" );	  
	  pr->showcross=lesePref( pr->showcross, s, "showcross" );
	  pr->radius=lesePref( pr->radius, s, "searchradius" );
	  pr->showradius=lesePref( pr->showradius, s, "showradius" );
	  pr->interpol=lesePref( pr->interpol, s, "interpolatezmap" );
	  pr->interpoltype=lesePref( pr->interpoltype, s, "interpoltype" );
	  pr->transzmap=lesePref( pr->transzmap, s, "transparentzmap" );
	  pr->lockz=lesePref( pr->lockz, s, "lockmaxzmap" );
	  pr->oglzmapdense=lesePref( pr->oglzmapdense, s, "oglzmapdense" );
	  pr->digits=lesePref( pr->digits, s, "digits" );
	  pr->hideitems=lesePref( pr->hideitems, s, "hideitems" );
	  pr->tc_minlength=lesePref( pr->tc_minlength, s, "tc_minlength" );
	  // added by A. Hagert for vgview
	  pr->gcoltype=lesePref( pr->gcoltype, s, "colortable graph" );
	  pr->crossize=lesePref( pr->crossize, s, "size of the cross" );
	  pr->glassbrain=lesePref( pr->glassbrain, s, "glassbrain" );
	  pr->fog=lesePref( pr->fog, s, "fog" );
	  pr->openvis=lesePref( pr->openvis, s, "openvis" );
	  pr->exact=lesePref( pr->exact, s, "exact" );
	  pr->spheresize=lesePref( pr->spheresize, s, "spheresize" );
	  //pr->synchronize=lesePref( pr->synchronize, s, "synchronization (IPC)" );
	  pr->equidistantColorTable=(VBoolean)(lesePref( pr->equidistantColorTable, s, "equidistantColorTable" ));

	  if ( s.contains ( "maxzmapwert", TRUE ) == 1 && pr->lockz==1) {
	    s = s.right( s.length()-12 );	    
	    char *ons = (char*)strdup(s);
	    // int i = 0;
	    token = strtok(ons," ");
	    if (token != NULL) {
	      pr->pmax=atof(token);
	      if ((token = strtok(NULL, " ")) != NULL) { 
		pr->nmax=atof(token);
	      }
	      if ((token = strtok(NULL, " ")) != NULL) { 
		pr->zeropoint=atof(token);
	      }	      
	    }
	  }

	  if ( s.contains ( "showpicture", TRUE ) == 1) {
	    s = s.right( s.length()-12 );	    
	    char *ons = (char*)strdup(s);

	    int i=0;
	    token = strtok(ons, " ");
	    if (token != NULL) {
	      pr->picture[i++] = atoi(token);

	      while ((token = strtok(NULL, " ")) != NULL) { 
		pr->picture[i++]=atoi(token);
	      }
	    }
	  }
	  if ( s.contains ( "bgcolor", TRUE ) == 1) {
	    s = s.right( s.length()-8 );	    
	    char *ons = (char*)strdup(s);
	    int i = 0;
	    token = strtok(ons," ");
	    if (token != NULL) {
	      pr->hgfarbe[i]=(int)atoi(token);
	      while ((token = strtok(NULL, " ")) != NULL) { 
		pr->hgfarbe[i+1]=atoi(token);
	      }
	    }

	    //	    hgfarbe[0]=0;
	    //hgfarbe[1]=1;
	  }
	  pr->coltype=lesePref( pr->coltype, s, "colortype" );
	  if ( s.contains ( "linecolors", TRUE ) == 1) {
	    s = s.right( s.length()-11 );	    
	    char *ons = (char*)strdup(s);
	    int i = 0;
	    token = strtok(ons," ");
	    if (token != NULL) {
	      pr->crosscolor.setRgb(atoi(token));
	      while ((token = strtok(NULL, " ")) != NULL) { 
		pr->radiuscolor.setRgb(atoi(token));
		i++;
	      }
	    }
	  }
        }
        f.close();
      }    
      if (DEBUGING) qWarning("config loaded" );
    } else {
      /* create file for next try, if segmentation fault */
      f.open( IO_WriteOnly );
      f.writeBlock( tr("%1\n").arg(program_name), strlen(tr("%1\n").arg(program_name)) );       // write to stderr
      f.writeBlock( tr("version %1\n").arg(program_id_m), strlen(tr("version %1\n").arg(program_id_m)) );       // write to stderr
      f.close();
      if (DEBUGING) qWarning("config created" );
    }
  }
  if (rw==2 || rw==0) {
    f.open( IO_WriteOnly );
    f.writeBlock( tr("%1\n").arg(program_name), strlen(tr("%1\n").arg(program_name)) );       // write to stderr
    f.writeBlock( tr("version %1\n").arg(program_id_m), strlen(tr("version %1\n").arg(program_id_m)) );       // write to stderr
    f.writeBlock( tr("pixelcoordinates %1\n").arg(pr->pixelco), strlen(tr("pixelcoordinates %1\n").arg(pr->pixelco)) );       // write to stderr
    f.writeBlock( tr("talairach %1\n").arg(pr->talairach), strlen(tr("talairach %1\n").arg(pr->talairach)) );       // write to stderr
    f.writeBlock( tr("baselinecorrection %1\n").arg(pr->baco), strlen(tr("baselinecorrection %1\n").arg(pr->baco)) );       // write to stderr
    f.writeBlock( tr("percentsignalchange %1\n").arg(pr->persi), strlen(tr("percentsignalchange %1\n").arg(pr->persi)) );       // write to stderr
    f.writeBlock( tr("fitted/evoked %1\n").arg(pr->fitevobool), strlen(tr("fitted/evoked %1\n").arg(pr->fitevobool)) );       // write to stderr
    f.writeBlock( tr("standarderror %1\n").arg(pr->stdera), strlen(tr("standarderror %1\n").arg(pr->stdera)) );       // write to stderr
    f.writeBlock( tr("trialaverage %1\n").arg(pr->bolres), strlen(tr("trialaverage %1\n").arg(pr->bolres)) );       // write to stderr
    f.writeBlock( tr("triallength %1\n").arg(pr->triallength), strlen(tr("triallength %1\n").arg(pr->triallength)) );       // write to stderr
    f.writeBlock( tr("trialresolution %1\n").arg(pr->trialresolution), strlen(tr("trialresolution %1\n").arg(pr->trialresolution)) );       // write to stderr
    f.writeBlock( tr("showcross %1\n").arg(pr->showcross), strlen(tr("showcross %1\n").arg(pr->showcross)) );       // write to stderr
    f.writeBlock( tr("searchradius %1\n").arg(pr->radius), strlen(tr("searchradius %1\n").arg(pr->radius)) );
    f.writeBlock( tr("showradius %1\n").arg(pr->showradius), strlen(tr("showradius %1\n").arg(pr->showradius)) );
    f.writeBlock( tr("interpolatezmap %1\n").arg(pr->interpol), strlen(tr("interpolatezmap %1\n").arg(pr->interpol)) );
    f.writeBlock( tr("interpoltype %1\n").arg(pr->interpoltype), strlen(tr("interpoltype %1\n").arg(pr->interpoltype)) );
    f.writeBlock( tr("transparentzmap %1\n").arg(pr->transzmap), strlen(tr("transparentzmap %1\n").arg(pr->transzmap)) );
    f.writeBlock( tr("lockmaxzmap %1\n").arg(pr->lockz), strlen(tr("lockmaxzmap %1\n").arg(pr->lockz)) );
    f.writeBlock( tr("oglzmapdense %1\n").arg(pr->oglzmapdense), strlen(tr("oglzmapdense %1\n").arg(pr->oglzmapdense)) );
    f.writeBlock( tr("digits %1\n").arg(pr->digits), strlen(tr("digits %1\n").arg(pr->digits)) );
    f.writeBlock( tr("hideitems %1\n").arg(pr->hideitems), strlen(tr("hideitems %1\n").arg(pr->hideitems)) );
    f.writeBlock( tr("tc_minlength %1\n").arg(pr->tc_minlength), strlen(tr("tc_minlength %1\n").arg(pr->tc_minlength)) );
    f.writeBlock( tr("maxzmapwert %1 %2 %3\n").arg(pr->pmax).arg(pr->nmax).arg(pr->zeropoint), strlen(tr("maxzmapwert %1 %2 %3\n").arg(pr->pmax).arg(pr->nmax).arg(pr->zeropoint) ) );
    f.writeBlock( tr("showpicture %1 %2 %3 %4\n").arg(pr->picture[0]).arg(pr->picture[1]).arg(pr->picture[2]).arg(pr->picture[3]), strlen(tr("showpicture %1 %2 %3 %4\n").arg(pr->picture[0]).arg(pr->picture[1]).arg(pr->picture[2]).arg(pr->picture[3]) ) );
    f.writeBlock( tr("bgcolor %1 %2\n").arg(pr->hgfarbe[0]).arg(pr->hgfarbe[1]), strlen( tr("bgcolor %1 %2\n").arg(pr->hgfarbe[0]).arg(pr->hgfarbe[1]) ) );
    f.writeBlock( tr("colortype %1\n").arg(pr->coltype), strlen( tr("colortype %1\n").arg(pr->coltype) ) );
    f.writeBlock( tr("equidistantColorTable %1\n").arg((int)(pr->equidistantColorTable)), strlen( tr("equidistantColorTable %1\n").arg((int)(pr->equidistantColorTable)) ) );
    f.writeBlock( tr("linecolors %1 %2\n").arg(pr->crosscolor.rgb()).arg(pr->radiuscolor.rgb()), strlen( tr("linecolors %1 %2\n").arg(pr->crosscolor.rgb()).arg(pr->radiuscolor.rgb()) ) );
    // added by A. Hagert for vgview
    f.writeBlock( tr("colortable graph %1\n").arg(pr->gcoltype), strlen(tr("colortable graph %1\n").arg(pr->gcoltype) ) );
    f.writeBlock( tr("size of the cross %1\n").arg(pr->crossize), strlen(tr("size of the cross %1\n").arg(pr->crossize) ) );
    f.writeBlock( tr("glassbrain %1\n").arg(pr->glassbrain), strlen(tr("glassbrain %1\n").arg(pr->glassbrain) ) );
    f.writeBlock( tr("fog %1\n").arg(pr->fog), strlen(tr("fog %1\n").arg(pr->fog) ) ); 
    f.writeBlock( tr("openvis %1\n").arg(pr->openvis), strlen(tr("openvis %1\n").arg(pr->openvis) ) ); 
    f.writeBlock( tr("exact %1\n").arg(pr->exact), strlen(tr("exact %1\n").arg(pr->exact) ) );
    f.writeBlock( tr("spheresize %1\n").arg(pr->spheresize), strlen(tr("spheresize %1\n").arg(pr->spheresize) ) );
    //f.writeBlock( tr("synchronization (IPC) %1\n").arg(pr->synchronize), strlen(tr("synchronization (IPC) %1\n").arg(pr->synchronize) ) );
    f.close();
    if (DEBUGING) qWarning("config saved" );
  }
}

int MyDialog::lesePref( int var, const char *s, const char *name ) {
  char *token=NULL;
  QString qs = s;
  QString na = name;
  if ( qs.contains ( na, TRUE ) == 1) {
    qs = qs.right( qs.length()-na.length()-1 );	    
    char *ons = (char*)strdup(qs);
    token = strtok(ons," ");
    if (token != NULL)
      return (int)atoi(token);
    else 
      return var;
  }
  else return var;
}
