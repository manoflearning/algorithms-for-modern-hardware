# Computer Architecture

## Instruction Set Architectures

Instruction Set Architecture (ISA):
- an abstraction of a CPU
- defines how a computer should work from a programmer's perspective

Similar to software interfaces, it gives computer engineers the ability:
- to improve on existing CPU designs
- giving its users - us, programmers - the confidence that things that worked before won't break on newer chips

An ISA essentially defines how the hardware should interpret the machine language:
- instructions and their binary encodings
- counts, sizes, and purposes of registers
- the memory model
- the input/output model

## RISC vs CISC

- **Arm**
    - Arm CPUs are reduced instruction set computers (RISC)
    - used in:
        - almost all mobile devices
        - other computer-like devices such as TVs, smart fridges, microwaves, car autopilots, and so on
        - few servers and desktops:
            - Apple Silicon (M1, M2, M3)
            - AWS's Graviton processors
            - [Fugaku (supercomputer)](https://en.wikipedia.org/wiki/Fugaku_(supercomputer))
- **x86**
    - x86 CPUs are complex instruction set computers (CISC)
    - used in almost all servers and desktops

## Assembly Language

machine language: binary-encoded instructions that specify
- the instruction number (called opcode)
- what its operands are (if there are any)
- and where to store the result (if one is produced)

assembly language: human-friendly rendition of machine language
- mnemonic codes to refer to machine code instructions
- symbolic names to refer to registers and other storage locations

`*c = *a + *b` in Arm assembly
```nasm
; *a = x0, *b = x1, *c = x2
ldr w0, [x0]    ; load 4 bytes from wherever x0 points into w0
ldr w1, [x1]    ; load 4 bytes from wherever x1 points into w1
add w0, w0, w1  ; add w0 with w1 and save the result to w0
str w0, [x2]    ; write contents of w0 to wherever x2 points
```

- LDR: LoaD Register
- ADD: Add
- STR: STore Register

`*c = *a + *b` in x86 assembly
```nasm
; *a = rsi, *b = rdi, *c = rdx 
mov eax, DWORD PTR [rsi]  ; load 4 bytes from wherever rsi points into eax
add eax, DWORD PTR [rdi]  ; add whatever is stored at rdi to eax
mov DWORD PTR [rdx], eax  ; write contents of eax to wherever rdx points
```

- MOV: MOVe
- DWORD PTR: Double WORd PoinTeR
    - word: 16 bits, double word: 32 bits (in x86)

more:
- there is almost 1:1 correspondence between machine code and assembly
- can turn any compiled program back into its assembly form using a process called dissembly
- `add eax, DWORD PTR [rdi]` is what's called fused instruction that does a load and an add in one go

### Instructions and Registers

- move: store/load a word
- inc: increment by 1
- mul: multiply
- idiv: integer division

Most instructions write their result into the first operand, which can also be involved in the computation like in the `add eax, [rdi]`. Operands can be either registers, constant values, or memory locations.

Registers are named:
- `rax`, `rbx`, `rcx`, `rdx`, `rdi`, `rsi`, `rbp`, `rsp`
- `r8`-`r15`

there are also 32-, 16-, 8-bit registers that have similar names
- `rax` is 64-bit
- `rax` -> `eax` -> `ax` -> `al`
- they are not fully separate but aliased
    - the lowest 32 bits of `rax` are `eax`
    - the lowest 16 bits of `eax` are `ax`
    - and so on
    - **the reason why basic type casts in compiled programming languages are usually free**

- general-purpose registers:
    - can use however you like in most instructions (with some exceptions)
- separate set of registers for floating-point arithmetic
- bunch of very wide registers used in vector extensions
- few special ones that are needed for confrol flow

Constants are more commonly called **immediate values** because they are embedded right into the machine code. Because it may considerably increase the complexity of the instruction encoding, some instructinos don't support immediate values or allow just a fixed subset of them.

### Moving Data

Some instructions may have the same mnemonic, but have different operand types, 
in which case they are considered distinct instructions as they may perform slightly different operations.
`mov` instruction is a vivid example of that, as it comes in around 20 different forms, all related to moving data: either between the memory and registers or just between two registers. Despite the name, it doesn't move a value into a register, but copies it, preserving the original.

When used to copy data between two registers, the `mov` instruction instead performs register renaming internally.

### Addressing Modes

`lea`:
- instruction calulates the memory address of the operand and stores it in a register in one cycle
- often used as an arithmetic trick (1 multiplication and 2 additions)

### Alternative Syntax

two x86 syntaxes are widely used now:
- AT&T syntax (or GAS), used by default by all Linux tools
    - use at GNU Asssembler
- The Intel syntax (or NASM), used by default, well, by Intel
    - use at Netwide Assembler

## Loops and Conditionals

```nasm
loop:
    add edx, DWORD PTR [rax]
    add rax, 4
    cmp rax, rcx
    jne loop
```

### Jumps

Assembly doens't have if-s, for-s, functions, or other control flow structures that high-level languages have. What it does have is `goto`, or "jump"

Jump moves the instruction pointer to a location specified by its operand.
This location may be:
- absolute address in memory
- relative to the current address
- computed during runtime

Unconditional jump `jmp` can only be used to implement `while (true)` kind of loops or stitch parts of a program together.

Conditional operations use a special `FLAGS` register, which first needs to be populated by executing instructions that perform some kind of check.

### Loop Unrolling

Unroll the loop by grouping iterations together
```c
for (int i = 0; i < n; i += 4) {
    s += a[i];
    s += a[i + 1];
    s += a[i + 2];
    s += a[i + 3];
}
```

```nasm
loop:
    add  edx, [rax]
    add  edx, [rax+4]
    add  edx, [rax+8]
    add  edx, [rax+12]
    add  rax, 16
    cmp  rax, rsi
    jne  loop
```

In practice, unrolling loops isn't always necessary for performance because modern processors don't actually execute instructions one-by-one, but maintain a queue of pending instructions so that two independent operations can be executed concurrently without waiting for each other to finish.

### An Alternative Approach

SKIPPED

## Functions and Recursion

To "call a function" in assembly, you need to jump to its beginning and then jump back.
But then two important problems arise:
- What if the caller stores data in the same registers as the callee?
- Where is "back"?

Both of these concerns can be solved by having a dedicated location in memory where we can write all the information we need to return from the function before calling it. This location is called the **stack**.

### The Stack

The hardware stack works the same way software stacks do and is similarly implemented as just two pointers:
- base pointer: marks the start of the stack and is conventionally stored in `rbp`
- stack pointer: marks the last element of the stack and is conventionally stored in `rsp`

You can implement all that with the usual memory operations and jumps, but because of how frequently it is used, there are 4-special instructions for doing this:
- `push`: writes data at the stack pointer and decrements it
- `pop`: reads data from the stack pointer and increments it
- `call`: puts the address of the following instruction on top of the stack and jumps to a label
- `ret`: reads the return address from the top of the stack and jumps to it

```nasm
; "push rax"
sub rsp, 8
mov QWORD PTR[rsp], rax

; "pop rax"
mov rax, QWORD PTR[rsp]
add rsp, 8

; "call func"
push rip ; <- instruction pointer (although accessing it like that is probably illegal)
jmp func

; "ret"
pop rcx ; <- choose any unused register
jmp rcx
```

the memory region between `rbp` and `rsp` is called a **stack frame**, and this is where local variables of functions are typically stored. it is pre-allocated at the start of the program, and if you push more data on the stack than its capacity, you encounter a stack overflow error. because modern operating systems don't actually give you memory pages until you read or write to their address space, you can freely specify a very large stack size, which acts more like a limit on how much stack memory can be used, and not a fixed amount every program has to use.

(Hardware stack is in RAM)

### Calling Conventions

SKIPPED

### Inlining

to avoid accessing stack memory

### Tail Call Elimination

if the recursive function returns right after making a recursive call, we called it **tail recursive**.

factorial function:
```cpp
int factorial(int n) {
    if (n == 0)
        return 1;
    return factorial(n - 1) * n;
}
```

tail-recursive factorial function:
```cpp
int factorial(int n, int p = 1) {
    if (n == 0)
        return p;
    return factorial(n - 1, p * n);
}
```

the primary reason why recursion can be slow is that it needs to read and write data to the stack, while iterative and tail-recursive algorithms do not.

## Indirect Branching

During assembly, all labels are converted to addresses (absolute or relative) and then encoded into jump instructions.

You can also jump by a non-constant value stored inside a register, which is called a computed jump:
```nasm
jmp rax
```

This has a few interesting applications related to dynamic languages and implementing more complex control flow.
- dynamic language: 
    - language which determines a lot in runtime (not compile time)
    - Python, JavaScript, Ruby, Lua, ...

### Multiway Branch

When we have control over the range of values that the variable can take, we can use the following trick utilizing computed jumps. create a branch table that contains pointers/offsets to possible jump locations, and just index it with the `state` variable taking values in the $[0, n)$ range.

Compilers use this technique when the values are densely packed together (not necessarily strictly sequentially)

### Dynamic Dispatch

Indirect branching is also instrumental in implementing runtime polymorphism.

## Machine Code Layout

pipeline of a CPU:
- front-end: where instructions are fetched from memory and decoded
- back-end: where they are scheduled and finally executed

typically, the performance is bottlenecked by the execution stage, and for this reason, most of out efforts in this book are going to. be spent towards optimizing around the back-end.

the reverse can happen when the front-end doesn't feed instructions to the back-end fast enough to saturate it.

### CPU Front-End

two important stages: fetch and decode

fetch stage:
- CPU simply loads a fixed-size chunk of bytes from the main memory, which contains the binary encodings of some number of instructions.
- block size is typically 32 bytes on x86, although it may vary on different machines.
- Important nuance: this block has to be aligned
    - the address of the chunk must be multiple of its size (32B, in our case)

decode stage:
- the CPU looks at this chunk of bytes, discards everything that comes before the instruction pointer, and splits the rest of them into instructions.
- machine instructions are encoded using a variable number of bytes
- we have a certain machine-dependent limit called the decode width, which means that on each cycle up to decode width instructions can be decoded and passed to the next stage

### Code Alignment

compilers typically prefer instructions with shorter machine code, because this way more instructions can fit in a single 32B fetch block, and also because it reduces the size of the binary.

but sometimes the reverse is prefereable, due to the fact that the fetched instructions' blocks must be aligned.

### Instruction Cache

SKIPPED

### Unequal Branches

SKIPPED
