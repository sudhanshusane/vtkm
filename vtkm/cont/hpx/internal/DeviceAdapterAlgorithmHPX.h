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
//  Copyright 2014. Los Alamos National Security
//
//  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
//  the U.S. Government retains certain rights in this software.
//
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//============================================================================
#ifndef vtk_m_cont_internal_DeviceAdapterAlgorithmHPX_h
#define vtk_m_cont_internal_DeviceAdapterAlgorithmHPX_h

// include HPX headers before vtkm+boost to avoid problems with definitions
#include <vtkm/cont/hpx/internal/DeviceAdapterTagHPX.h>
#include <hpx/parallel/algorithms/for_each.hpp>
#include <hpx/include/parallel_scan.hpp> 
//
#include <vtkm/cont/ArrayHandle.h>
#include <vtkm/cont/ArrayPortalToIterators.h>
#include <vtkm/cont/DeviceAdapterAlgorithm.h>
#include <vtkm/cont/ErrorExecution.h>
#include <vtkm/cont/internal/DeviceAdapterAlgorithmGeneral.h>

#include <vtkm/exec/internal/ErrorMessageBuffer.h>

#include <boost/iterator/counting_iterator.hpp>
#include <boost/utility/enable_if.hpp>


#include <algorithm>
#include <numeric>

namespace vtkm {
namespace cont {

template<>
struct DeviceAdapterAlgorithm<vtkm::cont::DeviceAdapterTagHPX> :
    vtkm::cont::internal::DeviceAdapterAlgorithmGeneral<
        DeviceAdapterAlgorithm<vtkm::cont::DeviceAdapterTagHPX>,
        vtkm::cont::DeviceAdapterTagHPX>
{
private:
  typedef vtkm::cont::DeviceAdapterTagHPX Device;

public:
  template<typename T, class CIn, class COut>
  VTKM_CONT_EXPORT static T ScanInclusive(
      const vtkm::cont::ArrayHandle<T,CIn> &input,
      vtkm::cont::ArrayHandle<T,COut>& output)
  {
    typedef typename vtkm::cont::ArrayHandle<T,COut>
        ::template ExecutionTypes<Device>::Portal PortalOut;
    typedef typename vtkm::cont::ArrayHandle<T,CIn>
        ::template ExecutionTypes<Device>::PortalConst PortalIn;

    vtkm::Id numberOfValues = input.GetNumberOfValues();

    PortalIn inputPortal = input.PrepareForInput(Device());
    PortalOut outputPortal = output.PrepareForOutput(numberOfValues, Device());

    if (numberOfValues <= 0) { return 0; }

    T result =  0;
/*
    std::cout << "\nInput values " ;
    std::copy(
      vtkm::cont::ArrayPortalToIteratorEnd(inputPortal)-10,
      vtkm::cont::ArrayPortalToIteratorEnd(inputPortal),
      std::ostream_iterator<T>(std::cout, ", ")
    );
*/

    hpx::parallel::inclusive_scan(hpx::parallel::par, 
      vtkm::cont::ArrayPortalToIteratorBegin(inputPortal),
      vtkm::cont::ArrayPortalToIteratorEnd(inputPortal),
      vtkm::cont::ArrayPortalToIteratorBegin(outputPortal));

/*
    std::cout << "\nOutput values " ;
    std::copy(
      vtkm::cont::ArrayPortalToIteratorEnd(outputPortal)-10,
      vtkm::cont::ArrayPortalToIteratorEnd(outputPortal),
      std::ostream_iterator<T>(std::cout, ", ")
    );
*/
    result =  outputPortal.Get(numberOfValues - 1);
    return result;
  }

  template<typename T, class CIn, class COut>
  VTKM_CONT_EXPORT static T ScanExclusive(
      const vtkm::cont::ArrayHandle<T,CIn> &input,
      vtkm::cont::ArrayHandle<T,COut>& output)
  {
    typedef typename vtkm::cont::ArrayHandle<T,COut>
        ::template ExecutionTypes<Device>::Portal PortalOut;
    typedef typename vtkm::cont::ArrayHandle<T,CIn>
        ::template ExecutionTypes<Device>::PortalConst PortalIn;

    vtkm::Id numberOfValues = input.GetNumberOfValues();

    PortalIn inputPortal = input.PrepareForInput(Device());
    PortalOut outputPortal = output.PrepareForOutput(numberOfValues, Device());

    if (numberOfValues <= 0) { return 0; }
     
    T result =  0;

    // vtkm::cont::ArrayPortalToIterators<PortalOut>::IteratorType fullValue = 
    hpx::parallel::exclusive_scan(hpx::parallel::par, 
      vtkm::cont::ArrayPortalToIteratorBegin(inputPortal),
      vtkm::cont::ArrayPortalToIteratorEnd(inputPortal),
      vtkm::cont::ArrayPortalToIteratorBegin(outputPortal), T());

    result =  outputPortal.Get(numberOfValues - 1) + inputPortal.Get(numberOfValues - 1);
    return result;
  }

private:
  // This runs in the execution environment.
  template<class FunctorType>
  class ScheduleKernel
  {
  public:
    ScheduleKernel(const FunctorType &functor)
      : Functor(functor) {  }

    //needed for when calling from schedule on a range
    VTKM_EXEC_EXPORT void operator()(vtkm::Id index) const
    {
      this->Functor(index);
    }

  private:
    const FunctorType Functor;
  };

public:
  template<class Functor>
  VTKM_CONT_EXPORT static void Schedule(Functor functor,
                                        vtkm::Id numInstances)
  {
    const vtkm::Id MESSAGE_SIZE = 1024;
    char errorString[MESSAGE_SIZE];
    errorString[0] = '\0';
    vtkm::exec::internal::ErrorMessageBuffer
        errorMessage(errorString, MESSAGE_SIZE);

    functor.SetErrorMessageBuffer(errorMessage);

    DeviceAdapterAlgorithm<Device>::ScheduleKernel<Functor> kernel(functor);

    hpx::parallel::for_each(
          hpx::parallel::par,
          ::boost::counting_iterator<vtkm::Id>(0),
          ::boost::counting_iterator<vtkm::Id>(numInstances),
          kernel);

    if (errorMessage.IsErrorRaised())
    {
      throw vtkm::cont::ErrorExecution(errorString);
    }
  }

  template<class FunctorType>
  VTKM_CONT_EXPORT
  static void Schedule(FunctorType functor, vtkm::Id3 rangeMax)
  {
    DeviceAdapterAlgorithm<Device>::Schedule(functor,
                                     rangeMax[0] * rangeMax[1] * rangeMax[2] );
  }

  template<typename T, class Storage>
  VTKM_CONT_EXPORT static void Sort(vtkm::cont::ArrayHandle<T,Storage>& values)
  {
    typedef typename vtkm::cont::ArrayHandle<T,Storage>
        ::template ExecutionTypes<Device>::Portal PortalType;

    PortalType arrayPortal = values.PrepareForInPlace(Device());
    vtkm::cont::ArrayPortalToIterators<PortalType> iterators(arrayPortal);
    std::sort(iterators.GetBegin(), iterators.GetEnd());
  }

  template<typename T, class Storage, class Compare>
  VTKM_CONT_EXPORT static void Sort(vtkm::cont::ArrayHandle<T,Storage>& values,
                                    Compare comp)
  {
    typedef typename vtkm::cont::ArrayHandle<T,Storage>
        ::template ExecutionTypes<Device>::Portal PortalType;

    PortalType arrayPortal = values.PrepareForInPlace(Device());
    vtkm::cont::ArrayPortalToIterators<PortalType> iterators(arrayPortal);
    std::sort(iterators.GetBegin(), iterators.GetEnd(), comp);
  }

  VTKM_CONT_EXPORT static void Synchronize()
  {
    // Nothing to do. This device is HPX and has no asynchronous operations.
  }

};

}
} // namespace vtkm::cont

#endif //vtk_m_cont_internal_DeviceAdapterAlgorithmHPX_h