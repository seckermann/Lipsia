/****************************************************************
 *
 * Copyright (C) 2010 Max Planck Institute for Human Cognitive and Brain Sciences, Leipzig
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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
 * Author: Erik Tuerke, tuerke@cbs.mpg.de, 2010
 *
 *****************************************************************/


#include "itkTransformBase.h"
#include "itkVersorRigid3DTransform.h"
#include "itkAffineTransform.h"
#include "itkBSplineDeformableTransform.h"
#include "itkImageRegistrationMethod.h"
#include "itkAddImageFilter.h"
#include "itkImage.h"
#include "itkVectorResampleImageFilter.h"
#include "itkWarpVectorImageFilter.h"

#include <list>

namespace isis
{
namespace extitk
{

	typedef itk::Vector<float, 3> VectorType;
	typedef itk::Image<VectorType, 3> DeformationFieldType;
	typedef itk::Image<float, 3> ImageType;
	
class TransformMerger3D : public std::list<DeformationFieldType::Pointer>
{
public:

	typedef itk::WarpVectorImageFilter<DeformationFieldType, DeformationFieldType, DeformationFieldType> ResampleDeformationImageFilterType;

	typedef itk::AddImageFilter<DeformationFieldType, DeformationFieldType, DeformationFieldType> AddImageFilterType;
	typedef itk::WarpVectorImageFilter<DeformationFieldType, DeformationFieldType, DeformationFieldType> WarpImageFilterType;


	TransformMerger3D();
	bool merge(void );
	DeformationFieldType::Pointer getTransform(void ) const;
	
	void setNumberOfThreads( const size_t &nt ) { m_NT = nt; }
	void setTemplateImage( const ImageType::Pointer image ) { m_TemplateImage = image; }
		

private:
	
	DeformationFieldType::Pointer m_DeformationField;
	ImageType::Pointer m_TemplateImage;
	ResampleDeformationImageFilterType::Pointer m_Resampler;
	AddImageFilterType::Pointer m_AddImageFilter;
	size_t m_NT;
	
	

};

}//end namespace itk
}//end namespace isis
