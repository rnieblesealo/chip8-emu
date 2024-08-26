# CHIP-8 Emulator Components

## Instructions
- 2 bytes
    - Byte 1 contains instruction itself
    - Byte 2 contains operation value
    > e.g. `$7522` says ADD 22 to register 5

## Data Registers
- 16x8-bit
- `V0-VF`; uses hex digits
- Can hold values from `0x00-0xFF`
- Reg. `VF` holds info abt. operation result

## Index Register
- 16-bit 
- Stores memory addresses for use in operations

## Program Counter
- 16-bit
- Stores next instruction
- Instruction is 2 bytes, but we can only address 1 byte; we need to:
    1. Fetch PC byte 1
    2. Fetch PC byte 2 (PC + 1)
    3. Increment PC by 2 
- Increment before instructions ran, since some instructions can change program flow

## Stack
- 16-level; 16 different PC's
- `CALL` instr. causes instructions in a diff. part of memory to run
- `RET` puts us back where the PC was
    - Stack holds PC addr. when `CALL` was ran
    - `RET` pulls PC addr. held by stack again so CPU runs it next cycle
- **Push**: Put PC into stack
- **Pop**: Pull PC from stack
- Stands to reason that stack base is PC at call time

### In Short
- `CALL` pushes curr. PC onto curr. addr, SP moves up
- `RET` decrements the SP, writes addr. to actual PC for execution 

### Stack Pointer
- 8-bit
- Tells us what PC in the stack we're at
- In implementation, stack will be arr., pointer will be index
- Popping, we'll just copy val. and decrement SP so it goes to prev. value

## Delay Timer
- 8-bit
- If 0, stays there
- If not, decrements at rate of 60hz (60 times per sec.)
- In our implementation, we'll just decrement at rate of cycle clock

## Sound Timer
- 8-bit
- Behaves same as delay timer but tone buzzes when non-zero
- In our implemetation, we won't generate sound, but we can using SDL

## Input Keys
- 16 keys matching hex values
- Bool state, either pressed or not pressed
- See keymapping in guide 

## Memory 
- 4096 bytes; address space `0x000-0xFFF`
- Address space div'd in 3 sections:
    - `0x000-0x1FF` held CHIP-8 interpreter info but we won't use it since we're emu'ing
    - `0x050-0x0A0` holds 16 built-in chars `0-F`; ROMs will look for these!
    - `0x200-0xFFF` is space for ROM instructions; anything left over is free to use

## Display
- Monochrome
- 64x32 resolution
- Pixels either on or off, 2 colors

### Drawing
- `uint32` represents each pixel
    - `0xFFFFFFFF` white
    - `0x00000000` black
- Draw instruction iterates over each pixel in sprite and XORs with display pixel
> Makes sense, think of what XOR does
- If redrawing at other location:
    - Issue draw again at curr. location; XOR will clear everything
    - Issue draw one more time at upated location
    > This causes some flicker :/
- Sprites should wrap around if drawing off-screen!

# C++ Notes

## Hex
- **[Guide!](https://learn.sparkfun.com/tutorials/hexadecimal/all)**
- Appending `u` to hex values tells the compiler they're unsigned
    - e.g. `0xFFFu`

## Functions of Bitwise Operations
- **AND** is intersection; **OR** is union
> Like in discrete!

### What Bitwise is Doing
- CHIP-8 instructions are stored MSB-first
- `&` extracts bits; if we do that to an opcode, we're like "extracting" parameters from it
> Pretty much everything follows from this!
- Within an opcode:
    - `nnn` represents an address
    - `kk` represents a byte; a value
    - `Vx` represents a register

## Code
- `class std::ifstream(filename, traits)` input class to operate on files
    - Traits is a single stream of bits that represents opening params
        - `std::ios::binary` opens in binary mode
        - `std::ios::ate` seeks end of stream after open
        > Use a bitwise OR to combine them!
- `type std::streampos` represents a position within a file  
- `function std::ifstream::tellg()` returns curr. position in input sequence/stream
- `function std::ifstream::seekg(x, y)` moves x characters (- for left, + for right) starting at y 
- `function std::ifstream::read(dest, n)` copies file bytes up to n into dest buffer
- `function memset(d, r, n)` takes buffer d and replaces up to n characters (bytes) for r

# To-Do

- [X] Fix bus error; happens when trying to decode
    - Learn GDB!
- [X] Implement RNG correctly
- [X] What do all the bitwise operations do?
