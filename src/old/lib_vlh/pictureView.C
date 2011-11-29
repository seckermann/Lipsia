/****************************************************************
**
** Implementation picture class
** this class prints all views (sagittal, coronar, axial)
** (c) 1999-2002 by Heiko Mentzel (hemen@gmx.de)
**
** different types of interpolation added
** Gert Wollny <wollny@cns.mpg.de> 
****************************************************************/


#include <qfont.h> 
#include <qinputdialog.h>
#include <qpopupmenu.h> 
#include <iostream>
#include <memory>
#include <limits.h>

using namespace std; 

#include "pictureView.h"
#include "showVImage.h"
#include "VLTools.h"

#define NP -32768
#define MFT 65535

using namespace std; 

pictureView::pictureView( QWidget *parent, const char *name, 
			  prefs *pr_, int typ, int ifile, int files, 
			  double *ca, double *cp, double *extent, double *fixpoint, 
			  int crossoff, double ppp, double nnn, int *scalingfaktor )
        : QWidget( parent, name ), pr(pr_), 
	  type_m(typ), files_m(files), 
	  ca_m(ca), cp_m(ca), extent_m(extent), 
	  fixpoint_m(fixpoint), ifile_m(ifile), 
	  crossoff_m(crossoff), ppp_m(ppp), nnn_m(nnn), sf(scalingfaktor),
	  m_ipt(ip_nn),
	  //m_ipt(ip_bicub),
	  imagebackup(NULL),
	  image(NULL)
{


  /* if ( pr->interpoltype==0 ) m_ipt=ip_nn;
        if ( pr->interpoltype==1 ) m_ipt=ip_bilin;
        if ( pr->interpoltype==2 ) m_ipt=ip_bicub; */

	switch(pr->interpoltype){
			case 0:
				m_ipt=ip_nn;
				break;
			case 1:
				m_ipt=ip_bilin;
				break;
			case 2:
				m_ipt=ip_bicub;
				break;
			case 3:
				m_ipt=ip_bicub_6;
				break;
	                case 4:
				m_ipt=ip_bspline;
				break;	

			default:
				m_ipt=ip_nn;
				break;
		}
       
	pr->mousemove=0;
	pr->zmapview=1;
	recreate=1;
	letzte=0;
	Xplus=0; Yplus=0; Zplus=0;

	if (pr->verbose) qWarning( "initialize view %d",typ );

	setPalette( QPalette( QColor( 0,0,0) ) );
	QWidget::setMouseTracking ( TRUE );
  
	rows=VImageNRows(src[0]);
	columns=VImageNColumns(src[0]);
	bands=VImageNFrames(src[0]);
	
  
	if (fnc[0]) {
		fnc_rows=VImageNRows(fnc[ifile]);
		fnc_columns=VImageNColumns(fnc[ifile]);
		fnc_bands=VImageNFrames(fnc[ifile]);
	} else {
		fnc_rows=0;
		fnc_columns=0;
		fnc_bands=0;
	}

	ppmax=(double)ppp+pr->zeropoint;
	nnmax=(double)nnn+pr->zeropoint;

	if (pr->verbose) fprintf (stderr,"ppmax=%f nnmax=%f pmax=%f nmax=%f\n",ppmax,nnmax,pr->pmax,pr->nmax);

	rgbfarbe = (QColor *) malloc(sizeof(QColor) * 65536);
	rgbfarbeoverlay = (QColor *) malloc(sizeof(QColor) * 256);
  
	colorMap();
}

void pictureView::changeInterpol(interpol_type ipt)
{
	m_ipt = ipt; 
}

void pictureView::colorMap() {

	VLShow myshow;

	/* create colormaps for anatomie and zmap */
	myshow.vlhColorMap( rgbfarbe, rgbfarbeoverlay, pr->acoltype, pr->coltype, src[ifile_m], fnc, pr);
  

	/* create legend (the small color beams below the window) */
	if (type_m==1 && fnc[0]) {
		myshow.vlhCreateLegend( cpm, rgbfarbeoverlay, ppmax, pr->pmax, nnmax, pr->nmax, pr->equidistantColorTable );
	}

	/* repaint windows to see the changes */
	repaintf();
}

CMagBase *pictureView::get_interpolator(int s1, int s2)const 
{
	switch (m_ipt) {
	  /* case ip_nn: return new CNnMag(s1, s2);
	     case ip_bilin:return new CBilinMag(s1, s2);
	     default: return new CBicubSplineMag(s1, s2, -1); */
	case ip_nn: return new CNnMag(s1, s2);
	case ip_bilin:return new CBilinMag(s1, s2);
	case ip_bicub:return new CBicubSplineMag(s1, s2, -0.5);
	case ip_bicub_6:return new CBicub6Mag(s1, s2); /* dmoeller 02-22-06 */
	case ip_bspline:return new CBSplineMag(s1, s2); /* dmoeller 03-28-06 */
	default: return new CNnMag(s1, s2);
	}
}

void pictureView::paintEvent( QPaintEvent * )
{	
	QPainter* qPaint = new QPainter(this);
	paint(qPaint);
	delete qPaint;
}

void pictureView::paint(QPainter *paint)
{

        if (pr->mousemove==1) {
	  m_ipt=ip_nn;
	} else {
	  

	  /* if ( pr->interpoltype==0 ) m_ipt=ip_nn;
	     if ( pr->interpoltype==1 ) m_ipt=ip_bilin;
	     if ( pr->interpoltype==2 ) m_ipt=ip_bicub; */

	  	  switch(pr->interpoltype){
			case 0:
				m_ipt=ip_nn;
				break;
			case 1:
				m_ipt=ip_bilin;
				break;
			case 2:
				m_ipt=ip_bicub;
				break;
			case 3:
				m_ipt=ip_bicub_6;
				break;
			case 4:
				m_ipt=ip_bspline;
				break;

			default:
				m_ipt=ip_nn;
				break;
		}

	}


	double vgroesse=width(), hgroesse=height();

	if (pr->hgfarbe[0]==0) setPalette( QPalette( QColor( 0,0,0) ) );
	else setPalette( QPalette( QColor( 255,255,255) ) );
  
	
	int int1  = (int)rint(-fixpoint_m[1*files_m+0]+fixpoint_m[1*files_m+ifile_m]);
	int int2  = (int)rint(-fixpoint_m[2*files_m+0]+fixpoint_m[2*files_m+ifile_m]);
	int int6  = (int)rint(-fixpoint_m[0*files_m+0]+fixpoint_m[0*files_m+ifile_m]);
	int int7  = (int)rint(-fixpoint_m[0*files_m+0]+fixpoint_m[3*files_m+ifile_m]);
	int int12 = (int)rint(-fixpoint_m[1*files_m+0]+fixpoint_m[4*files_m+ifile_m]);
	int int13 = (int)rint(-fixpoint_m[2*files_m+0]+fixpoint_m[5*files_m+ifile_m]);


	int int03  = (int)rint(pr->cursorp[2])+int6;   // (int)rint() added by A. Hagert
	int int10  = (int)rint(pr->cursorp[2])+int7;   // (int)rint() added by A. Hagert
	int int100 = (int)rint(pr->cursorp[1])+int12;  // (int)rint() added by A. Hagert
	int int101 = (int)rint(pr->cursorp[1])+int1;   // (int)rint() added by A. Hagert
	int int013 = (int)rint(pr->cursorp[0])+int13;  // (int)rint() added by A. Hagert

	int int1000=0;
	int int1006=0;
	int int1007=0;
	int int1012=0;
	int int2012=0;
	int int2013=0;

	int  zoomn = 1; 
	
	double p128=127/(pr->pmax-ppmax);
	double n128=127/(pr->nmax-nnmax);


	// Bildzoom;
	prz=pr->bildzoom;
	

	// Zomm added by karstenm
	Xplus=0; newrows  = rows;
	Yplus=0; newcols  = columns;
	Zplus=0; newbands = bands;
	if (prz > 1) {
	  //newcols  = (int)rint(columns/prz +prz/16);
	  //newrows  = (int)rint(rows/prz    +prz/16);
	  //newbands = (int)rint(bands/prz   +prz/16);
	  newcols  = (int)ceil(columns/prz);
	  newrows  = (int)ceil(rows/prz);
	  newbands = (int)ceil(bands/prz);
		Xplus=(int)rint((double)pr->cursorp[0]/(double)prz*((double)prz-1.0));
		Yplus=(int)rint((double)pr->cursorp[1]/(double)prz*((double)prz-1.0));
		Zplus=(int)rint((double)pr->cursorp[2]/(double)prz*((double)prz-1.0));
	}
	Ymax=Yplus+newrows;
	Xmax=Xplus+newcols;
	Zmax=Zplus+newbands;

	framefaktor=(int)VImageNBands(src[ifile_m])/VImageNFrames(src[ifile_m]);
	if (framefaktor!=3) framefaktor=1;
	int *farbe;
	farbe = (int *) malloc(sizeof(int)*framefaktor);


	if (recreate) {
	  if (type_m==3) {
	    image = new QScaleImage(newcols, newrows);
	    
	    sc1=(double)vgroesse/(double)columns;
	    sc2=(double)hgroesse/(double)rows;
	    zoomn = columns;
	    
	    if (pr->cursorp[2]>=bands) 
	      pr->cursorp[2]=bands/2;
	    
	    if (pr->cursorp[2]<bands) {
	      for ( int i=Yplus; i<Ymax; i++ ) {
		int1000=i+int1;
		if (pr->sw2) 
		  int1012=i+int12;
		
		for ( int j=Xplus; j<Xmax; j++ ) {
		  for (int ff=0; ff<framefaktor; ff++) {
		    farbe[ff]=0; 
		    if (pr->infilenum>1) {
		      if (int1000>=0 && int1000<rows && j+int2>=0 && j+int2<columns && int03>=0 && int03<bands) 
			if (VPixelRepn(src[ifile_m])==VShortRepn)
			  farbe[ff]=(int)VPixel(src[ifile_m],framefaktor*(int)rint(pr->cursorp[2])+ff,i,j,VShort);
			else
			  farbe[ff]=(int)VPixel(src[ifile_m],framefaktor*(int)rint(pr->cursorp[2])+ff,i,j,VUByte); 
		    } else { 
		      if (i>=0 && i<rows && j>=0 && j<columns && pr->cursorp[2]>=0 && pr->cursorp[2]<bands) 
			if (VPixelRepn(src[0])==VShortRepn)
			  farbe[ff]=(int)VPixel(src[0],framefaktor*(int)rint(pr->cursorp[2])+ff,i,j,VShort);
			else
			  farbe[ff]=(int)VPixel(src[0],framefaktor*(int)rint(pr->cursorp[2])+ff,i,j,VUByte);
		    }
		  }
		  if (pr->glassbrain) {
		    int value1=0;
		    for (int iii=0; iii<VImageNFrames(src[ifile_m]); iii++) {
		      if (VPixelRepn(src[ifile_m])==VShortRepn)
			value1=(int)VPixel(src[ifile_m],framefaktor*iii,i,j,VShort);
		      else
			value1=(int)VPixel(src[ifile_m],framefaktor*iii,i,j,VUByte);
		      if (value1>farbe[0]) farbe[0]=value1;
		    }
		    
		    if (farbe[0]<100) farbe[0]=0;
		  }	
		  if (framefaktor==3) {
		    rgbfarbe[0].setRgb(farbe[0],farbe[1],farbe[2]);
		    image->setPixel ( j-Xplus, i-Yplus, rgbfarbe[0].rgb() );
		  } else {		    						
		    if (farbe[0]!=0)
		      image->setPixel ( j-Xplus, i-Yplus, rgbfarbe[farbe[0]-NP].rgb() );
		    else if (pr->hgfarbe[0]==0) 
		      image->setPixel ( j-Xplus, i-Yplus, rgbfarbe[0].rgb() );
		    else 
		      image->setPixel ( j-Xplus, i-Yplus, rgbfarbe[MFT-1].rgb() );	    
		  }
				      
		  // functional data
		  if (pr->sw2) {
		    int2013=j+int13;
		    if (int10>=0 && int10<fnc_bands && int1012>=0 && int1012<fnc_rows && int2013>=0 && int2013<fnc_columns) {
		      int tzm;
		      double pixelwert=(double)VPixel(fnc[ifile_m],int10,int1012,int2013,VFloat);
		      if (pr->glassbrain) {
			pixelwert=0.0;
			double value1=0.0;
			for (int iii=0; iii<VImageNFrames(fnc[ifile_m]); iii++) {
			  value1=(double)VPixel(fnc[ifile_m],iii,int1012,int2013,VFloat);
			  if (value1>pixelwert) pixelwert=value1;
			}
		      }
		      if (pixelwert>pr->pmax) pixelwert=pr->pmax;
		      if (pr->transzmap) tzm=220-(int)rint((double)farbe[0]/1.5); else tzm=0;
		      if ( pixelwert > ppmax && pixelwert <= pr->pmax && pr->zmapview==1 ) {
			if ((int)rint(p128*(pixelwert-ppmax)) > 127) 
			  image->setPixel ( j-Xplus, i-Yplus, rgbfarbeoverlay[127].dark(tzm).rgb() );
			else
				if(pr->equidistantColorTable) {
					image->setPixel ( j-Xplus, i-Yplus, rgbfarbeoverlay[(int)rint(255*((pixelwert + pr->nmax) / (pr->nmax + pr->pmax)))].dark(tzm).rgb() );
				} else {
					image->setPixel ( j-Xplus, i-Yplus, rgbfarbeoverlay[(int)rint(p128*(pixelwert-ppmax))+128].dark(tzm).rgb() );
				}
		      }
		      if (pixelwert<-pr->nmax) pixelwert=-pr->nmax;
		      if ( pixelwert < -nnmax && pixelwert >= -pr->nmax && pr->zmapview==1 ) {
			if ((int)rint(n128*(-pixelwert-nnmax)) > 127)
			  image->setPixel ( j-Xplus, i-Yplus, rgbfarbeoverlay[255].dark(tzm).rgb() );
			else 
				if(pr->equidistantColorTable) {
					image->setPixel ( j-Xplus, i-Yplus, rgbfarbeoverlay[(int)rint(255*((pixelwert + pr->nmax) / (pr->nmax + pr->pmax)))].dark(tzm).rgb() );
				} else {
					image->setPixel ( j-Xplus, i-Yplus, rgbfarbeoverlay[127-(int)rint(n128*(-pixelwert-nnmax))].dark(tzm).rgb() );
				}
		      }
		    }
		  }
		}
	      } 
	    }
	  } else 
	    if (type_m==2) {
	      image = new QScaleImage(newrows,newbands);
	      zoomn = rows;
	      sc1=(double)vgroesse/(double)rows;
	      sc2=(double)hgroesse/(double)bands;
	      if (pr->cursorp[0]<columns) 
		for ( int i=Zplus; i<Zmax; i++ ) {
		  int1006=i+int6;
		  if (pr->sw2) int1007=i+int7;
		  for ( int j=Yplus; j<Ymax; j++ ) { 
		    for (int ff=0; ff<framefaktor; ff++) {
		      farbe[ff]=0;
		      if (pr->infilenum>1) {
			if (int1006>=0 && int1006<bands && j+int1>=0 && j+int1<rows && pr->cursorp[0]+int2>=0 && pr->cursorp[0]+int2<columns) {
			  if (VPixelRepn(src[ifile_m])==VShortRepn)
			    farbe[ff]=(int)VPixel(src[ifile_m],framefaktor*int1006+ff,j+int1,(int)rint(pr->cursorp[0])+int2,VShort);
			  else
			    farbe[ff]=(int)VPixel(src[ifile_m],framefaktor*int1006+ff,j+int1,(int)rint(pr->cursorp[0])+int2,VUByte);
			} 
		      } else { 
			if (j>=0 && j<rows && pr->cursorp[0]>=0 && pr->cursorp[0]<columns && i>=0 && i<bands) {
			  if (VPixelRepn(src[0])==VShortRepn)
			    farbe[ff]=(int)VPixel(src[0],framefaktor*i+ff, j, (int)rint(pr->cursorp[0]), VShort);
			  else
			    farbe[ff]=(int)VPixel(src[0],framefaktor*i+ff, j, (int)rint(pr->cursorp[0]), VUByte);
			} 
		      }
		    }
		    if (pr->glassbrain) {
		      int value1=0;
		      for (int iii=0; iii<VImageNColumns(src[ifile_m]); iii++) {
			if (VPixelRepn(src[ifile_m])==VShortRepn)
			  value1=(int)VPixel(src[ifile_m],framefaktor*i,j,iii,VShort);
			else
			  value1=(int)VPixel(src[ifile_m],framefaktor*i,j,iii,VUByte);
			if (value1>farbe[0]) farbe[0]=value1;
		      }
		      if (farbe[0]<100) farbe[0]=0;
		    }
		    if (framefaktor==3) {
		      rgbfarbe[0].setRgb(farbe[0],farbe[1],farbe[2]);
		      image->setPixel ( j-Yplus, i-Zplus, rgbfarbe[0].rgb() );
		    } else {	   
		      if (farbe[0]!=0) 
			image->setPixel ( j-Yplus, i-Zplus, rgbfarbe[farbe[0]-NP].rgb() );
		      else if (pr->hgfarbe[0]==0)
			image->setPixel ( j-Yplus, i-Zplus, rgbfarbe[0].rgb() );
		      else 
			image->setPixel ( j-Yplus, i-Zplus, rgbfarbe[MFT-1].rgb() );
		    }		    
		    
		    // functional data
		    if (pr->sw2) {
		      int2012=j+int12;
		      if (int1007>=0 && int1007<fnc_bands && int013>=0 && int013<fnc_columns && int2012>=0 && int2012<fnc_rows) {
			int tzm;
			double pixelwert=(double)VPixel(fnc[ifile_m],int1007,int2012,int013,VFloat);
			if (pr->glassbrain) {
			  pixelwert=0.0;
			  double value1=0.0;
			  for (int iii=0; iii<VImageNColumns(fnc[ifile_m]); iii++) {
			    value1=(double)VPixel(fnc[ifile_m],int1007,int2012,iii,VFloat);
			    if (value1>pixelwert) pixelwert=value1;
			  }
			}
			if (pr->transzmap) tzm=220-(int)rint((double)farbe[0]/1.5); else tzm=0;
			if (pixelwert>pr->pmax) pixelwert=pr->pmax;
			if ( pixelwert > ppmax && pixelwert <= pr->pmax && pr->zmapview==1 ) {
			  if ((int)rint(p128*(pixelwert-ppmax)) > 127) 
			    image->setPixel ( j-Yplus, i-Zplus, rgbfarbeoverlay[127].dark(tzm).rgb() );
			  else
					if(pr->equidistantColorTable) {
						image->setPixel ( j-Yplus, i-Zplus, rgbfarbeoverlay[(int)rint(255*((pixelwert + pr->nmax) / (pr->nmax + pr->pmax)))].dark(tzm).rgb() );
					} else {
						image->setPixel ( j-Yplus, i-Zplus, rgbfarbeoverlay[(int)rint(p128*(pixelwert-ppmax))+128].dark(tzm).rgb() );
					}
			}
			if (pixelwert<-pr->nmax) pixelwert=-pr->nmax;
			if ( pixelwert < -nnmax && pixelwert >= -pr->nmax && pr->zmapview==1 )
			  if ((int)rint(n128*(-pixelwert-nnmax)) > 127)
			    image->setPixel ( j-Yplus, i-Zplus, rgbfarbeoverlay[255].dark(tzm).rgb() );
			  else 
					if(pr->equidistantColorTable) {
						image->setPixel ( j-Yplus, i-Zplus, rgbfarbeoverlay[(int)rint(255*((pixelwert + pr->nmax) / (pr->nmax + pr->pmax)))].dark(tzm).rgb() );
					} else {
						image->setPixel ( j-Yplus, i-Zplus, rgbfarbeoverlay[127-(int)rint(n128*(-pixelwert-nnmax))].dark(tzm).rgb() );
					}
		      }
		    }
		  }
		}
	      
	    } else 
	      if (type_m==1) {
		image =  new QScaleImage(newcols, newbands);
		sc1=(double)vgroesse/(double)columns;
		sc2=(double)hgroesse/(double)bands;
		
		zoomn = columns;
		
		
		if (pr->cursorp[1]<rows) 
		  for ( int i=Zplus; i<Zmax; i++ ) {
		    int1006=i+int6;
		    if (pr->sw2) int1007=i+int7;
		    for ( int j=Xplus; j<Xmax; j++ ) {
		      for (int ff=0; ff<framefaktor; ff++) {
			farbe[ff]=0;
			if (pr->infilenum>1) {
			  if (int1006>=0 && int1006<bands && j+int2>=0 && j+int2<columns && int101>=0 && int101<rows) {
			    if (VPixelRepn(src[ifile_m])==VShortRepn)
			      farbe[ff]=(int)VPixel(src[ifile_m],framefaktor*int1006+ff,int101,j+int2,VShort);
			    else
			      farbe[ff]=(int)VPixel(src[ifile_m],framefaktor*int1006+ff,int101,j+int2,VUByte);
			  } 
			} else { 
			  if (pr->cursorp[1]>=0 && pr->cursorp[1]<rows && j>=0 && j<columns && i>=0 && i<bands) {
			    if (VPixelRepn(src[0])==VShortRepn)
			      farbe[ff]=(int)VPixel(src[0],framefaktor*i+ff,(int)rint(pr->cursorp[1]),j,VShort);
			    else
			      farbe[ff]=(int)VPixel(src[0],framefaktor*i+ff,(int)rint(pr->cursorp[1]),j,VUByte);
			  } 
			}
		      }
		      if (pr->glassbrain) {
			int value1=0;
			for (int iii=0; iii<VImageNRows(src[ifile_m]); iii++) {
			  
			  if (VPixelRepn(src[ifile_m])==VShortRepn)
			    value1=(int)VPixel(src[ifile_m],framefaktor*i,iii,j,VShort);
			  else
			    value1=(int)VPixel(src[ifile_m],framefaktor*i,iii,j,VUByte);
			  if (value1>farbe[0]) farbe[0]=value1;
			}
			if (farbe[0]<100) farbe[0]=0;
		      }
		      if (framefaktor==3) {
			rgbfarbe[0].setRgb(farbe[0],farbe[1],farbe[2]);
			image->setPixel ( j-Xplus, i-Zplus, rgbfarbe[0].rgb() );
		      } else {	
			if (farbe[0]!=0) 
			  image->setPixel ( j-Xplus, i-Zplus, rgbfarbe[farbe[0]-NP].rgb() );
			else if (pr->hgfarbe[0]==0) 
			  image->setPixel ( j-Xplus, i-Zplus, rgbfarbe[0].rgb() );
			else 
			  image->setPixel ( j-Xplus, i-Zplus, rgbfarbe[MFT-1].rgb() );
		      }
			     
		      // functional data
		      if (pr->sw2) {
			int2013=j+int13;
			if ( int1007>=0 && int1007<fnc_bands && int2013>=0 && int2013<fnc_columns && int100>=0 && int100<fnc_rows) {
			  int tzm;
			  double pixelwert=(double)VPixel(fnc[ifile_m],int1007,int100,int2013,VFloat);
			  if (pr->glassbrain) {
			    pixelwert=0.0;
			    double value1=0.0;
			    for (int iii=0; iii<VImageNRows(fnc[ifile_m]); iii++) {
			      value1=(double)VPixel(fnc[ifile_m],int1007,iii,int2013,VFloat);
			      if (value1>pixelwert) pixelwert=value1;
			    }
			  }
			  if (pr->transzmap) tzm=220-(int)rint((double)farbe[0]/1.5); else tzm=0;

			  if (pixelwert>pr->pmax) pixelwert=pr->pmax;
			  if ( pixelwert > ppmax && pixelwert <= pr->pmax && pr->zmapview==1 ) {
			    if ((int)rint(p128*(pixelwert-ppmax)) > 127) 
			      image->setPixel ( j-Xplus, i-Zplus, rgbfarbeoverlay[127].dark(tzm).rgb() );
			    else
					if(pr->equidistantColorTable) {
						image->setPixel ( j-Xplus, i-Zplus, rgbfarbeoverlay[(int)rint(255*((pixelwert + pr->nmax) / (pr->nmax + pr->pmax)))].dark(tzm).rgb() );
					} else {
						image->setPixel ( j-Xplus, i-Zplus, rgbfarbeoverlay[(int)rint(p128*(pixelwert-ppmax))+128].dark(tzm).rgb() );
					}
			  }
			  if (pixelwert<-pr->nmax) pixelwert=-pr->nmax;
			  if ( pixelwert < -nnmax && pixelwert >= -pr->nmax && pr->zmapview==1 ) {
				  if ((int)rint(n128*(-pixelwert-nnmax)) > 127) 
			      image->setPixel ( j-Xplus, i-Zplus, rgbfarbeoverlay[255].dark(tzm).rgb() );
			    else 
					if(pr->equidistantColorTable) {
						image->setPixel ( j-Xplus, i-Zplus, rgbfarbeoverlay[(int)rint(255*((pixelwert + pr->nmax) / (pr->nmax + pr->pmax)))].dark(tzm).rgb() );
					} else {
						image->setPixel ( j-Xplus, i-Zplus, rgbfarbeoverlay[127-(int)rint(n128*(-pixelwert-nnmax))].dark(tzm).rgb() );
					}
			  }
			}
		      }
		    }
		  }
	      }
	  
	  
	  // Now scale the image to the actual drawing size using the active interpolation method
	  // assume zoom in poth directions equal
	  float zoomz = (float)vgroesse * prz;
	  
	  // create a scaler object
	  auto_ptr<CMagBase> scaler(get_interpolator((int)rint((double)zoomz), zoomn ));
	  
	  // scale the image and replace the original with the scaled version
	  QScaleImage *temp= (*scaler)(*image); 
	  delete image; 
	  image = temp; 
	  
	  if (imagebackup)
	    delete imagebackup;
	  
	  imagebackup=image;
	} else {
	  image=imagebackup;
	}

	//paint.scale(sc1, sc2);
	

	// ######################## DRAW IMAGE ################################
	paint->drawImage(0, 0, *image, 0, 0 );

	paint->scale(sc1 * prz, sc2* prz);
	
	paint->setFont( QFont( "arial", 8, QFont::Bold ) );
	paint->setPen(QColor(white));

	int mysize=1, mymove=0;
	int XPLUS=(int)(-Xplus*mysize+mymove);
	int YPLUS=(int)(-Yplus*mysize+mymove);
	int ZPLUS=(int)(-Zplus*mysize+mymove);
	
	int cur0=(XPLUS+(int)rint(pr->cursorp[0])-pr->radius)*mysize;  // (int)rint added by A.Hagert
	int cur1=(YPLUS+(int)rint(pr->cursorp[1])-pr->radius)*mysize;  // (int)rint added by A.Hagert
	int cur2=(ZPLUS+(int)rint(pr->cursorp[2])-pr->radius)*mysize;  // (int)rint added by A.Hagert
	
	int cur0p=(XPLUS+(int)rint(pr->cursorp[0])+pr->radius)*mysize; // (int)rint added by A.Hagert
	int cur1p=(YPLUS+(int)rint(pr->cursorp[1])+pr->radius)*mysize; // (int)rint added by A.Hagert
	int cur2p=(ZPLUS+(int)rint(pr->cursorp[2])+pr->radius)*mysize; // (int)rint added by A.Hagert
	

	if (pr->midclick) {
		mysize=2;
		mymove=1;
	}
  
	if (pr->showradius == 1) {
		paint->setPen(pr->radiuscolor);
		if (type_m==1) {
			paint->drawLine ( cur0, cur2, cur0p, cur2 );
			paint->drawLine ( cur0, cur2p, cur0p, cur2p );
			paint->drawLine ( cur0, cur2, cur0, cur2p );
			paint->drawLine ( cur0p, cur2, cur0p, cur2p );
		}
		else if (type_m==2) {
			paint->drawLine ( cur1, cur2, cur1p, cur2 );
			paint->drawLine ( cur1, cur2p, cur1p, cur2p );
			paint->drawLine ( cur1, cur2, cur1, cur2p );
			paint->drawLine ( cur1p, cur2, cur1p, cur2p );
		}
		else if (type_m==3) {
			paint->drawLine ( cur0, cur1, cur0p, cur1 );
			paint->drawLine ( cur0, cur1p, cur0p, cur1p );
			paint->drawLine ( cur0, cur1, cur0, cur1p );
			paint->drawLine ( cur0p, cur1, cur0p, cur1p );
		}
	}

	//crosscolor=QColor(yellow);
	if ( pr->showcross == 1 && crossoff_m==0 ) {
		paint->setPen(pr->crosscolor);
		//int XPLUS=(int)(-Xplus*mysize+mymove);
		//int YPLUS=(int)(-Yplus*mysize+mymove);
		//int ZPLUS=(int)(-Zplus*mysize+mymove);
    
    int te1=(int)(XPLUS+rint(pr->cursorp[0])*mysize);	// (int)rint() added by A. Hagert
    int te2=(int)(YPLUS+rint(pr->cursorp[1])*mysize);   // (int)rint() added by A. Hagert
    int te3=(int)(ZPLUS+rint(pr->cursorp[2])*mysize);   // (int)rint() added by A. Hagert

    int de1=(int)(XPLUS+(rint(pr->cursorp[0])-3)*mysize);   // (int)rint() added by A. Hagert
    int de2=(int)(YPLUS+(rint(pr->cursorp[1])+3)*mysize);   // (int)rint() added by A. Hagert
    int de3=(int)(ZPLUS+(rint(pr->cursorp[2])+3)*mysize);   // (int)rint() added by A. Hagert
    int de4=(int)(XPLUS+(rint(pr->cursorp[0])+3)*mysize);   // (int)rint() added by A. Hagert
    int de5=(int)(YPLUS+(rint(pr->cursorp[1])-3)*mysize);   // (int)rint() added by A. Hagert
    int de6=(int)(ZPLUS+(rint(pr->cursorp[2])-3)*mysize);   // (int)rint() added by A. Hagert

		int kkk3=(int)(XPLUS+(columns-2)*mysize);
		int kkk4=(int)(YPLUS+(rows-2)*mysize);
		int kkk5=(int)(ZPLUS+(bands-2)*mysize);

		if (type_m==1) {
			if (pr->active==ifile_m) paint->setPen(pr->crosscolor);
			else paint->setPen(QColor(green));
			paint->drawLine (XPLUS,te3,de1,te3);
			paint->drawLine (te1,ZPLUS,te1,de6);
			paint->drawLine (de4,te3,kkk3,te3);
			paint->drawLine (te1,de3,te1,kkk5);
			paint->drawPoint (te1,te3);
		}
		else if (type_m==2) {
			if (pr->active==ifile_m) paint->setPen(pr->crosscolor);
			else paint->setPen(QColor(green));
			paint->drawLine (te2,ZPLUS,te2,de6);
			paint->drawLine (YPLUS,te3,de5,te3);
			paint->drawLine (te2,de3,te2,kkk5);
			paint->drawLine (de2,te3,kkk4,te3);
			paint->drawPoint (te2,te3);
		}
    
		// ver=xx hor=zz
		else if (type_m==3) {
			if (pr->active==ifile_m) paint->setPen(pr->crosscolor);
			else paint->setPen(QColor(green));
			paint->drawLine (XPLUS,te2,de1,te2);
			paint->drawLine (te1,YPLUS,te1,de5);
			paint->drawLine (de4,te2,kkk3,te2);
			paint->drawLine (te1,de2,te1,kkk4);
			paint->drawPoint (te1,te2);
		}
	}
}

void pictureView::mousePressEvent( QMouseEvent *e  )
{
  int x=0, y=0, z=0;
  pr->active = ifile_m;
  recreate=1;
  
  // Bildzoom;
  prz=pr->bildzoom;
  
  int addpixel_y1 =(int)rint((double)e->pos().y()/sc1/(double)prz);
  int addpixel_y2 =(int)rint((double)e->pos().y()/sc2/(double)prz);
  int addpixel_x1 =(int)rint((double)e->pos().x()/sc1/(double)prz);
  int addpixel_x2 =(int)rint((double)e->pos().x()/sc2/(double)prz);
  pr->mousemove=0;
  
  // cursor position
  y=(int)rint(pr->cursorp[1]);
  x=(int)rint(pr->cursorp[0]);
  z=(int)rint(pr->cursorp[2]);

  if (e->button()==LeftButton || e->button()==MidButton || e->button()==RightButton) {
    if (type_m==1) {
      if (sc1 >= sc2) {
	z=Zplus+addpixel_y2;
	x=Xplus+addpixel_x2;
      } else {
	z=Zplus+addpixel_y1;
	x=Xplus+addpixel_x1;
      }
    }
    else if (type_m==2) {
      if (sc1 >= sc2) {
	z=Zplus+addpixel_y2;
	y=Yplus+addpixel_x2;
      } else {
	z=Zplus+addpixel_y1;
	y=Yplus+addpixel_x1;
      }
    }
    else if (type_m==3) {
      if (sc1 >= sc2) {
	y=Yplus+addpixel_y2;
	x=Xplus+addpixel_x2;
      } else {
	y=Yplus+addpixel_y1;
	x=Xplus+addpixel_x1;
      }      
    }
    if (y<0) y=0;
    if (x<0) x=0;
    if (z<0) z=0;
    if (y>rows-1)    y=rows-1;
    if (x>columns-1) x=columns-1;
    if (z>bands-1)   z=bands-1;
  } 
  
  if (e->button()==MidButton) {
    if (pr->bildzoom<8)
      pr->bildzoom *= 2;
  }  
  
  if (e->button()==RightButton) {
    if (pr->bildzoom>2) pr->bildzoom /= 2;
    else pr->bildzoom=1;
  }
  
  // Bildzoom;
  prz=pr->bildzoom;
  
  
  pr->cursorp[1]=y;pr->cursorp[0]=x;pr->cursorp[2]=z;
  emit viewChanged();  
  talCross(x,y,z);  
  if (fnc[0]) {
    if (z<fnc_bands && y<fnc_rows && x<fnc_columns)
      emit z2Wert(VPixel(fnc[ifile_m],z,y,x,VFloat));
  } else if (z<bands && y<rows && x<columns) {
    emit z2Wert(VGetPixel(src[ifile_m],z,y,x));
  }
}

void pictureView::mouseReleaseEvent( QMouseEvent *e )
{
        // Bildzoom;
	prz=pr->bildzoom;


	int x=0, y=0, z=0, x2=0, y2=0, z2=0;
	int addpixel_y1 =(int)rint((double)e->pos().y()/sc1/(double)prz);
	int addpixel_y2 =(int)rint((double)e->pos().y()/sc2/(double)prz);
	int addpixel_x1 =(int)rint((double)e->pos().x()/sc1/(double)prz);
	int addpixel_x2 =(int)rint((double)e->pos().x()/sc2/(double)prz);
	pr->mousemove=0;

	// cursor position
	y=y2=(int)rint(pr->cursorp[1]);
	x=x2=(int)rint(pr->cursorp[0]);
	z=z2=(int)rint(pr->cursorp[2]);

	if (type_m==1) {
		if (sc1 >= sc2) {
			z=Zplus+addpixel_y2;
			x=Xplus+addpixel_x2;
		} else {
			z=Zplus+addpixel_y1;
			x=Xplus+addpixel_x1;
		}
	}
	else if (type_m==2) {
		if (sc1 >= sc2) {
			z=Zplus+addpixel_y2;
			y=Yplus+addpixel_x2;
		} else {
			z=Zplus+addpixel_y1;
			y=Yplus+addpixel_x1;
		}
	}
	else if (type_m==3) {
		if (sc1 >= sc2) {
			y=Yplus+addpixel_y2;
			x=Xplus+addpixel_x2;
		} else {
			y=Yplus+addpixel_y1;
			x=Xplus+addpixel_x1;
		}
	}
	if (y>rows-1)    y=rows-1;    if (y2>rows-1)    y2=rows-1;
	if (x>columns-1) x=columns-1; if (x2>columns-1) x2=columns-1;
	if (z>bands-1)   z=bands-1;   if (z2>bands-1)   z2=bands-1;
	if (y<0) y=0; if (y2<0) y2=0;
	if (x<0) x=0; if (x2<0) x2=0;
	if (z<0) z=0; if (z2<0) z2=0;

	// NEU
	if (pr->interpoltype!=0) emit viewChanged();  //NEU

	// Koordianten und Anzeige der z-Werte syncronisieren!
	talEcht(x,y,z);
	talCross(x2,y2,z2);    
	if (fnc[0]) {
		if (z<fnc_bands && y<fnc_rows && x<fnc_columns) {
			emit zWert(VPixel(fnc[ifile_m],z,y,x,VFloat));
			emit z2Wert(VPixel(fnc[ifile_m],z2,y2,x2,VFloat));
		} else {
			emit zWert(0.0);
			emit z2Wert(0.0);
		}
	} else {
		if (z<bands && y<rows && x<columns) {
		  emit zWert(VGetPixel(src[ifile_m],z,y,x));
		  emit z2Wert(VGetPixel(src[ifile_m],z2,y2,x2));
		}
	}
	
	emit sendtoserver();
}


//pictureView::hasMouse ()

void pictureView::mouseMoveEvent( QMouseEvent *e )
{

       	// Bildzoom;
	prz=pr->bildzoom;  

	int x=0, y=0, z=0;
	int addpixel_y1 =(int)rint((double)e->pos().y()/sc1/(double)prz);
	int addpixel_y2 =(int)rint((double)e->pos().y()/sc2/(double)prz);
	int addpixel_x1 =(int)rint((double)e->pos().x()/sc1/(double)prz);
	int addpixel_x2 =(int)rint((double)e->pos().x()/sc2/(double)prz);
	int richtung = (int)e->pos().y()-letzte;
	pr->mousemove=0;

	if (type_m==1) {
		y=(int)rint(pr->cursorp[1]);
		if (sc1 >= sc2) {
			z=Zplus+addpixel_y2;
			x=Xplus+addpixel_x2;
		} else {
			z=Zplus+addpixel_y1;
			x=Xplus+addpixel_x1;
		}
	}
	else if (type_m==2) {
		x=(int)rint(pr->cursorp[0]);
		if (sc1 >= sc2) {
			z=Zplus+addpixel_y2;
			y=Yplus+addpixel_x2;
		} else {
			z=Zplus+addpixel_y1;
			y=Yplus+addpixel_x1;
		}
	}
	else if (type_m==3) {
		z=(int)rint(pr->cursorp[2]);
		if (sc1 >= sc2) {
			y=Yplus+addpixel_y2;
			x=Xplus+addpixel_x2;
		} else {
			y=Yplus+addpixel_y1;
			x=Xplus+addpixel_x1;
		}
	}
	if (y>rows-1) y=rows-1;
	if (x>columns-1) x=columns-1;
	if (z>bands-1) z=bands-1;
	if (y<0) y=0;
	if (x<0) x=0;
	if (z<0) z=0;

	if (e->state()==LeftButton) {
	        pr->cursorp[1]=y;pr->cursorp[0]=x;pr->cursorp[2]=z;
		pr->mousemove=1;
		emit viewChanged();
		pr->mousemove=0;
	} else {
		talEcht(x,y,z);    
		if (fnc[0]) {
		  if(z < fnc_bands && y < fnc_rows && x < fnc_columns) {
			  emit zWert(VPixel(fnc[ifile_m], z, y, x, VFloat));
		  } else {
			  emit zWert(0.0);
		  }
		} else if (z<bands && y<rows && x<columns) {
		  emit zWert(VGetPixel(src[ifile_m],z,y,x));
		}
	}
}

void pictureView::talEcht( int col, int row, int band ) {
	int x=col, y=row, z=band;
	int files=0;
	double XXt = (double)x; 
	double YYt = (double)y; 
	double ZZt = (double)z; 

	if (pr->infilenum>pr->zmapfilenum) 
		files=pr->infilenum;
	else 
		files=pr->zmapfilenum;
  
	if ( pr->talairach==1 && pr->talairachoff==0 ) {
		VLTools mytools;
		mytools.VPixel3Tal(XXt,YYt,ZZt, extent_m, ca_m ,cp_m, (int)files, pr->pixelmult);
		emit echtPosit((float)XXt,(float)YYt,(float)ZZt,"t");
	} else {
	  XXt *= pr->pixelmult[0]; 
	  YYt *= pr->pixelmult[1]; 
	  ZZt *= pr->pixelmult[2]; 

	  if (pr->pixelco==1)
	    emit echtPosit((float)x,(float)y,(float)z,"a");
	  else if (pr->pixelco==2)
	    emit echtPosit((float)floor(XXt/pr->pixelm2[0]),(float)floor(YYt/pr->pixelm2[1]),(float)floor(ZZt/pr->pixelm2[2]),"z");
	  else
	    emit echtPosit((float)floor(XXt),(float)floor(YYt),(float)floor(ZZt),"m");
	}
}

void pictureView::talCross( int col, int row, int band ) {
	int x=col, y=row, z=band;
	int files=0;
	double XXt = (double)x; 
	double YYt = (double)y; 
	double ZZt = (double)z; 

	if (pr->infilenum>pr->zmapfilenum) 
		files=pr->infilenum;
	else 
		files=pr->zmapfilenum;
  
	if (pr->talairach==1 && pr->talairachoff==0) {
		VLTools mytools;
		mytools.VPixel3Tal(XXt,YYt,ZZt, extent_m, ca_m ,cp_m, (int)files, pr->pixelmult);
		emit crossPosit((float)XXt,(float)YYt,(float)ZZt,"t");
	} else {
	  XXt *= pr->pixelmult[0]; 
	  YYt *= pr->pixelmult[1]; 
	  ZZt *= pr->pixelmult[2]; 
	  
	  if (pr->pixelco==1) 
	    emit crossPosit((float)x,(float)y,(float)z,"a");
	  else if (pr->pixelco==2)
	    emit crossPosit((float)floor(XXt/pr->pixelm2[0]),(float)floor(YYt/pr->pixelm2[1]),(float)floor(ZZt/pr->pixelm2[2]),"z");
	  else 
	    emit crossPosit((float)floor(XXt),(float)floor(YYt),(float)floor(ZZt),"m");
	}
}

void pictureView::repaintf() {
	recreate=1;
	repaint( 0,0,width(),height(),FALSE);
}

QSizePolicy pictureView::sizePolicy() const {
	return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

void pictureView::posChanged( int pos ) {
	ppmax=(double)pos/pr->slidefaktor[0];
	pr->tpos=(float)ppmax;
	repaint( 0,0,width(),height(),FALSE);
	colorMap();
	emit colbarRepaint();
}

void pictureView::negChanged( int neg ) {
	nnmax=(double)neg/pr->slidefaktor[1];
	pr->tneg=(float)nnmax;
	repaint( 0,0,width(),height(),FALSE);
	colorMap();
	emit colbarRepaint();
}

