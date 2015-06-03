//============================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//
//  Copyright 2014 Sandia Corporation.
//  Copyright 2014 UT-Battelle, LLC.
//  Copyright 2014 Los Alamos National Security.
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#ifndef vtk_m_exec_AtomicArray_h
#define vtk_m_exec_AtomicArray_h

#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/exec/ExecutionObjectBase.h>

namespace vtkm {
namespace exec {

/// A class that can be used to atomically operate on an array of values
/// safely across multiple instances of the same worklet. This is useful when
/// you have an algorithm that needs to accumulate values in parallel, but writing
/// out a value per worklet might be memory prohibitive.
///
/// To construct an AtomicArray you will need to pass in an vtkm::cont::ArrayHandle
/// that is used as the underlying storage for the AtomicArray
///
template<typename T, typename DeviceAdapterTag>
class AtomicArray : public vtkm::exec::ExecutionObjectBase
{
public:
  template<typename StorageType>
  VTKM_CONT_EXPORT
  AtomicArray(vtkm::cont::ArrayHandle<T, StorageType> handle):
    AtomicImplementation( handle )
  {
  }

  VTKM_EXEC_EXPORT
  T Add(vtkm::Id index, const T& value) const
  {
    return this->AtomicImplementation.Add(index,value);
  }

  VTKM_EXEC_EXPORT
  T Exchange(vtkm::Id index, const T& value) const
  {
    return this->AtomicImplementation.Exchange(index,value);
  }

private:
    vtkm::cont::DeviceAdapterAtomicArrayImplementation<T,DeviceAdapterTag>
      AtomicImplementation;
};
}
} // namespace vtkm::exec

#endif //vtk_m_exec_AtomicArray_h


