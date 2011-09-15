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
    : m_NT(1),
    m_DeformationField( DeformationFieldType::New())
{
	
}

bool TransformMerger3D::merge(void ) const  {
	
    if ( size() < 2 ) {
	return false;
    }
    DeformationFieldType::Pointer tmpDefField = DeformationFieldType::New();
    BOOST_FOREACH( TransformMerger3D::const_reference fieldRef, *this)
    {
	
    }
}


DeformationFieldType::Pointer TransformMerger3D::getTransform(void ) const
{
	return m_DeformationField;
}


}//end namespace extitk
}//end namespace isis

#endif //ISISTRANSFORMMERGER3D_H_
