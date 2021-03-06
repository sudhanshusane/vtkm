##=============================================================================
##
##  Copyright (c) Kitware, Inc.
##  All rights reserved.
##  See LICENSE.txt for details.
##
##  This software is distributed WITHOUT ANY WARRANTY; without even
##  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
##  PURPOSE.  See the above copyright notice for more information.
##
##  Copyright 2015 Sandia Corporation.
##  Copyright 2015 UT-Battelle, LLC.
##  Copyright 2015 Los Alamos National Security.
##
##  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
##  the U.S. Government retains certain rights in this software.
##  Under the terms of Contract DE-AC52-06NA25396 with Los Alamos National
##  Laboratory (LANL), the U.S. Government retains certain rights in
##  this software.
##
##=============================================================================

cmake_minimum_required(VERSION 2.8.11)
find_package(OpenGL)
find_package(GLUT)
find_package(TBB)

if(OPENGL_FOUND AND GLUT_FOUND)
  add_executable(IsosurfaceUniformGrid_SERIAL IsosurfaceUniformGrid.cxx)
  target_include_directories(IsosurfaceUniformGrid_SERIAL PRIVATE ${GLUT_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
  target_link_libraries(IsosurfaceUniformGrid_SERIAL ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})

  if(VTKm_Cuda_FOUND)
    vtkm_disable_troublesome_thrust_warnings()
    cuda_add_executable(IsosurfaceUniformGrid_CUDA IsosurfaceUniformGrid.cu)
    target_link_libraries(IsosurfaceUniformGrid_CUDA ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES})
  endif()

  if(VTKm_ENABLE_TBB)
    add_executable(IsosurfaceUniformGrid_TBB IsosurfaceUniformGridTBB.cxx)
    target_include_directories(IsosurfaceUniformGrid_TBB PRIVATE ${GLUT_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} ${TBB_INCLUDE_DIRS})
    target_link_libraries(IsosurfaceUniformGrid_TBB ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${TBB_LIBRARIES})
  endif()

endif()
