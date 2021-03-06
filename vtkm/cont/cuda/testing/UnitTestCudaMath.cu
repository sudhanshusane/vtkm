//=============================================================================
//
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
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
//  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
//  Laboratory (LANL), the U.S. Government retains certain rights in
//  this software.
//
//=============================================================================

#define VTKM_DEVICE_ADAPTER VTKM_DEVICE_ADAPTER_ERROR
#define BOOST_SP_DISABLE_THREADS

#include <vtkm/testing/TestingMath.h>

#include <vtkm/cont/cuda/DeviceAdapterCuda.h>

int UnitTestCudaMath(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(
        UnitTestMathNamespace::RunMathTests<vtkm::cont::DeviceAdapterTagCuda>);
}
