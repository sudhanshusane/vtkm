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

#ifndef vtk_m_benchmarking_Benchmarker_h
#define vtk_m_benchmarking_Benchmarker_h

#include <vtkm/Math.h>

#include <vector>
#include <algorithm>
#include <iostream>

/*
 * Writing a Benchmark
 * -------------------
 * To write a benchmark you must provide a functor that will run the operations
 * you want to time and return the run time of those operations using the timer
 * for the device. The benchmark should also be templated on the value type being
 * operated on. Then use VTKM_MAKE_BENCHMARK to generate a maker functor and
 * VTKM_RUN_BENCHMARK to run the benchmark on a list of types.
 *
 * For Example:
 *
 * template<typename Value>
 * struct BenchSilly {
 *   // Setup anything that doesn't need to change per run in the constructor
 *   VTKM_CONT_EXPORT BenchSilly(){}
 *
 *   // The overloaded call operator will run the operations being timed and
 *   // return the execution time
 *   VTKM_CONT_EXPORT
 *   vtkm::Float64 operator()(){
 *     return 0.05;
 *   }
 *
 *   // The benchmark must also provide a method describing itself, this is
 *   // used when printing out run time statistics
 *   VTKM_CONT_EXPORT
 *   std::string Description() const {
 *     return "A silly benchmark";
 *   }
 * };
 *
 * // Now use the VTKM_MAKE_BENCHMARK macro to generate a maker functor for
 * // your benchmark. This lets us generate the benchmark functor for each type
 * // we want to test
 * VTKM_MAKE_BENCHMARK(Silly, BenchSilly);
 *
 * // You can also optionally pass arguments to the constructor like so:
 * // VTKM_MAKE_BENCHMARK(Blah, BenchBlah, 1, 2, 3);
 * // Note that benchmark names (the first argument) must be unique so different
 * // parameters to the constructor should have different names
 *
 * // We can now run our benchmark using VTKM_RUN_BENCHMARK, passing the
 * // benchmark name and type list to run on
 * int main(int, char**){
 *   VTKM_RUN_BENCHMARK(Silly, vtkm::ListTagBase<vtkm::Float32>());
 *   return 0;
 * }
 *
 * Check out vtkm/benchmarking/BenchmarkDeviceAdapter.h for some example usage
 */

/*
 * Use the VTKM_MAKE_BENCHMARK macro to define a maker functor for your benchmark.
 * This is used to allow you to template the benchmark functor on the type being benchmarked
 * so you can write init code in the constructor. Then the maker will return a constructed
 * instance of your benchmark for the type being benchmarked. The VA_ARGS are used to
 * pass any extra arguments needed by your benchmark
 */
#define VTKM_MAKE_BENCHMARK(Name, Bench, ...) \
  struct MakeBench##Name { \
    template<typename Value> \
    VTKM_CONT_EXPORT \
    Bench<Value> operator()(const Value vtkmNotUsed(v)) const { \
      return Bench<Value>(__VA_ARGS__); \
    } \
  }

/*
 * Use the VTKM_RUN_BENCHMARK macro to run your benchmark on the type list passed.
 * You must have previously defined a maker functor with VTKM_MAKE_BENCHMARK that this
 * macro will look for and use
 */
#define VTKM_RUN_BENCHMARK(Name, Types) \
  vtkm::benchmarking::BenchmarkTypes(MakeBench##Name(), (Types))

namespace vtkm {
namespace benchmarking {
namespace stats {
// Checks that the sequence is sorted, returns true if it's sorted, false
// otherwise
template<typename ForwardIt>
bool is_sorted(ForwardIt first, ForwardIt last){
  ForwardIt next = first;
  ++next;
  for (; next != last; ++next, ++first){
    if (*first > *next){
      return false;
    }
  }
  return true;
}

// Get the value representing the `percent` percentile of the
// sorted samples using linear interpolation
vtkm::Float64 PercentileValue(const std::vector<vtkm::Float64> &samples, const vtkm::Float64 percent){
  VTKM_ASSERT_CONT(!samples.empty());
  if (samples.size() == 1){
    return samples.front();
  }
  VTKM_ASSERT_CONT(percent >= 0.0);
  VTKM_ASSERT_CONT(percent <= 100.0);
  VTKM_ASSERT_CONT(
        vtkm::benchmarking::stats::is_sorted(samples.begin(), samples.end()));
  if (percent == 100.0){
    return samples.back();
  }
  // Find the two nearest percentile values and linearly
  // interpolate between them
  const vtkm::Float64 rank = percent / 100.0 * (static_cast<vtkm::Float64>(samples.size()) - 1.0);
  const vtkm::Float64 low_rank = vtkm::Floor(rank);
  const vtkm::Float64 dist = rank - low_rank;
  const size_t k = static_cast<size_t>(low_rank);
  const vtkm::Float64 low = samples[k];
  const vtkm::Float64 high = samples[k + 1];
  return low + (high - low) * dist;
}
// Winsorize the samples to clean up any very extreme outliers
// Will replace all samples below `percent` and above 100 - `percent` percentiles
// with the value at the percentile
// NOTE: Assumes the samples have been sorted, as we make use of PercentileValue
void Winsorize(std::vector<vtkm::Float64> &samples, const vtkm::Float64 percent){
  const vtkm::Float64 low_percentile = PercentileValue(samples, percent);
  const vtkm::Float64 high_percentile = PercentileValue(samples, 100.0 - percent);
  for (std::vector<vtkm::Float64>::iterator it = samples.begin(); it != samples.end(); ++it){
    if (*it < low_percentile){
      *it = low_percentile;
    }
    else if (*it > high_percentile){
      *it = high_percentile;
    }
  }
}
// Compute the mean value of the dataset
vtkm::Float64 Mean(const std::vector<vtkm::Float64> &samples){
  vtkm::Float64 mean = 0;
  for (std::vector<vtkm::Float64>::const_iterator it = samples.begin(); it != samples.end(); ++it){
    mean += *it;
  }
  return mean / static_cast<vtkm::Float64>(samples.size());
}
// Compute the sample variance of the samples
vtkm::Float64 Variance(const std::vector<vtkm::Float64> &samples){
  vtkm::Float64 mean = Mean(samples);
  vtkm::Float64 square_deviations = 0;
  for (std::vector<vtkm::Float64>::const_iterator it = samples.begin(); it != samples.end(); ++it){
    square_deviations += vtkm::Pow(*it - mean, 2.0);
  }
  return square_deviations / (static_cast<vtkm::Float64>(samples.size()) - 1.0);
}
// Compute the standard deviation of the samples
vtkm::Float64 StandardDeviation(const std::vector<vtkm::Float64> &samples){
  return vtkm::Sqrt(Variance(samples));
}
// Compute the median absolute deviation of the dataset
vtkm::Float64 MedianAbsDeviation(const std::vector<vtkm::Float64> &samples){
  std::vector<vtkm::Float64> abs_deviations;
  abs_deviations.reserve(samples.size());
  const vtkm::Float64 median = PercentileValue(samples, 50.0);
  for (std::vector<vtkm::Float64>::const_iterator it = samples.begin(); it != samples.end(); ++it){
    abs_deviations.push_back(vtkm::Abs(*it - median));
  }
  std::sort(abs_deviations.begin(), abs_deviations.end());
  return PercentileValue(abs_deviations, 50.0);
}
} // stats

/*
 * The benchmarker takes a functor to benchmark and runs it multiple times,
 * printing out statistics of the run time at the end.
 * The functor passed should return the run time of the thing being benchmarked
 * in seconds, this lets us avoid including any per-run setup time in the benchmark.
 * However any one-time setup should be done in the functor's constructor
 */
struct Benchmarker {
  const vtkm::Float64 MAX_RUNTIME;
  const size_t MAX_ITERATIONS;

  Benchmarker() : MAX_RUNTIME(30), MAX_ITERATIONS(500){}

  template<typename Functor>
  VTKM_CONT_EXPORT
  void operator()(Functor func) const {
    std::vector<vtkm::Float64> samples;
    // Do a warm-up run. If the benchmark allocates any additional memory
    // eg. storage for output results, this will let it do that and
    // allow us to avoid measuring the allocation time in the actual benchmark run
    func();

    samples.reserve(MAX_ITERATIONS);
    // Run each benchmark for MAX_RUNTIME seconds or MAX_ITERATIONS iterations, whichever
    // takes less time. This kind of assumes that running for 500 iterations or 1.5s will give
    // good statistics, but if median abs dev and/or std dev are too high both these limits
    // could be increased
    size_t iter = 0;
    for (vtkm::Float64 elapsed = 0.0; elapsed < MAX_RUNTIME && iter < MAX_ITERATIONS;
        elapsed += samples.back(), ++iter)
    {
      samples.push_back(func());
    }
    std::sort(samples.begin(), samples.end());
    stats::Winsorize(samples, 5.0);
    std::cout << "Benchmark \'"
      << func.Description() << "\' results:\n"
      << "\tmedian = " << stats::PercentileValue(samples, 50.0) << "s\n"
      << "\tmedian abs dev = " << stats::MedianAbsDeviation(samples) << "s\n"
      << "\tmean = " << stats::Mean(samples) << "s\n"
      << "\tstd dev = " << stats::StandardDeviation(samples) << "s\n"
      << "\tmin = " << samples.front() << "s\n"
      << "\tmax = " << samples.back() << "s\n";
  }
};

template<typename MakerFunctor>
class InternalPrintTypeAndBench {
  MakerFunctor Maker;

public:
  VTKM_CONT_EXPORT
  InternalPrintTypeAndBench(MakerFunctor maker) : Maker(maker) {}

  template<typename T>
  VTKM_CONT_EXPORT
  void operator()(T t) const {
    std::cout << "*** "
              << vtkm::testing::TypeName<T>::Name()
              << " ***************" << std::endl;
    Benchmarker bench;
    bench(Maker(t));
  }
};

template<class MakerFunctor, class TypeList>
VTKM_CONT_EXPORT
void BenchmarkTypes(const MakerFunctor &maker, TypeList){
  vtkm::ListForEach(InternalPrintTypeAndBench<MakerFunctor>(maker), TypeList());
}

}
}

#endif

