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


#ifndef ISISTRANSFORMMERGER3D_H_
#define ISISTRANSFORMMERGER3D_H_

#include "isisTransformMerger3D.hpp"
#include "itkImageFileWriter.h"
#include "boost/foreach.hpp"

namespace isis
{
namespace extitk
{

TransformMerger3D::TransformMerger3D()
{
	transformType_ = 0;
	tmpTransform_ = BSplineDeformableTransformType::New();
	outputTransform_ = BSplineDeformableTransformType::New();
	addImageFilter_ = AddImageFilterType::New();
	resampler = ResampleDeformationImageFilterType::New();
}

bool TransformMerger3D::merge(
	void 
){
		typedef std::list< DeformationFieldType::Pointer > ListType;
		if( m_FieldList.size() <= 1 ) {
			return false;
		}

		
		DeformationFieldType::Pointer temporaryDeformationField;
		for( size_t i = 0; i < m_FieldList.size() - 1; i++ ) {
			resampler->SetInput( m_FieldList[i] );
			resampler->SetOutputOrigin( m_Image->GetOrigin() );
			resampler->SetOutputSpacing( m_Image->GetSpacing() );
			resampler->SetSize( m_Image->GetLargestPossibleRegion().GetSize() );
			resampler->SetOutputDirection( m_Image->GetDirection() );
			
			resampler->Update();
			temporaryDeformationField = resampler->GetOutput();
			addImageFilter_->SetInput1( temporaryDeformationField );
			temporaryDeformationField->DisconnectPipeline();
			
			resampler->SetInput( m_FieldList[i+1] );
			resampler->SetOutputOrigin( m_Image->GetOrigin() );
			resampler->SetOutputSpacing( m_Image->GetSpacing() );
			resampler->SetSize( m_Image->GetLargestPossibleRegion().GetSize() );
			resampler->SetOutputDirection( m_Image->GetDirection() );
			resampler->Update();
			temporaryDeformationField = resampler->GetOutput();
			addImageFilter_->SetInput2( temporaryDeformationField );	
			addImageFilter_->Update();
			temporaryDeformationField->DisconnectPipeline();
			
		}
		
		deformationField_ = addImageFilter_->GetOutput();
		return true;
}


TransformMerger3D::DeformationFieldType::Pointer TransformMerger3D::getTransform(
	void )
{
	return deformationField_;
}


}//end namespace extitk
}//end namespace isis

#endif //ISISTRANSFORMMERGER3D_H_
