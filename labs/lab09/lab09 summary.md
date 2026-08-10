# CS61C Lab 09：SIMD & Loop Unrolling

## 1. SIMD

SIMD（Single Instruction Multiple Data）：

> 一条指令同时处理多个数据，利用 Data-Level Parallelism（DLP）。

SSE 使用 128-bit `__m128i`，一个 `int` 为 32-bit，因此一次可以处理：

128 / 32 = 4 个 int

常用操作：

- `_mm_loadu_si128`：一次加载 4 个 int
- `_mm_cmpgt_epi32`：同时比较 4 个 int
- `_mm_and_si128`：利用 mask 过滤数据
- `_mm_add_epi32`：同时进行 4 个加法
- `_mm_storeu_si128`：vector → memory

## 2. Branch → Mask

原始代码：

if (x >= 128)
    sum += x;

SIMD 中转换为：

compare → mask → AND → add

例如：

values = [100, 200, 50, 300]
mask   = [0,   1,   0,  1]
result = [0, 200, 0, 300]

核心思想：

Control Flow → Data Flow

即使用 masking / branchless programming，使多个数据可以统一进行 SIMD 运算。

## 3. SIMD Reduction

SIMD accumulator：

sum_vec = [sum0, sum1, sum2, sum3]

最后需要：

sum0 + sum1 + sum2 + sum3

把 vector 转换成最终 scalar result，这类操作称为 Reduction。

## 4. Tail Case

SIMD 每次处理 4 个元素，因此数组长度不一定能被 4 整除。

处理方式：

SIMD 主循环
→ 处理最大的 4 的倍数部分
→ Scalar Tail 处理剩余 0~3 个元素

## 5. Loop Unrolling

SIMD 一次处理 4 个 int。

Unroll 4 次：

4 × 4 = 16 ints / iteration

作用：

- 减少 loop overhead
- 暴露更多独立指令
- 提高 Instruction-Level Parallelism（ILP）

Unroll 并非越多越好，过度 unrolling 会增加 register pressure、code size，甚至导致 register spilling。

## 6. Multiple Accumulators

不要只使用一个：

sum = add(sum, x0)
sum = add(sum, x1)
sum = add(sum, x2)

这会形成 dependency chain。

使用多个 accumulator：

sum0 += x0
sum1 += x1
sum2 += x2
sum3 += x3

可以产生多条独立 dependency chain，让 CPU 更好地利用 ILP。

## 7. Benchmark

Benchmark = 用实际测量评价程序性能，而不是凭感觉判断。

本次结果：

| Version | Time | Speedup |
|---|---:|---:|
| Naive | 4.929s | 1.00× |
| Unrolled | 4.051s | 1.22× |
| SIMD | 0.842s | 5.85× |
| SIMD + Unrolled | 0.685s | 7.19× |

核心原则：

> Measure, don't guess.

## 8. 核心性能优化思维

Scalar
→ Branch → Mask
→ SIMD（DLP）
→ Loop Unrolling
→ Multiple Accumulators
→ 减少 Dependency Chain
→ ILP
→ Benchmark

同时尽量：

- 减少不必要的 load/store
- 让数据留在 register/cache
- 减少 data movement

## 9. 与 Database 的联系

例如：

SELECT SUM(x)
FROM table
WHERE x >= 128;

可以向量化为：

Column Data
→ Vector Load
→ SIMD Compare
→ Selection Mask
→ Vector Aggregation
→ Reduction

这与数据库中的 Vectorized Query Execution 思想直接相关。

## Takeaway

不需要背 SSE intrinsic API，重点记住：

- SIMD → Data-Level Parallelism
- Branch → Mask
- Loop Unrolling → 减少循环开销
- Multiple Accumulators → 减少 dependency chain、提高 ILP
- 减少 Data Movement
- Tail Case 处理无法整除的数据
- Benchmark 验证优化效果