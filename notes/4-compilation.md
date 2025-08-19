# Compilation

## Stages of Compilation

4 stages of turning C programs into executables:
1. **Peprocessing**: expands macros, pulls included source from header files, and strips off comments from source code: `gcc -E source.c` (outputs preprocessed source to stdout)
2. **Compiling**: parses the source, checks for syntax errors, converts it into an intermediate representation, performs optimizations, and finally translates it into assembly language: `gcc -S file.c` (emits an `.s` file)
3. **Assembly**: turns assembly language into machine code, except that any external function calls like `printf` are substituted with placeholders: `gcc -c file.c` (emits an `.o` file, called object file)
4. **Linking**: finally resolves the function calls by plugging in their actual addresses, and produces an executable binary: `gcc -o binary file.c`

There are possibilities to improve program performance in each of these stages.

### Interprocedural Optimization

We have the last stage, linking, because it is both easier and faster to compile programs on a file-by-file basis and then link those files together. This way you can do this in parallel and also cache intermediate results.

It also gives the ability to distribute code as libraries, which can be either static or shared:
- **Static**:
    - conllections of precompiled object files that are merged with other sources by the compiler to produce a single executable
- **Dynamic** or **shared**: 
    - precompiled executables that have additional meta-information about where their callables are, references to which are resolved during runtime. As the name suggests, this allows sharing the compiled binaries between multiple programs.

**Link-time Optimization (LTO)**:
    - main advantage of static libraries
    - can perform various interprocedural optimizations that require more context than just the signatures of library functions, such as function inlining or dead code elimination
    - it is possible because modern compilers also store some form of IR in object files, which allows them to perform certain lightweight optimizations on the program as a whole
    - this also allows using differenct compiled languages in the same program, which can even be optimized across language barriers if their compilers use the same IR.
    - in C and C++, the way to make sure no performance is lost due to separate compilation is to create a header-only library

### Inspecting the Output

Apart from the assembly, the other most helpful level of abstraction is the IR on which compilers perform optimizations. The IR defines the flow of computation itself and is much less dependent on architecture features like # of registers or a particular instruction set.

## Flags and Targets

### Optimization Levels

There are 4 and a half main levels of optimization for speed in GCC:
- `O0`: default. no optimizations
- `O1`: does a few "low-hanging fruit" optimizations, almost not affecting the compilation time
- `O2`: enables all optimizations that are known to have little to no negative side effects and take a reasonable time to complete (what most projects use for production builds)
- `O3`: does very aggressive optimization, enabling almost all correct optimizations implemented in GCC.
- `Ofast`: does everything in `-O3`, plus a few more optimizations flags that may break strict standard compliance, but not in a way that would be critial for most applications

### Specifying Targets

SKIPPED

### Multiversioned Functions

can use attribute-based syntax to select between multiversioned functions automatically during compile time.

## Situational Optimizations

Most compiler optimizations enabled by `-O2`, `-O2` are guaranteed to either improve or at least not seriously hurt performance. Those that aren't included in `-O3` are either not strictly standard-compliant, or highly circumstantional and require some additional input to help decide whether using them is beneficial.

### Loop Unrolling

Loop unrolling is disabled by default, unless the loop takes a small constant number of iterations known at compile time (in which case it will be replaced with a completely jump-free, repeated sequence of instructions)

Loop unrolling makes binary larger, and may or may not make it run faster.

### Function Inlining

Inlining is best left for the compiler to decide, but you can influence it with `inline` keyword:
```cpp
inline int square(int x) { return x * x; }
```

The hint may be ignored though if the compiler thinks that the potential performance gains are not worth it. You can force inlining by adding the `always_inline` attribute:
```cpp
#define FORCE_INLINE inline __attribute__((always_inline))
```

### Likeliness of Branches

example:
```cpp
int factorial(int n) {
    if (n > 1) [[likely]]
        return n * factorial(n - 1);
    else [[unlikely]]
        return 1;
}
```

### Profile-Guided Optimization

To make a decision about branch reordering, function inlining, or loop unrolling, we need answers to questions like these:
- How often is this branch taken?
- How often is this function called?
- What is the average number of iterations in this loop?

**Profile-guided optimization (PGO)**:
- a technique that uses profiling data to improve performance beyond what can be achieved with just static analysis.
- in a nutshell: adding timers and counters to the points of interest in the program, compiling and running it on real data, and then compiling it again, but this time supplying additional information from the test run
- the whole process is automated by modern compilers

## Contract Programming

### Why Undefined Behavior Exists

two major groups of actions that cause UB:
- operations that are almost certainly unintentional bugs: dividing by zero, dereferencing a null pointer, or reading from uninitialized memory
    - can compile and run a program with sanitizers to catch undefined behavior early. `-fsanitize=undefined`
- operations that have slightly different observable behavior on different platforms

Designating something as undefined instead of implementation-defined behavior also helps compilers in optimization

integer overflow of signed integer types is UB.
but integer overflow of unsigned integer types are defined..

### Removing Corner Cases

In C++ STL, `vector` and `array` have an "unsafe" [] operator and a "safe" `.at()` method that goes something like this:
```cpp
T at(size_t k) {
    if (k >= size())
        throw std::out_of_range("Array index exceeds its size");
    return _memory[k];
}
```

These checks are rarely actually executed during runtime because the compiler can often prove (during compile time) that each access will be within bounds.

### Assumptions

When the compiler can't prove that inexistence of corner cases, you can provide the additional information using the mechanism of undefined behavior.

Clang: `__builtin_assume`
GCC: `__builtin_unreachable`
```cpp
void assume(bool pred) {
    if (!pred)
        __builtin_unreachable(); // Declare it as UB
}
```
usage: put `assume(k < vector.size())` before `at` in the example above.

### Arithmetic

For floating-point arithmetic, you can just disable strict standard compliance with the `-ffast-math` flag. Unless you do it explicitly, the compiler can't do anything but execute arithmetic operations in the same order as in the source code without any optimizations.

For integer arithmetic, **corner cases** are important in the context of optimizing arithmetic.

Consider the case of division by 2:
```cpp
unsigned div_unsigned(unsigned x) {
    return x / 2
};
```
with is identical to single right shift.

But think of it with signed integer and `x` is negative, then right shift doesn't work!

So we can assume there's no such case where `x` is negative.
```cpp
int div_signed(int x) {
    assume(x >= 0);
    return x / 2;
}
```

### Memory Aliasing

Memory aliasing: situation where same data address has more than two variable names.

Example:
```cpp
void add(int *a, int *b, int n) {
    for (int i = 0; i < n; i++)
        a[i] += b[i]
}
```

It seems like each iteration is independent, and it can be executed in parallel and vctorized.
But it it true only if `a` and `b` do not intersect.

This is why we have `const` and `restrict` keywords.

```cpp
void add(int * __restrict__ a, const int * __restrict__ b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] += b[i];
    }
}
```

### C++ Contracts

- `expects`: precondition, i.e., the function's expectation of its arguments and/or the state of other objects upon into the function
- `ensures`: postcondition, i.e., a condition that a function should ensure for the return value and/or the state of objects upon expit from the function
- `assert`: assertion, i.e., a condition that should be satisfied where it appears in a function body

```cpp
bool is_power_of_two(int m) {
    return m > 0 && (m & (m - 1) == 0);
}

int mod_power_of_two(int x, int m)
    [[ expects: x >= 0 ]]
    [[ expects: is_power_of_two(m) ]]
    [[ ensures r: r >= 0 && r < m ]]
{
    int r = x & (m - 1);
    [[ assert: r = x % m ]]
    return r;
}
```

## Precomputation

When compilers can infer that a certain variable does not depend on any user-provided data, they can compute its value during compile time and turn it into a constant by embedding it into the generated machine code.

### Constant Expressions

`constexpr`: if it is called by passing constants its value is guaranteed to be computed during compile time:
```cpp
constexpr int fibonacci(int n) {
    if (n <= 2) return 1;
    return fibonacci(n - 1) + fibonacci(n - 2);
}
```

even though technically we perform a constant number of iterations and call `fibonacci` with parameters known at compile time, they are technically not compile-time constants. It's up to the compiler whether to optimize this loop or not.