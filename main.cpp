#include <chrono>
#include <cstdint>
#include <fstream>
#include <random>

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_SIZE = 80;
const unsigned int FONTSET_START_ADDRESS = 0x50;

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

  CHIP8() {
    // initialize pc at start address
    pc = START_ADDRESS;

    // load fonts into memory
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
      memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    // seed random generator using system clock
    randGen(); // is the system clock in the room with us? (this should take a parameter!)
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U); // wat does this do? call it to get random number!
  }
};
