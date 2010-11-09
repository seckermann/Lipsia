/****************************************************************
 *
 * prefs.C
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Heiko Mentzel, 1999, <lipsia@cbs.mpg.de>
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
 * $Id: prefs.C 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


/* Vista Include Files */
#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include "prefs.h"


prefs * prefsini()
{
  prefs *pr;
  
  pr= new prefs();

  pr->crosscolor.setRgb(255,255,255); pr->lipstyle=1;
  pr->active=0; pr->acoltype=0; pr->atlas=0; pr->baco=0; pr->bolres=0; pr->bildzoom=1; pr->coltype=0;
  pr->cutoutside=0; pr->firesp=1; pr->fitevobool=0;
  pr->files=0; pr->interpol=1; pr->lockz=0; pr->midclick=0; pr->pixelco=1; pr->persi=1;
  pr->radius=10; pr->showcross=1; pr->showradius=0; pr->stdera=1; pr->sw2=0; pr->tiline=1; pr->triallength=20;
  pr->transzmap=0; pr->talairachoff=0; pr->zmapview=1; pr->transzmap=0;
  pr->zeropoint=0; pr->nmax=0; pr->pmax=0; pr->trialresolution=100;  
  pr->pixelmult[0]=1, pr->pixelmult[1]=1, pr->pixelmult[2]=1;
  pr->cursorp[0]=1; pr->cursorp[1]=1; pr->cursorp[2]=1;
  pr->condim=NULL; pr->designim=NULL;
  pr->raw=NULL; pr->beta=NULL; pr->des=NULL; pr->colortable=NULL;
  pr->equidistantColorTable=0;
  for (int i=0; i<5; i++) pr->graph[i]=NULL;
  pr->sr=NULL; pr->fn=NULL; pr->gr=NULL;
  pr->pixelm2[0]=1; pr->pixelm2[1]=1; pr->pixelm2[2]=1;
  pr->infilenum=0; pr->zmapfilenum=0; pr->thresh=0;
  pr->nba=1; pr->nro=1; pr->nco=1;
  pr->ncols_mult=1; pr->nrows_mult=1; pr->nbands_mult=1;

  pr->ogl=0; pr->oglzmapdense=0;
  pr->digits=5; pr->mousemove=0;
  pr->slidefaktor[0]=10.0; pr->slidefaktor[1]=10.0;
  pr->interpoltype=0; //0=NN, 1=bilinear, 2=bicubic
  
  pr->infilenum_graph=0; pr->gcoltype=0; pr->crossize=1; pr->fog=1; 
  pr->glassbrain=0;
  pr->synchronize=1; /* added by A. Hagert */
  pr->exact=0; /* added by A. Hagert */
  pr->openvis=0;

  pr->only_sulci=0; pr->graphtype=0; pr->polygons=0; pr->g_number=0;
  pr->extent_match=1;
  pr->ipc=2;

  pr->prg_name=NULL; pr->hideitems=0; pr->tc_minlength=5;
  pr->tpos=0.0; pr->tneg=0.0;

  pr->anamean=0; pr->anaalpha=1;
  pr->minwert=0; pr->maxwert=255; pr->shift=0; pr->spread=10;
  pr->minwert1=0; pr->maxwert1=255; pr->background0=0; pr->background1=0;

  pr->spheresize=20;

  return pr;
}
