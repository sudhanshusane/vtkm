##============================================================================
##  Copyright (c) Kitware, Inc.
##  All rights reserved.
##  See LICENSE.txt for details.
##  This software is distributed WITHOUT ANY WARRANTY; without even
##  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
##  PURPOSE.  See the above copyright notice for more information.
##
##  Copyright 2014 Sandia Corporation.
##  Copyright 2014 UT-Battelle, LLC.
##  Copyright 2014. Los Alamos National Security
##
##  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
##  the U.S. Government retains certain rights in this software.
##
##  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
##  Laboratory (LANL), the U.S. Government retains certain rights in
##  this software.
##============================================================================

set(headers
  Testing.h
  TestingDeviceAdapter.h
  )

vtkm_declare_headers(${headers})

set(unit_tests
  UnitTestArrayHandle.cxx
  UnitTestArrayHandleCompositeVector.cxx
  UnitTestArrayHandleCounting.cxx
  UnitTestArrayHandleUniformPointCoordinates.cxx
  UnitTestArrayPortalToIterators.cxx
  UnitTestContTesting.cxx
  UnitTestDeviceAdapterAlgorithmDependency.cxx
  UnitTestDeviceAdapterAlgorithmGeneral.cxx
  UnitTestDeviceAdapterSerial.cxx
  UnitTestDynamicArrayHandle.cxx
  UnitTestDynamicPointCoordinates.cxx
  UnitTestPointCoordinates.cxx
  UnitTestStorageBasic.cxx
  UnitTestStorageImplicit.cxx
  UnitTestStorageListTag.cxx
  UnitTestTimer.cxx
  )

vtkm_unit_tests(SOURCES ${unit_tests})
