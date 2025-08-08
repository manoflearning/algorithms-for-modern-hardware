# 8. External Memory

when we do `add` operation, `add` by itself only takes one cycle to execute.

but if:
- the data is stored in the main memory (RAM): ~100ns, 400 cycles
- if it was accessed recently, it is probably cached: 4-5 cycles ~ 50 cycles
- external memory (e.g., hard drive): 5ms, 10^7 cycles

## 8.1. Memory Hierarchy

- CPU registers
- CPU Caches (L1, L2, L3, L4)
- RAM
- SSD/HDD
- Internet

each type of memory has a few important characteristics:
- total size $M$;
- block size $B$;
- latency, that is, how much time it takes to fetch one byte;
- bandwidth, which may be higher than just the block size times latency, meaning that I/O operations can "overlap";
- cost in the amortized sense, including the price for the chip, its energy requirements, maintenance, and so on.

| type | $M$ | $B$ | latency | bandwidth | $/GB/mo |
| ---- | --- | --- | ------- | --------- | ------- |
| L1 | 10K (~=$10^5$) | 64B | 2ns | 80G/s | - |
| L2 | 100K (~=$10^6$) | 64B | 5ns | 40G/s | - |
| L3 | 1M/core (~=$10^7$) | 64B | 20ns | 20/s | - |
| RAM | GBs (~=$10^9$) | 64B | 100ns | 10G/s | 1.5 |
| SSD | TBs (~=$10^{12}$) | 4K | 0.1ms | 5G/s | 0.17 |
| HDD | TBs (~=$10^{12}$) | - | 10ms | 1G/s | 0.04 |
| S3 | $\infty$| - | 150ms | $\infty$ | 0.02 |

### Volatile Memory

volatile memory: CPU registers, CPU caches, RAM

CPU cache system has a concept of a **cache line**, which is the basic unit of data transfer between the CPU and the RAM. the size of a cache line is mostly 64 bytes

programmer cannot control caching on the CPU level explicitly

### Non-Volatile Memory

- solid state drives (SSD): relatively low latency on the order of 0.1ms ($10^5$ ns). have limited lifespans as each cell can only be written to a limited number of times
- hard disk drives (HDD):
    - rotating physical disks with read/write head attached to them
    - to read a memory location, you need to wait until the disk rotates to the right position and then very precisely move the head to it. this results in some very weird access patterns where reading one byte randomly may take the same time as reading the next 1MB of data
- network-attached storage
    - network file system (NFS): protocol for mounting the file system of another computer over the network
    - API-based distributed storage systems

## 8.2. Virtual Memory

early operating systems gave every process a freedom of reading and modifying any memory region they want, including those allocated for other processes. but it has problems..:
- what if one of the processes is buggy or outright malicious? we need to:
    - prevent it from modifying the memory allocated for other processes while keeping inter-process communication through memory
    - memory fragmentation: lack of contiguous memory
    - how do we access non-RAM memory types? how do we plug a flash drive and read a specific file from it?
        - we cannot access non-RAM memory types with read & write physical addresses directly

### Memory Paging

virtual memory gives each process the impression that it fully controls a contiguous region of memory, but in reality may be mapped to multiple smaller blocks of the physical memory (RAM, HDD, SSD)

- pages (typically 4KB in size)
- page table:
    - special hardware data structure, which contains the mappings of virtual page addresses to the physical ones
    - lives in RAM
- address translation: page number to physical address (using page table)

address translation poses a difficult problem: the number of memory pages itself may be large.

there are two ways to make it faster:
- use a special cache for the page table itself, called translation lookaside buffer (TLB)
- increase the page size

### Mapping External Memory

modern operating systems support **memory mapping**, which lets you open a file and use its contents as if they were in the main memory.

if we map file that is bigger than single memory page:
- its reads will be done lazily
- its writes will be buffered and committed to the file system when the operating decides to (usually on the program termination or when the system runs of out of RAM)

swap file:
- lets the os automatically use parts of an SSD or an HDD as an extension of the main memory when there is not enough real RAM
- when out of memory occurs, no crashing but terrible slowdown happens

## 8.3. External Memory Model

to reason about the performance of memory-bound algorithms, Big-O notation is not enough! it counts every primitive operation (`add`, `compare`, `load`) as the same unit.

### Cache-Aware Model

**external memory model**:
- simply ignore every operation that is not an I/O operation
- consider one level of cache hierarchy and assume the following about the hardware and the problem:
    - the size of the dataset is $N$, and it is all stored in external memory, which we can read/write in blocks of $B$ elements in a unit time (reading a whole block and just one element takes the same time)
    - can store $M$ elements in internal memory, meaning that we can store up to $\lfloor M / B \rfloor$ blocks
    - only care about I/O operations: any computations done in-between the reads and the writes are free
    - additionally assume $N \gg M \gg B$
        - def of $\gg$: if $x \gg y$, $x \geq y^{1+\epsilon}$ for some fixed $\epsilon > 0$

mostly focus on the case where the internal memory is RAM, and the external memory is SSD or HDD. under these settings, reasonable block size $B$ is about 1MB, internal memory size $M$ is usually a few gigabytes, and $N$ is up to a few terabytes.
- NOTE: the "block" here is a different concept with physical minimum block of each memory types. it's effective moving size. it's not a rule, it's a guideline.

def: $\text{SCAN}(N) = O(\lceil N / B \rceil) \text{ IOPS}$ <br>
(IOPS: input/output operations per second)

## 8.4. External Sorting

the algorithm will be based on the standard merge sorting algorithm, but designed for the external memory model

### Merge

problem: given two sorted arrays $a$ and $b$ of lengths $N$ and $M$, produce a single sorted array $c$ of length $N + M$ containing all of their elements

```cpp
void merge(int *a, int *b, int *c, int n, int m) {
    int i = 0, j = 0;
    for (int k = 0; k < n + m; k++) {
        if (i < n && (j == m || a[i] < b[j]))
            c[k] = a[i++];
        else
            c[k] = b[j++];
    }
}
```

it works in $SCAN(N + M)$ I/O operations

but think of $k$-way merging: you are given $k$ arrays of total size $N$
- in the standard RAM model, the asymptotic complexity would be multiplied $k$, since we would need to perform $O(k)$ comparisons to fill each next elements
- but in the external memory model, since everything we do in-memory doesn't cost us anything, its asymptotic complexity would not change as long as we can fit $(k + 1)$ full blocks in memory, that is, if $k = O(M / B)$

**tall cache asumption**: We alreay assume $M \gg B$, i.e., $M \geq B^{1+\epsilon}$ for $\epsilon > 0$.
then, we can fit any sub-polynomial number of blocks in memory, certainly including $O(M/B)$ 
- what does "sub-polynomial" mean? <br>
if $f(n)$ is sub-polynomial, $f(n) = o(n^{\alpha})$ for all $\alpha > 0$, positive integer $n$.
    - what is Little-o notation? <br>
    $f(n) = o(g(n)) \iff \lim_{n \to \infty} \frac{f(n)}{g(n)} = 0$

### Merge Sorting

- split the arrays into $O(\frac{N}{M})$ chunks of consecutive elements
- complexity: $O(\frac{N}{B} \log_2 \frac{N}{M})$

TODO

### $k$-way Mergesort

TODO

### Joining

TODO

## 8.5. List Ranking

problem: given a singly-linked list, compute the rank of each element, equal to its distance from the last element

the list nodes are stored arbitrarily, and in the worst case, reading each new node may require reading a new block

<!-- **basic assumption**:
- we have record table `(id, next_id, w)`
    - but HOW THE FUCK can we sort the record table?
- unlike the original linked list, record table is contiguous in memory -->

### Algorithm

consider a slightly more general version of the problem.
now, each element has a weight $w_i$, and for each element, we need to compute the sum of weights of all its preceding elements instead of just its rank.
initially, all weights equal to $1$

TODO

### Applications

TODO

## 8.6. Eviction Policies

**eviction policy**:
1. automatic caching comes with its own challenges
2. when a program runs out of working memory to store its intermediate data, it needs to get rid of one block for a new one
3. a concrete rule ofr deciding which data to retain in the cache in case of conflicts is called an eviction policy

rule can be arbitrary, popular choices are:
- first in first out (FIFO)
- least recently used (LRU)
- last in first out (LIFO) and most recently used (MRU)
- least-frequently used (LFU)
- random replacement (RR): does not need to maintain any data structures with block information

there is a natural trade-off between the accuracy of eviction policies and the additional overhead due to the complexity of their implementations

### Optimal Caching

there is the theoretical **optimal policy**, denoted as $OPT$ or $MIN$,
which determines, for a given sequence of queries, which blocks should be retained to minimize the total number of cache misses.

this is just a simple greedy approach called **Bélády algorithm**:
- keep the latest-to-be-used block
    - can be shown by contradiction that doing so is always one of the optimal solutions
    - (no prefetch)
- downside is that you either need to:
    - have these queries in advance
    - somehow be able to pedict the future

in terms of asymptotic complexity, it doesn't really matter which particular method is used.

theorem. let $LRU_{M}$ and $OPT_{M}$ denote the number of blocks a computer with $M$ internal memory 
would need to access while executing the same algorithm following the least recently used cache replacement policy and the theoretical minimum respectively. then:
$LRU_M \leq 2 \times OPT_{M/2}$

TODO: understand the proof of the theorem

### Implement Caching

we need to implement an LRU cache, and we are storing some moderately large objects.

we need:
- hash table to find the data itself
    - hash of the query as the key
    - pointer to a heap-allocated result string as the value
- queue
    - where we put:
        - the current time
        - IDs/keys of the objects
        - store when each object was accessed the last time (not necessarily as a timestamp, any increasing counter will suffice)
    - we cannot simply remove element from the queue, because it may be that they were accessed again since their record was added to the queue
    - since we may store unnecessarily duplicated elements
        1. implement the queue over a doubly-linked list and store a pointer to the block's node in the queue in the hash table
        2. when we have a cache hit, we follow the pointer and remove the node from the linked list in constant time
        3. add a newer node to the end of the queue

## 8.7. Cache-Oblivious Algorithms

in the context of the external memory model, there are two types of efficient algorithms:
- cache-aware algorithms that are efficient for known $B$ and $M$.
- **cache-oblivious algorithms** that are efficient for any $B$ and $M$.

cache-oblivious algorithms are interesting because they automatically become **optimal for all memory levels in the cache hierarchy**, and not just the one for which they were specifically tuned

### Matrix Transposition

naive approach:
```cpp
for (int i = 0; i < n; i++)
    for (int j = 0; j < i; j++)
        swap(a[j * N + i], a[i * N + j])
```

used a single pointer to the beginning of the memory region instead of a 2d array to be more explicit about its memory operations

I/O complexity: $O(N^2)$ (because the writes are not sequential)

### Algorithm

the cache-oblivious algorithm relies on the following block matrix identity:
$$\begin{bmatrix}A & B \\ C & D\end{bmatrix}^T = \begin{bmatrix}A^T & C^T \\ B^T & D^T\end{bmatrix}$$

we can solve the problem recursively using a divide-and-conquer approach:
- divide the input matrix into 4 smaller matrices
- transpose each one recursively
- combine results by swapping the corner result matrices

```cpp
void transpose(int *a, int n, int N) {
    if (n <= 32) {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < i; j++)
                swap(a[i * N + j], a[j * N + i]);
    } else {
        int k = n / 2;

        transpose(a, k, N);
        transpose(a + k, k, N);
        transpose(a + k * N, k, N);
        transpose(a + k * N + k, k, N);
        
        for (int i = 0; i < k; i++)
            for (int j = 0; j < k; j++)
                swap(a[i * N + (j + k)], a[(i + k) * N + j]);
        
        if (n & 1)
            for (int i = 0; i < n - 1; i++)
                swap(a[i * N + n - 1], a[(n - 1) * N + i]);
    }
}
```

I/O complexity: $O(\frac{N^2}{B})$

### Matrix Multiplication

naive approach:
```cpp
// don't forget to initialize c[][] with zeroes
for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
        for (int k = 0; k < n; k++)
            c[i * n + j] += a[i * n + k] * b[k * n + j];
```

I/O complexity: $O(N^3)$

one well-known optimization is to transpose $B$ first:
```cpp
for (int i = 0; i < n; i++)
    for (int j = 0; j < i; j++)
        swap(b[j][i], b[i][j])
// ^ or use our faster transpose from before

for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
        for (int k = 0; k < n; k++)
            c[i * n + j] += a[i * n + k] * b[j * n + k]; // <- note the indices
```

I/O complexity: $O(N^3 / B + N^2)$

### Algorithm

we need to divide the data until it fits into lowest cache (i.e., $N^2 \leq M$). for matrix multiplication, this equates to using this formula:
$$\begin{bmatrix}A_{11} & A_{12} \\ A_{21} & A_{22}\end{bmatrix} \begin{bmatrix}B_{11} & B_{12} \\ B_{21} & B_{22}\end{bmatrix} = \begin{bmatrix}A_{11} B_{11} + A_{12} B_{21} & A_{11} B_{12} + A_{12} B_{22} \\ A_{21} B_{11} + A_{22} B_{21} & A_{21} B_{12} + A_{22} B_{22}\end{bmatrix}$$

```cpp
void matmul(const float *a, const float *b, float *c, int n, int N) {
    if (n <= 32) {
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                for (int k = 0; k < n; k++)
                    c[i * N + j] += a[i * N + k] * b[k * N + j];
    } else {
        int k = n / 2;

        // c11 = a11 b11 + a12 b21
        matmul(a,     b,         c, k, N);
        matmul(a + k, b + k * N, c, k, N);
        
        // c12 = a11 b12 + a12 b22
        matmul(a,     b + k,         c + k, k, N);
        matmul(a + k, b + k * N + k, c + k, k, N);
        
        // c21 = a21 b11 + a22 b21
        matmul(a + k * N,     b,         c + k * N, k, N);
        matmul(a + k * N + k, b + k * N, c + k * N, k, N);
        
        // c22 = a21 b12 + a22 b22
        mul(a + k * N,     b + k,         c + k * N + k, k, N);
        mul(a + k * N + k, b + k * N + k, c + k * N + k, k, N);

        if (n & 1) {
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++)
                    for (int k = (i < n - 1 && j < n - 1) ? n - 1 : 0; k < n; k++)
                        c[i * N + j] += a[i * N + k] * b[k * N + j];
        }
    }
}
```

### Analysis

TODO

### Strassen Algorithm

TODO

## 8.8. Spatial and Temporal Locality

to precisely assess the performance of an algorithm in terms of its memory operations, we need to take into account multiple characteristics of the cache system:
- the number of cache layers
- the memory and block sizes of each layer
- the exact strategy used for cache eviction by each layer
- (sometimes) details of the memory paging mechanism

it often makes more sense to reason about cache performance in more qualitative terms

degree of cache reuse primarily in two ways:
- temporal locality
- spatial locailty

### Depth-First vs. Breadth-First

consider a divide-and-conquer algorithm such as merge sorting. we have two approaches to implementing it:
- recursively, or "depth-first"
- iteratively, or "breadth-first"

seems like "breadth-first" approach is more cumbersome, but faster. but really?

generally, recursion is indeed slow, but this is not the case for this and many similar divide-and-conquer algorithms:
- iterative approach: has the advantage of only doing sequential I/O
- recursive approach: has the advantage of temporal locality, when a segment fully fits into the cache

### Dynamic Programming

iterative approach is faster (at least in most cases) because of spatial locality, and moreover, some memory optimizations

### Sparse Table

SKIPPED

### Array-of-Structs vs. Struct-of-Arrays

Struct-of-Arrays (SoA):
```cpp
int left_child[maxn], right_child[maxn], key[maxn], size[maxn];
```

vs.

Array-of-Structs (AoS):
```cpp
struct Node {
    int left_child, right_child, key, size;
};

Node t[maxn];
```

AoS layout is usually preferred for data structures, but SoA still has good uses:
while it is worse for searching, it is much better for linear scanning

this difference in design is important in data processing applications.
for example, databases can be either row- or column-oriented