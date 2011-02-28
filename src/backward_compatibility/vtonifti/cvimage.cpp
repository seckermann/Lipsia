//
// C++ Implementation: cvimage
//
// Description:
//
//
// Author:  <>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "cvimage.h"
#include <assert.h>

CVImage::CVImage(VAttrListPosn &posn){
	VImage image;
	for (; VAttrExists (& posn) && (VGetAttrRepn (& posn) == VImageRepn); VNextAttr (& posn)) {
		const VBoolean result=VGetAttrValue (&posn, NULL, VImageRepn, &image);assert(result);
		if(!images.empty() && !fit(image))break;
		else addImage(image);
	}
}

CVImage::CVImage(const CVImage &src)
{
	*this = src;
	for(std::list<VImage>::iterator it=images.begin();it!=images.end();it++)
	{
		assert(refCnt[*it]>0);
		refCnt[*it]++;
	}
}

CVImage::~CVImage()
{
	for(std::list<VImage>::iterator it=images.begin();it!=images.end();it++)
	{
		assert(refCnt[*it]>0);
		if(refCnt[*it]==1)VDestroyImage(*it);
		else refCnt[*it]--;
	}
}

unsigned short CVImage::getSize(Direction dir)const{
	if(images.empty())return 0;
	VImage img=NULL;
	for(std::list<VImage>::const_iterator it=images.begin();it!=images.end();it++)
		if(!isZero(*it))
		{
			img=*it;break;
		}
	assert(img);
	assert(!isZero(img));
	const size_t s=images.size();

	if(dir==slice)return s>1 ? s:getSize(slice,img);
	else if(dir == time)return s>1 ? getSize(slice,img):s;
	else return getSize(dir,img);
}
unsigned short CVImage::getSize(Direction dir,VImage image)
{
	switch(dir)
	{
	case read:return VImageNColumns(image);break;
	case phase:return VImageNRows(image);break;
	case slice:return VImageNBands(image);break;
	}
}

bool CVImage::isZero(VImage img){return (getSize(read,img) < 2 && getSize(phase,img) < 2);}

float CVImage::getVoxelSize(Direction dir)const{
	return images.empty() ? 0:getVoxelSize(dir,*images.begin());
}
float CVImage::getVoxelSize(Direction dir,VImage img){
	float res[3];
	char* value;
	const char *voxel=getStringAttrib("voxel",img);
	if(!voxel || !sscanf(voxel,"%f %f %f",&res[0],&res[1],&res[2])){
		VWarning("Cannot find voxel attribute. Using \"1 1 1\"");
		std::fill_n(res,3,1);
	}
	return res[dir];
}

std::map<VImage,unsigned short> CVImage::refCnt;


const char* CVImage::getStringAttrib(const char* name)const{
	return images.empty() ? 0:getStringAttrib(name,*images.begin());
}
const char* CVImage::getStringAttrib(const char* name,const VImage image){
	char* value;
	VGetAttrResult vresult=VGetAttr(VImageAttrList(image), name, NULL, VStringRepn, &value);
	if(vresult!=VAttrFound)return NULL;
	else return value;
}

const unsigned long CVImage::getNumberAttrib(const char* name)const{
	return images.empty() ? 0:getNumberAttrib(name,*images.begin());
}
const unsigned long CVImage::getNumberAttrib(const char* name,VImage image){
	long value;
	VGetAttrResult vresult=VGetAttr(VImageAttrList(image), name, NULL, VLongRepn, &value);
	if(vresult!=VAttrFound)return 0;
	else return value;
}

const float CVImage::getFloatAttrib(const char* name)const{
	return images.empty() ? 0:getFloatAttrib(name,*images.begin());
}
const float CVImage::getFloatAttrib(const char* name,VImage image){
	float value;
	VGetAttrResult vresult=VGetAttr(VImageAttrList(image), name, NULL, VFloatRepn, &value);
	if(vresult!=VAttrFound)return 0;
	else return value;
}


void CVImage::addImage(VImage img){
	images.push_back(img);
	refCnt[img]++;
}

bool CVImage::fit(VImage img){
	const std::string myConv(getStringAttrib("convetion")?:"");
	const std::string myVSize(getStringAttrib("voxel")?:"");
	const std::string myMPIL_vista_0(getStringAttrib("MPIL_vista_0")?:"");
	const std::string myOrientation(getStringAttrib("orientation")?:"");
	const VRepnKind myRepn(getRepn());

	const std::string theyConv(getStringAttrib("convetion",img)?:"");
	const std::string theyVSize(getStringAttrib("voxel",img)?:"");
	const std::string theyMPIL_vista_0(getStringAttrib("MPIL_vista_0",img)?:"");
	const std::string theyOrientation(getStringAttrib("orientation",img)?:"");
	const VRepnKind theyRepn(getRepn(img));
	if(myConv==theyConv && myVSize == theyVSize && myMPIL_vista_0 == theyMPIL_vista_0 && myOrientation == theyOrientation && myRepn == theyRepn)
		return true;
	else
		return false;
}

void CVImage::getTransformMat(float M[4][4])const{
        VString xaxis=NULL, yaxis=NULL, zaxis=NULL;
	std::fill_n((float*)M, 4*4, 0);
        float d[3];
	for (int o=0; o<3; o++) M[o][o]=1;
	M[0][0]=-1; 

	/* VImage img=NULL;
	   for(std::list<VImage>::const_iterator it=images.begin();it!=images.end();it++)
	   if(!isZero(*it))
	   {
	   img=*it;break;
	   }
	   if (VGetAttr(VImageAttrList(img),"x-axis",NULL,VStringRepn, &xaxis) == VAttrFound)
	   sscanf(xaxis,"%f %f %f",&M[0][1],&M[1][1],&M[2][1]);
	   if (VGetAttr(VImageAttrList(img),"y-axis",NULL,VStringRepn, &yaxis) == VAttrFound)
	   sscanf(yaxis,"%f %f %f",&M[0][2],&M[1][2],&M[2][2]);       
	   if (VGetAttr(VImageAttrList(img),"z-axis",NULL,VStringRepn, &zaxis) == VAttrFound)
	   sscanf(zaxis,"%f %f %f",&M[0][0],&M[1][0],&M[2][0]);
	   for (int i=0;i<3;i++) M[i][0]=-M[i][0];
	   
	   d[0]=M[0][0]*VImageNColumns(img);
	   d[1]=M[1][0]*VImageNColumns(img);
	   d[2]=M[2][0]*VImageNColumns(img);
	   
	   d[0]+=M[0][1]*VImageNRows(img);
	   d[1]+=M[1][1]*VImageNRows(img);
	   d[2]+=M[2][1]*VImageNRows(img);
	   
	   d[0]+=M[0][2]*VImageNBands(img);
	   d[1]+=M[1][2]*VImageNBands(img);
	   d[2]+=M[2][2]*VImageNBands(img);
	   
	   for (int o=0; o<3; o++) M[o][3]=-d[o]/2;
	*/
}

VRepnKind CVImage::getRepn()const{return images.empty() ? (VRepnKind)0:getRepn(*images.begin());}
VRepnKind CVImage::getRepn(VImage img){return VPixelRepn(img);}
