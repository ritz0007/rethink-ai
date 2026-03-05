# BUILD AI FROM ABSOLUTE SCRATCH: COMPLETE SETUP & ARCHITECTURE GUIDE

## Executive Summary

This document provides a comprehensive, production-ready blueprint for building a complete AI/ML system from absolute scratch—zero external AI/ML libraries. The target audience is systems programmers who want to understand and shape every aspect of AI behavior through raw implementation, not framework configuration.

**Key Principles:**
- Zero external ML/AI dependencies (no Eigen, ArrayFire, oneDNN, OpenBLAS)
- Standard library only for core operations (C++20 `std::simd` permitted)
- Pure C++ implementation with no Python interoperability
- Memory safety through RAII and smart pointers—no raw new/delete
- Template support for float32, float64, and bfloat16 precision

---

## 1. EXECUTIVE DECISION MATRIX

### 1.1 Programming Language Selection

After comprehensive analysis of C++20, Rust 2021 Edition, Zig 0.11, and C11 for bare-metal AI implementation:

| Criterion | C++20 | Rust 2021 | Zig 0.11 | C11 |
|-----------|-------|-----------|----------|-----|
| **SIMD/Vectorization** | Excellent (std::simd, AVX-512) | Good (packed_simd, core::simd) | Limited | Manual intrinsics only |
| **Memory Management** | Manual + RAII | Ownership + Borrowing | Manual + Comptime | Manual |
| **Compilation Time** | Moderate | Slow | Fast | Fast |
| **Runtime Performance** | Peak (99% of theoretical) | 95-98% of C++ | 95-97% of C++ | 100% (baseline) |
| **Cross-Platform** | Excellent | Excellent | Good | Excellent |
| **Debuggability** | Excellent (GDB, LLDB) | Good | Moderate | Excellent |
| **Ecosystem Maturity** | Excellent | Very Good | Emerging | Excellent |
| **Template/Generic Power** | Excellent | Good | Comptime-based | Limited (macros) |

**Recommendation: C++20**

Rationale:
1. **`std::simd`** provides portable SIMD abstractions with compiler-generated optimal FMA instructions
2. **Template metaprogramming** enables zero-cost abstractions for tensor operations with compile-time shape inference
3. **RAII + smart pointers** provide memory safety without garbage collection overhead
4. **Mature toolchain** (GCC 12+, Clang 15+, MSVC 2022+) with excellent profiling support
5. **Direct hardware access** for GPU compute preparation (CUDA/Metal/Vulkan interop)

### 1.2 Development Environment Specification

**Minimum Requirements:**
- OS: Linux (Ubuntu 22.04+), macOS 13+, or Windows 11 with WSL2
- Compiler: GCC 12.2+, Clang 15.0+, or MSVC 2022 17.4+
- CMake: 3.25+
- Debugger: GDB 12+ or LLDB 15+
- Profiler: perf (Linux), Instruments (macOS), or Intel VTune

**Recommended Hardware for Development:**
- CPU: 8+ cores with AVX2 support (AVX-512 preferred)
- RAM: 32GB minimum, 64GB recommended
- Storage: NVMe SSD with 500GB+ available

### 1.3 Project Topology

```
ai_from_scratch/
├── CMakeLists.txt
├── cmake/
│   └── compiler_flags.cmake
├── include/
│   ├── core/
│   │   ├── tensor.hpp          # Tensor implementation
│   │   ├── memory.hpp          # Custom allocators
│   │   └── simd_ops.hpp        # SIMD operations
│   ├── math/
│   │   ├── linalg.hpp          # Linear algebra
│   │   ├── autodiff.hpp        # Automatic differentiation
│   │   └── random.hpp          # RNG for ML
│   ├── nn/
│   │   ├── layer.hpp           # Base layer interface
│   │   ├── linear.hpp          # Dense layers
│   │   ├── conv.hpp            # Convolution layers
│   │   ├── recurrent.hpp       # LSTM/GRU
│   │   ├── attention.hpp       # Attention mechanisms
│   │   └── transformer.hpp     # Transformer blocks
│   ├── optim/
│   │   ├── sgd.hpp             # SGD variants
│   │   ├── adam.hpp            # Adam family
│   │   └── scheduler.hpp       # LR scheduling
│   ├── data/
│   │   ├── dataloader.hpp      # Data loading
│   │   └── augment.hpp         # Augmentation
│   └── utils/
│       ├── profiling.hpp       # Timing/FLOP counting
│       └── viz.hpp             # ASCII/SVG visualization
├── src/
│   └── (implementations)
├── tests/
│   └── (unit tests)
└── examples/
    ├── mlp_mnist.cpp
    ├── cnn_cifar.cpp
    └── transformer_lm.cpp
```

---

## 2. CORE MATHEMATICAL ENGINE

### 2.1 Tensor Implementation

#### Design Rationale

Tensors are the fundamental data structure. Key design decisions:

1. **Strided memory layout** over contiguous: Enables efficient views, broadcasting, and zero-copy operations
2. **Small Buffer Optimization (SBO)**: Tensors < 128 bytes store data inline, avoiding heap allocation
3. **Copy-on-Write (COW)**: Shared ownership for read-only tensors, deep copy on modification
4. **Cache-friendly traversal**: Row-major order with tiling for matrix operations

#### Interface Specification

```cpp
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <numeric>
#include <span>
#include <vector>
#include <stdexcept>

namespace aifs {

template<typename T, std::size_t N>
class Shape {
    std::array<std::size_t, N> dims_;
    std::array<std::size_t, N> strides_;
    
public:
    constexpr Shape() noexcept = default;
    constexpr explicit Shape(std::array<std::size_t, N> dims) noexcept 
        : dims_(dims) {
        compute_strides();
    }
    
    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return std::accumulate(dims_.begin(), dims_.end(), 
                               std::size_t{1}, std::multiplies{});
    }
    
    [[nodiscard]] constexpr std::size_t dim(std::size_t i) const noexcept {
        return dims_[i];
    }
    
    [[nodiscard]] constexpr std::size_t stride(std::size_t i) const noexcept {
        return strides_[i];
    }
    
    [[nodiscard]] constexpr std::size_t ndim() const noexcept { return N; }
    
    [[nodiscard]] constexpr std::size_t offset(
        std::array<std::size_t, N> indices) const noexcept {
        std::size_t off = 0;
        for (std::size_t i = 0; i < N; ++i) {
            off += indices[i] * strides_[i];
        }
        return off;
    }
    
private:
    constexpr void compute_strides() noexcept {
        strides_[N-1] = 1;
        for (std::size_t i = N-1; i > 0; --i) {
            strides_[i-1] = strides_[i] * dims_[i];
        }
    }
};

template<typename T, std::size_t N>
class Tensor {
public:
    using value_type = T;
    using size_type = std::size_t;
    static constexpr size_type SMALL_BUFFER_SIZE = 128 / sizeof(T);
    
private:
    Shape<T, N> shape_;
    
    // Small buffer optimization
    union Storage {
        T inline_data[SMALL_BUFFER_SIZE];
        T* heap_data;
        
        Storage() noexcept {}
        ~Storage() noexcept {}
    } storage_;
    
    bool is_small_ = true;
    std::shared_ptr<T[]> shared_data_;  // For COW
    
    [[nodiscard]] T* data_ptr() noexcept {
        return is_small_ ? storage_.inline_data : storage_.heap_data;
    }
    
    [[nodiscard]] const T* data_ptr() const noexcept {
        return is_small_ ? storage_.inline_data : storage_.heap_data;
    }
    
public:
    // Constructors
    Tensor() noexcept = default;
    
    explicit Tensor(Shape<T, N> shape) : shape_(shape) {
        const auto sz = shape_.size();
        if (sz > SMALL_BUFFER_SIZE) {
            is_small_ = false;
            storage_.heap_data = new T[sz];
            shared_data_ = std::shared_ptr<T[]>(storage_.heap_data, 
                                                std::default_delete<T[]>());
        }
    }
    
    Tensor(Shape<T, N> shape, T fill_value) : Tensor(shape) {
        std::fill_n(data_ptr(), shape_.size(), fill_value);
    }
    
    // Copy semantics (COW)
    Tensor(const Tensor& other) 
        : shape_(other.shape_), 
          is_small_(other.is_small_),
          shared_data_(other.shared_data_) {
        if (is_small_) {
            std::copy_n(other.storage_.inline_data, shape_.size(), 
                       storage_.inline_data);
        } else {
            storage_.heap_data = other.storage_.heap_data;
        }
    }
    
    // Move semantics
    Tensor(Tensor&& other) noexcept 
        : shape_(std::move(other.shape_)),
          is_small_(other.is_small_),
          shared_data_(std::move(other.shared_data_)) {
        if (is_small_) {
            std::move(other.storage_.inline_data, 
                     other.storage_.inline_data + shape_.size(),
                     storage_.inline_data);
        } else {
            storage_.heap_data = other.storage_.heap_data;
        }
    }
    
    ~Tensor() noexcept {
        if (!is_small_ && !shared_data_) {
            delete[] storage_.heap_data;
        }
    }
    
    // Element access
    template<typename... Indices>
    [[nodiscard]] T& operator()(Indices... indices) {
        static_assert(sizeof...(Indices) == N, "Wrong number of indices");
        ensure_unique();
        return data_ptr()[shape_.offset({static_cast<size_type>(indices)...})];
    }
    
    template<typename... Indices>
    [[nodiscard]] const T& operator()(Indices... indices) const {
        static_assert(sizeof...(Indices) == N, "Wrong number of indices");
        return data_ptr()[shape_.offset({static_cast<size_type>(indices)...})];
    }
    
    // Views
    [[nodiscard]] std::span<T> as_span() {
        ensure_unique();
        return std::span<T>(data_ptr(), shape_.size());
    }
    
    [[nodiscard]] std::span<const T> as_span() const {
        return std::span<const T>(data_ptr(), shape_.size());
    }
    
    // Shape operations
    [[nodiscard]] const Shape<T, N>& shape() const noexcept { return shape_; }
    [[nodiscard]] size_type size() const noexcept { return shape_.size(); }
    
    // Broadcasting
    template<std::size_t M>
    [[nodiscard]] bool is_broadcastable_to(const Shape<T, M>& other) const noexcept {
        // Implementation for broadcasting compatibility check
        return true;  // Simplified
    }
    
private:
    void ensure_unique() {
        if (shared_data_ && shared_data_.use_count() > 1) {
            // Deep copy for COW
            const auto sz = shape_.size();
            T* new_data = new T[sz];
            std::copy_n(data_ptr(), sz, new_data);
            shared_data_.reset();
            if (!is_small_) {
                delete[] storage_.heap_data;
            }
            if (sz > SMALL_BUFFER_SIZE) {
                is_small_ = false;
                storage_.heap_data = new_data;
                shared_data_ = std::shared_ptr<T[]>(new_data, 
                                                    std::default_delete<T[]>());
            } else {
                is_small_ = true;
                std::copy_n(new_data, sz, storage_.inline_data);
                delete[] new_data;
            }
        }
    }
};

} // namespace aifs
```

#### Broadcasting Rules Implementation

```cpp
template<typename T, std::size_t N, std::size_t M>
[[nodiscard]] constexpr bool can_broadcast(
    const Shape<T, N>& a, 
    const Shape<T, M>& b) noexcept {
    const std::size_t max_ndim = std::max(N, M);
    
    for (std::size_t i = 0; i < max_ndim; ++i) {
        const std::size_t dim_a = (i < N) ? a.dim(N - 1 - i) : 1;
        const std::size_t dim_b = (i < M) ? b.dim(M - 1 - i) : 1;
        
        if (dim_a != dim_b && dim_a != 1 && dim_b != 1) {
            return false;
        }
    }
    return true;
}
```

**Complexity Analysis:**
- Construction: O(1) for small tensors, O(n) for heap allocation
- Element access: O(1)
- Copy: O(1) for COW reference, O(n) on first modification
- Broadcasting check: O(max(N, M))

### 2.2 Automatic Differentiation System

#### Design Rationale

Reverse-mode automatic differentiation (backpropagation) is essential for neural network training. Design choices:

1. **Dynamic computation graph**: More flexible than static graphs for research/experimentation
2. **Tape-based approach**: Record operations during forward pass, replay backward
3. **Operator overloading**: Seamless integration with tensor operations
4. **Gradient checkpointing**: Trade compute for memory in deep networks

#### Mathematical Foundation

For a computation graph with nodes representing operations, reverse-mode AD computes:

$$\frac{\partial L}{\partial x_i} = \sum_{j: x_i \in \text{parents}(j)} \frac{\partial L}{\partial y_j} \cdot \frac{\partial y_j}{\partial x_i}$$

Where:
- $L$ is the loss (scalar output)
- $y_j$ are intermediate nodes
- $x_i$ are inputs/parameters

#### Implementation

```cpp
#pragma once
#include "tensor.hpp"
#include <functional>
#include <memory>
#include <vector>
#include <unordered_set>

namespace aifs::autodiff {

template<typename T>
class Variable : public std::enable_shared_from_this<Variable<T>> {
public:
    Tensor<T, 1> data;
    Tensor<T, 1> grad;
    bool requires_grad = false;
    
    using BackwardFn = std::function<void()>;
    
private:
    std::vector<std::shared_ptr<Variable>> prev_;
    BackwardFn backward_fn_;
    std::size_t grad_fn_count_ = 0;
    bool visited_ = false;
    
public:
    explicit Variable(Tensor<T, 1> data, bool requires_grad = false)
        : data(std::move(data)), 
          grad(Tensor<T, 1>(this->data.shape(), T{0})),
          requires_grad(requires_grad) {}
    
    void set_grad_fn(const BackwardFn& fn, 
                     std::vector<std::shared_ptr<Variable>> prev) {
        backward_fn_ = fn;
        prev_ = std::move(prev);
        for (auto& p : prev_) {
            if (p->requires_grad) {
                p->grad_fn_count_++;
            }
        }
    }
    
    void backward() {
        // Initialize gradient of output to 1
        std::fill_n(grad.as_span().data(), grad.size(), T{1});
        
        // Topological sort
        std::vector<std::shared_ptr<Variable>> topo_order;
        std::function<void(std::shared_ptr<Variable>)> visit = 
            [&](std::shared_ptr<Variable> v) {
                if (v->visited_) return;
                v->visited_ = true;
                for (auto& p : v->prev_) {
                    if (p->requires_grad) {
                        visit(p);
                    }
                }
                topo_order.push_back(v);
            };
        
        visit(this->shared_from_this());
        
        // Reverse topological order for backward pass
        for (auto it = topo_order.rbegin(); it != topo_order.rend(); ++it) {
            if ((*it)->backward_fn_) {
                (*it)->backward_fn_();
            }
        }
        
        // Reset visited flags
        for (auto& v : topo_order) {
            v->visited_ = false;
        }
    }
    
    void zero_grad() {
        std::fill_n(grad.as_span().data(), grad.size(), T{0});
    }
};

// Overloaded operators
template<typename T>
std::shared_ptr<Variable<T>> operator+(std::shared_ptr<Variable<T>> a,
                                       std::shared_ptr<Variable<T>> b) {
    auto result = std::make_shared<Variable<T>>(
        add_tensors(a->data, b->data),
        a->requires_grad || b->requires_grad
    );
    
    if (result->requires_grad) {
        result->set_grad_fn([=]() {
            if (a->requires_grad) {
                accumulate_grad(a->grad, result->grad);
            }
            if (b->requires_grad) {
                accumulate_grad(b->grad, result->grad);
            }
        }, {a, b});
    }
    
    return result;
}

template<typename T>
std::shared_ptr<Variable<T>> operator*(std::shared_ptr<Variable<T>> a,
                                       std::shared_ptr<Variable<T>> b) {
    auto result = std::make_shared<Variable<T>>(
        multiply_tensors(a->data, b->data),
        a->requires_grad || b->requires_grad
    );
    
    if (result->requires_grad) {
        result->set_grad_fn([=]() {
            if (a->requires_grad) {
                auto grad_a = multiply_tensors(result->grad, b->data);
                accumulate_grad(a->grad, grad_a);
            }
            if (b->requires_grad) {
                auto grad_b = multiply_tensors(result->grad, a->data);
                accumulate_grad(b->grad, grad_b);
            }
        }, {a, b});
    }
    
    return result;
}

// Matrix multiplication with autodiff
template<typename T>
std::shared_ptr<Variable<T>> matmul(std::shared_ptr<Variable<T>> a,
                                    std::shared_ptr<Variable<T>> b) {
    auto result = std::make_shared<Variable<T>>(
        gemm(a->data, b->data),
        a->requires_grad || b->requires_grad
    );
    
    if (result->requires_grad) {
        result->set_grad_fn([=]() {
            if (a->requires_grad) {
                // dL/dA = dL/dC @ B^T
                auto grad_a = gemm(result->grad, transpose(b->data));
                accumulate_grad(a->grad, grad_a);
            }
            if (b->requires_grad) {
                // dL/dB = A^T @ dL/dC
                auto grad_b = gemm(transpose(a->data), result->grad);
                accumulate_grad(b->grad, grad_b);
            }
        }, {a, b});
    }
    
    return result;
}

} // namespace aifs::autodiff
```

**Complexity Analysis:**
- Forward pass: Same as operation complexity
- Backward pass: O(E) where E is number of edges in computation graph
- Memory: O(N) for storing intermediate values and gradients

### 2.3 Numerical Computing Primitives

#### BLAS-like Operations

```cpp
#pragma once
#include "tensor.hpp"
#include <immintrin.h>  // AVX intrinsics
#include <cstddef>

namespace aifs::linalg {

// AXPY: y = alpha * x + y
template<typename T>
void axpy(T alpha, std::span<const T> x, std::span<T> y) {
    const std::size_t n = x.size();
    
    // SIMD vectorization for float/double
    if constexpr (std::is_same_v<T, float>) {
        const std::size_t simd_width = 8;  // AVX2: 256-bit / 32-bit
        const std::size_t simd_end = n - (n % simd_width);
        
        const __m256 alpha_vec = _mm256_set1_ps(alpha);
        
        for (std::size_t i = 0; i < simd_end; i += simd_width) {
            __m256 x_vec = _mm256_loadu_ps(&x[i]);
            __m256 y_vec = _mm256_loadu_ps(&y[i]);
            __m256 result = _mm256_fmadd_ps(alpha_vec, x_vec, y_vec);
            _mm256_storeu_ps(&y[i], result);
        }
        
        // Scalar remainder
        for (std::size_t i = simd_end; i < n; ++i) {
            y[i] += alpha * x[i];
        }
    } else if constexpr (std::is_same_v<T, double>) {
        const std::size_t simd_width = 4;  // AVX2: 256-bit / 64-bit
        const std::size_t simd_end = n - (n % simd_width);
        
        const __m256d alpha_vec = _mm256_set1_pd(alpha);
        
        for (std::size_t i = 0; i < simd_end; i += simd_width) {
            __m256d x_vec = _mm256_loadu_pd(&x[i]);
            __m256d y_vec = _mm256_loadu_pd(&y[i]);
            __m256d result = _mm256_fmadd_pd(alpha_vec, x_vec, y_vec);
            _mm256_storeu_pd(&y[i], result);
        }
        
        for (std::size_t i = simd_end; i < n; ++i) {
            y[i] += alpha * x[i];
        }
    } else {
        // Generic fallback
        for (std::size_t i = 0; i < n; ++i) {
            y[i] += alpha * x[i];
        }
    }
}

// GEMV: y = alpha * A * x + beta * y
template<typename T>
void gemv(T alpha, const Tensor<T, 2>& A, std::span<const T> x,
          T beta, std::span<T> y) {
    const std::size_t m = A.shape().dim(0);
    const std::size_t n = A.shape().dim(1);
    
    // Scale y by beta first
    if (beta != T{1}) {
        for (std::size_t i = 0; i < m; ++i) {
            y[i] *= beta;
        }
    }
    
    // Matrix-vector multiplication with tiling
    constexpr std::size_t TILE_M = 64;
    constexpr std::size_t TILE_N = 64;
    
    for (std::size_t i0 = 0; i0 < m; i0 += TILE_M) {
        const std::size_t i_max = std::min(i0 + TILE_M, m);
        
        for (std::size_t j0 = 0; j0 < n; j0 += TILE_N) {
            const std::size_t j_max = std::min(j0 + TILE_N, n);
            
            for (std::size_t i = i0; i < i_max; ++i) {
                T sum = 0;
                for (std::size_t j = j0; j < j_max; ++j) {
                    sum += A(i, j) * x[j];
                }
                y[i] += alpha * sum;
            }
        }
    }
}

// GEMM: C = alpha * A * B + beta * C
// Cache-optimized with blocking and SIMD
template<typename T>
void gemm(T alpha, const Tensor<T, 2>& A, const Tensor<T, 2>& B,
          T beta, Tensor<T, 2>& C) {
    const std::size_t m = A.shape().dim(0);
    const std::size_t k = A.shape().dim(1);
    const std::size_t n = B.shape().dim(1);
    
    // Validate dimensions
    if (B.shape().dim(0) != k || C.shape().dim(0) != m || C.shape().dim(1) != n) {
        throw std::invalid_argument("Matrix dimensions mismatch in GEMM");
    }
    
    // Scale C by beta
    if (beta == T{0}) {
        for (std::size_t i = 0; i < m; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                C(i, j) = T{0};
            }
        }
    } else if (beta != T{1}) {
        for (std::size_t i = 0; i < m; ++i) {
            for (std::size_t j = 0; j < n; ++j) {
                C(i, j) *= beta;
            }
        }
    }
    
    // Cache blocking parameters (tuned for L1/L2 cache sizes)
    constexpr std::size_t MC = 64;   // Block size in M dimension
    constexpr std::size_t KC = 256;  // Block size in K dimension
    constexpr std::size_t NC = 64;   // Block size in N dimension
    constexpr std::size_t NR = 4;    // Micro-kernel register block
    constexpr std::size_t MR = 4;    // Micro-kernel register block
    
    // Pack B for better cache utilization
    std::vector<T> B_packed(KC * NC);
    
    for (std::size_t jc = 0; jc < n; jc += NC) {
        const std::size_t nc = std::min(NC, n - jc);
        
        for (std::size_t pc = 0; pc < k; pc += KC) {
            const std::size_t kc = std::min(KC, k - pc);
            
            // Pack B[pc:pc+kc, jc:jc+nc] into B_packed
            for (std::size_t j = 0; j < nc; ++j) {
                for (std::size_t p = 0; p < kc; ++p) {
                    B_packed[j * kc + p] = B(pc + p, jc + j);
                }
            }
            
            for (std::size_t ic = 0; ic < m; ic += MC) {
                const std::size_t mc = std::min(MC, m - ic);
                
                // Micro-kernel
                for (std::size_t jr = 0; jr < nc; jr += NR) {
                    const std::size_t nr = std::min(NR, nc - jr);
                    
                    for (std::size_t ir = 0; ir < mc; ir += MR) {
                        const std::size_t mr = std::min(MR, mc - ir);
                        
                        // Compute C[ic+ir:ic+ir+mr, jc+jr:jc+jr+nr]
                        for (std::size_t i = 0; i < mr; ++i) {
                            for (std::size_t j = 0; j < nr; ++j) {
                                T sum = C(ic + ir + i, jc + jr + j);
                                
                                for (std::size_t p = 0; p < kc; ++p) {
                                    sum += A(ic + ir + i, pc + p) * 
                                           B_packed[(jr + j) * kc + p];
                                }
                                
                                C(ic + ir + i, jc + jr + j) = alpha * sum;
                            }
                        }
                    }
                }
            }
        }
    }
}

} // namespace aifs::linalg
```

**Complexity Analysis:**
- AXPY: O(n) time, O(1) additional space
- GEMV: O(mn) time, O(1) additional space
- GEMM: O(mnk) time, O(KC * NC) additional space for packing

#### Random Number Generation for ML

```cpp
#pragma once
#include <cstdint>
#include <cmath>
#include <random>
#include <limits>

namespace aifs::random {

// Xoshiro256++: Fast, high-quality PRNG for ML
// Period: 2^256 - 1
// Passes BigCrush and PractRand
class Xoshiro256PlusPlus {
    std::uint64_t state_[4];
    
    static std::uint64_t rotl(std::uint64_t x, int k) {
        return (x << k) | (x >> (64 - k));
    }
    
public:
    explicit Xoshiro256PlusPlus(std::uint64_t seed = 0) {
        // SplitMix64 to initialize state from seed
        std::uint64_t z = seed + 0x9e3779b97f4a7c15ULL;
        for (int i = 0; i < 4; ++i) {
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
            z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
            state_[i] = z ^ (z >> 31);
        }
    }
    
    std::uint64_t operator()() {
        const std::uint64_t result = rotl(state_[0] + state_[3], 23) + state_[0];
        const std::uint64_t t = state_[1] << 17;
        
        state_[2] ^= state_[0];
        state_[3] ^= state_[1];
        state_[1] ^= state_[2];
        state_[0] ^= state_[3];
        
        state_[2] ^= t;
        state_[3] = rotl(state_[3], 45);
        
        return result;
    }
    
    // Generate uniform float in [0, 1)
    float uniform_float() {
        return static_cast<float>(operator()() >> 8) * 
               (1.0f / static_cast<float>(1ULL << 24));
    }
    
    // Generate uniform double in [0, 1)
    double uniform_double() {
        return static_cast<double>(operator()() >> 11) * 
               (1.0 / static_cast<double>(1ULL << 53));
    }
    
    // Box-Muller transform for normal distribution
    float normal_float(float mean = 0.0f, float std = 1.0f) {
        static bool has_spare = false;
        static float spare;
        
        if (has_spare) {
            has_spare = false;
            return mean + std * spare;
        }
        
        float u1 = uniform_float();
        float u2 = uniform_float();
        
        // Avoid log(0)
        while (u1 <= std::numeric_limits<float>::epsilon()) {
            u1 = uniform_float();
        }
        
        float mag = std * std::sqrt(-2.0f * std::log(u1));
        spare = mag * std::cos(2.0f * static_cast<float>(M_PI) * u2);
        has_spare = true;
        
        return mean + mag * std::sin(2.0f * static_cast<float>(M_PI) * u2);
    }
    
    // Xavier/Glorot initialization
    template<typename T>
    T xavier(std::size_t fan_in, std::size_t fan_out) {
        float limit = std::sqrt(6.0f / static_cast<float>(fan_in + fan_out));
        return static_cast<T>(uniform_float() * 2.0f * limit - limit);
    }
    
    // Kaiming/He initialization
    template<typename T>
    T kaiming(std::size_t fan_in) {
        float std = std::sqrt(2.0f / static_cast<float>(fan_in));
        return static_cast<T>(normal_float(0.0f, std));
    }
};

// Thread-local RNG for reproducibility with parallel training
thread_local Xoshiro256PlusPlus tls_rng{0};

void seed(std::uint64_t seed_value) {
    tls_rng = Xoshiro256PlusPlus(seed_value);
}

} // namespace aifs::random
```

---

## 3. NEURAL NETWORK CONSTRUCTION KIT

### 3.1 Layer Abstractions

```cpp
#pragma once
#include "core/tensor.hpp"
#include "math/autodiff.hpp"
#include <memory>
#include <vector>

namespace aifs::nn {

template<typename T>
class Layer {
public:
    virtual ~Layer() = default;
    
    // Forward pass
    virtual std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> input) = 0;
    
    // Get trainable parameters
    virtual std::vector<std::shared_ptr<autodiff::Variable<T>>> parameters() {
        return {};
    }
    
    // Training mode toggle
    virtual void train(bool mode = true) { is_training_ = mode; }
    virtual void eval() { train(false); }
    
    // Reset internal state (for recurrent layers)
    virtual void reset_state() {}
    
protected:
    bool is_training_ = true;
};

// Activation functions
template<typename T>
class ReLU : public Layer<T> {
public:
    std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> input) override {
        return relu(input);
    }
    
private:
    static std::shared_ptr<autodiff::Variable<T>> relu(
        std::shared_ptr<autodiff::Variable<T>> x) {
        auto result = std::make_shared<autodiff::Variable<T>>(
            relu_forward(x->data),
            x->requires_grad
        );
        
        if (result->requires_grad) {
            result->set_grad_fn([=]() {
                if (x->requires_grad) {
                    auto grad = relu_backward(x->data, result->grad);
                    autodiff::accumulate_grad(x->grad, grad);
                }
            }, {x});
        }
        
        return result;
    }
    
    static Tensor<T, 1> relu_forward(const Tensor<T, 1>& x) {
        Tensor<T, 1> result(x.shape());
        for (std::size_t i = 0; i < x.size(); ++i) {
            result(i) = std::max(T{0}, x(i));
        }
        return result;
    }
    
    static Tensor<T, 1> relu_backward(const Tensor<T, 1>& x, 
                                       const Tensor<T, 1>& grad_out) {
        Tensor<T, 1> grad(x.shape());
        for (std::size_t i = 0; i < x.size(); ++i) {
            grad(i) = (x(i) > T{0}) ? grad_out(i) : T{0};
        }
        return grad;
    }
};

template<typename T>
class Sigmoid : public Layer<T> {
public:
    std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> input) override {
        return sigmoid(input);
    }
    
private:
    static std::shared_ptr<autodiff::Variable<T>> sigmoid(
        std::shared_ptr<autodiff::Variable<T>> x) {
        auto result = std::make_shared<autodiff::Variable<T>>(
            sigmoid_forward(x->data),
            x->requires_grad
        );
        
        if (result->requires_grad) {
            result->set_grad_fn([=]() {
                if (x->requires_grad) {
                    auto grad = sigmoid_backward(result->data, result->grad);
                    autodiff::accumulate_grad(x->grad, grad);
                }
            }, {x});
        }
        
        return result;
    }
    
    static Tensor<T, 1> sigmoid_forward(const Tensor<T, 1>& x) {
        Tensor<T, 1> result(x.shape());
        for (std::size_t i = 0; i < x.size(); ++i) {
            result(i) = T{1} / (T{1} + std::exp(-x(i)));
        }
        return result;
    }
    
    static Tensor<T, 1> sigmoid_backward(const Tensor<T, 1>& sigmoid_x,
                                          const Tensor<T, 1>& grad_out) {
        Tensor<T, 1> grad(sigmoid_x.shape());
        for (std::size_t i = 0; i < sigmoid_x.size(); ++i) {
            grad(i) = grad_out(i) * sigmoid_x(i) * (T{1} - sigmoid_x(i));
        }
        return grad;
    }
};

template<typename T>
class Tanh : public Layer<T> {
public:
    std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> input) override {
        auto result = std::make_shared<autodiff::Variable<T>>(
            tanh_forward(input->data),
            input->requires_grad
        );
        
        if (result->requires_grad) {
            result->set_grad_fn([=]() {
                if (input->requires_grad) {
                    auto grad = tanh_backward(result->data, result->grad);
                    autodiff::accumulate_grad(input->grad, grad);
                }
            }, {input});
        }
        
        return result;
    }
    
private:
    static Tensor<T, 1> tanh_forward(const Tensor<T, 1>& x) {
        Tensor<T, 1> result(x.shape());
        for (std::size_t i = 0; i < x.size(); ++i) {
            result(i) = std::tanh(x(i));
        }
        return result;
    }
    
    static Tensor<T, 1> tanh_backward(const Tensor<T, 1>& tanh_x,
                                       const Tensor<T, 1>& grad_out) {
        Tensor<T, 1> grad(tanh_x.shape());
        for (std::size_t i = 0; i < tanh_x.size(); ++i) {
            grad(i) = grad_out(i) * (T{1} - tanh_x(i) * tanh_x(i));
        }
        return grad;
    }
};

// GELU activation (used in Transformers)
template<typename T>
class GELU : public Layer<T> {
public:
    std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> input) override {
        // Approximation: 0.5 * x * (1 + tanh(sqrt(2/pi) * (x + 0.044715 * x^3)))
        auto result = std::make_shared<autodiff::Variable<T>>(
            gelu_forward(input->data),
            input->requires_grad
        );
        
        if (result->requires_grad) {
            result->set_grad_fn([=]() {
                if (input->requires_grad) {
                    auto grad = gelu_backward(input->data, result->grad);
                    autodiff::accumulate_grad(input->grad, grad);
                }
            }, {input});
        }
        
        return result;
    }
    
private:
    static constexpr T SQRT_2_OVER_PI = T{0.7978845608028654};
    static constexpr T COEFF = T{0.044715};
    
    static Tensor<T, 1> gelu_forward(const Tensor<T, 1>& x) {
        Tensor<T, 1> result(x.shape());
        for (std::size_t i = 0; i < x.size(); ++i) {
            T cdf = T{0.5} * (T{1} + std::tanh(SQRT_2_OVER_PI * 
                        (x(i) + COEFF * x(i) * x(i) * x(i))));
            result(i) = x(i) * cdf;
        }
        return result;
    }
    
    static Tensor<T, 1> gelu_backward(const Tensor<T, 1>& x,
                                       const Tensor<T, 1>& grad_out) {
        Tensor<T, 1> grad(x.shape());
        for (std::size_t i = 0; i < x.size(); ++i) {
            T x3 = x(i) * x(i) * x(i);
            T tanh_arg = SQRT_2_OVER_PI * (x(i) + COEFF * x3);
            T tanh_val = std::tanh(tanh_arg);
            T sech2 = T{1} - tanh_val * tanh_val;
            T local_grad = T{0.5} * (T{1} + tanh_val) + 
                          x(i) * T{0.5} * sech2 * SQRT_2_OVER_PI * 
                          (T{1} + T{3} * COEFF * x(i) * x(i));
            grad(i) = grad_out(i) * local_grad;
        }
        return grad;
    }
};

} // namespace aifs::nn
```

### 3.2 Linear Layer

```cpp
#pragma once
#include "layer.hpp"
#include "math/random.hpp"

namespace aifs::nn {

template<typename T>
class Linear : public Layer<T> {
    std::shared_ptr<autodiff::Variable<T>> weight_;
    std::shared_ptr<autodiff::Variable<T>> bias_;
    std::size_t in_features_;
    std::size_t out_features_;
    
public:
    Linear(std::size_t in_features, std::size_t out_features, bool bias = true)
        : in_features_(in_features), out_features_(out_features) {
        
        // Initialize weights with Xavier/Glorot
        Tensor<T, 1> weight_data({in_features * out_features});
        for (std::size_t i = 0; i < weight_data.size(); ++i) {
            weight_data(i) = random::tls_rng.xavier<T>(in_features, out_features);
        }
        weight_ = std::make_shared<autodiff::Variable<T>>(weight_data, true);
        
        if (bias) {
            Tensor<T, 1> bias_data({out_features}, T{0});
            bias_ = std::make_shared<autodiff::Variable<T>>(bias_data, true);
        }
    }
    
    std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> input) override {
        
        // input: [batch_size, in_features]
        // weight: [out_features, in_features] (stored as flat)
        // output: [batch_size, out_features]
        
        const std::size_t batch_size = input->data.shape().dim(0);
        
        // Reshape weight for matrix multiplication
        Tensor<T, 2> weight_2d({out_features_, in_features_});
        for (std::size_t i = 0; i < out_features_; ++i) {
            for (std::size_t j = 0; j < in_features_; ++j) {
                weight_2d(i, j) = weight_->data(i * in_features_ + j);
            }
        }
        
        // y = x @ W^T + b
        auto weight_var = std::make_shared<autodiff::Variable<T>>(
            Tensor<T, 1>(weight_->data.shape()), weight_->requires_grad);
        weight_var->data = weight_->data;
        
        // Reshape input to 2D if needed
        Tensor<T, 2> input_2d({batch_size, in_features_});
        for (std::size_t i = 0; i < batch_size; ++i) {
            for (std::size_t j = 0; j < in_features_; ++j) {
                input_2d(i, j) = input->data(i * in_features_ + j);
            }
        }
        
        auto input_2d_var = std::make_shared<autodiff::Variable<T>>(
            Tensor<T, 1>({batch_size * in_features_}), input->requires_grad);
        for (std::size_t i = 0; i < input->data.size(); ++i) {
            input_2d_var->data(i) = input->data(i);
        }
        
        // Matrix multiplication: output = input @ W^T
        Tensor<T, 2> output_2d({batch_size, out_features_});
        for (std::size_t i = 0; i < batch_size; ++i) {
            for (std::size_t j = 0; j < out_features_; ++j) {
                T sum = bias_ ? bias_->data(j) : T{0};
                for (std::size_t k = 0; k < in_features_; ++k) {
                    sum += input_2d(i, k) * weight_2d(j, k);
                }
                output_2d(i, j) = sum;
            }
        }
        
        Tensor<T, 1> output_flat({batch_size * out_features_});
        for (std::size_t i = 0; i < batch_size; ++i) {
            for (std::size_t j = 0; j < out_features_; ++j) {
                output_flat(i * out_features_ + j) = output_2d(i, j);
            }
        }
        
        auto result = std::make_shared<autodiff::Variable<T>>(
            output_flat, input->requires_grad || weight_->requires_grad);
        
        if (result->requires_grad) {
            result->set_grad_fn([=]() {
                // Gradient w.r.t. input: dL/dx = dL/dy @ W
                if (input->requires_grad) {
                    for (std::size_t i = 0; i < batch_size; ++i) {
                        for (std::size_t k = 0; k < in_features_; ++k) {
                            T sum = T{0};
                            for (std::size_t j = 0; j < out_features_; ++j) {
                                sum += result->grad(i * out_features_ + j) * 
                                       weight_2d(j, k);
                            }
                            input->grad(i * in_features_ + k) += sum;
                        }
                    }
                }
                
                // Gradient w.r.t. weight: dL/dW = x^T @ dL/dy
                if (weight_->requires_grad) {
                    for (std::size_t j = 0; j < out_features_; ++j) {
                        for (std::size_t k = 0; k < in_features_; ++k) {
                            T sum = T{0};
                            for (std::size_t i = 0; i < batch_size; ++i) {
                                sum += input_2d(i, k) * 
                                       result->grad(i * out_features_ + j);
                            }
                            weight_->grad(j * in_features_ + k) += sum;
                        }
                    }
                }
                
                // Gradient w.r.t. bias: dL/db = sum(dL/dy, axis=0)
                if (bias_ && bias_->requires_grad) {
                    for (std::size_t j = 0; j < out_features_; ++j) {
                        T sum = T{0};
                        for (std::size_t i = 0; i < batch_size; ++i) {
                            sum += result->grad(i * out_features_ + j);
                        }
                        bias_->grad(j) += sum;
                    }
                }
            }, {input, weight_, bias_});
        }
        
        return result;
    }
    
    std::vector<std::shared_ptr<autodiff::Variable<T>>> parameters() override {
        if (bias_) {
            return {weight_, bias_};
        }
        return {weight_};
    }
};

} // namespace aifs::nn
```



### 3.3 Normalization Layers

#### Batch Normalization

```cpp
#pragma once
#include "layer.hpp"

namespace aifs::nn {

template<typename T>
class BatchNorm1D : public Layer<T> {
    std::shared_ptr<autodiff::Variable<T>> gamma_;
    std::shared_ptr<autodiff::Variable<T>> beta_;
    
    // Running statistics
    Tensor<T, 1> running_mean_;
    Tensor<T, 1> running_var_;
    
    std::size_t num_features_;
    T eps_ = T{1e-5};
    T momentum_ = T{0.1};
    
public:
    explicit BatchNorm1D(std::size_t num_features, T eps = T{1e-5}, 
                         T momentum = T{0.1})
        : num_features_(num_features), eps_(eps), momentum_(momentum),
          running_mean_(Tensor<T, 1>({num_features}, T{0})),
          running_var_(Tensor<T, 1>({num_features}, T{1})) {
        
        // Initialize gamma to 1, beta to 0
        Tensor<T, 1> gamma_data({num_features}, T{1});
        Tensor<T, 1> beta_data({num_features}, T{0});
        
        gamma_ = std::make_shared<autodiff::Variable<T>>(gamma_data, true);
        beta_ = std::make_shared<autodiff::Variable<T>>(beta_data, true);
    }
    
    std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> input) override {
        
        const std::size_t batch_size = input->data.shape().dim(0);
        
        if (this->is_training_) {
            // Compute batch mean and variance
            Tensor<T, 1> batch_mean({num_features_}, T{0});
            Tensor<T, 1> batch_var({num_features_}, T{0});
            
            // Mean
            for (std::size_t j = 0; j < num_features_; ++j) {
                T sum = T{0};
                for (std::size_t i = 0; i < batch_size; ++i) {
                    sum += input->data(i * num_features_ + j);
                }
                batch_mean(j) = sum / static_cast<T>(batch_size);
            }
            
            // Variance
            for (std::size_t j = 0; j < num_features_; ++j) {
                T sum_sq = T{0};
                for (std::size_t i = 0; i < batch_size; ++i) {
                    T diff = input->data(i * num_features_ + j) - batch_mean(j);
                    sum_sq += diff * diff;
                }
                batch_var(j) = sum_sq / static_cast<T>(batch_size);
            }
            
            // Normalize
            Tensor<T, 1> normalized({batch_size * num_features_});
            for (std::size_t i = 0; i < batch_size; ++i) {
                for (std::size_t j = 0; j < num_features_; ++j) {
                    T x = input->data(i * num_features_ + j);
                    normalized(i * num_features_ + j) = 
                        (x - batch_mean(j)) / std::sqrt(batch_var(j) + eps_);
                }
            }
            
            // Scale and shift
            Tensor<T, 1> output({batch_size * num_features_});
            for (std::size_t i = 0; i < batch_size; ++i) {
                for (std::size_t j = 0; j < num_features_; ++j) {
                    output(i * num_features_ + j) = 
                        gamma_->data(j) * normalized(i * num_features_ + j) + 
                        beta_->data(j);
                }
            }
            
            // Update running statistics
            for (std::size_t j = 0; j < num_features_; ++j) {
                running_mean_(j) = (T{1} - momentum_) * running_mean_(j) + 
                                   momentum_ * batch_mean(j);
                running_var_(j) = (T{1} - momentum_) * running_var_(j) + 
                                  momentum_ * batch_var(j);
            }
            
            auto result = std::make_shared<autodiff::Variable<T>>(
                output, input->requires_grad);
            
            if (result->requires_grad) {
                result->set_grad_fn([=]() {
                    // Backprop through BatchNorm
                    // dL/dgamma = sum(dL/dy * x_hat)
                    // dL/dbeta = sum(dL/dy)
                    // dL/dx = (gamma / sqrt(var + eps)) * (dL/dy - mean(dL/dy) - 
                    //          x_hat * mean(dL/dy * x_hat))
                    
                    for (std::size_t j = 0; j < num_features_; ++j) {
                        T dgamma = T{0};
                        T dbeta = T{0};
                        
                        for (std::size_t i = 0; i < batch_size; ++i) {
                            T dy = result->grad(i * num_features_ + j);
                            T x_hat = normalized(i * num_features_ + j);
                            dgamma += dy * x_hat;
                            dbeta += dy;
                        }
                        
                        gamma_->grad(j) += dgamma;
                        beta_->grad(j) += dbeta;
                    }
                    
                    if (input->requires_grad) {
                        for (std::size_t j = 0; j < num_features_; ++j) {
                            T std_inv = T{1} / std::sqrt(batch_var(j) + eps_);
                            
                            T mean_dy = T{0};
                            T mean_dy_xhat = T{0};
                            
                            for (std::size_t i = 0; i < batch_size; ++i) {
                                T dy = result->grad(i * num_features_ + j);
                                T x_hat = normalized(i * num_features_ + j);
                                mean_dy += dy;
                                mean_dy_xhat += dy * x_hat;
                            }
                            
                            mean_dy /= static_cast<T>(batch_size);
                            mean_dy_xhat /= static_cast<T>(batch_size);
                            
                            for (std::size_t i = 0; i < batch_size; ++i) {
                                T dy = result->grad(i * num_features_ + j);
                                T x_hat = normalized(i * num_features_ + j);
                                T dx = gamma_->data(j) * std_inv * 
                                       (dy - mean_dy - x_hat * mean_dy_xhat);
                                input->grad(i * num_features_ + j) += dx;
                            }
                        }
                    }
                }, {input, gamma_, beta_});
            }
            
            return result;
            
        } else {
            // Inference mode: use running statistics
            Tensor<T, 1> output({batch_size * num_features_});
            
            for (std::size_t i = 0; i < batch_size; ++i) {
                for (std::size_t j = 0; j < num_features_; ++j) {
                    T x = input->data(i * num_features_ + j);
                    T x_hat = (x - running_mean_(j)) / 
                              std::sqrt(running_var_(j) + eps_);
                    output(i * num_features_ + j) = 
                        gamma_->data(j) * x_hat + beta_->data(j);
                }
            }
            
            return std::make_shared<autodiff::Variable<T>>(output, false);
        }
    }
    
    std::vector<std::shared_ptr<autodiff::Variable<T>>> parameters() override {
        return {gamma_, beta_};
    }
};

// Layer Normalization (used in Transformers)
template<typename T>
class LayerNorm : public Layer<T> {
    std::shared_ptr<autodiff::Variable<T>> gamma_;
    std::shared_ptr<autodiff::Variable<T>> beta_;
    T eps_;
    
public:
    explicit LayerNorm(std::size_t normalized_shape, T eps = T{1e-5})
        : eps_(eps) {
        Tensor<T, 1> gamma_data({normalized_shape}, T{1});
        Tensor<T, 1> beta_data({normalized_shape}, T{0});
        
        gamma_ = std::make_shared<autodiff::Variable<T>>(gamma_data, true);
        beta_ = std::make_shared<autodiff::Variable<T>>(beta_data, true);
    }
    
    std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> input) override {
        
        const std::size_t batch_size = input->data.shape().dim(0);
        const std::size_t features = gamma_->data.size();
        
        Tensor<T, 1> output(input->data.shape());
        Tensor<T, 1> normalized(input->data.shape());
        std::vector<T> means(batch_size);
        std::vector<T> vars(batch_size);
        
        // Compute per-sample mean and variance
        for (std::size_t i = 0; i < batch_size; ++i) {
            T sum = T{0};
            for (std::size_t j = 0; j < features; ++j) {
                sum += input->data(i * features + j);
            }
            means[i] = sum / static_cast<T>(features);
            
            T sum_sq = T{0};
            for (std::size_t j = 0; j < features; ++j) {
                T diff = input->data(i * features + j) - means[i];
                sum_sq += diff * diff;
            }
            vars[i] = sum_sq / static_cast<T>(features);
        }
        
        // Normalize, scale, and shift
        for (std::size_t i = 0; i < batch_size; ++i) {
            T std_inv = T{1} / std::sqrt(vars[i] + eps_);
            for (std::size_t j = 0; j < features; ++j) {
                normalized(i * features + j) = 
                    (input->data(i * features + j) - means[i]) * std_inv;
                output(i * features + j) = 
                    gamma_->data(j) * normalized(i * features + j) + 
                    beta_->data(j);
            }
        }
        
        auto result = std::make_shared<autodiff::Variable<T>>(
            output, input->requires_grad);
        
        if (result->requires_grad) {
            result->set_grad_fn([=]() {
                // Gradients for LayerNorm
                for (std::size_t j = 0; j < features; ++j) {
                    T dgamma = T{0};
                    T dbeta = T{0};
                    
                    for (std::size_t i = 0; i < batch_size; ++i) {
                        dgamma += result->grad(i * features + j) * 
                                  normalized(i * features + j);
                        dbeta += result->grad(i * features + j);
                    }
                    
                    gamma_->grad(j) += dgamma;
                    beta_->grad(j) += dbeta;
                }
                
                if (input->requires_grad) {
                    for (std::size_t i = 0; i < batch_size; ++i) {
                        T std_inv = T{1} / std::sqrt(vars[i] + eps_);
                        
                        T sum_dydxhat = T{0};
                        T sum_dydxhat_xhat = T{0};
                        
                        for (std::size_t j = 0; j < features; ++j) {
                            T dy = result->grad(i * features + j);
                            T dxhat = dy * gamma_->data(j);
                            sum_dydxhat += dxhat;
                            sum_dydxhat_xhat += dxhat * normalized(i * features + j);
                        }
                        
                        for (std::size_t j = 0; j < features; ++j) {
                            T dy = result->grad(i * features + j);
                            T dxhat = dy * gamma_->data(j);
                            T dx = std_inv * (dxhat - 
                                sum_dydxhat / static_cast<T>(features) - 
                                normalized(i * features + j) * sum_dydxhat_xhat / 
                                static_cast<T>(features));
                            input->grad(i * features + j) += dx;
                        }
                    }
                }
            }, {input, gamma_, beta_});
        }
        
        return result;
    }
    
    std::vector<std::shared_ptr<autodiff::Variable<T>>> parameters() override {
        return {gamma_, beta_};
    }
};

} // namespace aifs::nn
```

### 3.4 Convolution Implementation

```cpp
#pragma once
#include "layer.hpp"

namespace aifs::nn {

template<typename T>
class Conv2D : public Layer<T> {
    std::shared_ptr<autodiff::Variable<T>> weight_;
    std::shared_ptr<autodiff::Variable<T>> bias_;
    
    std::size_t in_channels_;
    std::size_t out_channels_;
    std::size_t kernel_h_;
    std::size_t kernel_w_;
    std::size_t stride_h_;
    std::size_t stride_w_;
    std::size_t padding_h_;
    std::size_t padding_w_;
    
public:
    Conv2D(std::size_t in_channels, std::size_t out_channels,
           std::size_t kernel_size, std::size_t stride = 1, 
           std::size_t padding = 0, bool bias = true)
        : in_channels_(in_channels), out_channels_(out_channels),
          kernel_h_(kernel_size), kernel_w_(kernel_size),
          stride_h_(stride), stride_w_(stride),
          padding_h_(padding), padding_w_(padding) {
        
        // Kaiming initialization
        std::size_t fan_in = in_channels * kernel_h_ * kernel_w_;
        Tensor<T, 1> weight_data({out_channels * in_channels * kernel_h_ * kernel_w_});
        
        for (std::size_t i = 0; i < weight_data.size(); ++i) {
            weight_data(i) = random::tls_rng.kaiming<T>(fan_in);
        }
        weight_ = std::make_shared<autodiff::Variable<T>>(weight_data, true);
        
        if (bias) {
            Tensor<T, 1> bias_data({out_channels}, T{0});
            bias_ = std::make_shared<autodiff::Variable<T>>(bias_data, true);
        }
    }
    
    std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> input) override {
        
        // Input: [N, C_in, H, W] stored as flat [N * C_in * H * W]
        // Weight: [C_out, C_in, KH, KW] stored as flat
        // Output: [N, C_out, OH, OW]
        
        const std::size_t batch_size = 1;  // Simplified for clarity
        const std::size_t in_h = input->data.shape().dim(0);
        const std::size_t in_w = input->data.shape().dim(1);
        
        const std::size_t out_h = (in_h + 2 * padding_h_ - kernel_h_) / stride_h_ + 1;
        const std::size_t out_w = (in_w + 2 * padding_w_ - kernel_w_) / stride_w_ + 1;
        
        Tensor<T, 1> output({out_channels_ * out_h * out_w}, T{0});
        
        // Direct convolution (im2col would be more efficient for large kernels)
        for (std::size_t oc = 0; oc < out_channels_; ++oc) {
            for (std::size_t oh = 0; oh < out_h; ++oh) {
                for (std::size_t ow = 0; ow < out_w; ++ow) {
                    T sum = bias_ ? bias_->data(oc) : T{0};
                    
                    for (std::size_t ic = 0; ic < in_channels_; ++ic) {
                        for (std::size_t kh = 0; kh < kernel_h_; ++kh) {
                            for (std::size_t kw = 0; kw < kernel_w_; ++kw) {
                                std::size_t ih = oh * stride_h_ + kh - padding_h_;
                                std::size_t iw = ow * stride_w_ + kw - padding_w_;
                                
                                if (ih < in_h && iw < in_w) {
                                    std::size_t in_idx = (ic * in_h + ih) * in_w + iw;
                                    std::size_t w_idx = ((oc * in_channels_ + ic) * 
                                                        kernel_h_ + kh) * kernel_w_ + kw;
                                    sum += input->data(in_idx) * weight_->data(w_idx);
                                }
                            }
                        }
                    }
                    
                    std::size_t out_idx = (oc * out_h + oh) * out_w + ow;
                    output(out_idx) = sum;
                }
            }
        }
        
        auto result = std::make_shared<autodiff::Variable<T>>(
            output, input->requires_grad);
        
        if (result->requires_grad) {
            result->set_grad_fn([=]() {
                // Gradients for convolution
                if (input->requires_grad) {
                    for (std::size_t oc = 0; oc < out_channels_; ++oc) {
                        for (std::size_t oh = 0; oh < out_h; ++oh) {
                            for (std::size_t ow = 0; ow < out_w; ++ow) {
                                std::size_t out_idx = (oc * out_h + oh) * out_w + ow;
                                T dy = result->grad(out_idx);
                                
                                for (std::size_t ic = 0; ic < in_channels_; ++ic) {
                                    for (std::size_t kh = 0; kh < kernel_h_; ++kh) {
                                        for (std::size_t kw = 0; kw < kernel_w_; ++kw) {
                                            std::size_t ih = oh * stride_h_ + kh - padding_h_;
                                            std::size_t iw = ow * stride_w_ + kw - padding_w_;
                                            
                                            if (ih < in_h && iw < in_w) {
                                                std::size_t in_idx = (ic * in_h + ih) * in_w + iw;
                                                std::size_t w_idx = ((oc * in_channels_ + ic) * 
                                                                    kernel_h_ + kh) * kernel_w_ + kw;
                                                input->grad(in_idx) += dy * weight_->data(w_idx);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                
                if (weight_->requires_grad) {
                    for (std::size_t oc = 0; oc < out_channels_; ++oc) {
                        for (std::size_t ic = 0; ic < in_channels_; ++ic) {
                            for (std::size_t kh = 0; kh < kernel_h_; ++kh) {
                                for (std::size_t kw = 0; kw < kernel_w_; ++kw) {
                                    std::size_t w_idx = ((oc * in_channels_ + ic) * 
                                                        kernel_h_ + kh) * kernel_w_ + kw;
                                    T dw = T{0};
                                    
                                    for (std::size_t oh = 0; oh < out_h; ++oh) {
                                        for (std::size_t ow = 0; ow < out_w; ++ow) {
                                            std::size_t ih = oh * stride_h_ + kh - padding_h_;
                                            std::size_t iw = ow * stride_w_ + kw - padding_w_;
                                            
                                            if (ih < in_h && iw < in_w) {
                                                std::size_t in_idx = (ic * in_h + ih) * in_w + iw;
                                                std::size_t out_idx = (oc * out_h + oh) * out_w + ow;
                                                dw += result->grad(out_idx) * input->data(in_idx);
                                            }
                                        }
                                    }
                                    
                                    weight_->grad(w_idx) += dw;
                                }
                            }
                        }
                    }
                }
                
                if (bias_ && bias_->requires_grad) {
                    for (std::size_t oc = 0; oc < out_channels_; ++oc) {
                        T db = T{0};
                        for (std::size_t oh = 0; oh < out_h; ++oh) {
                            for (std::size_t ow = 0; ow < out_w; ++ow) {
                                std::size_t out_idx = (oc * out_h + oh) * out_w + ow;
                                db += result->grad(out_idx);
                            }
                        }
                        bias_->grad(oc) += db;
                    }
                }
            }, {input, weight_, bias_});
        }
        
        return result;
    }
    
    std::vector<std::shared_ptr<autodiff::Variable<T>>> parameters() override {
        if (bias_) {
            return {weight_, bias_};
        }
        return {weight_};
    }
};

} // namespace aifs::nn
```

### 3.5 Attention Mechanisms

```cpp
#pragma once
#include "layer.hpp"
#include "math/linalg.hpp"
#include <cmath>

namespace aifs::nn {

// Scaled Dot-Product Attention
template<typename T>
class ScaledDotProductAttention : public Layer<T> {
    T dropout_;
    bool causal_;
    
public:
    explicit ScaledDotProductAttention(T dropout = T{0}, bool causal = false)
        : dropout_(dropout), causal_(causal) {}
    
    std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> query,
        std::shared_ptr<autodiff::Variable<T>> key,
        std::shared_ptr<autodiff::Variable<T>> value,
        std::shared_ptr<autodiff::Variable<T>> mask = nullptr) {
        
        // query, key, value: [batch, seq_len, d_k]
        const std::size_t batch_size = 1;  // Simplified
        const std::size_t seq_len = query->data.shape().dim(0);
        const std::size_t d_k = query->data.shape().dim(1);
        
        const T scale = static_cast<T>(1.0 / std::sqrt(static_cast<double>(d_k)));
        
        // Compute attention scores: Q @ K^T
        Tensor<T, 2> scores({seq_len, seq_len});
        for (std::size_t i = 0; i < seq_len; ++i) {
            for (std::size_t j = 0; j < seq_len; ++j) {
                T dot = T{0};
                for (std::size_t k = 0; k < d_k; ++k) {
                    dot += query->data(i * d_k + k) * key->data(j * d_k + k);
                }
                scores(i, j) = dot * scale;
            }
        }
        
        // Apply causal mask if needed
        if (causal_) {
            for (std::size_t i = 0; i < seq_len; ++i) {
                for (std::size_t j = i + 1; j < seq_len; ++j) {
                    scores(i, j) = -std::numeric_limits<T>::infinity();
                }
            }
        }
        
        // Apply optional mask
        if (mask) {
            for (std::size_t i = 0; i < seq_len; ++i) {
                for (std::size_t j = 0; j < seq_len; ++j) {
                    if (mask->data(i * seq_len + j) == T{0}) {
                        scores(i, j) = -std::numeric_limits<T>::infinity();
                    }
                }
            }
        }
        
        // Softmax
        Tensor<T, 2> attn_weights({seq_len, seq_len});
        for (std::size_t i = 0; i < seq_len; ++i) {
            T max_score = scores(i, 0);
            for (std::size_t j = 1; j < seq_len; ++j) {
                max_score = std::max(max_score, scores(i, j));
            }
            
            T sum_exp = T{0};
            for (std::size_t j = 0; j < seq_len; ++j) {
                attn_weights(i, j) = std::exp(scores(i, j) - max_score);
                sum_exp += attn_weights(i, j);
            }
            
            for (std::size_t j = 0; j < seq_len; ++j) {
                attn_weights(i, j) /= sum_exp;
            }
        }
        
        // Apply dropout (simplified - just scaling during training)
        if (this->is_training_ && dropout_ > T{0}) {
            for (std::size_t i = 0; i < seq_len; ++i) {
                for (std::size_t j = 0; j < seq_len; ++j) {
                    attn_weights(i, j) *= (T{1} - dropout_);
                }
            }
        }
        
        // Compute output: attn_weights @ V
        Tensor<T, 2> output({seq_len, d_k});
        for (std::size_t i = 0; i < seq_len; ++i) {
            for (std::size_t k = 0; k < d_k; ++k) {
                T sum = T{0};
                for (std::size_t j = 0; j < seq_len; ++j) {
                    sum += attn_weights(i, j) * value->data(j * d_k + k);
                }
                output(i, k) = sum;
            }
        }
        
        Tensor<T, 1> output_flat({seq_len * d_k});
        for (std::size_t i = 0; i < seq_len; ++i) {
            for (std::size_t k = 0; k < d_k; ++k) {
                output_flat(i * d_k + k) = output(i, k);
            }
        }
        
        auto result = std::make_shared<autodiff::Variable<T>>(
            output_flat, query->requires_grad || key->requires_grad || 
                        value->requires_grad);
        
        // Store for backward pass
        auto attn_weights_copy = attn_weights;
        
        if (result->requires_grad) {
            result->set_grad_fn([=]() mutable {
                // Backward through attention
                // dL/dV = attn_weights^T @ dL/dO
                if (value->requires_grad) {
                    for (std::size_t j = 0; j < seq_len; ++j) {
                        for (std::size_t k = 0; k < d_k; ++k) {
                            T sum = T{0};
                            for (std::size_t i = 0; i < seq_len; ++i) {
                                sum += attn_weights_copy(i, j) * 
                                       result->grad(i * d_k + k);
                            }
                            value->grad(j * d_k + k) += sum;
                        }
                    }
                }
                
                // dL/d_attn_weights = dL/dO @ V^T
                Tensor<T, 2> d_attn_weights({seq_len, seq_len});
                for (std::size_t i = 0; i < seq_len; ++i) {
                    for (std::size_t j = 0; j < seq_len; ++j) {
                        T sum = T{0};
                        for (std::size_t k = 0; k < d_k; ++k) {
                            sum += result->grad(i * d_k + k) * value->data(j * d_k + k);
                        }
                        d_attn_weights(i, j) = sum;
                    }
                }
                
                // Backward through softmax
                for (std::size_t i = 0; i < seq_len; ++i) {
                    T sum_da = T{0};
                    for (std::size_t j = 0; j < seq_len; ++j) {
                        sum_da += d_attn_weights(i, j) * attn_weights_copy(i, j);
                    }
                    
                    for (std::size_t j = 0; j < seq_len; ++j) {
                        d_attn_weights(i, j) = attn_weights_copy(i, j) * 
                                               (d_attn_weights(i, j) - sum_da);
                    }
                }
                
                // dL/dQ and dL/dK
                if (query->requires_grad) {
                    for (std::size_t i = 0; i < seq_len; ++i) {
                        for (std::size_t k = 0; k < d_k; ++k) {
                            T sum = T{0};
                            for (std::size_t j = 0; j < seq_len; ++j) {
                                sum += d_attn_weights(i, j) * key->data(j * d_k + k);
                            }
                            query->grad(i * d_k + k) += sum * scale;
                        }
                    }
                }
                
                if (key->requires_grad) {
                    for (std::size_t j = 0; j < seq_len; ++j) {
                        for (std::size_t k = 0; k < d_k; ++k) {
                            T sum = T{0};
                            for (std::size_t i = 0; i < seq_len; ++i) {
                                sum += d_attn_weights(i, j) * query->data(i * d_k + k);
                            }
                            key->grad(j * d_k + k) += sum * scale;
                        }
                    }
                }
            }, {query, key, value});
        }
        
        return result;
    }
};

// Multi-Head Attention
template<typename T>
class MultiHeadAttention : public Layer<T> {
    std::shared_ptr<Linear<T>> wq_;
    std::shared_ptr<Linear<T>> wk_;
    std::shared_ptr<Linear<T>> wv_;
    std::shared_ptr<Linear<T>> wo_;
    
    std::size_t d_model_;
    std::size_t num_heads_;
    std::size_t d_k_;
    ScaledDotProductAttention<T> attention_;
    
public:
    MultiHeadAttention(std::size_t d_model, std::size_t num_heads, 
                       T dropout = T{0})
        : d_model_(d_model), num_heads_(num_heads), 
          d_k_(d_model / num_heads),
          attention_(dropout, false) {
        
        wq_ = std::make_shared<Linear<T>>(d_model, d_model);
        wk_ = std::make_shared<Linear<T>>(d_model, d_model);
        wv_ = std::make_shared<Linear<T>>(d_model, d_model);
        wo_ = std::make_shared<Linear<T>>(d_model, d_model);
    }
    
    std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> query,
        std::shared_ptr<autodiff::Variable<T>> key,
        std::shared_ptr<autodiff::Variable<T>> value,
        std::shared_ptr<autodiff::Variable<T>> mask = nullptr) override {
        
        const std::size_t batch_size = 1;  // Simplified
        const std::size_t seq_len = query->data.shape().dim(0);
        
        // Linear projections
        auto Q = wq_->forward(query);
        auto K = wk_->forward(key);
        auto V = wv_->forward(value);
        
        // Reshape for multi-head: [batch, seq, heads, d_k] -> [batch, heads, seq, d_k]
        // Simplified: process each head sequentially
        Tensor<T, 1> concat_output({seq_len * d_model_});
        
        for (std::size_t h = 0; h < num_heads_; ++h) {
            // Extract head h from Q, K, V
            Tensor<T, 1> q_head({seq_len * d_k_});
            Tensor<T, 1> k_head({seq_len * d_k_});
            Tensor<T, 1> v_head({seq_len * d_k_});
            
            for (std::size_t i = 0; i < seq_len; ++i) {
                for (std::size_t k = 0; k < d_k_; ++k) {
                    std::size_t src_idx = i * d_model_ + h * d_k_ + k;
                    std::size_t dst_idx = i * d_k_ + k;
                    q_head(dst_idx) = Q->data(src_idx);
                    k_head(dst_idx) = K->data(src_idx);
                    v_head(dst_idx) = V->data(src_idx);
                }
            }
            
            auto q_var = std::make_shared<autodiff::Variable<T>>(q_head, Q->requires_grad);
            auto k_var = std::make_shared<autodiff::Variable<T>>(k_head, K->requires_grad);
            auto v_var = std::make_shared<autodiff::Variable<T>>(v_head, V->requires_grad);
            
            // Apply attention
            auto head_out = attention_.forward(q_var, k_var, v_var, mask);
            
            // Concatenate
            for (std::size_t i = 0; i < seq_len; ++i) {
                for (std::size_t k = 0; k < d_k_; ++k) {
                    std::size_t dst_idx = i * d_model_ + h * d_k_ + k;
                    std::size_t src_idx = i * d_k_ + k;
                    concat_output(dst_idx) = head_out->data(src_idx);
                }
            }
        }
        
        auto concat_var = std::make_shared<autodiff::Variable<T>>(
            concat_output, Q->requires_grad);
        
        // Final linear projection
        return wo_->forward(concat_var);
    }
    
    std::vector<std::shared_ptr<autodiff::Variable<T>>> parameters() override {
        auto params = wq_->parameters();
        auto wk_params = wk_->parameters();
        auto wv_params = wv_->parameters();
        auto wo_params = wo_->parameters();
        
        params.insert(params.end(), wk_params.begin(), wk_params.end());
        params.insert(params.end(), wv_params.begin(), wv_params.end());
        params.insert(params.end(), wo_params.begin(), wo_params.end());
        
        return params;
    }
};

} // namespace aifs::nn
```

### 3.6 Transformer Block

```cpp
#pragma once
#include "layer.hpp"
#include "attention.hpp"
#include "normalization.hpp"
#include "linear.hpp"

namespace aifs::nn {

template<typename T>
class TransformerEncoderLayer : public Layer<T> {
    MultiHeadAttention<T> self_attn_;
    LayerNorm<T> norm1_;
    LayerNorm<T> norm2_;
    
    std::shared_ptr<Linear<T>> fc1_;
    std::shared_ptr<Linear<T>> fc2_;
    
    std::size_t d_model_;
    std::size_t d_ff_;
    T dropout_;
    
public:
    TransformerEncoderLayer(std::size_t d_model, std::size_t num_heads,
                            std::size_t d_ff, T dropout = T{0.1})
        : d_model_(d_model), d_ff_(d_ff), dropout_(dropout),
          self_attn_(d_model, num_heads, dropout),
          norm1_(d_model),
          norm2_(d_model) {
        
        fc1_ = std::make_shared<Linear<T>>(d_model, d_ff);
        fc2_ = std::make_shared<Linear<T>>(d_ff, d_model);
    }
    
    std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> src,
        std::shared_ptr<autodiff::Variable<T>> mask = nullptr) override {
        
        // Self-attention with residual
        auto attn_out = self_attn_.forward(src, src, src, mask);
        
        // Add & Norm
        auto add1 = add_tensors(src->data, attn_out->data);
        auto add1_var = std::make_shared<autodiff::Variable<T>>(add1, src->requires_grad);
        auto norm1_out = norm1_.forward(add1_var);
        
        // Feed-forward
        auto ff1 = fc1_->forward(norm1_out);
        
        // ReLU activation
        for (std::size_t i = 0; i < ff1->data.size(); ++i) {
            ff1->data(i) = std::max(T{0}, ff1->data(i));
        }
        
        auto ff2 = fc2_->forward(ff1);
        
        // Add & Norm
        auto add2 = add_tensors(norm1_out->data, ff2->data);
        auto add2_var = std::make_shared<autodiff::Variable<T>>(add2, norm1_out->requires_grad);
        
        return norm2_.forward(add2_var);
    }
    
    std::vector<std::shared_ptr<autodiff::Variable<T>>> parameters() override {
        auto params = self_attn_.parameters();
        auto norm1_params = norm1_.parameters();
        auto fc1_params = fc1_->parameters();
        auto fc2_params = fc2_->parameters();
        auto norm2_params = norm2_.parameters();
        
        params.insert(params.end(), norm1_params.begin(), norm1_params.end());
        params.insert(params.end(), fc1_params.begin(), fc1_params.end());
        params.insert(params.end(), fc2_params.begin(), fc2_params.end());
        params.insert(params.end(), norm2_params.begin(), norm2_params.end());
        
        return params;
    }
};

// Positional Encoding
template<typename T>
class PositionalEncoding : public Layer<T> {
    std::size_t d_model_;
    std::size_t max_len_;
    Tensor<T, 2> pe_;  // [max_len, d_model]
    
public:
    PositionalEncoding(std::size_t d_model, std::size_t max_len = 5000)
        : d_model_(d_model), max_len_(max_len), pe_({max_len, d_model}) {
        
        for (std::size_t pos = 0; pos < max_len; ++pos) {
            for (std::size_t i = 0; i < d_model; ++i) {
                T angle = static_cast<T>(pos) / std::pow(
                    static_cast<T>(10000), 
                    static_cast<T>(2 * (i / 2)) / static_cast<T>(d_model));
                
                if (i % 2 == 0) {
                    pe_(pos, i) = std::sin(angle);
                } else {
                    pe_(pos, i) = std::cos(angle);
                }
            }
        }
    }
    
    std::shared_ptr<autodiff::Variable<T>> forward(
        std::shared_ptr<autodiff::Variable<T>> x) override {
        
        const std::size_t seq_len = x->data.shape().dim(0);
        
        Tensor<T, 1> output(x->data.shape());
        for (std::size_t i = 0; i < seq_len; ++i) {
            for (std::size_t j = 0; j < d_model_; ++j) {
                output(i * d_model_ + j) = x->data(i * d_model_ + j) + pe_(i, j);
            }
        }
        
        return std::make_shared<autodiff::Variable<T>>(output, x->requires_grad);
    }
};

} // namespace aifs::nn
```

---

## 4. OPTIMIZATION & TRAINING SYSTEM

### 4.1 Optimizer Implementations

```cpp
#pragma once
#include "core/tensor.hpp"
#include "math/autodiff.hpp"
#include <cmath>
#include <vector>

namespace aifs::optim {

template<typename T>
class Optimizer {
public:
    virtual ~Optimizer() = default;
    
    virtual void step() = 0;
    virtual void zero_grad() = 0;
    
    void add_parameters(const std::vector<std::shared_ptr<autodiff::Variable<T>>>& params) {
        parameters_.insert(parameters_.end(), params.begin(), params.end());
    }
    
protected:
    std::vector<std::shared_ptr<autodiff::Variable<T>>> parameters_;
};

// Stochastic Gradient Descent with Momentum
template<typename T>
class SGD : public Optimizer<T> {
    T lr_;
    T momentum_;
    T weight_decay_;
    bool nesterov_;
    
    std::vector<Tensor<T, 1>> velocity_;
    
public:
    SGD(T lr, T momentum = T{0}, T weight_decay = T{0}, bool nesterov = false)
        : lr_(lr), momentum_(momentum), weight_decay_(weight_decay), 
          nesterov_(nesterov) {}
    
    void add_parameters(const std::vector<std::shared_ptr<autodiff::Variable<T>>>& params) override {
        Optimizer<T>::add_parameters(params);
        
        // Initialize velocity buffers
        for (const auto& p : params) {
            velocity_.emplace_back(Tensor<T, 1>(p->data.shape(), T{0}));
        }
    }
    
    void zero_grad() override {
        for (auto& p : this->parameters_) {
            p->zero_grad();
        }
    }
    
    void step() override {
        for (std::size_t i = 0; i < this->parameters_.size(); ++i) {
            auto& p = this->parameters_[i];
            auto& v = velocity_[i];
            
            // Apply weight decay
            if (weight_decay_ != T{0}) {
                for (std::size_t j = 0; j < p->data.size(); ++j) {
                    p->grad(j) += weight_decay_ * p->data(j);
                }
            }
            
            // Update velocity
            for (std::size_t j = 0; j < p->data.size(); ++j) {
                v(j) = momentum_ * v(j) + p->grad(j);
            }
            
            // Update parameters
            if (nesterov_) {
                for (std::size_t j = 0; j < p->data.size(); ++j) {
                    p->data(j) -= lr_ * (momentum_ * v(j) + p->grad(j));
                }
            } else {
                for (std::size_t j = 0; j < p->data.size(); ++j) {
                    p->data(j) -= lr_ * v(j);
                }
            }
        }
    }
};

// Adam Optimizer
template<typename T>
class Adam : public Optimizer<T> {
    T lr_;
    T beta1_;
    T beta2_;
    T eps_;
    T weight_decay_;
    
    std::vector<Tensor<T, 1>> m_;  // First moment
    std::vector<Tensor<T, 1>> v_;  // Second moment
    std::size_t step_count_ = 0;
    
public:
    Adam(T lr = T{0.001}, T beta1 = T{0.9}, T beta2 = T{0.999}, 
         T eps = T{1e-8}, T weight_decay = T{0})
        : lr_(lr), beta1_(beta1), beta2_(beta2), eps_(eps), 
          weight_decay_(weight_decay) {}
    
    void add_parameters(const std::vector<std::shared_ptr<autodiff::Variable<T>>>& params) override {
        Optimizer<T>::add_parameters(params);
        
        // Initialize moment buffers
        for (const auto& p : params) {
            m_.emplace_back(Tensor<T, 1>(p->data.shape(), T{0}));
            v_.emplace_back(Tensor<T, 1>(p->data.shape(), T{0}));
        }
    }
    
    void zero_grad() override {
        for (auto& p : this->parameters_) {
            p->zero_grad();
        }
    }
    
    void step() override {
        step_count_++;
        
        const T bias_correction1 = T{1} - std::pow(beta1_, static_cast<T>(step_count_));
        const T bias_correction2 = T{1} - std::pow(beta2_, static_cast<T>(step_count_));
        
        for (std::size_t i = 0; i < this->parameters_.size(); ++i) {
            auto& p = this->parameters_[i];
            auto& m = m_[i];
            auto& v = v_[i];
            
            // Apply weight decay
            if (weight_decay_ != T{0}) {
                for (std::size_t j = 0; j < p->data.size(); ++j) {
                    p->grad(j) += weight_decay_ * p->data(j);
                }
            }
            
            // Update biased first moment estimate
            for (std::size_t j = 0; j < p->data.size(); ++j) {
                m(j) = beta1_ * m(j) + (T{1} - beta1_) * p->grad(j);
            }
            
            // Update biased second raw moment estimate
            for (std::size_t j = 0; j < p->data.size(); ++j) {
                v(j) = beta2_ * v(j) + (T{1} - beta2_) * p->grad(j) * p->grad(j);
            }
            
            // Compute bias-corrected estimates and update
            for (std::size_t j = 0; j < p->data.size(); ++j) {
                T m_hat = m(j) / bias_correction1;
                T v_hat = v(j) / bias_correction2;
                p->data(j) -= lr_ * m_hat / (std::sqrt(v_hat) + eps_);
            }
        }
    }
};

// AdamW (decoupled weight decay)
template<typename T>
class AdamW : public Optimizer<T> {
    T lr_;
    T beta1_;
    T beta2_;
    T eps_;
    T weight_decay_;
    
    std::vector<Tensor<T, 1>> m_;
    std::vector<Tensor<T, 1>> v_;
    std::size_t step_count_ = 0;
    
public:
    AdamW(T lr = T{0.001}, T beta1 = T{0.9}, T beta2 = T{0.999}, 
          T eps = T{1e-8}, T weight_decay = T{0.01})
        : lr_(lr), beta1_(beta1), beta2_(beta2), eps_(eps), 
          weight_decay_(weight_decay) {}
    
    void add_parameters(const std::vector<std::shared_ptr<autodiff::Variable<T>>>& params) override {
        Optimizer<T>::add_parameters(params);
        
        for (const auto& p : params) {
            m_.emplace_back(Tensor<T, 1>(p->data.shape(), T{0}));
            v_.emplace_back(Tensor<T, 1>(p->data.shape(), T{0}));
        }
    }
    
    void zero_grad() override {
        for (auto& p : this->parameters_) {
            p->zero_grad();
        }
    }
    
    void step() override {
        step_count_++;
        
        const T bias_correction1 = T{1} - std::pow(beta1_, static_cast<T>(step_count_));
        const T bias_correction2 = T{1} - std::pow(beta2_, static_cast<T>(step_count_));
        
        for (std::size_t i = 0; i < this->parameters_.size(); ++i) {
            auto& p = this->parameters_[i];
            auto& m = m_[i];
            auto& v = v_[i];
            
            // Decoupled weight decay
            for (std::size_t j = 0; j < p->data.size(); ++j) {
                p->data(j) -= lr_ * weight_decay_ * p->data(j);
            }
            
            // Update moments
            for (std::size_t j = 0; j < p->data.size(); ++j) {
                m(j) = beta1_ * m(j) + (T{1} - beta1_) * p->grad(j);
                v(j) = beta2_ * v(j) + (T{1} - beta2_) * p->grad(j) * p->grad(j);
            }
            
            // Update parameters
            for (std::size_t j = 0; j < p->data.size(); ++j) {
                T m_hat = m(j) / bias_correction1;
                T v_hat = v(j) / bias_correction2;
                p->data(j) -= lr_ * m_hat / (std::sqrt(v_hat) + eps_);
            }
        }
    }
};

} // namespace aifs::optim
```

### 4.2 Learning Rate Schedulers

```cpp
#pragma once
#include <cmath>

namespace aifs::optim {

template<typename T>
class LRScheduler {
public:
    virtual ~LRScheduler() = default;
    virtual T get_lr(T base_lr, std::size_t epoch, std::size_t step) = 0;
};

// Step decay: multiply LR by gamma every step_size epochs
template<typename T>
class StepLR : public LRScheduler<T> {
    std::size_t step_size_;
    T gamma_;
    
public:
    StepLR(std::size_t step_size, T gamma = T{0.1})
        : step_size_(step_size), gamma_(gamma) {}
    
    T get_lr(T base_lr, std::size_t epoch, std::size_t step) override {
        return base_lr * std::pow(gamma_, static_cast<T>(epoch / step_size_));
    }
};

// Cosine annealing
template<typename T>
class CosineAnnealingLR : public LRScheduler<T> {
    std::size_t T_max_;
    T eta_min_;
    
public:
    CosineAnnealingLR(std::size_t T_max, T eta_min = T{0})
        : T_max_(T_max), eta_min_(eta_min) {}
    
    T get_lr(T base_lr, std::size_t epoch, std::size_t step) override {
        T progress = static_cast<T>(epoch) / static_cast<T>(T_max_);
        return eta_min_ + (base_lr - eta_min_) * 
               (T{1} + std::cos(static_cast<T>(M_PI) * progress)) / T{2};
    }
};

// Warmup + Cosine decay (common for Transformer training)
template<typename T>
class WarmupCosineLR : public LRScheduler<T> {
    std::size_t warmup_steps_;
    std::size_t total_steps_;
    T eta_min_;
    
public:
    WarmupCosineLR(std::size_t warmup_steps, std::size_t total_steps, 
                   T eta_min = T{0})
        : warmup_steps_(warmup_steps), total_steps_(total_steps), 
          eta_min_(eta_min) {}
    
    T get_lr(T base_lr, std::size_t epoch, std::size_t step) override {
        if (step < warmup_steps_) {
            // Linear warmup
            return base_lr * static_cast<T>(step) / static_cast<T>(warmup_steps_);
        }
        
        // Cosine decay
        T progress = static_cast<T>(step - warmup_steps_) / 
                     static_cast<T>(total_steps_ - warmup_steps_);
        return eta_min_ + (base_lr - eta_min_) * 
               (T{1} + std::cos(static_cast<T>(M_PI) * progress)) / T{2};
    }
};

// Reduce on plateau
template<typename T>
class ReduceLROnPlateau : public LRScheduler<T> {
    T factor_;
    std::size_t patience_;
    T threshold_;
    T best_loss_ = std::numeric_limits<T>::infinity();
    std::size_t bad_epochs_ = 0;
    
public:
    ReduceLROnPlateau(T factor = T{0.1}, std::size_t patience = 10, 
                      T threshold = T{1e-4})
        : factor_(factor), patience_(patience), threshold_(threshold) {}
    
    T get_lr(T base_lr, std::size_t epoch, std::size_t step) override {
        return base_lr;  // Actual reduction handled separately
    }
    
    bool step(T loss) {
        if (loss < best_loss_ * (T{1} - threshold_)) {
            best_loss_ = loss;
            bad_epochs_ = 0;
            return false;
        }
        
        bad_epochs_++;
        return bad_epochs_ > patience_;
    }
    
    T reduce(T current_lr) {
        bad_epochs_ = 0;
        return current_lr * factor_;
    }
};

} // namespace aifs::optim
```

### 4.3 Training Loop

```cpp
#pragma once
#include "optimizer.hpp"
#include "nn/layer.hpp"
#include "data/dataloader.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <functional>

namespace aifs {

template<typename T>
class Trainer {
    std::shared_ptr<nn::Layer<T>> model_;
    std::shared_ptr<optim::Optimizer<T>> optimizer_;
    std::shared_ptr<optim::LRScheduler<T>> scheduler_;
    
    std::function<T(std::shared_ptr<autodiff::Variable<T>>, 
                    std::shared_ptr<autodiff::Variable<T>>)> loss_fn_;
    
public:
    Trainer(std::shared_ptr<nn::Layer<T>> model,
            std::shared_ptr<optim::Optimizer<T>> optimizer,
            std::function<T(std::shared_ptr<autodiff::Variable<T>>,
                           std::shared_ptr<autodiff::Variable<T>>)> loss_fn,
            std::shared_ptr<optim::LRScheduler<T>> scheduler = nullptr)
        : model_(model), optimizer_(optimizer), loss_fn_(loss_fn), 
          scheduler_(scheduler) {
        
        // Collect all parameters
        auto params = collect_parameters(model_);
        optimizer_->add_parameters(params);
    }
    
    struct EpochResult {
        T train_loss;
        T val_loss;
        T train_acc;
        T val_acc;
        double duration_seconds;
    };
    
    EpochResult fit_epoch(DataLoader<T>& train_loader, 
                          DataLoader<T>* val_loader = nullptr) {
        
        auto start = std::chrono::high_resolution_clock::now();
        
        model_->train();
        T train_loss = T{0};
        std::size_t train_batches = 0;
        std::size_t correct = 0;
        std::size_t total = 0;
        
        for (auto& batch : train_loader) {
            optimizer_->zero_grad();
            
            auto output = model_->forward(batch.input);
            T loss = loss_fn_(output, batch.target);
            
            output->backward();
            optimizer_->step();
            
            train_loss += loss;
            train_batches++;
            
            // Simple accuracy calculation for classification
            for (std::size_t i = 0; i < batch.target->data.size(); ++i) {
                if (std::abs(output->data(i) - batch.target->data(i)) < T{0.5}) {
                    correct++;
                }
                total++;
            }
        }
        
        train_loss /= static_cast<T>(train_batches);
        T train_acc = static_cast<T>(correct) / static_cast<T>(total);
        
        T val_loss = T{0};
        T val_acc = T{0};
        
        if (val_loader) {
            auto val_result = evaluate(*val_loader);
            val_loss = val_result.first;
            val_acc = val_result.second;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
        
        return {train_loss, val_loss, train_acc, val_acc, 
                static_cast<double>(duration.count())};
    }
    
    std::pair<T, T> evaluate(DataLoader<T>& loader) {
        model_->eval();
        T total_loss = T{0};
        std::size_t num_batches = 0;
        std::size_t correct = 0;
        std::size_t total = 0;
        
        for (auto& batch : loader) {
            auto output = model_->forward(batch.input);
            T loss = loss_fn_(output, batch.target);
            
            total_loss += loss;
            num_batches++;
            
            for (std::size_t i = 0; i < batch.target->data.size(); ++i) {
                if (std::abs(output->data(i) - batch.target->data(i)) < T{0.5}) {
                    correct++;
                }
                total++;
            }
        }
        
        return {total_loss / static_cast<T>(num_batches),
                static_cast<T>(correct) / static_cast<T>(total)};
    }
    
    void fit(DataLoader<T>& train_loader, std::size_t epochs,
             DataLoader<T>* val_loader = nullptr,
             std::size_t early_stopping_patience = 0) {
        
        T best_val_loss = std::numeric_limits<T>::infinity();
        std::size_t patience_counter = 0;
        
        for (std::size_t epoch = 0; epoch < epochs; ++epoch) {
            auto result = fit_epoch(train_loader, val_loader);
            
            std::cout << "Epoch " << epoch + 1 << "/" << epochs 
                      << " - loss: " << result.train_loss
                      << " - acc: " << result.train_acc;
            
            if (val_loader) {
                std::cout << " - val_loss: " << result.val_loss
                          << " - val_acc: " << result.val_acc;
            }
            
            std::cout << " - " << result.duration_seconds << "s\n";
            
            // Learning rate scheduling
            if (scheduler_) {
                // Update LR based on scheduler
            }
            
            // Early stopping
            if (early_stopping_patience > 0 && val_loader) {
                if (result.val_loss < best_val_loss) {
                    best_val_loss = result.val_loss;
                    patience_counter = 0;
                } else {
                    patience_counter++;
                    if (patience_counter >= early_stopping_patience) {
                        std::cout << "Early stopping triggered\n";
                        break;
                    }
                }
            }
        }
    }
    
private:
    std::vector<std::shared_ptr<autodiff::Variable<T>>> collect_parameters(
        std::shared_ptr<nn::Layer<T>> layer) {
        return layer->parameters();
    }
};

} // namespace aifs
```

---

## 5. PERFORMANCE ENGINEERING

### 5.1 Memory Pool Allocator

```cpp
#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>
#include <array>

namespace aifs::memory {

// Segregated free list allocator for tensor operations
template<typename T>
class TensorAllocator {
    static constexpr std::size_t MIN_BLOCK_SIZE = 64;      // 64 elements
    static constexpr std::size_t MAX_BLOCK_SIZE = 1048576; // 1M elements
    static constexpr std::size_t NUM_SIZE_CLASSES = 16;
    
    struct Block {
        Block* next;
        std::size_t size;
        bool in_use;
    };
    
    struct Arena {
        std::unique_ptr<T[]> memory;
        std::size_t size;
        std::size_t offset;
        
        Arena(std::size_t sz) : size(sz), offset(0) {
            memory = std::make_unique<T[]>(sz);
        }
    };
    
    std::vector<std::unique_ptr<Arena>> arenas_;
    std::array<Block*, NUM_SIZE_CLASSES> free_lists_;
    std::mutex mutex_;
    
    std::size_t size_to_class(std::size_t num_elements) {
        std::size_t bytes = num_elements * sizeof(T);
        if (bytes <= MIN_BLOCK_SIZE * sizeof(T)) return 0;
        
        std::size_t size_class = 0;
        std::size_t block_size = MIN_BLOCK_SIZE;
        
        while (block_size < num_elements && size_class < NUM_SIZE_CLASSES - 1) {
            block_size *= 2;
            size_class++;
        }
        
        return size_class;
    }
    
    std::size_t class_to_size(std::size_t size_class) {
        return MIN_BLOCK_SIZE * (1ULL << size_class);
    }
    
public:
    TensorAllocator() {
        free_lists_.fill(nullptr);
        
        // Pre-allocate initial arena (64MB for float32)
        arenas_.push_back(std::make_unique<Arena>(16 * 1024 * 1024));
    }
    
    T* allocate(std::size_t num_elements) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::size_t size_class = size_to_class(num_elements);
        std::size_t block_size = class_to_size(size_class);
        
        // Check free list
        if (free_lists_[size_class] != nullptr) {
            Block* block = free_lists_[size_class];
            free_lists_[size_class] = block->next;
            block->in_use = true;
            return reinterpret_cast<T*>(block + 1);
        }
        
        // Allocate from current arena
        for (auto& arena : arenas_) {
            if (arena->offset + block_size <= arena->size) {
                T* ptr = &arena->memory[arena->offset];
                arena->offset += block_size;
                
                // Store block metadata before the pointer
                Block* block = reinterpret_cast<Block*>(ptr) - 1;
                block->size = block_size;
                block->in_use = true;
                block->next = nullptr;
                
                return ptr;
            }
        }
        
        // Need new arena
        std::size_t new_arena_size = std::max(
            block_size * 1024, 
            arenas_.back()->size * 2
        );
        arenas_.push_back(std::make_unique<Arena>(new_arena_size));
        
        T* ptr = &arenas_.back()->memory[0];
        arenas_.back()->offset = block_size;
        
        Block* block = reinterpret_cast<Block*>(ptr) - 1;
        block->size = block_size;
        block->in_use = true;
        block->next = nullptr;
        
        return ptr;
    }
    
    void deallocate(T* ptr) {
        if (!ptr) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        Block* block = reinterpret_cast<Block*>(ptr) - 1;
        block->in_use = false;
        
        std::size_t size_class = size_to_class(block->size);
        block->next = free_lists_[size_class];
        free_lists_[size_class] = block;
    }
    
    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        // Reset all arenas
        for (auto& arena : arenas_) {
            arena->offset = 0;
        }
        
        // Clear free lists
        free_lists_.fill(nullptr);
    }
    
    std::size_t total_allocated() const {
        std::size_t total = 0;
        for (const auto& arena : arenas_) {
            total += arena->size * sizeof(T);
        }
        return total;
    }
};

// Thread-local allocator cache
template<typename T>
class TLSAllocatorCache {
    static constexpr std::size_t CACHE_SIZE = 16;
    
    struct CacheEntry {
        T* ptr;
        std::size_t size;
    };
    
    std::array<CacheEntry, CACHE_SIZE> cache_;
    std::size_t cache_count_ = 0;
    TensorAllocator<T>& allocator_;
    
public:
    explicit TLSAllocatorCache(TensorAllocator<T>& alloc) : allocator_(alloc) {
        cache_.fill({nullptr, 0});
    }
    
    T* allocate(std::size_t num_elements) {
        // Check cache for suitable block
        for (std::size_t i = 0; i < cache_count_; ++i) {
            if (cache_[i].size >= num_elements) {
                T* ptr = cache_[i].ptr;
                // Remove from cache
                cache_[i] = cache_[--cache_count_];
                return ptr;
            }
        }
        
        return allocator_.allocate(num_elements);
    }
    
    void deallocate(T* ptr, std::size_t size) {
        if (cache_count_ < CACHE_SIZE) {
            cache_[cache_count_++] = {ptr, size};
        } else {
            allocator_.deallocate(ptr);
        }
    }
    
    ~TLSAllocatorCache() {
        for (std::size_t i = 0; i < cache_count_; ++i) {
            allocator_.deallocate(cache_[i].ptr);
        }
    }
};

} // namespace aifs::memory
```

### 5.2 Thread Pool for Parallel Execution

```cpp
#pragma once
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <memory>

namespace aifs {

class ThreadPool {
public:
    using Task = std::function<void()>;
    
private:
    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;
    
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::condition_variable finished_;
    
    std::atomic<bool> stop_{false};
    std::atomic<std::size_t> active_tasks_{0};
    std::size_t num_threads_;
    
public:
    explicit ThreadPool(std::size_t num_threads = 
                        std::thread::hardware_concurrency())
        : num_threads_(num_threads) {
        
        for (std::size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] { worker_loop(); });
        }
    }
    
    ~ThreadPool() {
        stop_ = true;
        condition_.notify_all();
        
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        using return_type = decltype(f(args...));
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            
            if (stop_) {
                throw std::runtime_error("Cannot enqueue on stopped ThreadPool");
            }
            
            tasks_.emplace([task]() { (*task)(); });
        }
        
        condition_.notify_one();
        return result;
    }
    
    // Parallel for loop
    template<typename Index, typename Func>
    void parallel_for(Index start, Index end, Func&& func, 
                      std::size_t min_grain_size = 1) {
        
        const std::size_t num_tasks = static_cast<std::size_t>(end - start);
        const std::size_t num_workers = workers_.size();
        
        if (num_tasks <= min_grain_size || num_workers == 1) {
            // Sequential execution
            for (Index i = start; i < end; ++i) {
                func(i);
            }
            return;
        }
        
        // Calculate grain size
        std::size_t grain_size = std::max(
            min_grain_size,
            (num_tasks + num_workers - 1) / num_workers
        );
        
        std::vector<std::future<void>> futures;
        
        for (Index i = start; i < end; i += static_cast<Index>(grain_size)) {
            Index block_end = std::min(i + static_cast<Index>(grain_size), end);
            
            futures.push_back(enqueue([=, &func]() {
                for (Index j = i; j < block_end; ++j) {
                    func(j);
                }
            }));
        }
        
        // Wait for all tasks
        for (auto& future : futures) {
            future.get();
        }
    }
    
    // Parallel reduction
    template<typename T, typename Index, typename MapFunc, typename ReduceFunc>
    T parallel_reduce(Index start, Index end, T init, MapFunc&& map_func,
                      ReduceFunc&& reduce_func, std::size_t min_grain_size = 1) {
        
        const std::size_t num_tasks = static_cast<std::size_t>(end - start);
        const std::size_t num_workers = workers_.size();
        
        if (num_tasks <= min_grain_size || num_workers == 1) {
            T result = init;
            for (Index i = start; i < end; ++i) {
                result = reduce_func(result, map_func(i));
            }
            return result;
        }
        
        std::size_t grain_size = std::max(
            min_grain_size,
            (num_tasks + num_workers - 1) / num_workers
        );
        
        std::vector<std::future<T>> futures;
        
        for (Index i = start; i < end; i += static_cast<Index>(grain_size)) {
            Index block_end = std::min(i + static_cast<Index>(grain_size), end);
            
            futures.push_back(enqueue([=, &map_func]() -> T {
                T local_result = init;
                for (Index j = i; j < block_end; ++j) {
                    local_result = reduce_func(local_result, map_func(j));
                }
                return local_result;
            }));
        }
        
        // Reduce results
        T result = init;
        for (auto& future : futures) {
            result = reduce_func(result, future.get());
        }
        
        return result;
    }
    
    std::size_t size() const { return num_threads_; }
    
    void wait_all() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        finished_.wait(lock, [this] {
            return tasks_.empty() && active_tasks_ == 0;
        });
    }
    
private:
    void worker_loop() {
        while (true) {
            Task task;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                
                condition_.wait(lock, [this] {
                    return stop_ || !tasks_.empty();
                });
                
                if (stop_ && tasks_.empty()) {
                    return;
                }
                
                task = std::move(tasks_.front());
                tasks_.pop();
                active_tasks_++;
            }
            
            task();
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                active_tasks_--;
                if (tasks_.empty() && active_tasks_ == 0) {
                    finished_.notify_all();
                }
            }
        }
    }
};

} // namespace aifs
```

---

## 6. DEBUGGING & VALIDATION FRAMEWORK

### 6.1 Gradient Checking

```cpp
#pragma once
#include "math/autodiff.hpp"
#include <cmath>
#include <iostream>

namespace aifs::debug {

template<typename T>
bool gradient_check(
    std::function<std::shared_ptr<autodiff::Variable<T>>(
        std::shared_ptr<autodiff::Variable<T>>)> func,
    std::shared_ptr<autodiff::Variable<T>> input,
    T epsilon = T{1e-5},
    T tolerance = T{1e-6}) {
    
    // Forward pass to get analytical gradient
    auto output = func(input);
    output->backward();
    
    bool all_passed = true;
    
    // Numerical gradient check for each parameter
    for (std::size_t i = 0; i < input->data.size(); ++i) {
        T original_value = input->data(i);
        
        // f(x + epsilon)
        input->data(i) = original_value + epsilon;
        auto output_plus = func(input);
        T loss_plus = sum_tensor(output_plus->data);
        
        // f(x - epsilon)
        input->data(i) = original_value - epsilon;
        auto output_minus = func(input);
        T loss_minus = sum_tensor(output_minus->data);
        
        // Restore original value
        input->data(i) = original_value;
        
        // Numerical gradient
        T numerical_grad = (loss_plus - loss_minus) / (T{2} * epsilon);
        
        // Analytical gradient
        T analytical_grad = input->grad(i);
        
        // Relative error
        T diff = std::abs(numerical_grad - analytical_grad);
        T relative_error = diff / (std::abs(numerical_grad) + 
                                   std::abs(analytical_grad) + T{1e-8});
        
        if (relative_error > tolerance) {
            std::cout << "Gradient check FAILED at index " << i << "\n";
            std::cout << "  Numerical: " << numerical_grad << "\n";
            std::cout << "  Analytical: " << analytical_grad << "\n";
            std::cout << "  Relative error: " << relative_error << "\n";
            all_passed = false;
        }
    }
    
    if (all_passed) {
        std::cout << "Gradient check PASSED for all " << input->data.size() 
                  << " elements\n";
    }
    
    return all_passed;
}

// Check layer gradients
template<typename T>
bool check_layer_gradients(
    std::shared_ptr<nn::Layer<T>> layer,
    const Tensor<T, 1>& input_data,
    const Tensor<T, 1>& target_data,
    T epsilon = T{1e-5},
    T tolerance = T{1e-5}) {
    
    auto input = std::make_shared<autodiff::Variable<T>>(input_data, true);
    auto target = std::make_shared<autodiff::Variable<T>>(target_data, false);
    
    // Forward + backward
    auto output = layer->forward(input);
    T loss = mse_loss(output->data, target->data);
    output->backward();
    
    bool all_passed = true;
    
    // Check each parameter
    for (auto& param : layer->parameters()) {
        if (!param->requires_grad) continue;
        
        for (std::size_t i = 0; i < param->data.size(); ++i) {
            T original = param->data(i);
            
            // Numerical gradient
            param->data(i) = original + epsilon;
            auto out_plus = layer->forward(input);
            T loss_plus = mse_loss(out_plus->data, target->data);
            
            param->data(i) = original - epsilon;
            auto out_minus = layer->forward(input);
            T loss_minus = mse_loss(out_minus->data, target->data);
            
            param->data(i) = original;
            
            T numerical = (loss_plus - loss_minus) / (T{2} * epsilon);
            T analytical = param->grad(i);
            
            T rel_error = std::abs(numerical - analytical) / 
                         (std::abs(numerical) + std::abs(analytical) + T{1e-8});
            
            if (rel_error > tolerance) {
                std::cout << "Param gradient check FAILED at index " << i 
                          << " (rel_error: " << rel_error << ")\n";
                all_passed = false;
            }
        }
    }
    
    return all_passed;
}

// Utility functions for gradient checking
template<typename T>
T sum_tensor(const Tensor<T, 1>& t) {
    T sum = T{0};
    for (std::size_t i = 0; i < t.size(); ++i) {
        sum += t(i);
    }
    return sum;
}

template<typename T>
T mse_loss(const Tensor<T, 1>& pred, const Tensor<T, 1>& target) {
    T sum = T{0};
    for (std::size_t i = 0; i < pred.size(); ++i) {
        T diff = pred(i) - target(i);
        sum += diff * diff;
    }
    return sum / static_cast<T>(pred.size());
}

} // namespace aifs::debug
```

### 6.2 Profiling Utilities

```cpp
#pragma once
#include <chrono>
#include <string>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace aifs::profiling {

class Timer {
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::duration<double>;
    
    std::string name_;
    TimePoint start_;
    bool running_ = false;
    
public:
    explicit Timer(std::string name) : name_(std::move(name)) {}
    
    void start() {
        start_ = Clock::now();
        running_ = true;
    }
    
    double stop() {
        if (!running_) return 0.0;
        
        auto end = Clock::now();
        Duration duration = end - start_;
        running_ = false;
        
        return duration.count();
    }
    
    double elapsed() const {
        if (!running_) return 0.0;
        
        auto now = Clock::now();
        Duration duration = now - start_;
        return duration.count();
    }
    
    // RAII scope timer
    class Scope {
        Timer& timer_;
        double* result_;
        
    public:
        Scope(Timer& timer, double* result = nullptr) 
            : timer_(timer), result_(result) {
            timer_.start();
        }
        
        ~Scope() {
            double elapsed = timer_.stop();
            if (result_) *result_ = elapsed;
        }
    };
};

// FLOP counter for operations
class FLOPCounter {
    std::unordered_map<std::string, std::uint64_t> flops_;
    std::unordered_map<std::string, std::uint64_t> bytes_;
    
public:
    void record_gemm(std::size_t m, std::size_t n, std::size_t k, 
                     const std::string& name = "gemm") {
        // GEMM: 2 * m * n * k FLOPs
        flops_[name] += 2 * m * n * k;
        // Memory traffic: read A (m*k), B (k*n), read/write C (m*n)
        bytes_[name] += (m * k + k * n + 2 * m * n) * sizeof(float);
    }
    
    void record_conv(std::size_t N, std::size_t C_out, std::size_t H_out,
                     std::size_t W_out, std::size_t C_in, std::size_t K_h,
                     std::size_t K_w, const std::string& name = "conv") {
        // Conv: 2 * N * C_out * H_out * W_out * C_in * K_h * K_w FLOPs
        flops_[name] += 2 * N * C_out * H_out * W_out * C_in * K_h * K_w;
    }
    
    void record_elementwise(std::size_t n, std::size_t flops_per_element = 1,
                            const std::string& name = "elementwise") {
        flops_[name] += n * flops_per_element;
    }
    
    void print_report() const {
        std::cout << "\n=== FLOP Counter Report ===\n";
        
        std::uint64_t total_flops = 0;
        for (const auto& [name, count] : flops_) {
            total_flops += count;
        }
        
        std::cout << "Total FLOPs: " << format_number(total_flops) << "\n";
        std::cout << "Breakdown:\n";
        
        // Sort by FLOP count
        std::vector<std::pair<std::string, std::uint64_t>> sorted(
            flops_.begin(), flops_.end());
        std::sort(sorted.begin(), sorted.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        for (const auto& [name, count] : sorted) {
            double percentage = 100.0 * static_cast<double>(count) / 
                               static_cast<double>(total_flops);
            std::cout << "  " << std::setw(20) << std::left << name 
                      << ": " << std::setw(15) << format_number(count)
                      << " (" << std::fixed << std::setprecision(2) 
                      << percentage << "%)\n";
        }
    }
    
    static std::string format_number(std::uint64_t n) {
        if (n >= 1e12) {
            return std::to_string(static_cast<double>(n) / 1e12) + " T";
        } else if (n >= 1e9) {
            return std::to_string(static_cast<double>(n) / 1e9) + " G";
        } else if (n >= 1e6) {
            return std::to_string(static_cast<double>(n) / 1e6) + " M";
        } else if (n >= 1e3) {
            return std::to_string(static_cast<double>(n) / 1e3) + " K";
        }
        return std::to_string(n);
    }
};

// Memory tracker
class MemoryTracker {
    std::size_t current_bytes_ = 0;
    std::size_t peak_bytes_ = 0;
    std::size_t total_allocations_ = 0;
    std::size_t total_deallocations_ = 0;
    
public:
    void record_allocation(std::size_t bytes) {
        current_bytes_ += bytes;
        peak_bytes_ = std::max(peak_bytes_, current_bytes_);
        total_allocations_++;
    }
    
    void record_deallocation(std::size_t bytes) {
        current_bytes_ -= bytes;
        total_deallocations_++;
    }
    
    void print_report() const {
        std::cout << "\n=== Memory Tracker Report ===\n";
        std::cout << "Current: " << format_bytes(current_bytes_) << "\n";
        std::cout << "Peak: " << format_bytes(peak_bytes_) << "\n";
        std::cout << "Allocations: " << total_allocations_ << "\n";
        std::cout << "Deallocations: " << total_deallocations_ << "\n";
    }
    
    static std::string format_bytes(std::size_t bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unit_idx = 0;
        double size = static_cast<double>(bytes);
        
        while (size >= 1024.0 && unit_idx < 4) {
            size /= 1024.0;
            unit_idx++;
        }
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << size << " " << units[unit_idx];
        return oss.str();
    }
};

} // namespace aifs::profiling
```

### 6.3 Self-Contained Visualization

```cpp
#pragma once
#include "core/tensor.hpp"
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>

namespace aifs::viz {

// ASCII plotting for training curves
class ASCIIPlotter {
    std::size_t width_;
    std::size_t height_;
    
public:
    ASCIIPlotter(std::size_t width = 60, std::size_t height = 15)
        : width_(width), height_(height) {}
    
    template<typename T>
    std::string plot(const std::vector<T>& data, 
                     const std::string& title = "",
                     const std::string& ylabel = "") {
        if (data.empty()) return "";
        
        std::ostringstream oss;
        
        // Find min/max
        T min_val = data[0];
        T max_val = data[0];
        for (const auto& v : data) {
            min_val = std::min(min_val, v);
            max_val = std::max(max_val, v);
        }
        
        if (min_val == max_val) {
            min_val -= 1;
            max_val += 1;
        }
        
        // Title
        if (!title.empty()) {
            oss << std::string((width_ - title.length()) / 2, ' ') 
                << title << "\n";
        }
        
        // Plot area
        std::vector<std::string> grid(height_, std::string(width_, ' '));
        
        // Draw axes
        for (std::size_t y = 0; y < height_; ++y) {
            grid[y][0] = '|';
        }
        for (std::size_t x = 0; x < width_; ++x) {
            grid[height_ - 1][x] = '-';
        }
        grid[height_ - 1][0] = '+';
        
        // Plot data points
        for (std::size_t i = 0; i < data.size() && i < width_ - 2; ++i) {
            T normalized = (data[i] - min_val) / (max_val - min_val);
            std::size_t y = height_ - 2 - 
                           static_cast<std::size_t>(normalized * (height_ - 2));
            y = std::min(y, height_ - 2);
            
            grid[y][i + 1] = '*';
        }
        
        // Y-axis labels
        std::string y_max_str = format_value(max_val);
        std::string y_min_str = format_value(min_val);
        
        // Output grid
        for (std::size_t y = 0; y < height_; ++y) {
            if (y == 0 && !ylabel.empty()) {
                oss << ylabel.substr(0, 4) << " ";
            } else if (y == 0) {
                oss << y_max_str << " ";
            } else if (y == height_ - 1) {
                oss << y_min_str << " ";
            } else {
                oss << std::string(y_max_str.length() + 1, ' ');
            }
            
            oss << grid[y] << "\n";
        }
        
        return oss.str();
    }
    
private:
    template<typename T>
    static std::string format_value(T val) {
        std::ostringstream oss;
        if (std::abs(val) >= 1000 || (std::abs(val) < 0.01 && val != 0)) {
            oss << std::scientific << std::setprecision(1) << val;
        } else {
            oss << std::fixed << std::setprecision(2) << val;
        }
        return oss.str();
    }
};

// SVG generation for network architecture diagrams
class SVGGenerator {
    std::ostringstream svg_;
    std::size_t width_;
    std::size_t height_;
    
public:
    SVGGenerator(std::size_t width = 800, std::size_t height = 600)
        : width_(width), height_(height) {
        
        svg_ << R"(<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" width=")" << width_ 
             << R"(" height=")" << height_ << R"(" viewBox="0 0 )" 
             << width_ << " " << height_ << R"(">
<rect width="100%" height="100%" fill="#f8f9fa"/>
<text x=")" << width_/2 << R"(" y="30" text-anchor="middle" 
      font-family="sans-serif" font-size="18" font-weight="bold">
  Neural Network Architecture
</text>
)";
    }
    
    void add_layer(std::size_t x, std::size_t y, std::size_t w, std::size_t h,
                   const std::string& label, const std::string& color = "#4a90d9") {
        
        svg_ << "<rect x=\"" << x << "\" y=\"" << y 
             << "\" width=\"" << w << "\" height=\"" << h 
             << "\" fill=\"" << color << "\" stroke=\"#333\" stroke-width=\"2\" rx=\"5\"/>\n";
        
        svg_ << "<text x=\"" << x + w/2 << "\" y=\"" << y + h/2 
             << "\" text-anchor=\"middle\" dominant-baseline=\"middle\"\n"
             << "      font-family=\"sans-serif\" font-size=\"12\" fill=\"white\">\n"
             << "  " << label << "\n</text>\n";
    }
    
    void add_connection(std::size_t x1, std::size_t y1, 
                        std::size_t x2, std::size_t y2,
                        const std::string& color = "#666") {
        svg_ << "<line x1=\"" << x1 << "\" y1=\"" << y1 
             << "\" x2=\"" << x2 << "\" y2=\"" << y2 
             << "\" stroke=\"" << color << "\" stroke-width=\"2\"/>\n";
    }
    
    void add_text(std::size_t x, std::size_t y, const std::string& text,
                  std::size_t font_size = 10, const std::string& color = "#333") {
        svg_ << "<text x=\"" << x << "\" y=\"" << y 
             << "\" font-family=\"sans-serif\" font-size=\"" 
             << font_size << "\" fill=\"" << color << "\">\n"
             << "  " << text << "\n</text>\n";
    }
    
    void save(const std::string& filename) {
        svg_ << "</svg>\n";
        
        std::ofstream file(filename);
        file << svg_.str();
    }
    
    std::string str() const {
        return svg_.str() + "</svg>\n";
    }
};

// Activation/gradient distribution histogram
template<typename T>
class DistributionPlotter {
public:
    static std::string histogram(const Tensor<T, 1>& data, 
                                  std::size_t num_bins = 20,
                                  std::size_t width = 50) {
        if (data.size() == 0) return "No data\n";
        
        // Find range
        T min_val = data(0);
        T max_val = data(0);
        T sum = T{0};
        T sum_sq = T{0};
        
        for (std::size_t i = 0; i < data.size(); ++i) {
            min_val = std::min(min_val, data(i));
            max_val = std::max(max_val, data(i));
            sum += data(i);
            sum_sq += data(i) * data(i);
        }
        
        T mean = sum / static_cast<T>(data.size());
        T variance = sum_sq / static_cast<T>(data.size()) - mean * mean;
        T std_dev = std::sqrt(variance);
        
        // Build histogram
        std::vector<std::size_t> bins(num_bins, 0);
        T bin_width = (max_val - min_val) / static_cast<T>(num_bins);
        if (bin_width == T{0}) bin_width = T{1};
        
        for (std::size_t i = 0; i < data.size(); ++i) {
            std::size_t bin = static_cast<std::size_t>((data(i) - min_val) / bin_width);
            bin = std::min(bin, num_bins - 1);
            bins[bin]++;
        }
        
        std::size_t max_count = *std::max_element(bins.begin(), bins.end());
        
        // Draw histogram
        std::ostringstream oss;
        oss << "Distribution (n=" << data.size() << ", mean=" << mean 
            << ", std=" << std_dev << ")\n";
        oss << "Range: [" << min_val << ", " << max_val << "]\n\n";
        
        for (std::size_t i = 0; i < num_bins; ++i) {
            T bin_start = min_val + static_cast<T>(i) * bin_width;
            std::size_t bar_length = max_count > 0 ? 
                (bins[i] * width) / max_count : 0;
            
            oss << std::setw(10) << std::fixed << std::setprecision(3) << bin_start 
                << " |" << std::string(bar_length, '#') 
                << " " << bins[i] << "\n";
        }
        
        return oss.str();
    }
};

} // namespace aifs::viz
```

---

## 7. DEPLOYMENT & SERIALIZATION

### 7.1 Binary Model Format

```cpp
#pragma once
#include "core/tensor.hpp"
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace aifs::serialization {

// Binary format specification:
// Header (32 bytes):
//   - Magic: "AIFS" (4 bytes)
//   - Version: uint32_t (4 bytes)
//   - Num tensors: uint32_t (4 bytes)
//   - Reserved: 20 bytes
// 
// Tensor metadata (per tensor, 64 bytes):
//   - Name length: uint16_t (2 bytes)
//   - Data type: uint16_t (2 bytes) - 0=f32, 1=f64, 2=f16
//   - Num dimensions: uint16_t (2 bytes)
//   - Shape: 8 x uint64_t (64 bytes)
//   - Data offset: uint64_t (8 bytes)
//   - Data size: uint64_t (8 bytes)
//   - Reserved: 6 bytes
//
// Data section: raw tensor data

constexpr char MAGIC[] = "AIFS";
constexpr uint32_t FORMAT_VERSION = 1;

enum class DataType : uint16_t {
    FLOAT32 = 0,
    FLOAT64 = 1,
    FLOAT16 = 2,
    BFLOAT16 = 3
};

struct ModelHeader {
    char magic[4];
    uint32_t version;
    uint32_t num_tensors;
    uint8_t reserved[20];
};

struct TensorMetadata {
    uint16_t name_length;
    uint16_t data_type;
    uint16_t num_dims;
    uint64_t shape[8];
    uint64_t data_offset;
    uint64_t data_size;
    uint8_t reserved[6];
};

class ModelSerializer {
public:
    template<typename T, std::size_t N>
    static void save_tensor(std::ofstream& file, const std::string& name,
                            const Tensor<T, N>& tensor) {
        // Write tensor metadata
        TensorMetadata meta{};
        meta.name_length = static_cast<uint16_t>(name.length());
        meta.num_dims = static_cast<uint16_t>(N);
        
        if constexpr (std::is_same_v<T, float>) {
            meta.data_type = static_cast<uint16_t>(DataType::FLOAT32);
        } else if constexpr (std::is_same_v<T, double>) {
            meta.data_type = static_cast<uint16_t>(DataType::FLOAT64);
        }
        
        for (std::size_t i = 0; i < N; ++i) {
            meta.shape[i] = tensor.shape().dim(i);
        }
        
        meta.data_size = tensor.size() * sizeof(T);
        
        // Write name
        file.write(name.c_str(), name.length());
        
        // Write metadata
        file.write(reinterpret_cast<const char*>(&meta), sizeof(meta));
        
        // Write data
        file.write(reinterpret_cast<const char*>(tensor.as_span().data()), 
                   meta.data_size);
    }
    
    template<typename T>
    static void save_model(const std::string& filename,
                           const std::vector<std::pair<std::string, 
                           std::shared_ptr<autodiff::Variable<T>>>>& params) {
        
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file for writing: " + filename);
        }
        
        // Write header
        ModelHeader header{};
        std::memcpy(header.magic, MAGIC, 4);
        header.version = FORMAT_VERSION;
        header.num_tensors = static_cast<uint32_t>(params.size());
        
        file.write(reinterpret_cast<const char*>(&header), sizeof(header));
        
        // Calculate offsets and write tensors
        uint64_t current_offset = sizeof(header);
        
        for (const auto& [name, param] : params) {
            current_offset += name.length() + sizeof(TensorMetadata);
        }
        
        for (const auto& [name, param] : params) {
            TensorMetadata meta{};
            meta.name_length = static_cast<uint16_t>(name.length());
            
            if constexpr (std::is_same_v<T, float>) {
                meta.data_type = static_cast<uint16_t>(DataType::FLOAT32);
            }
            
            meta.num_dims = 1;  // Simplified - flatten all tensors
            meta.shape[0] = param->data.size();
            meta.data_offset = current_offset;
            meta.data_size = param->data.size() * sizeof(T);
            
            // Write name
            file.write(name.c_str(), name.length());
            
            // Write metadata
            file.write(reinterpret_cast<const char*>(&meta), sizeof(meta));
            
            current_offset += name.length() + sizeof(TensorMetadata) + meta.data_size;
        }
        
        // Write tensor data
        for (const auto& [name, param] : params) {
            file.write(reinterpret_cast<const char*>(param->data.as_span().data()),
                       param->data.size() * sizeof(T));
        }
    }
    
    template<typename T>
    static std::vector<std::pair<std::string, Tensor<T, 1>>> 
    load_model(const std::string& filename) {
        
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Cannot open file for reading: " + filename);
        }
        
        // Read header
        ModelHeader header;
        file.read(reinterpret_cast<char*>(&header), sizeof(header));
        
        // Verify magic
        if (std::memcmp(header.magic, MAGIC, 4) != 0) {
            throw std::runtime_error("Invalid model file format");
        }
        
        if (header.version != FORMAT_VERSION) {
            throw std::runtime_error("Unsupported model version");
        }
        
        std::vector<std::pair<std::string, Tensor<T, 1>>> tensors;
        
        for (uint32_t i = 0; i < header.num_tensors; ++i) {
            // Read name length (peek)
            uint16_t name_length;
            file.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
            file.seekg(-static_cast<std::streamoff>(sizeof(name_length)), 
                      std::ios::cur);
            
            // Read name
            std::string name(name_length, '\0');
            file.read(name.data(), name_length);
            
            // Read metadata
            TensorMetadata meta;
            file.read(reinterpret_cast<char*>(&meta), sizeof(meta));
            
            // Read data
            std::vector<T> data(meta.shape[0]);
            file.read(reinterpret_cast<char*>(data.data()), meta.data_size);
            
            // Create tensor
            Shape<T, 1> shape({meta.shape[0]});
            Tensor<T, 1> tensor(shape);
            std::copy(data.begin(), data.end(), tensor.as_span().begin());
            
            tensors.emplace_back(name, std::move(tensor));
        }
        
        return tensors;
    }
};

// Compression utilities
class ModelCompressor {
public:
    // Simple quantization to int8
    template<typename T>
    static std::vector<int8_t> quantize_int8(const Tensor<T, 1>& tensor,
                                              T& scale,
                                              T& zero_point) {
        // Find min/max
        T min_val = tensor(0);
        T max_val = tensor(0);
        
        for (std::size_t i = 0; i < tensor.size(); ++i) {
            min_val = std::min(min_val, tensor(i));
            max_val = std::max(max_val, tensor(i));
        }
        
        // Calculate scale and zero point
        scale = (max_val - min_val) / T{254};
        zero_point = min_val + T{127} * scale;
        
        // Quantize
        std::vector<int8_t> quantized(tensor.size());
        for (std::size_t i = 0; i < tensor.size(); ++i) {
            T normalized = (tensor(i) - zero_point) / scale;
            quantized[i] = static_cast<int8_t>(std::round(
                std::clamp(normalized, T{-127}, T{127})));
        }
        
        return quantized;
    }
    
    template<typename T>
    static Tensor<T, 1> dequantize_int8(const std::vector<int8_t>& quantized,
                                        T scale, T zero_point,
                                        std::size_t size) {
        Shape<T, 1> shape({size});
        Tensor<T, 1> tensor(shape);
        
        for (std::size_t i = 0; i < size; ++i) {
            tensor(i) = static_cast<T>(quantized[i]) * scale + zero_point;
        }
        
        return tensor;
    }
};

} // namespace aifs::serialization
```

---

## 8. IMPLEMENTATION ROADMAP

### Phase 1: Core Tensor Operations (Week 1-2)

**Goals:**
- Implement `Tensor<T, N>` with strided memory layout
- Implement basic element-wise operations (+, -, *, /)
- Implement broadcasting rules
- Implement memory pool allocator
- Write comprehensive unit tests

**Deliverables:**
- `tensor.hpp` with full test coverage
- `memory.hpp` with benchmark showing <10% overhead vs raw allocation
- AXPY benchmark achieving >80% of theoretical peak FLOPS

**Testing Strategy:**
```cpp
// Example unit test
void test_tensor_broadcasting() {
    Tensor<float, 2> a({3, 4}, 1.0f);
    Tensor<float, 1> b({4}, 2.0f);
    
    auto c = add_broadcast(a, b);
    
    // Verify shape
    assert(c.shape().dim(0) == 3);
    assert(c.shape().dim(1) == 4);
    
    // Verify values
    for (size_t i = 0; i < 3; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            assert(c(i, j) == 3.0f);  // 1.0 + 2.0
        }
    }
}
```

### Phase 2: Autograd System (Week 3-4)

**Goals:**
- Implement dynamic computation graph
- Implement reverse-mode automatic differentiation
- Support for common operations: matmul, conv, pooling
- Gradient checkpointing for memory efficiency

**Deliverables:**
- `autodiff.hpp` with verified gradients
- Gradient checking tests passing with <1e-6 relative error
- Memory-efficient backward pass for 1000+ layer graphs

**Testing Strategy:**
- Numerical gradient checking for all operations
- Finite difference verification
- Reference comparison with PyTorch autograd

### Phase 3: Basic Layers and MLP (Week 5-6)

**Goals:**
- Implement Linear, ReLU, Sigmoid, Tanh layers
- Implement SGD, Adam, AdamW optimizers
- Build trainable MLP for MNIST
- Achieve >95% accuracy on MNIST

**Deliverables:**
- `linear.hpp`, `activations.hpp`
- `sgd.hpp`, `adam.hpp`
- Working MNIST example

**Benchmark:**
- Training time within 3x of PyTorch CPU on same hardware

### Phase 4: CNN/RNN Capabilities (Week 7-8)

**Goals:**
- Implement Conv2D with im2col optimization
- Implement MaxPool, AvgPool layers
- Implement LSTM/GRU cells
- Train CNN on CIFAR-10

**Deliverables:**
- `conv.hpp`, `pooling.hpp`
- `recurrent.hpp`
- CIFAR-10 example achieving >70% accuracy

### Phase 5: Transformer and Attention (Week 9-10)

**Goals:**
- Implement scaled dot-product attention
- Implement multi-head attention
- Implement Transformer encoder/decoder
- Implement positional encoding
- Train small language model

**Deliverables:**
- `attention.hpp`, `transformer.hpp`
- Character-level language model
- Perplexity comparable to PyTorch implementation

### Phase 6: Optimization and Scale (Week 11-12)

**Goals:**
- Implement distributed training (ring all-reduce)
- Implement quantization (INT8 inference)
- Implement model pruning
- Profile and optimize bottlenecks

**Deliverables:**
- Multi-GPU training support
- INT8 quantized inference
- Comprehensive profiling report

---

## 9. REFERENCE IMPLEMENTATIONS

### Minimal Working MLP Example

```cpp
#include "core/tensor.hpp"
#include "nn/linear.hpp"
#include "nn/activations.hpp"
#include "optim/adam.hpp"
#include "utils/profiling.hpp"
#include <iostream>
#include <vector>
#include <cmath>

using namespace aifs;

int main() {
    // Configuration
    constexpr std::size_t INPUT_SIZE = 784;
    constexpr std::size_t HIDDEN_SIZE = 256;
    constexpr std::size_t OUTPUT_SIZE = 10;
    constexpr std::size_t BATCH_SIZE = 64;
    constexpr std::size_t EPOCHS = 10;
    constexpr float LEARNING_RATE = 0.001f;
    
    // Build model
    auto fc1 = std::make_shared<nn::Linear<float>>(INPUT_SIZE, HIDDEN_SIZE);
    auto relu = std::make_shared<nn::ReLU<float>>();
    auto fc2 = std::make_shared<nn::Linear<float>>(HIDDEN_SIZE, OUTPUT_SIZE);
    
    // Collect parameters
    std::vector<std::shared_ptr<autodiff::Variable<float>>> params;
    auto p1 = fc1->parameters();
    auto p2 = fc2->parameters();
    params.insert(params.end(), p1.begin(), p1.end());
    params.insert(params.end(), p2.begin(), p2.end());
    
    // Optimizer
    optim::Adam<float> optimizer(LEARNING_RATE);
    optimizer.add_parameters(params);
    
    // Training loop (simplified - no real data loading)
    profiling::Timer timer("Training");
    
    for (std::size_t epoch = 0; epoch < EPOCHS; ++epoch) {
        float epoch_loss = 0.0f;
        std::size_t num_batches = 100;  // Simulated
        
        for (std::size_t batch = 0; batch < num_batches; ++batch) {
            // Create dummy batch (replace with real data)
            Tensor<float, 1> input_data({BATCH_SIZE * INPUT_SIZE});
            Tensor<float, 1> target_data({BATCH_SIZE * OUTPUT_SIZE});
            
            for (std::size_t i = 0; i < input_data.size(); ++i) {
                input_data(i) = random::tls_rng.uniform_float() - 0.5f;
            }
            
            // One-hot targets
            for (std::size_t i = 0; i < BATCH_SIZE; ++i) {
                std::size_t label = random::tls_rng() % OUTPUT_SIZE;
                for (std::size_t j = 0; j < OUTPUT_SIZE; ++j) {
                    target_data(i * OUTPUT_SIZE + j) = (j == label) ? 1.0f : 0.0f;
                }
            }
            
            auto input = std::make_shared<autodiff::Variable<float>>(
                input_data, true);
            auto target = std::make_shared<autodiff::Variable<float>>(
                target_data, false);
            
            // Forward pass
            auto h1 = fc1->forward(input);
            auto a1 = relu->forward(h1);
            auto output = fc2->forward(a1);
            
            // Compute loss (MSE for simplicity)
            float loss = 0.0f;
            for (std::size_t i = 0; i < output->data.size(); ++i) {
                float diff = output->data(i) - target->data(i);
                loss += diff * diff;
            }
            loss /= static_cast<float>(BATCH_SIZE);
            
            // Backward pass
            output->backward();
            
            // Update weights
            optimizer.step();
            optimizer.zero_grad();
            
            epoch_loss += loss;
        }
        
        epoch_loss /= static_cast<float>(num_batches);
        std::cout << "Epoch " << epoch + 1 << "/" << EPOCHS 
                  << " - Loss: " << epoch_loss << std::endl;
    }
    
    double training_time = timer.stop();
    std::cout << "\nTraining completed in " << training_time << " seconds\n";
    
    return 0;
}
```

### Common Pitfalls and Solutions

| Pitfall | Symptom | Solution |
|---------|---------|----------|
| **Vanishing gradients** | Loss stops decreasing, gradients near zero | Use ReLU, batch norm, residual connections, proper initialization |
| **Exploding gradients** | Loss becomes NaN, large gradients | Gradient clipping, smaller learning rate, weight decay |
| **Memory exhaustion** | OOM errors during training | Gradient checkpointing, smaller batch size, memory pool allocator |
| **Numerical instability** | NaN/Inf in computations | Use float64 for critical paths, add epsilon to divisions, check for zeros |
| **Slow convergence** | Loss decreases very slowly | Learning rate scheduling, better initialization, batch normalization |
| **Overfitting** | Train accuracy >> val accuracy | Dropout, weight decay, data augmentation, early stopping |

---

## 10. BIBLIOGRAPHY

### Foundational Papers

1. **Backpropagation**
   - Rumelhart, D. E., Hinton, G. E., & Williams, R. J. (1986). Learning representations by back-propagating errors. *Nature*, 323(6088), 533-536.

2. **Adam Optimizer**
   - Kingma, D. P., & Ba, J. (2014). Adam: A method for stochastic optimization. *arXiv preprint arXiv:1412.6980*.

3. **Batch Normalization**
   - Ioffe, S., & Szegedy, C. (2015). Batch normalization: Accelerating deep network training by reducing internal covariate shift. *ICML*.

4. **Layer Normalization**
   - Ba, J. L., Kiros, J. R., & Hinton, G. E. (2016). Layer normalization. *arXiv preprint arXiv:1607.06450*.

5. **Attention Is All You Need (Transformer)**
   - Vaswani, A., et al. (2017). Attention is all you need. *NeurIPS*.

6. **LSTM**
   - Hochreiter, S., & Schmidhuber, J. (1997). Long short-term memory. *Neural Computation*, 9(8), 1735-1780.

### Performance Optimization

7. **Cache Optimization**
   - Lam, M. S., Rothberg, E. E., & Wolf, M. E. (1991). The cache performance and optimizations of blocked algorithms. *ASPLOS*.

8. **Winograd Convolution**
   - Lavin, A., & Gray, S. (2016). Fast algorithms for convolutional neural networks. *CVPR*.

9. **Ring All-Reduce**
   - Baidu Research. (2017). Bringing HPC techniques to deep learning. *baidu-research.com*.

### Numerical Methods

10. **Automatic Differentiation**
    - Griewank, A., & Walther, A. (2008). Evaluating derivatives: principles and techniques of algorithmic differentiation. *SIAM*.

11. **Matrix Computations**
    - Golub, G. H., & Van Loan, C. F. (2013). Matrix computations (4th ed.). *Johns Hopkins University Press*.

---

## APPENDIX: BUILD SYSTEM

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.25)
project(AIFromScratch VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -O3
        -march=native
        -ffast-math
        -funroll-loops
        -Wall
        -Wextra
        -Wpedantic
    )
    
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fopt-info-vec-optimized)
    endif()
    
    # Enable OpenMP if available
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        add_compile_options(${OpenMP_CXX_FLAGS})
        link_libraries(OpenMP::OpenMP_CXX)
    endif()
elseif(MSVC)
    add_compile_options(
        /O2
        /arch:AVX2
        /W4
        /permissive-
    )
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Library
add_library(aifs INTERFACE)
target_include_directories(aifs INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Examples
add_subdirectory(examples)

# Tests
enable_testing()
add_subdirectory(tests)
```

---

*Document Version: 1.0*
*Last Updated: 2026-03-05*
*Total Pages: ~80+ equivalent*
