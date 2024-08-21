#include <cstdint>
#include <fstream>
#include <random>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;
const unsigned int VIDEO_WIDTH = 800;
const unsigned int VIDEO_HEIGHT = 600;

// 16 chars represented by 5 rows each; rows are like scanlines encoded as hex
// values so we need 16 characters * 5 bytes = 80 bytes array
const std::uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

class CHIP8 {
public:
  std::uint8_t registers[16]; // type is usigned fixed-width 8-bit integer,
                              // braces init. array to 0
  std::uint8_t memory[4096];
  std::uint16_t index;
  std::uint16_t pc;
  std::uint16_t stack[16];
  std::uint8_t sp;
  std::uint8_t delayTimer;
  std::uint8_t soundTimer;
  std::uint8_t keypad[16];
  std::uint32_t video[64 * 32];
  std::uint16_t opcode;

  // function pointer tables
  // see guide, this is kinda weird

  // LEFT OFF IMPLEMENTING THESE

  void (*table[0xF + 1u])();
  void (*table0[0xE + 1u])();
  void (*table8[0xE + 1u])();
  void (*tableE[0xE + 1u])();
  void (*tableF[0x65 + 1u])();

  // chip8 has a pseudo-random number generator that can write to a register!
  std::default_random_engine randGen;
  std::uniform_int_distribution<uint8_t> randByte;

  // we need to load roms first to get our instructions
  // pass filename as const since we won't modify that string
  void LoadROM(const char *filename) {
    // open file as binary stream; move pointer to end
    std::ifstream file(
        filename,
        std::ios::binary |
            std::ios::ate); // what does any of this mean? i think there's some
                            // sort of param. combining going on with the |

    if (file.is_open()) {
      // get filesize, allocate buf to hold contents
      std::streampos size = file.tellg();
      char *buffer = new char[size]; // the new operator is like malloc i think

      // go to beginning of file, fill buffer
      file.seekg(0, std::ios::beg);
      file.read(buffer, size);
      file.close();

      // load rom into chip8 memory!
      for (long i = 0; i < size; ++i) {
        memory[START_ADDRESS + i] = buffer[i];
      }

      // free buffer, we don't need dat shi
      delete[] buffer;
    }
  }

  // these are the actual instructions

  // 00E0: CLS
  void OP_00E0() {
    // set all pixels in display to 0
    //  memset sets a given number of chars (bytes) in dest to what we specify

    memset(video, 0, sizeof(video));
  }

  // 00EE: RET
  void OP_00EE() {
    // stack return
    // decrement stack pointer and set pc to instruction it points to now
    --sp;
    pc = stack[sp];
  }

  // 1xxx: JP addr
  void OP_1xxx() {
    // jump; sets program counter to value xxx
    std::uint16_t target_addr = opcode & 0x0FFFu; // what does this bitwise do?
    pc = target_addr;
  }

  // note ahead: cycle() automatically moves pc up by += 2 with it
  // if any instr. contain pc += 2, it means they're skipping therefore!

  // 2xxx: CALL addr
  void OP_2xxx() {
    // call subroutine at xxx
    // we want to return; put curr. pc at top of stack
    // pc += 2 when cycling; the pc we are pushing holds instruction after CALL
    // this "readies up" the next instruction after the subroutine
    std::uint16_t target_addr = opcode & 0x0FFFu;
    stack[sp] = pc;
    ++sp;
    pc = target_addr;
  }

  // 3xkk: SE Vx, byte
  void OP_3xkk() {
    // skip next instr if Vx = kk (what the fuck?)
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] == byte) {
      pc += 2;
    }
  }

  // 4xkk: SNE Vx, byte
  void OP_4xkk() {
    // skip next instr if Vx != kk
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] != byte) {
      pc += 2;
    }
  }

  // 5xy0: SE Vx, Vy
  void OP_5xy0() {
    // skip next instruction if Vx = Vy
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] == registers[Vy]) {
      pc += 2;
    }
  }

  // 6xkk - LD Vx, byte
  void OP_6xkk() {
    // set Vx = kk
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = byte;
  }

  // 7xkk - ADD Vx, byte
  void OP_7xkk() {
    // set Vx = Vx + kk
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t byte = opcode & 0x00FFu;

    registers[Vx] += byte;
  }

  // 8xy0 - LD Vx, Vy
  void OP_8xy0() {
    // set Vx = Vy
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
  }

  // 8xy1 - OR Vx, Vy
  void OP_8xy1() {
    // set Vx = Vx | Vy
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] |= registers[Vy];
  }

  // 8xy2 - AND Vx, Vy
  void OP_8xy2() {
    // set Vx = Vx & Vy
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] &= registers[Vy];
  }

  // 8xy3 - XOR Vx, Vy
  void OP_8xy3() {
    // set Vx = Vx ^ Vy
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] ^= registers[Vy];
  }

  // 8xy4 - ADD Vx, Vy
  void OP_8xy4() {
    // set Vx = Vx + Vy
    // set VF = 1 if overflow (res > 255) otherwise 0 (remember CDA?)
    // only lowest 8 bits of result are kept in Vx
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    std::uint16_t sum = registers[Vx] + registers[Vy];

    registers[0xF] = (sum > 255U) ? 1 : 0;

    registers[Vx] = sum & 0xFFu;
  }

  // 8xy5 - SUB Vx, Vy
  void OP_8xy5() {
    // set Vx = Vx - Vy
    // if Vx > Vy, VF is set to 1, otherwise 0
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[0xF] = (registers[Vx] > registers[Vy]) ? 1 : 0;

    registers[Vx] -= registers[Vy];
  }

  // 8xy6 - SHR Vx
  void OP_8xy6() {
    // right shift
    // if lsb of Vx is 1, VF set to 1, otherwise 0
    // then Vx divided by 2 (a right shift divides by 2 throwing out remainders)
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // save lsb
    registers[0xF] = (registers[Vx] & 0x1u);

    registers[Vx] >>= 1;
  }

  // 8xy7 - SUBN Vx, Vy
  void OP_8xy7() {
    // if Vy > Vx, VF set to 1, otherwise 0
    // set Vx = Vy - Vx; VF set to NOT borrow
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[0xF] = (registers[Vy] > registers[Vx]) ? 1 : 0;

    registers[Vx] = registers[Vy] - registers[Vx];
  }

  // 8xyE  SHL Vx, Vy
  void OP_8xyE() {
    // set Vx = Vx SHL 1
    // if msb of Vx is 1, VF set to 1, otherwise 0
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    // msb check
    // my soln: registers[0xF] = ((Vx & 0xF000u) == 1) ? 1 : 0;
    // guide soln:
    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

    registers[Vx] <<= 1;
  }

  // 9xy0 - SNE Vx, Vy
  void OP_9xy0() {
    // skip next instruction if Vx != Vy
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if (registers[Vx] != registers[Vy]) {
      pc += 2;
    }
  }

  // Annn - LD I, addr
  void OP_Annn() {
    // set val. of register I (index reg) to nnn
    std::uint16_t addr = opcode & 0x0FFFu;
    index = addr;
  }

  // Bnnn - JP V0, addr
  void OP_Bnnn() {
    // jump to location nnn + V0
    std::uint16_t addr = opcode & 0x0FFFu;
    pc = registers[0] + addr;
  }

  // Cxkk - RND Vx, byte
  void OP_Cxkk() {
    // set Vx = random byte AND kk
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t byte = opcode & 0x00FFu;

    // i'm not entirely sure how this works
    registers[Vx] = randByte(randGen) & byte;
  }

  // Dxyn - DRW Vx, Vy, nibble
  void OP_Dxyn() {
    // display n-byte sprite, stored starting at i
    // do so at (Vx, Vy)
    // VF = collision
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    std::uint8_t height = opcode & 0x000Fu;

    // modulus is so that wrapping occurs if over screen bounds
    std::uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    std::uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    // no collision as starting state
    registers[0xF] = 0;

    for (unsigned int row = 0; row < height; ++row) {
      std::uint8_t spriteByte = memory[index + row];

      for (unsigned int col = 0; col < 8; ++col) {
        // get sprite pixel value by masking with the byte row; 1 if on, 0 if
        // off
        std::uint8_t spritePixel = spriteByte & (0x80u >> col);

        // get corresponding screen pixel's address
        std::uint32_t *screenPixel = &video[(yPos + row) * VIDEO_WIDTH];

        if (spritePixel) {
          // if the sprite pixel is on and the screen pixel is also on, there is
          // collision!
          if (*screenPixel == 0xFFFFFFFF) {
            registers[0xF] = 1;
          }

          // since we get here if sprite pixel on, XOR with 0xFFFFFFFF
          // effectively does so with the pixel
          *screenPixel ^= 0xFFFFFFFF;
        }
      }
    }
  }

  // Ex9E - SKP Vx
  void OP_Ex9E() {
    // skip next instr if key with value in Vx is pressed
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t key = registers[Vx];

    if (keypad[key]) {
      pc += 2;
    }
  }

  // ExA1 - SKNP Vx
  void OP_ExA1() {
    // skip next instruction if key with value in Vx not pressed
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t key = registers[Vx];

    if (!keypad[key]) {
      pc += 2;
    }
  }

  // Fx07 - LD Vx, DT
  void OP_Fx07() {
    // set Vx = delay timer value
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] = delayTimer;
  }

  // Fx0A = LD Vx, K
  void OP_Fx0A() {
    // NOTE: this is my own solution
    // wait for keypress, store value of key in Vx
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    unsigned int pressed = 0;
    for (unsigned int i = 0; i < 16; ++i) {
      if (keypad[i]) {
        registers[Vx] = i;
        pressed = 1;
      }
    }

    // if nothing pressed, decrease pc, effectively leaving us at same instr.
    if (!pressed) {
      pc -= 2;
    }
  }

  // Fx15 - LD DT, Vx
  void OP_Fx15() {
    // set delay timer = Vx
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    delayTimer = registers[Vx];
  }

  // Fx18 - LD ST, Vx
  void OP_Fx18() {
    // set sound timer = Vx
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    soundTimer = registers[Vx];
  }

  // Fx1E - ADD I, Vx
  void OP_Fx1E() {
    // store I + Vx in register I
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    index += registers[Vx];
  }

  // Fx29 - LD F, Vx
  void OP_Fx29() {
    // set I = memory location of sprite for digit Vx
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t digit = registers[Vx];

    // a font char is 5 bytes; offset from start in multiples of this!
    index = FONTSET_START_ADDRESS + (digit * 5);
  }

  // Fx33 - LD B, Vx
  void OP_Fx33() {
    // TAKE NOTE: this is how we extract digits by mod division!
    // take Vx value and:
    // 1. place hundreds digit in memory at I
    // 2. tens at I + 1
    // 3. ones at I + 2
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    std::uint8_t value = registers[Vx];

    // ones
    memory[index + 2] = value % 10;
    value /= 10;

    // tens
    memory[index + 1] = value % 10;
    value /= 10;

    // hundreds
    memory[index] = value % 10;
  }

  // Fx55 - LD [I], Vx
  void OP_Fx55() {
    // store registers V0 through Vx in memory starting at location I
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (std::uint8_t i = 0; i <= Vx; ++i) {
      memory[index + i] = registers[i];
    }
  }

  // Fx65 - LD Vx, [I]
  void OP_Fx65() {
    // read values from mem starting at location I
    // copy reads to registers V0 thru Vx
    std::uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for (std::uint8_t i = 0; i <= Vx; ++i) {
      registers[i] = memory[index + i];
    }
  }


  // secondary table functions
  // what the fuck do they do?

  void Table0(){
    // this shit dont work
    //((*this).*(table0[opcode & 0x000Fu]))();
  }

  CHIP8() {
    // initialize pc at start address
    pc = START_ADDRESS;

    // load fonts into memory
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
      memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    // seed random generator using system clock
    randGen(); // is the system clock in the room with us? (this should take a
               // parameter!)
    randByte = std::uniform_int_distribution<uint8_t>(
        0, 255U); // wat does this do? call it to get random number!
    
    // set up function pointer table
    // table[0x0] = 
    // table[0x1] =
    // table[0x2] =
    // table[0x3] =
    // table[0x4] =
    // table[0x5] =
    // table[0x6] =
    // table[0x7] =
    // table[0x8] =
    // table[0x9] =
    // table[0xA] =
    // table[0xB] =
    // table[0xC] =
    // table[0xD] =
    // table[0xE] =
    // table[0xF] = 
  }
};
