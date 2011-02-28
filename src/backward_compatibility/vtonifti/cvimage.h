//
// C++ Interface: cvimage
//
// Description:
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef CVIMAGE_H
#define CVIMAGE_H

#include <Vlib.h>
#include <VImage.h>
#include <mu.h>

#include <algorithm>
#include <map>
#include <list>
#include <assert.h>

#include "cdatadealer.h"


class CVImage{
public:
	enum Direction{read=0,phase,slice,time};
private:
	float res[3];
	static std::map<VImage,unsigned short> refCnt;
	template <class R,class T> void store(T &dealer)
	{
		const size_t size[4]={getSize(read),getSize(phase),getSize(slice),getSize(time)};
		const size_t len=size[0]*size[1]*size[2]*size[3];
		if(size[3]<2)
		{
			if(dealer.continous())
				dealer.put(&VPixel(images.front(), 0, 0, 0, R),len);
			else
			{
				dealer.checkBuffer(len,R());
				for(unsigned short z=0;z<size[2];z++)
					for(unsigned short y=0;y<size[1];y++)
						for(unsigned short x=0;x<size[0];x++)
							dealer.putAt(VPixel(images.front(), z, y, x, R),dealer.getIndex(x,y,z,0));
				dealer.updateMetaInf(R());
			}
		}
		else
		{
			dealer.checkBuffer(len,R());
			for(unsigned int t=0,z=0;t<size[3];t++,z=0)
				for(std::list<VImage>::iterator it=images.begin();it!=images.end();it++,z++)
					for(unsigned int y=0;y<size[1];y++)
						for(unsigned int x=0;x<size[0];x++)
			{
				const size_t index=dealer.getIndex(x,y,z,t);
				if(isZero(*it))
					dealer.putAt(R(0),index);
				else
					dealer.putAt(VPixel(*it, t, y, x, R),index);
			}
			dealer.updateMetaInf(R());
		}
	}
public:
	std::list<VImage> images;
	CVImage(VAttrListPosn &attrPtr);
	CVImage(const CVImage &src);
	~CVImage();
	unsigned short getSize(Direction)const;
	static unsigned short getSize(Direction,VImage);
	float getVoxelSize(Direction)const;
	static float getVoxelSize(Direction,VImage);
	bool isZero()const;
	static bool isZero(VImage img);

	template<VRepnKind repn,class T> static const T& repn2type(void *ptr);
	void getTransformMat(float M[4][4])const;
	template <class T> void store_data(T &dealer)
	{
		switch(getRepn())
		{
			case VUByteRepn:store<VUByte>(dealer);break;
			case VSByteRepn:store<VSByte>(dealer);break;
			case VShortRepn:store<VShort>(dealer);break;
			case VLongRepn:store<VLong>(dealer);break;
			case VFloatRepn:store<VFloat>(dealer);break;
			case VDoubleRepn:store<VDouble>(dealer);break;
			default:
				VError("Unknown Pixel representation");
		}
	}
	const char* getStringAttrib(const char* name)const;
	const unsigned long getNumberAttrib(const char* name)const;
	const float getFloatAttrib(const char* name)const;
	static const char* getStringAttrib(const char* name,const VImage img);
	static const unsigned long getNumberAttrib(const char* name,const VImage img);
	static const float getFloatAttrib(const char* name,const VImage img);
	void addImage(VImage img);
	bool fit(VImage img);
	VRepnKind getRepn()const;
	static VRepnKind getRepn(VImage img);
};

#endif
