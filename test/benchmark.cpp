#define JM_CIRCULAR_BUFFER_CXX14
#include <circular_buffer.hpp>
#include <benchmark/benchmark.h>

#include <iostream>
#include <exception>

#include <ctime>

namespace {
  constexpr size_t k1kB = 1000;
  constexpr size_t k1MB = k1kB * 1000;
  constexpr size_t k1GB = k1MB * 1000;
  constexpr size_t k10GB = k1GB * 10;
  char generateRandomString() {
    return rand() % 255;
  }
  void BM_StaticCircleBufferCreation_k1kB(benchmark::State& state) {
    for (auto _ : state)
      jm::static_circular_buffer<char, k1kB> data;
  }

  void BM_DynamicCircleBufferCreation_k1kB(benchmark::State& state) {
    for (auto _ : state) {
      jm::dynamic_circular_buffer<char> data(k1kB);
    }
  }

  void BM_DynamicCircleBufferCreation_k1MB(benchmark::State& state) {
    for (auto _ : state) {
      jm::dynamic_circular_buffer<char> data;
      data.reserve(k1MB);
    }
  }

  void BM_StaticCircleBufferCreation_k1kB_push_back(benchmark::State& state) {
    srand(time(0));
    jm::static_circular_buffer<char, k1kB> data;
    for (auto _ : state) {
      for (size_t i = 0; i < state.range(0); i++) {
        data.push_back(generateRandomString());
      }
    }
  }

  void BM_DynamicCircleBufferCreation_k1kB_push_back(benchmark::State& state) {

    srand(time(0));
    jm::dynamic_circular_buffer<char> data;
    data.reserve(k1kB);
    for (auto _ : state) {
      for (size_t i = 0; i < state.range(0); i++) {
        data.push_back(generateRandomString());
      }
    }
  }

  void BM_StaticCircleBufferCreation_k1kB_iteration(benchmark::State& state) {
    jm::static_circular_buffer<char, k1kB> data;
    for (size_t i = 0; i < state.range(0); i++) {
      data.push_back(generateRandomString());
    }
    for (auto _ : state) {
      std::for_each(data.begin(), data.end(), [](auto& value) {
        value = generateRandomString();
      });
    }
  }

  void BM_DynamicCircleBufferCreation_k1kB_iteration(benchmark::State& state) {
    jm::dynamic_circular_buffer<char> data(k1kB);
    for (size_t i = 0; i < state.range(0); i++) {
      data.push_back(generateRandomString());
    }
    for (auto _ : state) {
      std::for_each(data.begin(), data.end(), [](auto& value) {
        value = generateRandomString();
      });
    }
  }

}

#include <Eigen/Geometry>
#include <Eigen/StdVector>

void BM_STDVectorEigen_1K_elements_without_Allocator(benchmark::State& state) {
  const auto randomValue = Eigen::Vector4f::Random();
  std::vector<Eigen::Vector4f> data(state.range(0), randomValue);
  for (auto _ : state) {

    std::for_each(data.begin(), data.end(), [&](auto& value) {
      const auto randomValue = Eigen::Vector4f::Random();
      benchmark::DoNotOptimize(value.dot(randomValue));
    });
  }
}

void BM_STDVectorEigen_1K_elements_with_Allocator(benchmark::State& state) {
  const auto randomValue = Eigen::Vector4f::Random();
  std::vector<Eigen::Vector4f, Eigen::aligned_allocator<Eigen::Vector4f>> data(state.range(0), randomValue);

  for (auto _ : state) {


    std::for_each(data.begin(), data.end(), [](auto& value) {
      const auto randomValue = Eigen::Vector4f::Random();
      benchmark::DoNotOptimize(value.dot(randomValue));
    });
  }
}

void BM_DynamicCircleBufferEigen_1K_elements_without_Allocator(benchmark::State& state) {
  const auto randomValue = Eigen::Vector4f::Random();
  jm::dynamic_circular_buffer<Eigen::Vector4f> data(state.range(0), randomValue);

  for (auto _ : state) {
    std::for_each(data.begin(), data.end(), [](auto& value) {
      const auto randomValue = Eigen::Vector4f::Random();
      benchmark::DoNotOptimize(value.dot(randomValue));
    });
  }
}

void BM_DynamicCircleBufferEigen_1K_elements_with_Allocator(benchmark::State& state) {
  const auto randomValue = Eigen::Vector4f::Random();
  jm::dynamic_circular_buffer<Eigen::Vector4f, Eigen::aligned_allocator< Eigen::Vector4f >> data(state.range(0), randomValue);

  for (auto _ : state) {
    std::for_each(data.begin(), data.end(), [](auto& value) {
      const auto randomValue = Eigen::Vector4f::Random();
      benchmark::DoNotOptimize(value.dot(randomValue));
    });
  }
}

//Register the function as a benchmark
BENCHMARK(BM_StaticCircleBufferCreation_k1kB);
BENCHMARK(BM_DynamicCircleBufferCreation_k1kB);
BENCHMARK(BM_DynamicCircleBufferCreation_k1MB);

BENCHMARK(BM_StaticCircleBufferCreation_k1kB_push_back)->Arg(8)->Arg(64)->Arg(512)->Arg(1 << 10)->Arg(8 << 10);
BENCHMARK(BM_DynamicCircleBufferCreation_k1kB_push_back)->Arg(8)->Arg(64)->Arg(512)->Arg(1 << 10)->Arg(8 << 10);

BENCHMARK(BM_StaticCircleBufferCreation_k1kB_iteration)->Arg(8)->Arg(64)->Arg(512)->Arg(1 << 10)->Arg(8 << 10);
BENCHMARK(BM_DynamicCircleBufferCreation_k1kB_iteration)->Arg(8)->Arg(64)->Arg(512)->Arg(1 << 10)->Arg(8 << 10);

BENCHMARK(BM_DynamicCircleBufferEigen_1K_elements_without_Allocator)->Arg(8)->Arg(64)->Arg(512)->Arg(1 << 10)->Arg(8 << 10);
BENCHMARK(BM_STDVectorEigen_1K_elements_without_Allocator)->Arg(8)->Arg(64)->Arg(512)->Arg(1 << 10)->Arg(8 << 10);


BENCHMARK(BM_DynamicCircleBufferEigen_1K_elements_with_Allocator)-> Arg(8)->Arg(64)->Arg(512)->Arg(1 << 10)->Arg(8 << 10);
BENCHMARK(BM_STDVectorEigen_1K_elements_with_Allocator)->Arg(8)->Arg(64)->Arg(512)->Arg(1 << 10)->Arg(8 << 10);



BENCHMARK_MAIN();



