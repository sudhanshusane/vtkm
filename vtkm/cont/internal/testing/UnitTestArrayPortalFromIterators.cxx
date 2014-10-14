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

#include <vtkm/cont/internal/ArrayPortalFromIterators.h>

#include <vtkm/cont/testing/Testing.h>
#include <vtkm/VecTraits.h>

namespace {

template<typename T>
struct TemplatedTests
{
  static const vtkm::Id ARRAY_SIZE = 10;

  typedef T ValueType;
  typedef typename vtkm::VecTraits<ValueType>::ComponentType ComponentType;

  ValueType ExpectedValue(vtkm::Id index, ComponentType value)
  {
    return ValueType(index + value);
  }

  template<class IteratorType>
  void FillIterator(IteratorType begin, IteratorType end, ComponentType value)
  {
    vtkm::Id index = 0;
    for (IteratorType iter = begin; iter != end; iter++)
    {
      *iter = ExpectedValue(index, value);
      index++;
    }
  }

  template<class IteratorType>
  bool CheckIterator(IteratorType begin,
                     IteratorType end,
                     ComponentType value)
  {
    vtkm::Id index = 0;
    for (IteratorType iter = begin; iter != end; iter++)
    {
      if (*iter != ExpectedValue(index, value)) { return false; }
      index++;
    }
    return true;
  }

  template<class PortalType>
  bool CheckPortal(const PortalType &portal, ComponentType value)
  {
    vtkm::cont::ArrayPortalToIterators<PortalType> iterators(portal);
    return CheckIterator(iterators.GetBegin(), iterators.GetEnd(), value);
  }

  void operator()()
  {
    ValueType array[ARRAY_SIZE];

    static const ComponentType ORIGINAL_VALUE = 239;
    FillIterator(array, array+ARRAY_SIZE, ORIGINAL_VALUE);

    ::vtkm::cont::internal::ArrayPortalFromIterators<ValueType *>
        portal(array, array+ARRAY_SIZE);
    ::vtkm::cont::internal::ArrayPortalFromIterators<const ValueType *>
        const_portal(array, array+ARRAY_SIZE);

    std::cout << "  Check that ArrayPortalToIterators is not doing indirection."
              << std::endl;
    // If you get a compile error here about mismatched types, it might be
    // that ArrayPortalToIterators is not properly overloaded to return the
    // original iterator.
#ifndef VTKM_MSVC
    VTKM_TEST_ASSERT(vtkm::cont::ArrayPortalToIteratorBegin(portal) == array,
                     "Begin iterator wrong.");
    VTKM_TEST_ASSERT(vtkm::cont::ArrayPortalToIteratorEnd(portal) ==
                     array+ARRAY_SIZE,
                     "End iterator wrong.");
    VTKM_TEST_ASSERT(vtkm::cont::ArrayPortalToIteratorBegin(const_portal) ==
                     array,
                     "Begin const iterator wrong.");
    VTKM_TEST_ASSERT(vtkm::cont::ArrayPortalToIteratorEnd(const_portal) ==
                     array+ARRAY_SIZE,
                     "End const iterator wrong.");
#else //VTKM_MSVC
    // The Microsoft compiler likes to complain about using raw pointers in
    // generic functions, so we use the non-portable wrapper
    // stdext::checked_array_iterator instead. In this case, check for that
    // type. Same comment about mismatched type compiler errors in the comment
    // above applies here.
    VTKM_TEST_ASSERT(vtkm::cont::ArrayPortalToIteratorBegin(portal) ==
      stdext::checked_array_iterator<ValueType *>(array, ARRAY_SIZE),
      "Begin iterator wrong.");
    VTKM_TEST_ASSERT(vtkm::cont::ArrayPortalToIteratorEnd(portal) ==
      stdext::checked_array_iterator<ValueType *>(array, ARRAY_SIZE) + ARRAY_SIZE,
      "End iterator wrong.");
    VTKM_TEST_ASSERT(vtkm::cont::ArrayPortalToIteratorBegin(const_portal) ==
      stdext::checked_array_iterator<const ValueType *>(array, ARRAY_SIZE),
      "Begin const iterator wrong.");
    VTKM_TEST_ASSERT(vtkm::cont::ArrayPortalToIteratorEnd(const_portal) ==
      stdext::checked_array_iterator<const ValueType *>(array, ARRAY_SIZE) + ARRAY_SIZE,
      "End const iterator wrong.");
#endif // VTKM_MSVC

    VTKM_TEST_ASSERT(portal.GetNumberOfValues() == ARRAY_SIZE,
                     "Portal array size wrong.");
    VTKM_TEST_ASSERT(const_portal.GetNumberOfValues() == ARRAY_SIZE,
                     "Const portal array size wrong.");

    std::cout << "  Check inital value." << std::endl;
    VTKM_TEST_ASSERT(CheckPortal(portal, ORIGINAL_VALUE),
                     "Portal iterator has bad value.");
    VTKM_TEST_ASSERT(CheckPortal(const_portal, ORIGINAL_VALUE),
                     "Const portal iterator has bad value.");

    static const ComponentType SET_VALUE = 62;

    std::cout << "  Check get/set methods." << std::endl;
    for (vtkm::Id index = 0; index < ARRAY_SIZE; index++)
    {
      VTKM_TEST_ASSERT(portal.Get(index)
                       == ExpectedValue(index, ORIGINAL_VALUE),
                       "Bad portal value.");
      VTKM_TEST_ASSERT(const_portal.Get(index)
                       == ExpectedValue(index, ORIGINAL_VALUE),
                       "Bad const portal value.");

      portal.Set(index, ExpectedValue(index, SET_VALUE));
    }

    std::cout << "  Make sure set has correct value." << std::endl;
    VTKM_TEST_ASSERT(CheckPortal(portal, SET_VALUE),
                     "Portal iterator has bad value.");
    VTKM_TEST_ASSERT(CheckIterator(array, array+ARRAY_SIZE, SET_VALUE),
                     "Array has bad value.");
  }
};

struct TestFunctor
{
  template<typename T>
  void operator()(T) const
  {
    TemplatedTests<T> tests;
    tests();
  }
};

void TestArrayPortalFromIterators()
{
  vtkm::testing::Testing::TryAllTypes(TestFunctor());
}

} // Anonymous namespace

int UnitTestArrayPortalFromIterators(int, char *[])
{
  return vtkm::cont::testing::Testing::Run(TestArrayPortalFromIterators);
}