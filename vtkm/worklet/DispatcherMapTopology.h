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
#ifndef vtk_m_worklet_Dispatcher_MapTopology_h
#define vtk_m_worklet_Dispatcher_MapTopology_h

#include <vtkm/TopologyElementTag.h>
#include <vtkm/cont/DeviceAdapter.h>
#include <vtkm/worklet/WorkletMapTopology.h>
#include <vtkm/worklet/internal/DispatcherBase.h>

namespace vtkm {
namespace worklet {

/// \brief Dispatcher for worklets that inherit from \c WorkletMapTopology.
///
template<typename WorkletType,
         typename Device = VTKM_DEFAULT_DEVICE_ADAPTER_TAG>
class DispatcherMapTopology :
    public vtkm::worklet::internal::DispatcherBase<
      DispatcherMapTopology<WorkletType,Device>,
      WorkletType,
      vtkm::worklet::template WorkletMapTopology<typename WorkletType::FromTopologyType, typename WorkletType::ToTopologyType>,
      Device>
{
  typedef vtkm::worklet::internal::DispatcherBase<
    DispatcherMapTopology<WorkletType,Device>,
    WorkletType,
    vtkm::worklet::template WorkletMapTopology<typename WorkletType::FromTopologyType, typename WorkletType::ToTopologyType>,
    Device> Superclass;

public:
  VTKM_CONT_EXPORT
  DispatcherMapTopology(const WorkletType &worklet = WorkletType())
    : Superclass(worklet) {  }

  template<typename Invocation>
  VTKM_CONT_EXPORT
  void DoInvoke(const Invocation &invocation) const
  {
    // The parameter for the input domain is stored in the Invocation. (It is
    // also in the worklet, but it is safer to get it from the Invocation
    // in case some other dispatch operation had to modify it.)
    static const vtkm::IdComponent InputDomainIndex =
        Invocation::InputDomainIndex;

    // ParameterInterface (from Invocation) is a FunctionInterface type
    // containing types for all objects passed to the Invoke method (with
    // some dynamic casting performed so objects like DynamicArrayHandle get
    // cast to ArrayHandle).
    typedef typename Invocation::ParameterInterface ParameterInterface;

    // This is the type for the input domain (derived from the last two things
    // we got from the Invocation).
    typedef typename ParameterInterface::
        template ParameterType<InputDomainIndex>::type InputDomainType;

    // If you get a compile error on this line, then you have tried to use
    // something that is not a vtkm::cont::CellSet as the input domain to a
    // topology operation (that operates on a cell set connection domain).
    VTKM_IS_CELL_SET(InputDomainType);

    // We can pull the input domain parameter (the data specifying the input
    // domain) from the invocation object.
    InputDomainType inputDomain =
        invocation.Parameters.template GetParameter<InputDomainIndex>();

    // Now that we have the input domain, we can extract the range of the
    // scheduling and call BadicInvoke.
    this->BasicInvoke(invocation,
                      inputDomain.GetSchedulingRange(
                            typename WorkletType::ToTopologyType()));
  }
};

}
} // namespace vtkm::worklet

#endif //vtk_m_worklet_Dispatcher_MapTopology_h
