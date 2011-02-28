//
// C++ Implementation: cniftifile
//
// Description:
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "cniftifile.h"
#include <iostream>
#include <limits>
#include <algorithm>
#include <assert.h>
#include <limits>

CNiftiFile::CNiftiFile():myDealer(*this)
{
	memset(this,0, sizeof(nifti_image));
	nifti_type=1;
	scl_slope=1;
	freq_dim=1;
	phase_dim=2;
	slice_dim=3;
	slice_code=0;
	nt=nu=nv=nw=1;
	cal_max=std::numeric_limits<float>::min();
	cal_min=std::numeric_limits<float>::max();

	xyz_units=NIFTI_UNITS_MM;
}

CNiftiFile::DataDealer::DataDealer(CNiftiFile &client_):CDataDealer<CNiftiFile>(client_){
	std::fill_n(swap,4,false);
	std::fill_n(mirror,4,false);
}
size_t CNiftiFile::DataDealer::sizePrognosis(){
	return client.nx*client.ny*client.nz*client.nt;
}

void CNiftiFile::setupForm(FormID formID, const float matrix[4][4])
{
	assert(ndim>1);
        qform_code=0;
	sform_code=0;

	//create space tranformation matrix - transforms the space when reading _NOT_ the data
	for(int y =0;y<4;y++)
		for(int x=0;x<4;x++)
		{
			qto_xyz.m[y][x]=matrix[y][x] * (myDealer.swap[y]?1:-1); //no voxelsize for qto_xyz - is done by pixdim directly
			sto_xyz.m[y][x]=matrix[y][x] * pixdim[y+1] * (myDealer.swap[y]?1:-1);
		}

	//generate matching quaternions
	nifti_mat44_to_quatern(
			qto_xyz,
			&quatern_b,&quatern_c,&quatern_d,
			&qoffset_x,&qoffset_y,&qoffset_z,
			NULL,NULL,NULL,
			&qfac);

#ifndef __OPTIMIZE__
	int icod=0, jcod=0, kcod=0;
	nifti_mat44_to_orientation(sto_xyz,&icod,&jcod,&kcod);
	fprintf(stderr,"icod %d   jcod %d   kcod %d\n",icod, jcod, kcod);
	nifti_mat44_to_orientation(qto_xyz,&icod,&jcod,&kcod);
	fprintf(stderr,"icod %d   jcod %d   kcod %d qfac %f\n",icod, jcod, kcod,qfac);
#endif
}

/*!
    \fn CNiftiFile::write(char *filename)
 */
bool CNiftiFile::write(std::string filename,const char *descr)
{

	fname=(char*)filename.c_str();
	iname=(char*)filename.c_str();
	snprintf(descrip, 79, "%s", descr ? descr:"lipsia");
	if(!data)
	{
		data=myDealer.getBuffer();
		assert(data);
	}
	nifti_image_write(this);
	fname=iname=NULL; //remove pointers to temporary c-strings

	std::cout << filename << " written" << std::endl;
	return true;
}



void CNiftiFile::setVoxelSize(float x,float y, float z)
{
	pixdim[1]=dx=x;
	pixdim[2]=dy=y;
	pixdim[3]=dz=z;
}

void CNiftiFile::setTimeRes(float mseconds)
{
	time_units=NIFTI_UNITS_MSEC;
	pixdim[4]=dt=mseconds;
}

void CNiftiFile::setRepititions(unsigned short cnt)
{
	nt=cnt ? cnt:1;
}

template<> short CNiftiFile::type2id(signed char)     {return NIFTI_TYPE_INT8;}
template<> short CNiftiFile::type2id(signed short)    {return NIFTI_TYPE_INT16;}
template<> short CNiftiFile::type2id(signed int)      {return NIFTI_TYPE_INT32;}
template<> short CNiftiFile::type2id(signed long long){return NIFTI_TYPE_INT64;}

template<> short CNiftiFile::type2id(unsigned char)     {return NIFTI_TYPE_UINT8;}
template<> short CNiftiFile::type2id(unsigned short)    {return NIFTI_TYPE_UINT16;}
template<> short CNiftiFile::type2id(unsigned int)      {return NIFTI_TYPE_UINT32;}
template<> short CNiftiFile::type2id(unsigned long long){return NIFTI_TYPE_UINT64;}

template<> short CNiftiFile::type2id(float)      {return NIFTI_TYPE_FLOAT32;}
template<> short CNiftiFile::type2id(double)     {return NIFTI_TYPE_FLOAT64;}
template<> short CNiftiFile::type2id(long double){return NIFTI_TYPE_FLOAT128;}

template<> short CNiftiFile::type2id(long)	{return sizeof(long)==8 ? NIFTI_TYPE_INT64 : NIFTI_TYPE_INT32;}