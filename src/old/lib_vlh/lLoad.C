/****************************************************************
 *
 * lLoad.C
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

#include "lLoad.h"
#include "VLTools.h"
#include <viaio/headerinfo.h>

#define NP -32768
#define MP 32767

/* only for vlcorr */
extern "C" {
  extern void VImageInfoIni(VImageInfo *);
  extern VBoolean  ReadHeader(FILE *);
  extern VBoolean  VGetImageInfo(FILE *,VAttrList,int,VImageInfo *);
  extern VBoolean  VReadBlockData(FILE *,VImageInfo *,int,int,VImage *);
  extern VAttrList ReadAttrList(FILE *);
}

extern prefs *pr;
extern VImage *src, *fnc, *tmp;
extern double *ca, *cp, *extent, *fixpoint;
extern double *scalec, *scaler, *scaleb;
extern VArgVector in_files, zmap_files, graph_files;  
extern VBoolean zmap_found, graph_found;
extern VStringConst zmap_filename, in_filename;

/* Parameter for vlcorr */
int firstfuncobj=-1, nobjects=0, hist_items=0;
VImage rawobjektbild=NULL;
VImageInfo *tempInfo=NULL;
VLTools myTools;


/* by A. Hagert - need that 2 times */
void lLoad::ScanGraph(int *max_band, int *min_band, int *max_row, int *min_row, int *max_col, int *min_col) {
  typedef struct MyNodeStruct_short {   /* added by A. Hagert: "short graph" */
    VNodeBaseRec base;
    VShort type;
    VShort col;
    VShort row; 
    VShort band;
    VShort label;
  } MyNodeRec_short, *MyNode_short;
	
  typedef struct MyNodeStruct_float {   /* added by A. Hagert: "float graph" */
    VNodeBaseRec base;
    VFloat type;
    VFloat col;
    VFloat row;
    VFloat band;
    VFloat label;
  } MyNodeRec_float, *MyNode_float;
	
  MyNode_short node_s, node1_s;      /* added by A. Hagert: for short-graph */
  MyNode_float node_f, node1_f;      /* added by A. Hagert: for float-graph */
  VAdjacency neighb;
  VNodeBaseRec base;
  FILE *graph_file;
  VAttrList graph_list;
  VGraph gsrc;
  VAttrListPosn posn1;
  short firstnode=0;
  float u=0, v=0, w=0;
  char *token=NULL;
  VString str, str1, str2;

  graph_file = VOpenInputFile (pr->graph[0], TRUE);
  if (! (graph_list = VReadFile (graph_file, NULL))) exit(1);
	  
  for (VFirstAttr (graph_list, & posn1); VAttrExists (& posn1); VNextAttr (& posn1)) 
  {
    if (VGetAttrRepn (& posn1) != VGraphRepn) continue;
    VGetAttrValue (& posn1, NULL, VGraphRepn, & gsrc);
  }
	
  if (VNodeRepn(gsrc) == VFloatRepn) pr->graphtype = 1;    /* added by A. Hagert: -> graph of type float */

  if (pr->only_sulci==1) {
    // Get CA of graph for Talairach transform
    ca[0] = ca[1] = ca[2] = 0.0;
    if (VGetAttr (VGraphAttrList (gsrc), "ca", NULL,VStringRepn, (VPointer) & str) == VAttrFound) {
      char aaa;
      strncpy(&aaa,str,(size_t)1);
      if (aaa!='N')
	if ((token = strtok(str, " ")) != NULL) {
	  u = atof(token);
	  if ((token = strtok(NULL, " ")) != NULL) {
	    v = atof(token);
	    if ((token = strtok(NULL, " ")) != NULL) {
	      w = atof(token);
	    }
	  }
	}
      if (aaa=='N') {
	pr->talairachoff=1;
	if (pr->verbose>=1) fprintf(stderr,"attribute 'ca' = N.\n");
      } else {
	ca[0]  = u;
	ca[1]  = v;
	ca[2]  = w;
	if (pr->verbose>=1) fprintf(stderr,"got 'ca' from graph for talairach transform... (%f,%f,%f)\n",u,v,w);
      }
    }
    else {
      if (pr->verbose>=1) fprintf(stderr,"attribute 'ca' missing in graph file.\n");
      pr->talairachoff=1;
    }  

    // Get CP for Talairach transform
    cp[0] = cp[1] = cp[2] = 0.0;
    if (VGetAttr (VGraphAttrList (gsrc), "cp", NULL,VStringRepn, (VPointer) & str) == VAttrFound) {
      char aaa;
      strncpy(&aaa,str,(size_t)1);
      if (aaa!='N')
	if ((token = strtok(str, " ")) != NULL) {
	  u = atof(token);
	  if ((token = strtok(NULL, " ")) != NULL) {
	    v = atof(token);
	    if ((token = strtok(NULL, " ")) != NULL) {
	      w = atof(token);
	    }
	  }
	}
      if (aaa=='N') {
	pr->talairachoff=1;
	if (pr->verbose>=1) fprintf(stderr,"attribute 'cp' = N.\n");
      } else {
	cp[0]  = u;
	cp[1]  = v;
	cp[2]  = w;
	if (pr->verbose>=1) fprintf(stderr,"got 'cp' from graph for talairach transform... (%f,%f,%f)\n",u,v,w);
      }
    }
    else {
      if (pr->verbose>=1) fprintf(stderr,"attribute 'cp' missing in graph file.\n");
      pr->talairachoff=1;
    }

    // get extent for linear scaling
    extent[0] = extent[1] = extent[2] = 0.0;
    if (VGetAttr (VGraphAttrList (gsrc), "extent", NULL,VStringRepn, (VPointer) & str1) == VAttrFound) {
      char aaa;
      strncpy(&aaa,str1,(size_t)1);
      if (aaa!='N')
	if ((token = strtok(str1, " ")) != NULL) {
	  u = atof(token);
	  if ((token = strtok(NULL, " ")) != NULL) {
	    v = atof(token);
	    if ((token = strtok(NULL, " ")) != NULL) {
	      w = atof(token);
	    }
	  }
	}	  
      if (aaa=='N') {
	pr->talairachoff=1;
	if (pr->verbose>=1) fprintf(stderr,"attribute 'extent' = N.\nStarting without talairach coordinates!\n");
      } else {
	extent[0] = u;
	extent[1] = v;
	extent[2] = w;
	if (pr->verbose>=1) fprintf(stderr,"got 'extent' from graph for talairach transform...\n");
      }
    } else {
      if (pr->verbose>=1) fprintf(stderr,"attribute 'extent' missing in graph file.\nStarting without talairach coordinates!\n");
      pr->talairachoff=1;
    }  

    // talairach ************************************************
    if (VGetAttr (VGraphAttrList (gsrc), "talairach", NULL,VStringRepn, (VPointer) & str2) == VAttrFound) {
      if (strcmp(str2,"atlas") == 0) pr->atlas = 1;
      else pr->talairachoff=1;
    } else pr->talairachoff=1;
  }
  
  for (int i=1; i<=gsrc->lastUsed; i++) {   /* added by A. Hagert: find x,y,z - maxima of the graph*/
    if (pr->graphtype == 0)   /* added by A. Hagert: short-graph */
    {
      node_s = (MyNode_short) VGraphGetNode (gsrc,i);
      if (node_s == NULL) continue;
      
      if (*max_band == 0 && *min_band == 0 && *max_row == 0 && *min_row == 0 && *max_col == 0 && *min_col == 0) {
        *max_band = node_s->band;
	*min_band = node_s->band;
	*max_row = node_s->row;
	*min_row = node_s->row;
	*max_col = node_s->col;
	*min_col = node_s->col;
      }

      if (node_s->band > *max_band) *max_band = node_s->band;      /* added by A. Hagert: band-max for nodes without links */
      if (node_s->row  > *max_row)  *max_row  = node_s->row;       /* added by A. Hagert: row-max for nodes without links */
      if (node_s->col  > *max_col)  *max_col  = node_s->col;       /* added by A. Hagert: col-max for nodes without links */
      if (node_s->band < *min_band) *min_band = node_s->band;      /* added by A. Hagert: band-max for nodes without links */
      if (node_s->row  < *min_row)  *min_row  = node_s->row;       /* added by A. Hagert: row-max for nodes without links */
      if (node_s->col  < *min_col)  *min_col  = node_s->col;       /* added by A. Hagert: col-max for nodes without links */
      if (pr->zmapfilenum>0){
        if (firstnode==0 && pr->col_min==0 && pr->col_max==0) {
	  firstnode=1; pr->col_min = node_s->label; pr->col_max = node_s->label; 
	}
        if (node_s->label < pr->col_min) pr->col_min = node_s->label;  /* nodes without links */
        if (node_s->label > pr->col_max) pr->col_max = node_s->label;  /* -- " -- */
      }

      base   = node_s->base;
      neighb = base.head;
	
      while ((neighb != NULL)) {
        node1_s = (MyNode_short) VGraphGetNode(gsrc,neighb->id);
        if (node1_s->band > *max_band) *max_band = node1_s->band;      /* added by A. Hagert: band-max for nodes with links */
        if (node1_s->row  > *max_row)  *max_row  = node1_s->row;       /* added by A. Hagert: col-max for nodes with links */
        if (node1_s->col  > *max_col)  *max_col  = node1_s->col;       /* added by A. Hagert: row-max for nodes with links */
        if (node1_s->band < *min_band) *min_band = node_s->band;       /* added by A. Hagert: band-max for nodes without links */
        if (node1_s->row  < *min_row)  *min_row  = node_s->row;        /* added by A. Hagert: row-max for nodes without links */
        if (node1_s->col  < *min_col)  *min_col  = node_s->col;        /* added by A. Hagert: col-max for nodes without links */
        if (pr->zmapfilenum>0){
	  if (firstnode==0 && pr->col_min==0 && pr->col_max==0) { 
	    firstnode=1; pr->col_min = node1_s->label; pr->col_max = node1_s->label; 
	  }
          if (node1_s->label < pr->col_min) pr->col_min = node1_s->label;  /* nodes without links */
          if (node1_s->label > pr->col_max) pr->col_max = node1_s->label;  /* -- " -- */
	}
        neighb = neighb->next;
      }
    }
    else   /* added by A. Hagert: analog maximum determination for float-graph */
    {
      node_f = (MyNode_float) VGraphGetNode (gsrc,i);
      if (node_f == NULL) continue; 
      
      if (*max_band == 0 && *min_band == 0 && *max_row == 0 && *min_row == 0 && *max_col == 0 && *min_col == 0) {
        *max_band = (int)ceil(node_f->band);
	*min_band = (int)node_f->band;
	*max_row = (int)ceil(node_f->row);
	*min_row = (int)node_f->row;
	*max_col = (int)ceil(node_f->col);
	*min_col = (int)node_f->col;
      }
      
      if (node_f->band > *max_band) *max_band = (int)ceil(node_f->band);
      if (node_f->row  > *max_row)  *max_row  = (int)ceil(node_f->row);
      if (node_f->col  > *max_col)  *max_col  = (int)ceil(node_f->col);
      if (node_f->band < *min_band) *min_band = (int) node_f->band;
      if (node_f->row  < *min_row)  *min_row  = (int) node_f->row;
      if (node_f->col  < *min_col)  *min_col  = (int) node_f->col;
      if (pr->zmapfilenum>0){
        if (firstnode==0 && pr->col_min==0 && pr->col_max==0) { 
	  firstnode=1; pr->col_min = node_f->label; pr->col_max = node_f->label; 
	}
        if (node_f->label < pr->col_min) pr->col_min = node_f->label;  /* nodes without links */
        if (node_f->label > pr->col_max) pr->col_max = node_f->label;  /* -- " -- */
      }

      base   = node_f->base;
      neighb = base.head;

      while ((neighb != NULL)) {
        node1_f = (MyNode_float) VGraphGetNode(gsrc,neighb->id);
        if (node1_f->band > *max_band) *max_band = (int)ceil(node1_f->band);
        if (node1_f->row  > *max_row)  *max_row  = (int)ceil(node1_f->row);
        if (node1_f->col  > *max_col)  *max_col  = (int)ceil(node1_f->col);
        if (node1_f->band < *min_band) *min_band = (int)node1_f->band;
        if (node1_f->row  < *min_row)  *min_row  = (int)node1_f->row;
        if (node1_f->col  < *min_col)  *min_col  = (int)node1_f->col;
        if (pr->zmapfilenum>0){
	  if (firstnode==0 && pr->col_min==0 && pr->col_max==0) { 
	    firstnode=1; pr->col_min = node1_f->label; pr->col_max = node1_f->label; 
	  }
          if (node1_f->label < pr->col_min) pr->col_min = node1_f->label;  /* nodes without links */
          if (node1_f->label > pr->col_max) pr->col_max = node1_f->label;  /* -- " -- */
        }

        neighb = neighb->next;
      }
    }
  }
}


void lLoad::testFiles() 
{
  FILE *f, *graph_file;;
  VAttrList in_list, graph_list;
  VAttrListPosn posn, posn1;
  VGraph gsrc;
  VImage p_image=NULL;
  int ifn = pr->infilenum;

  //  str = (char *) malloc(sizeof(char)*100); // einige leute meinen, dies sei nötig;

  pr->infilenum_graph = 0; /* added by A. Hagert */
  pr->files=0;
  if (pr->infilenum > pr->zmapfilenum)
    pr->files=pr->infilenum;
  else 
    pr->files=pr->zmapfilenum;

  if (pr->infilenum==0) 
    VError("No infile specified");
  
  /* For each input file: */
  int ifile;
  for (ifile = 0; ifile < pr->infilenum; ifile++) {
    in_filename = ((VString/*Const*/ *) in_files.vector)[ifile];

    /* Read its contents: */
    if (strcmp (in_filename, "-") == 0)
      f = stdin;
    else {
      f = fopen (in_filename, "r");
      if (! f)
	VError ("Failed to open input file %s", in_filename);
    }
    if (! (in_list = VReadFile (f, NULL)))
      exit (EXIT_FAILURE);
    fclose (f);

    /* Count the number of objects in vista file */
    for (VFirstAttr (in_list, & posn); VAttrExists (& posn); VNextAttr (& posn))
      if (VGetAttrRepn (& posn) == 16) {
	if (ifn > 1) ifn--;
        pr->g_number++;  /* by A. Hagert: -> this is a graph */
	graph_file = VOpenInputFile (in_filename, TRUE);
        if (! (graph_list = VReadFile (graph_file, NULL))) exit(1);
    
        for (VFirstAttr (graph_list, & posn1); VAttrExists (& posn1); VNextAttr (& posn1)) {
          if (VGetAttrRepn (& posn1) != VGraphRepn) continue;
          VGetAttrValue (& posn1, NULL, VGraphRepn, & gsrc);
          VGetAttr(VGraphAttrList(gsrc), "image", NULL, VImageRepn, & p_image);
	  if (p_image) pr->polygons = 1;
        }
      }
  }
  if (graph_files.number>1) pr->g_number=graph_files.number;
  pr->infilenum = ifn;
}


/* load (or reload) all input files */
void lLoad::loadFiles() {
  FILE *f;
  VAttrList in_list, zmap_list;
  VAttrListPosn posn;
  VBoolean sw=0;
  char *token=NULL;
  int counter_zmap=0, counter_predisp=0, npixels=0;
  int nrows_sc=0, ncols_sc=0, nbands_sc=0;
  int nrows_z=0, ncols_z=0, nbands_z=0;
  float u=0, v=0, w=0;
  VString str, str1, str2, newstr;
  int funcobjs=0, anabands=0, ts_temp=0;
  int nbands, ncols, nrows;
  FILE *rawfp;
  int counter_graph_pid;
  short ok=0; // by A. Hagert - IPC - 1 if there is the right graph to show
 

  //  str = (char *) malloc(sizeof(char)*100); // einige leute meinen, dies sei nötig;

  int counter_anatomical=0;
  int counter_functional=0;
    
  if (pr->verbose>=1) {
    fprintf(stderr,"(re)loading files\n");
    fprintf(stderr,"in=%d zmap=%d files=%d\n",pr->infilenum,pr->zmapfilenum,pr->files);
  }

  pr->infilenum_graph = 0; /* added by A. Hagert */
  pr->files=0;
  if (pr->infilenum > pr->zmapfilenum)
    pr->files=pr->infilenum;
  else 
    pr->files=pr->zmapfilenum;

  if (pr->infilenum==0) 
    VError("No infile specified");
  
  counter_graph_pid = 0;
  /* For each input file: */
  int ifile;
  for (ifile = 0; ifile < pr->infilenum; ifile++) {
    in_filename = ((VString/*Const*/ *) in_files.vector)[ifile];

    /* Read its contents: */
    if (strcmp (in_filename, "-") == 0)
      f = stdin;
    else {
      f = fopen (in_filename, "r");
      if (! f)
	VError ("Failed to open input file %s", in_filename);
    }
    if (! (in_list = VReadFile (f, NULL)))
      exit (EXIT_FAILURE);
    fclose (f);

    if (pr->graph[0]) {
      FILE *graph_file;
      VAttrList graph_list;
      VGraph gsrc;
      VAttrListPosn posn1;

      graph_file = VOpenInputFile (pr->graph[0], TRUE);
      if (! (graph_list = VReadFile (graph_file, NULL))) exit(1);
	  
      for (VFirstAttr (graph_list, & posn1); VAttrExists (& posn1); VNextAttr (& posn1)) 
      {
        if (VGetAttrRepn (& posn1) != VGraphRepn) continue;
        VGetAttrValue (& posn1, NULL, VGraphRepn, & gsrc);
      }
	
      if (VNodeRepn(gsrc) == VFloatRepn) pr->graphtype = 1;    /* added by A. Hagert: -> graph of type float */
    }

    /* Count the number of objects in vista file */
    for (VFirstAttr (in_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {

      /* eingefügt von A.Hagert */
      if (VGetAttrRepn (& posn) == 16) {   /* added by A. Hagert: -> this is a graph */
        counter_graph_pid++;
        ok=0;

	if (pr->g_number==1 || ok == 1) { /* schliesse bei 2 Graphen je einen aus */

          /* added by A. Hagert: Vorspiegelung falscher Tatsachen - kreiere einfach das anatomische file ... */
 	  pr->graph[pr->infilenum_graph] = (VString)in_filename; /* erkenne "-in"-file als Graph */
	  pr->infilenum_graph++;
	  pr->only_sulci = 1;  
          	
	  int max_band=0, max_row=0, max_col=0, min_band=0, min_row=0, min_col=0;
	  int add_band=0, add_row=0, add_col=0;
	  ScanGraph(&max_band, &min_band, &max_row, &min_row, &max_col, &min_col);
	  if (max_band == min_band) add_band=1;
	  if (max_row == min_row) add_row=1;
	  if (max_col == min_col) add_col=1;
          src[0] = VCreateImage(max_band-min_band+add_band, max_row-min_row+add_row, max_col-min_col+add_col, VUByteRepn);
	  counter_anatomical++;     
	}
      }
      else if ((pr->graph[0]) && (pr->g_number==1 || ok==1)) /* added by A. Hagert: now graph _with_ anatomical data */
      {
        FILE *graph_file;
	VAttrList graph_list;
	VGraph gsrc;
	VAttrListPosn posn1;
	
        graph_file = VOpenInputFile (pr->graph[0], TRUE);
          if (! (graph_list = VReadFile (graph_file, NULL))) exit(1);

	for (VFirstAttr (graph_list, & posn1); VAttrExists (& posn1); VNextAttr (& posn1)) 
	{
          if (VGetAttrRepn (& posn1) != VGraphRepn) continue;
          VGetAttrValue (& posn1, NULL, VGraphRepn, & gsrc);
        }
	
	if (VNodeRepn(gsrc) == VFloatRepn) pr->graphtype = 1; /* added by A. Hagert: -> graph is from type float */
      }

      /* hier normal weiter von H.M. */
      if (VGetAttrRepn (& posn) != VImageRepn) continue;
      VGetAttrValue (& posn, NULL, VImageRepn, & tmp[ifile]);

      if ( (VPixelRepn(tmp[ifile])==VUByteRepn || VPixelRepn(tmp[ifile])==VBitRepn || VPixelRepn(tmp[ifile])==VShortRepn) && counter_anatomical==ifile) {
	counter_anatomical++;
	if (VPixelRepn(tmp[ifile])==VUByteRepn || VPixelRepn(tmp[ifile])==VShortRepn)
	  src[ifile]=VCopyImage(tmp[ifile], NULL, VAllBands);
	if (VPixelRepn(tmp[ifile])==VBitRepn)
	  src[ifile]=VConvertImageRange(tmp[ifile], NULL, VAllBands,VUByteRepn);
	if (VPixelRepn(tmp[ifile])==VShortRepn)
	  pr=myTools.vlhContrast(pr,tmp[ifile]);
	//VFree(tmp[ifile]);
	
	if (VImageNRows(src[ifile])!=VImageNRows(src[0]) || VImageNColumns(src[ifile])!=VImageNColumns(src[0]) || VImageNFrames(src[ifile])!=VImageNFrames(src[0]))
	  VError("Dimensions of infiles must agree");
		
	// Get CA for Talairach transform
	ca[0*pr->files+ifile] = ca[1*pr->files+ifile] = ca[2*pr->files+ifile] = 0.0;
	if (VGetAttr (VImageAttrList (src[ifile]), "ca", NULL,VStringRepn, (VPointer) & str) == VAttrFound) {
	  char aaa;
	  strncpy(&aaa,str,(size_t)1);
	  if (aaa!='N')
	    if ((token = strtok(str, " ")) != NULL) {
	      u = atof(token);
	      if ((token = strtok(NULL, " ")) != NULL) {
		v = atof(token);
		if ((token = strtok(NULL, " ")) != NULL) {
		  w = atof(token);
		}
	      }
	    }
	  if (aaa=='N') {
	    pr->talairachoff=1;
	    if (pr->verbose>=1) fprintf(stderr,"attribute 'ca' = N.\n");
	  } else {
	    ca[0*pr->files+ifile]  = u;
	    ca[1*pr->files+ifile]  = v;
	    ca[2*pr->files+ifile]  = w;
	    if (pr->verbose>=1) fprintf(stderr,"got 'ca' for talairach transform... (%f,%f,%f)\n",u,v,w);
	  }
	}
	else {
	  if (pr->verbose>=1) fprintf(stderr,"attribute 'ca' missing.\n");
	  pr->talairachoff=1;
	}

	// Get CP for Talairach transform
	cp[0*pr->files+ifile] = cp[1*pr->files+ifile] = cp[2*pr->files+ifile] = 0.0;

	if (VGetAttr (VImageAttrList (src[ifile]), "cp", NULL,VStringRepn, (VPointer) & str) == VAttrFound) {
	  char aaa;
	  strncpy(&aaa,str,(size_t)1);
	  if (aaa!='N')
	    if ((token = strtok(str, " ")) != NULL) {
	      u = atof(token);
	      if ((token = strtok(NULL, " ")) != NULL) {
		v = atof(token);
		if ((token = strtok(NULL, " ")) != NULL) {
		  w = atof(token);
		}
	      }
	    }
	  if (aaa=='N') {
	    pr->talairachoff=1;
	    if (pr->verbose>=1) fprintf(stderr,"attribute 'cp' = N.\n");
	  } else {
	    cp[0*pr->files+ifile]  = u;
	    cp[1*pr->files+ifile]  = v;
	    cp[2*pr->files+ifile]  = w;
	    if (pr->verbose>=1) fprintf(stderr,"got 'cp' for talairach transform... (%f,%f,%f)\n",u,v,w);
	  }
	}
	else {
	  if (pr->verbose>=1) fprintf(stderr,"attribute 'cp' missing.\n");
	  pr->talairachoff=1;
	}


	// get extent for linear scaling
	extent[0*pr->files+ifile] = extent[1*pr->files+ifile] = extent[2*pr->files+ifile] = 0.0;

	if (VGetAttr (VImageAttrList (src[ifile]), "extent", NULL,VStringRepn, (VPointer) & str1) == VAttrFound) {
	  char aaa;
	  strncpy(&aaa,str1,(size_t)1);
	  if (aaa!='N')
	    if ((token = strtok(str1, " ")) != NULL) {
	      u = atof(token);
	      if ((token = strtok(NULL, " ")) != NULL) {
		v = atof(token);
		if ((token = strtok(NULL, " ")) != NULL) {
		  w = atof(token);
		}
	      }
	    }	  
	  if (aaa=='N') {
	    pr->talairachoff=1;
	    if (pr->verbose>=1) fprintf(stderr,"attribute 'extent' = N.\nStarting without talairach coordinates!\n");
	  } else {
	    extent[0*pr->files+ifile] = u;
	    extent[1*pr->files+ifile] = v;
	    extent[2*pr->files+ifile] = w;
	    if (pr->verbose>=1) fprintf(stderr,"got 'extent' for talairach transform...\n");
	  }
	} else {
	  if (pr->verbose>=1) fprintf(stderr,"attribute 'extent' missing.\nStarting without talairach coordinates!\n");
	  pr->talairachoff=1;
	}

	// talairach ************************************************
	if (VGetAttr (VImageAttrList (src[ifile]), "talairach", NULL,
		      VStringRepn, (VPointer) & str2) == VAttrFound) {
	  if (strcmp(str2,"atlas") == 0) pr->atlas = 1;
	  else pr->talairachoff=1;
	} else pr->talairachoff=1;
	
	// voxel *****************************************************
	if (VGetAttr (VImageAttrList (src[ifile]), "voxel", NULL,
		      VStringRepn, (VPointer) & str1) == VAttrFound) {
	  newstr = VNewString(str1);
	  pr->ncols_mult  = atof(strtok(newstr," "));
	  pr->nrows_mult  = atof(strtok(NULL," "));
	  pr->nbands_mult = atof(strtok(NULL," "));
	  if (VImageNFrames(src[ifile])==1) {
	    pr->nbands_mult=1.0;
	    if (pr->verbose>=1) fprintf(stderr,"[voxel] set to 1\n");
	  }
	} else {
	  pr->ncols_mult  = 1.0;
	  pr->nrows_mult  = 1.0;
	  pr->nbands_mult = 1.0;
	}
	
	// fixpoint *************************************************
	if (VGetAttr (VImageAttrList (src[ifile]), "fixpoint", NULL,
		      VStringRepn, (VPointer) & str) == VAttrFound) {
	  u = atof(strtok(str," "));
	  v = atof(strtok(NULL," "));
	  w = atof(strtok(NULL," "));
	  fixpoint[0*pr->files+ifile] = u*pr->ncols_mult;
	  fixpoint[1*pr->files+ifile] = v*pr->nrows_mult;
	  fixpoint[2*pr->files+ifile] = w*pr->nbands_mult;
	  if (pr->verbose>=1) fprintf(stderr,"got 'fixpoint' for anatomie...\n");
	} else {
	  fixpoint[0*pr->files+ifile] = 80;
	  fixpoint[1*pr->files+ifile] = 95;
	  fixpoint[2*pr->files+ifile] = 90;
	  if (pr->verbose>=1) fprintf(stderr,"'fixpoint' not found using std fixpoint %d for anatomie 80 95 90...\n",ifile);
	}

	pr->pixelmult[0]=pr->ncols_mult;
	pr->pixelmult[1]=pr->nrows_mult;
	pr->pixelmult[2]=pr->nbands_mult;
	pr->voxel[0]=pr->ncols_mult;
	pr->voxel[1]=pr->nrows_mult;
	pr->voxel[2]=pr->nbands_mult;
	scalec[ifile] = 1.0;
	scaler[ifile] = 1.0;
	scaleb[ifile] = 1.0;       
	sw=1;
      } // end if UByte

      // **********************************************************
      // ************** ZMAP und anat in 1 Datei ******************
      // **********************************************************
      if (VPixelRepn(tmp[ifile])==VFloatRepn && !zmap_found && counter_zmap==ifile) {
	counter_zmap++;
	fnc[ifile]=VCopyImage(tmp[ifile], NULL, VAllBands);
	//VFree(tmp[ifile]);

	if (pr->verbose>=1) fprintf(stderr,"zmap found...\n");
	pr->sw2=1;
	/*
	  if (VGetAttr (VImageAttrList (fnc[ifile]), "threshold", NULL,
	  VStringRepn, (VPointer) & str2) == VAttrFound) {
	  newstr = VNewString(str2);
	  pr->thresh = atof(strtok(newstr," "));
	  if (pr->verbose>=1) fprintf(stderr,"threshold value %f found...\n",pr->thresh);
	  } else pr->thresh=0;
       
	  pr->thresh=0;
	*/

	if (VGetAttr (VImageAttrList (fnc[ifile]), "voxel", NULL,VStringRepn, (VPointer) & str) == VAttrFound) {
	  scalec[ifile] = atof(strtok(str," "));
	  scaler[ifile] = atof(strtok(NULL," "));
	  scaleb[ifile] = atof(strtok(NULL," "));
	} else {
	  if (pr->verbose>=1) fprintf(stderr,"attribute 'voxel' missing in input file! Setting to 1 1 1.\n");
	  scalec[ifile] = 1.0;
	  scaler[ifile] = 1.0;
	  scaleb[ifile] = 1.0;
	}

	pr->pixelm2[0]=scalec[ifile];
	pr->pixelm2[1]=scaler[ifile];
	pr->pixelm2[2]=scaleb[ifile];
	nrows  = VImageNRows (fnc[ifile]);
	ncols  = VImageNColumns (fnc[ifile]);
	nbands = VImageNFrames (fnc[ifile]);
	// if (nbands==0) nbands=1;
	npixels = nbands * ncols * nrows;

	//extent der zmap fuer Kontrolle
	if (pr->talairachoff==0) {
	  if (VGetAttr (VImageAttrList (fnc[ifile]), "extent", NULL,
			VStringRepn, (VPointer) & str1) == VAttrFound) {
	    char aaa;
	    strncpy(&aaa,str1,(size_t)1);	    
	    if (aaa!='N')
	      if ((token = strtok(str1, " ")) != NULL) {
		u = atof(token);
		if ((token = strtok(NULL, " ")) != NULL) {
		  v = atof(token);
		  if ((token = strtok(NULL, " ")) != NULL) {
		    w = atof(token);
		  }
		}
	      }	  
	    if (aaa=='N') pr->extent_match=0;
	    else {
	      if (u != extent[0*pr->files+ifile] ||
		  v != extent[1*pr->files+ifile] ||
		  w != extent[2*pr->files+ifile]) {
		pr->extent_match=0;
	      }
	    }
	  } else pr->extent_match=0;
	  if (pr->extent_match==0 && pr->sw2==1) pr->talairachoff=1;
	}	  

	/* read the FIXPOINT to move all images upon the other */
	if (VGetAttr (VImageAttrList (fnc[ifile]), "fixpoint", NULL,VStringRepn, (VPointer) & str) == VAttrFound) {
	  u = atof(strtok(str," "));
	  v = atof(strtok(NULL," "));
	  w = atof(strtok(NULL," "));
	  fixpoint[3*pr->files+ifile] = u*scalec[ifile];
	  fixpoint[4*pr->files+ifile] = v*scaler[ifile];
	  fixpoint[5*pr->files+ifile] = w*scaleb[ifile];
	  if (pr->verbose>=1) fprintf(stderr,"got 'fixpoint' for zmap...\n");
	} else {
	  fixpoint[3*pr->files+ifile] = 80;
	  fixpoint[4*pr->files+ifile] = 95;
	  fixpoint[5*pr->files+ifile] = 90;
	  if (pr->verbose>=1) fprintf(stderr,"'fixpoint' not found using std fixpoint %d for zmap 80 95 90...\n",counter_zmap);
	}

	if (nbands==1) scaleb[ifile]=1;
	nbands = (int)(nbands * scaleb[ifile]);
	nrows  = (int)(nrows * scaler[ifile]);
	ncols  = (int)(ncols * scalec[ifile]);

       	// get radiometic maximum
	pr=myTools.GetRadiometricMax(fnc[ifile],pr,npixels);
      } // end if float

    } // end loop throuh the objects
  }  // end for each input file 

  // Error message
  if (pr->g_number>1) { 
    if (graph_found) pr->infilenum=pr->files;
    else pr->infilenum=1; /* by A. Hagert - exception for 2+ graphs */
  }
  if (counter_anatomical != pr->infilenum) VError("file without anatomical data specified by -in");
  if (counter_predisp>0 && counter_predisp != counter_anatomical)
    VError("files specified by -in don't have the same structure (predisp)");
  if (counter_zmap>0 && counter_zmap != counter_anatomical)
    VError("files specified by -in don't have the same structure (zmap)");

  // **********************************************************
  // ************** ZMAP in extra Dateien *********************
  // **********************************************************
  int zzfile=0;

  if (sw==1 && zmap_found) {
    // For each input file: //
    for (int zfile = 0; zfile < pr->zmapfilenum; zfile++) {
      zmap_filename = ((VString/*Const*/ *) zmap_files.vector)[zfile];
      // Read its contents: //
      if (strcmp (zmap_filename, "-") == 0)
	f = stdin;
      else {
	f = fopen (zmap_filename, "r");
	if (! f)
	  VError ("Failed to open zmap file %s", zmap_filename);
      }
      if (! (zmap_list = VReadFile (f, NULL)))
	exit (EXIT_FAILURE);
      fclose (f);

      if (!zmap_list) exit(1); 

      for (VFirstAttr (zmap_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {

	if (VGetAttrRepn (& posn) != VImageRepn || zfile != zzfile) continue;
	VGetAttrValue (& posn, NULL, VImageRepn, & tmp[zzfile]);

	if (VPixelRepn(tmp[zzfile])==VFloatRepn) {
	  fnc[zzfile]=VCopyImage(tmp[zzfile], NULL, VAllBands);
	  //VFree(tmp[zzfile]);
	  if (pr->verbose>=1) fprintf(stderr,"zmap %d found...\n",zzfile);
	  pr->sw2=1;
	  /*
	    if (VGetAttr (VImageAttrList (fnc[ifile]), "threshold", NULL,
	    VStringRepn, (VPointer) & str2) == VAttrFound) {
	    newstr = VNewString(str2);
	    pr->thresh = atof(strtok(newstr," "));
	    if (pr->verbose>=1) fprintf(stderr,"threshold value %f found...\n",pr->thresh);
	    } else pr->thresh=0;
	  
	    pr->thresh=0;
	  */

	  if (VGetAttr (VImageAttrList (fnc[zzfile]), "voxel", NULL, VStringRepn, (VPointer) & str) == VAttrFound) {
	    scalec[zzfile] = atof(strtok(str," "));
	    scaler[zzfile] = atof(strtok(NULL," "));
	    scaleb[zzfile] = atof(strtok(NULL," "));
	  } else {
	    if (pr->verbose>=1) fprintf(stderr,"attribute 'voxel' missing in input file! Setting to 1 1 1.\n");
	    scalec[zzfile] = 1.0;
	    scaler[zzfile] = 1.0;
	    scaleb[zzfile] = 1.0;
	  }

	  // Check if the dimensions of Z-maps coincide
	  nrows   = VImageNRows (fnc[zzfile]);    
	  ncols   = VImageNColumns (fnc[zzfile]); 
	  nbands  = VImageNFrames (fnc[zzfile]); 
	  npixels = nbands * ncols * nrows;
	  if (zzfile==0) {
	    nrows_z=nrows;
	    ncols_z=ncols;
	    nbands_z=nbands;
	  } else {
	    if (nrows!=nrows_z || ncols!=ncols_z || nbands!=nbands_z) 
	      VError("Dimensions of zmaps must agree");
	  }

	  // Check if the dimension of scaled Maps coincide
	  if (nbands==1) scaleb[ifile]=1;
	  nbands = (int)(nbands * scaleb[zzfile]);
	  nrows  = (int)(nrows * scaler[zzfile]);
	  ncols  = (int)(ncols * scalec[zzfile]);
	  if (zzfile==0) {
	    nrows_sc=nrows;
	    ncols_sc=ncols;
	    nbands_sc=nbands;
	  } else {
	    if (nrows!=nrows_sc || ncols!=ncols_sc || nbands!=nbands_sc) 
	      VError("Dimensions of scaled zmaps must agree. Something wrong with voxel attribute");
	  }


	  pr->pixelm2[0]=scalec[zzfile];
	  pr->pixelm2[1]=scaler[zzfile];
	  pr->pixelm2[2]=scaleb[zzfile];

	  //extent der zmap fuer Kontrolle
	  if (pr->talairachoff==0) {
	    if (VGetAttr (VImageAttrList (fnc[zzfile]), "extent", NULL,VStringRepn, (VPointer) & str1) == VAttrFound) {
	      char aaa;
	      strncpy(&aaa,str1,(size_t)1);	    
	      if (aaa!='N')
		if ((token = strtok(str1, " ")) != NULL) {
		  u = atof(token);
		  if ((token = strtok(NULL, " ")) != NULL) {
		    v = atof(token);
		    if ((token = strtok(NULL, " ")) != NULL) {
		      w = atof(token);
		    }
		  }
		}	  
	      if (aaa=='N') pr->extent_match=0;
	      else {
		int zzfile1=0;
		if (counter_anatomical > 1) zzfile1=zzfile;
		if (u != extent[0*pr->files+zzfile1] ||
		    v != extent[1*pr->files+zzfile1] ||
		    w != extent[2*pr->files+zzfile1]) {
		  pr->extent_match=0;
		}
	      }
	    } else pr->extent_match=0;
	    if (pr->extent_match==0 && pr->sw2==1) pr->talairachoff=1;
	  }   
	  
	  // EINLESEN DES FIXPOINTS UM WEITERE BILDER VERSCHIEBEN ZU KOENNEN
	  if (VGetAttr (VImageAttrList (fnc[zzfile]), "fixpoint", NULL,VStringRepn, (VPointer) & str) == VAttrFound) {
	    u = atof(strtok(str," "));
	    v = atof(strtok(NULL," "));
	    w = atof(strtok(NULL," "));

	    fixpoint[3*pr->files+zzfile] = u*scalec[zzfile];
	    fixpoint[4*pr->files+zzfile] = v*scaler[zzfile];
	    fixpoint[5*pr->files+zzfile] = w*scaleb[zzfile];
	    if (pr->verbose>=1) fprintf(stderr,"got 'fixpoint' for zmap...\n");
	  } else {
	    fixpoint[3*pr->zmapfilenum+zzfile] = 80;
	    fixpoint[4*pr->zmapfilenum+zzfile] = 95;
	    fixpoint[5*pr->zmapfilenum+zzfile] = 90;
	    if (pr->verbose>=1) fprintf(stderr,"'fixpoint' not found using std fixpoint %d for zmap 80 95 90...\n",zzfile);
	  }

	  // get radiometic maximum
	  pr=myTools.GetRadiometricMax(fnc[zzfile],pr,npixels);
	  zzfile++;
	}
      }
    } // end for zmapfile
    if (counter_anatomical >1) {
      if (zzfile != counter_anatomical)
	VError("number of anatomical images and zmaps must agree");
    } else {
      if (zzfile>1) {
	if (pr->verbose>=1)  fprintf(stderr,"Setting data for all zmaps using 1 specified anatomical file!!!\n");
	for (int zfile = 1; zfile < zzfile; zfile++) {
	  src[zfile]=src[0];
	  ca[0*pr->files+zfile]=ca[0*pr->files];
	  ca[1*pr->files+zfile]=ca[1*pr->files];
	  ca[2*pr->files+zfile]=ca[2*pr->files];	  
	  cp[0*pr->files+zfile]=cp[0*pr->files];
	  cp[1*pr->files+zfile]=cp[1*pr->files];
	  cp[2*pr->files+zfile]=cp[2*pr->files];
	  extent[0*pr->files+zfile]=extent[0*pr->files];
	  extent[1*pr->files+zfile]=extent[1*pr->files];
	  extent[2*pr->files+zfile]=extent[2*pr->files];  
	}
      }
    }
  }

  // initialize cursor position
  if (ca[0*pr->files]>1 && ca[1*pr->files]>1 && ca[2*pr->files]>1) {
    pr->cursorp[0]=(int)rint(ca[0*pr->files]);
    pr->cursorp[1]=(int)rint(ca[1*pr->files]);
    pr->cursorp[2]=(int)rint(ca[2*pr->files]);
  } else {
    pr->cursorp[0]=(int)VImageNColumns(src[0])/2;
    pr->cursorp[1]=(int)VImageNRows(src[0])/2;
    pr->cursorp[2]=(int)VImageNFrames(src[0])/2;
  }
}


/* load (or reload) all input files in case of Correlation */
void lLoad::loadFilesForCorr() 
{  
  VAttrListPosn posn;
  VBoolean sw=0;
  char *token=NULL;
  float u=0, v=0, w=0;
  int funcobjs=0;
  int anabands=0;
  int ts_temp=0;
  int nbands, ncols, nrows;
  FILE *rawfp;

  
  pr->files=1;
  pr->infilenum=1;
  pr->zmapfilenum=0;
  
  /* Read the input file */
  VAttrList raw_list=NULL;
  rawfp = VOpenInputFile (pr->raw, TRUE);
  if (! ReadHeader (rawfp)) VError(" error reading raw data file");
  if (! (raw_list = ReadAttrList (rawfp))) VError(" error reading raw data attribute list");
    
  /* count the number of objects in vista file */
  hist_items=0; nobjects=0;
  for (VFirstAttr (raw_list, & posn); VAttrExists (& posn); VNextAttr (& posn)) {
    if ( strncmp(VGetAttrName(&posn),"history",7) == 0 ) {
      hist_items++;
    } 
    nobjects++;
  }  

  // allocate imageinfo
  tempInfo=(VImageInfo *) VMalloc(sizeof(VImageInfo) * (nobjects-hist_items));
  for (int i=0;i<nobjects-hist_items;i++) VImageInfoIni(&tempInfo[i]);

  /* check raw data file */
  for (int rawobjects=0; rawobjects<nobjects-hist_items; rawobjects++) {
    if (! VGetImageInfo(rawfp,NULL,rawobjects+hist_items,&tempInfo[rawobjects]))
      VError(" error reading imageinfo");
    
    if (tempInfo[rawobjects].repn==VShortRepn) {
      if (firstfuncobj==-1) firstfuncobj=rawobjects;
      funcobjs++;
      pr->sw2=1; 
      pr->thresh=0;
      if (tempInfo[rawobjects].nbands>2 && sw==0) {
	sw=1;
	nrows=tempInfo[rawobjects].nrows;
	ncols=tempInfo[rawobjects].ncolumns;
	ts_temp=tempInfo[rawobjects].nbands;
	// *** FIXPOINT ***
	fixpoint[3] = 80; fixpoint[4] = 95; fixpoint[5] = 90;
	// *** EXTENT ***
	if (strlen(tempInfo[rawobjects].extent)>2) {
	  if ((token = strtok(tempInfo[rawobjects].extent, " ")) != NULL) {
	    u = atof(token);
	    if ((token = strtok(NULL, " ")) != NULL) {
	      v = atof(token);
	      if ((token = strtok(NULL, " ")) != NULL) {
		w = atof(token);
	      }
	    }
	  }
	  if (u != extent[0] || v != extent[1] || w != extent[2]) {
	    pr->extent_match=0;
	    pr->talairachoff=1;
	  }
	  if (pr->verbose>=1) fprintf(stderr,"got raw 'extent'... (%f,%f,%f)\n",u,v,w);
	} else {
	  if (extent[0]>1 || extent[1]>1 || extent[2]>1) pr->extent_match=0;
	  pr->talairachoff=1;
	  if (pr->verbose>=1) fprintf(stderr,"attribute 'extent' not in raw data.\n");
	} 
	// *** VOXEL ***
	if (strlen(tempInfo[rawobjects].voxel)>2) {
	  if ((token = strtok(tempInfo[rawobjects].voxel, " ")) != NULL) {
	    scalec[0] = atof(token);
	    if ((token = strtok(NULL, " ")) != NULL) {
	      scaler[0] = atof(token);
	      if ((token = strtok(NULL, " ")) != NULL) {
		scaleb[0] = atof(token);
	      }
	    }
	  }
	  if (pr->verbose>=1) fprintf(stderr,"got raw 'voxel' ... (%f,%f,%f)\n",scalec[0],scaler[0],scaleb[0]);
	} else {
	  scalec[0] = pr->ncols_mult;
	  scaler[0] = pr->nrows_mult;
	  scaleb[0] = pr->nbands_mult;
	  if (pr->verbose>=1) fprintf(stderr,"raw 'voxel' missing\n");
	} 
	pr->pixelm2[0]=scalec[0];
	pr->pixelm2[1]=scaler[0];
	pr->pixelm2[2]=scaleb[0];		
      }
    }
  }

  // Zmap ist da :-)
  nbands=funcobjs;
  if (fnc[0]!=NULL) VDestroyImage(fnc[0]);
  fnc[0]=VCreateImage(nbands,nrows,ncols,VFloatRepn);
  if (rawobjektbild!=NULL) VDestroyImage(rawobjektbild);
  rawobjektbild=VCreateImage(ts_temp,nrows,ncols,VShortRepn);
  float *dest_pp; 
  dest_pp = (float *)VPixelPtr(fnc[0],0,0,0);
  memset(dest_pp,0,nbands*nrows*ncols * VPixelSize(fnc[0]));
  pr->nba=nbands;
  pr->nro=nrows;
  pr->nco=ncols;
  fclose(rawfp);
 
  //new bands, rows, and cols
  if (nbands==1) scaleb[0]=1;
  nbands = (int)(nbands * scaleb[0]);
  nrows  = (int)(nrows  * scaler[0]);
  ncols  = (int)(ncols  * scalec[0]);
  pr->pmax =  100.0;
  pr->nmax =  100.0;

}

