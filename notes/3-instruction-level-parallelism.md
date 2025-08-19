# Instruction-Level Parallelism

When programmers hear the word **parallelism**, they mostly think about multi-core parallelism. This type of parallelism is mainly about reducing latency and achieving scalability, but not about improving efficiency. You can solve a problem ten times as big with a parallel algorithm, but it would take at least ten times as many computatinoal resources.

But there are other types of parallelism, already existing inside a CPU core, that you can use for free.

### Instruction Pipelining

To execute any instruction, processors need to do a lot of preparatory work first, which includes:
- **fetching** a chunk of machine code from memory,
- **decoding** it and splitting into instructions,
- **executing** these instructions, which may involve doing some memory operations, and
- **writing** the results back into registers.

This whole sequence of operations takes up to 15-20 CPU cycles even for simple one. To hide this latency, modern CPUs use **pipelining**: after an instruction passes through the first stage, they start processing the next one right away, without waiting for the previous one to fully complete.

Pipelining does not reduce actual latency. You still need to pay these 15-20 cycles, but you only need to do it once after you've found the sequence of instructions you are going to execute.

**superscalar**:
- architectures that is capable of executing more than one instruction per cycle (most modern CPUs are superscalar)
- you can only take advantage of it if the stream of instructions cnotains groups of logically independent operations
- The instructions don't always arrive in the most convenient order, so, when possible, moden CPUs can execute them out of order to improve overall utilization and minimize pipeline stalls

## Pipeline Hazards

pipeline hazards: situations when the next instruction cannot execute on the following clock cycle

reasons:
- structural hazard: happens when two or more instructions need the sampe part of CPU
- data hazard: happens when you have to wait for an operand to be computed from some previous step
- control hazard: happens when a CPU can't tell which instructions it needs to execute next

the only way to resolve a hazard is to have a pipline stall: stop the progress of all previous steps until the cause of congestion is gone

## The Cost of Branching

When a CPU encounters a conditional jump or any other type of branching, it doesn't just sit idle until its condition is computed. Instread, it starts speculatively executing the branch that seems more likely to be taken immediately.

For this reason, the true "cost" of a branch largely depends on how well it can be predicted by the CPU.

### An Experiment

SKIPPED

### Branch Prediction

SKIPPED

### Pattern Detection

SKIPPED

### Hinting Likeliness of Branches

SKIPPED

## Branchless Programming

### Predication

with branches
```cpp
for (int i = 0; i < N; i++)
    if (a[i] < 50) s += a[i];
```

without branches
```cpp
for (int i = 0; i < N; i++)
    s += (a[i] < 50) * a[i];
```
`a[i] < 50` doesn't occur a branch.
Assembly doesn't have boolean operations, and it translates into integer operations

### When Predication Is Beneficial

SKIPPED

### Larget Examples

SKIPPED

## Instruction Tables

SKIPPED

## Throughput Computing

Optimizing for **latency** vs. **throughput**:
- When optimizing data structure queries or small one-time or branchy algorithms, you need to look up the latencies of its instructions, mentally construct the execution graph of the computation, and then try to reorganize it so that the critical path is shorter.
- When optimizing hot loops and large-dataset algorithms, you need to look up the throughputs of their instructions, count how many times each one is used per iteration, determine which of them is the bottleneck, and then try to restructure the loop so that it is used less often.

### Example

SKIPPED

### The General Case

If an instruction has a latency of $x$ and a throughput of $y$, then you would need to use $x \cdot y$ accumulators to saturate it.