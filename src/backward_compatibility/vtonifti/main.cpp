/****************************************************************
 *
 * Program: vtonifti
 *
 * Copyright (C) Max Planck Institute 
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Authors: Enrico Reimer, Karsten Mueller, 2001, <lipsia@cbs.mpg.de>
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
 * $Id$
 *
 *****************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <Vlib.h>
#include <VImage.h>
#include <mu.h>
#include <option.h>

#include "cvimage.h"
#include "cniftifile.h"

#include <list>
#include <iostream>
#include <sstream>

extern "C" {
   extern char * getLipsiaVersion();
}

typedef std::list<CVImage> ImageList;

template<class T> std::string toString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}


int main(int argc, char *argv[])
{
	/* Command line options: */
	static VString nifti = NULL;
	static VBoolean flipx = TRUE;
	static VBoolean flipy = FALSE;
	static VBoolean flipz = FALSE;
	static VOptionDescRec  options[] = {
		{"out",VStringRepn, 1, &nifti, VRequiredOpt, 0,"Output stub"},
		{"xflip",VBooleanRepn,1,(VPointer) &flipx, VOptionalOpt,NULL,"Flip x axis"},
		{"yflip",VBooleanRepn,1,(VPointer) &flipy, VOptionalOpt,NULL,"Flip y axis"},
		{"zflip",VBooleanRepn,1,(VPointer) &flipz, VOptionalOpt,NULL,"Flip z axis"},
	};
	FILE *in_file;
	VAttrList in_list;
	VAttrListPosn posn;

        char prg_name[50];	
        sprintf(prg_name,"vtonifti V%s", getLipsiaVersion());
        fprintf (stderr, "%s\n", prg_name);
	VWarning("It is highly recommended to use vvinidi for data conversion from vista to nifti!!! This program should only be used if you are too inert to modifiy your old scripts. Furthermore, there will be no support for this program anymore!!");

	/* Parse command line arguments and identify files: */
	VParseFilterCmd (VNumber (options), options, argc, argv,&in_file,/* &out_file */ NULL);

	/* Read the input file */
	in_list = VReadFile (in_file, NULL);
	if (!in_list) exit(1);

	ImageList images;

	for (VFirstAttr (in_list, & posn); VAttrExists (& posn);)
		if (VGetAttrRepn (& posn) == VImageRepn)
			images.push_back(CVImage(posn));
		else
			VNextAttr (& posn);

	std::cout << images.size() << " datasets read" << std::endl;

	unsigned short img_cnt=0;
	unsigned short volset,volsets=0;
	VBoolean orientationproblem=FALSE;
	for(ImageList::iterator it=images.begin();it!=images.end();it++)
	{
		CNiftiFile(file);
		file.nx=it->getSize(CVImage::read);
		file.ny=it->getSize(CVImage::phase);
		file.nz=it->getSize(CVImage::slice);
		file.nt=it->getSize(CVImage::time);
		file.ndim= file.nt<=1 ? 3:4;

		file.myDealer.swap[0]=true;
		file.myDealer.swap[1]=true;
		file.myDealer.swap[2]=true;

		file.myDealer.mirror[0]=flipx;
		file.myDealer.mirror[1]=flipy;
		file.myDealer.mirror[2]=flipz;

		if (it->getStringAttrib("talairach") && flipz) orientationproblem=TRUE;
		if (file.nz<128 && !it->getStringAttrib("talairach") && !flipz) orientationproblem=TRUE;

		float repetition_time=it->getFloatAttrib("repetition_time");
		if (!repetition_time && it->getStringAttrib("MPIL_vista_0")){
		    const char *mpilvista0=it->getStringAttrib("MPIL_vista_0");
		    sscanf(mpilvista0," repetition_time=%f",&repetition_time);
		}
		if (file.nt>1 && repetition_time<=1)
		{
		  VWarning(" no repetition time (TR) found in functional data defaulting to 1")  ;
			repetition_time=1;
		}
		file.setTimeRes(repetition_time);
		file.setVoxelSize(
				it->getVoxelSize(CVImage::read),
				it->getVoxelSize(CVImage::phase),
				it->getVoxelSize(CVImage::slice)
		);

		float mat[4][4];
		it->getTransformMat(mat);

		file.setupForm(CNiftiFile::scanner_anatomy,mat);
		it->store_data(file.myDealer);
		file.write(nifti+toString(img_cnt++)+".nii",it->getStringAttrib("name"));
	}

	if (orientationproblem)
	  VWarning("There might be a problem with the image orientation in the z axis. In Lipsia, 2D slices are ordered upside down, but not in 3D images. Please check the image orientation of all output images. Be sure to use the option '-zflip' in a correct way");

	fclose(in_file);
	return EXIT_SUCCESS;
}
