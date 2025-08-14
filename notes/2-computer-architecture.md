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

## Instructions and Registers

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

## Moving Data

Some instructions may have the same mnemonic, but have different operand types, 
in which case they are considered distinct instructions as they may perform slightly different operations.
`mov` instruction is a vivid example of that, as it comes in around 20 different forms, all related to moving data: either between the memory and registers or just between two registers. Despite the name, it doesn't move a value into a register, but copies it, preserving the original.

When used to copy data between two registers, the `mov` instruction instead performs register renaming internally.

## Addressing Modes

`lea`:
- instruction calulates the memory address of the operand and stores it in a register in one cycle
- often used as an arithmetic trick (1 multiplication and 2 additions)

## Alternative Syntax

two x86 syntaxes are widely used now:
- AT&T syntax (or GAS), used by default by all Linux tools
    - use at GNU Asssembler
- The Intel syntax (or NASM), used by default, well, by Intel
    - use at Netwide Assembler

