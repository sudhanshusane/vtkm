//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2015 Sandia Corporation.
//  Copyright 2015 UT-Battelle, LLC.
//  Copyright 2015 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#ifndef vtk_m_io_internal_Endian_h
#define vtk_m_io_internal_Endian_h

#include <vtkm/Types.h>

#include <algorithm>
#include <vector>

namespace vtkm {
namespace io {
namespace internal {

inline bool IsLittleEndian()
{
  static const vtkm::Int16 i16 = 0x1;
  const vtkm::Int8 *i8p = reinterpret_cast<const vtkm::Int8*>(&i16);
  return (*i8p == 1);
}

template<typename T>
inline void FlipEndianness(std::vector<T> &buffer)
{
  vtkm::UInt8 *bytes = reinterpret_cast<vtkm::UInt8*>(&buffer[0]);
  const std::size_t tsize = sizeof(T);
  const std::size_t bsize = buffer.size();
  for (std::size_t i = 0; i < bsize; i++, bytes+=tsize)
  {
    std::reverse(bytes, bytes + tsize);
  }
}

}
}
} // vtkm::io::internal

#endif //vtk_m_io_internal_Endian_h
