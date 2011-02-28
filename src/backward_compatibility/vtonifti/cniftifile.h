//
// C++ Interface: cniftifile
//
// Description:
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CNIFTIFILE_H
#define CNIFTIFILE_H


#include <nifti1_io.h>
#include <string>
#include <assert.h>

#include "cdatadealer.h"

/**
	@author
*/
class CNiftiFile : public nifti_image{
public:
	enum FormID{
		unknown=NIFTI_XFORM_UNKNOWN,
		scanner_anatomy=NIFTI_XFORM_SCANNER_ANAT,
		aligned_anat=NIFTI_XFORM_ALIGNED_ANAT,
		talairach=NIFTI_XFORM_TALAIRACH,
		mni=NIFTI_XFORM_MNI_152
	};
	class DataDealer:public CDataDealer<CNiftiFile>
	{
	public:
		DataDealer(CNiftiFile &client_);
		template<class T> void updateMetaInf(T){
			client.nvox=sizePrognosis();
			client.datatype=type2id(T());
			client.nbyper=sizeof(T);
		}
		template <class T> void put(T* src,size_t cnt)
		{
			client.data = src;
			for(size_t i=0;i<cnt;i++)
			{
				if(client.cal_max<src[i])client.cal_max=src[i];
				if(client.cal_min>src[i])client.cal_min=src[i];
			}
			updateMetaInf(T());
		}
		inline size_t getIndex(const unsigned short &x,const unsigned short &y,const unsigned short &z,const unsigned short &t)const{
			assert(client.nx>x);
			assert(client.ny>y);
			assert(client.nz>z);
			assert(client.nt>t);
			const unsigned short _x=(swap[0]^mirror[0]) ? x:client.nx-x-1;
			const unsigned short _y=(swap[1]^mirror[1]) ? client.ny-y-1:y;
			const unsigned short _z=(swap[2]^mirror[2]) ? client.nz-z-1:z;
			const unsigned short _t=(swap[3]^mirror[3]) ? client.nt-t-1:t;
			const size_t lineSize=client.nx;
			const size_t sliceSize=lineSize*client.ny;
			const size_t brickSize=sliceSize*client.nz;
			return
					_x+
					_y*lineSize+
					_z*sliceSize+
					_t*brickSize;
		}
		template<class T> void putAt(T data,size_t index)
		{
			((T*)data_buffer)[index]=data;
			if(client.cal_max<data)client.cal_max=data;
			if(client.cal_min>data)client.cal_min=data;
		}
		size_t sizePrognosis();
	}myDealer;
	
	CNiftiFile();
	void setupForm(FormID formID, const float matrix[4][4]);
	template<class T> static short type2id(T);
	bool write(std::string filename,const char *descr=0);
	void setVoxelSize(float x,float y, float z);
	void setTimeRes(float mseconds);
	void setRepititions(unsigned short cnt);
};


#endif
