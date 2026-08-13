# numc

Here's what I did in project 4:
-

# CS61C Project 4 Summary

## 中文版

Proj4 对我来说最有价值的地方，并不是单纯实现一个矩阵库，而是把 CS61C 前面学过的很多知识真正串联了起来。

## 1. C、指针与内存管理

项目首先要求使用 C 实现矩阵，包括：

- 动态内存分配
- 矩阵基本运算
- Matrix Slice
- 内存释放
- Reference Count

其中 Slice 让我进一步理解了 **memory ownership**。

Slice 不一定复制原矩阵的数据，而是可以直接引用原矩阵的一部分：

```text
Parent Matrix
      ↓
Underlying Data
      ↑
    Slice
```

因此必须考虑对象之间的生命周期，否则可能出现：

- Dangling Pointer
- Double Free
- Use After Free

这让我把之前学习的 `pointer`、`malloc/free` 和 memory layout 真正应用到了一个完整项目中。

---

## 2. Python-C Interface

Proj4 还要求把 C 实现的矩阵包装成 Python 可以使用的 `numc.Matrix`。

整体结构可以理解成：

```text
Python
  ↓
PyObject / Matrix61c
  ↓
matrix *
  ↓
C implementation
```

Python 提供方便的高级接口，而真正的矩阵计算由 C 完成。

例如 Python 中：

```python
C = A * B
```

最终会通过 Python-C API 调用 C 中的矩阵乘法。

这让我理解了为什么很多 Python 科学计算库可以同时做到：

```text
Python → 易用
C/C++  → 高性能
```

---

## 3. Cache Locality

矩阵乘法最开始可以直接写成三重循环：

```text
i → j → k
```

但是改变循环顺序后：

```text
i → k → j
```

即使算法复杂度仍然是：

```text
O(n³)
```

实际运行速度也可能明显不同。

原因在于 **memory access pattern**。

让内层循环尽可能连续访问内存，可以提高：

- Spatial Locality
- Cache Hit Rate
- Memory Bandwidth Utilization

这让我真正理解了：

> 算法复杂度相同，并不意味着实际性能相同。

---

## 4. SIMD / AVX

接下来使用 AVX 对矩阵运算进行向量化。

一个 `double` 是 64 bit，而 AVX 的 YMM register 是 256 bit，因此一次可以处理：

```text
256 / 64 = 4 doubles
```

例如：

```c
__m256d a = _mm256_loadu_pd(...);
__m256d b = _mm256_loadu_pd(...);
__m256d c = _mm256_add_pd(a, b);
_mm256_storeu_pd(..., c);
```

原来 CPU 需要进行多次 scalar operation：

```text
a0 + b0
a1 + b1
a2 + b2
a3 + b3
```

现在可以使用一条 SIMD 指令同时处理多个数据。

这对应 CS61C 中的：

```text
Data-Level Parallelism
```

---

## 5. OpenMP

最后使用 OpenMP 实现多线程并行。

例如：

```c
#pragma omp parallel for
```

可以让不同线程同时处理矩阵的不同部分。

这对应：

```text
Thread-Level Parallelism
```

因此最终的性能优化可以理解成三个层次：

```text
Cache Locality
      ↓
让内存访问更高效

SIMD / AVX
      ↓
一个线程一次处理多个数据

OpenMP
      ↓
多个线程同时执行计算
```

这三种优化并不是互相替代，而是可以叠加使用。

---

## 6. Performance 的一个重要认识

这个项目还让我意识到：

> Parallelism 并不意味着任何情况下都会更快。

对于很小的矩阵，OpenMP 的线程管理、函数调用以及 Python-C interface 都存在额外开销。

因此可能出现：

```text
Small workload
Computation < Parallel overhead
→ slower
```

而对于较大的矩阵：

```text
Large workload
Computation >> Parallel overhead
→ parallelism becomes useful
```

所以性能优化需要考虑 workload size，而不是简单地认为“线程越多越快”。

---

## 总结

Proj4 最重要的收获，是把之前分散学习的知识连接了起来：

```text
C / Pointer / malloc
        ↓
Memory Layout
        ↓
Cache Locality
        ↓
SIMD / AVX
        ↓
OpenMP / Multithreading
        ↓
Python-C Interface
```

现在再看到 Python 中简单的一句：

```python
C = A * B
```

我会意识到它背后可能涉及：

```text
Python Operator
      ↓
Python-C API
      ↓
C Matrix Implementation
      ↓
Memory Access
      ↓
Cache
      ↓
SIMD
      ↓
Multithreading
      ↓
CPU
```

因此，这个项目让我开始从：

> **代码能不能运行？**

进一步思考：

> **代码在机器上到底是怎么运行的？为什么一种实现会比另一种实现更快？**

这也是我认为 Proj4 对学习 CS61C 最有价值的地方。

---

# CS61C Project 4 Summary

## English Version

The most valuable part of Project 4 was not simply implementing a matrix library. It connected many concepts that I had previously learned separately in CS61C.

## 1. C, Pointers, and Memory Management

The project first required implementing matrices in C, including:

- Dynamic memory allocation
- Matrix operations
- Matrix slices
- Memory deallocation
- Reference counting

Slices helped me better understand **memory ownership**.

A slice does not necessarily copy data. Instead, it can share the underlying memory of its parent matrix:

```text
Parent Matrix
      ↓
Underlying Data
      ↑
    Slice
```

Therefore, object lifetimes must be managed carefully to avoid problems such as:

- Dangling pointers
- Double frees
- Use-after-free errors

This connected C pointers, `malloc/free`, and memory layout to a real implementation.

---

## 2. Python-C Interface

Project 4 also required exposing the C matrix implementation as a Python `numc.Matrix` object.

Conceptually, the structure is:

```text
Python
  ↓
PyObject / Matrix61c
  ↓
matrix *
  ↓
C implementation
```

Python provides a convenient high-level interface, while the actual computation is performed in C.

For example:

```python
C = A * B
```

can eventually invoke the matrix multiplication implementation written in C through the Python-C API.

This helped me understand a common design used by numerical libraries:

```text
Python → usability
C/C++  → performance
```

---

## 3. Cache Locality

Matrix multiplication can initially be implemented using three nested loops:

```text
i → j → k
```

Changing the loop order to something like:

```text
i → k → j
```

does not change the asymptotic complexity:

```text
O(n³)
```

but it can significantly change real performance.

The reason is the **memory access pattern**.

Sequential memory access can improve:

- Spatial locality
- Cache hit rate
- Memory bandwidth utilization

This made an important systems idea much more concrete:

> The same algorithmic complexity does not imply the same real-world performance.

---

## 4. SIMD / AVX

The next optimization was SIMD using AVX instructions.

A `double` occupies 64 bits, while an AVX YMM register contains 256 bits. Therefore, one register can contain:

```text
256 / 64 = 4 doubles
```

For example:

```c
__m256d a = _mm256_loadu_pd(...);
__m256d b = _mm256_loadu_pd(...);
__m256d c = _mm256_add_pd(a, b);
_mm256_storeu_pd(..., c);
```

Instead of executing several scalar operations:

```text
a0 + b0
a1 + b1
a2 + b2
a3 + b3
```

SIMD allows multiple values to be processed together.

This corresponds to:

```text
Data-Level Parallelism
```

from CS61C.

---

## 5. OpenMP

Finally, OpenMP was used to introduce multithreading.

For example:

```c
#pragma omp parallel for
```

allows independent parts of a matrix computation to be executed by multiple threads.

This corresponds to:

```text
Thread-Level Parallelism
```

The overall optimization strategy can therefore be summarized as:

```text
Cache Locality
      ↓
More efficient memory access

SIMD / AVX
      ↓
Process multiple values per instruction

OpenMP
      ↓
Execute independent work with multiple threads
```

These techniques can also be combined.

---

## 6. An Important Performance Lesson

Another important lesson from the project was:

> Parallelism does not automatically make every workload faster.

For small matrices, thread management, function calls, and the Python-C interface introduce additional overhead.

Therefore:

```text
Small workload
Computation < Parallel overhead
→ slower
```

For larger workloads:

```text
Large workload
Computation >> Parallel overhead
→ parallelism becomes useful
```

Performance optimization therefore depends on workload size rather than simply using more threads.

---

## Conclusion

The main value of Project 4 was connecting many systems concepts together:

```text
C / Pointers / malloc
        ↓
Memory Layout
        ↓
Cache Locality
        ↓
SIMD / AVX
        ↓
OpenMP / Multithreading
        ↓
Python-C Interface
```

Now, when I see a simple Python expression such as:

```python
C = A * B
```

I can think about the layers underneath it:

```text
Python Operator
      ↓
Python-C API
      ↓
C Matrix Implementation
      ↓
Memory Access
      ↓
Cache
      ↓
SIMD
      ↓
Multithreading
      ↓
CPU
```

Project 4 therefore shifted my perspective from asking:

> **Does the code work?**

to also asking:

> **How does the code actually execute on the machine, and why can one implementation be faster than another?**

That was the most valuable lesson I learned from this project.