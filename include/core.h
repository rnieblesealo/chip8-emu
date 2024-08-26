#ifndef CORE_H
#define CORE_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <random>



const unsigned int START_ADDRESS = 0x200;
const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;
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
  // u = unsigned, n_t = integer bitwidth
  std::uint8_t memory[4096] = {0};
  std::uint8_t registers[16] = {0};
  std::uint16_t index = {0};
  std::uint16_t pc = {0};
  std::uint16_t stack[16] = {0};
  std::uint8_t sp = {0};
  std::uint8_t delayTimer = {0};
  std::uint8_t soundTimer = {0};
  std::uint8_t keypad[16] = {0};
  std::uint32_t video[64 * 32] = {0};
  std::uint16_t opcode;

  typedef void (CHIP8::*CHIP8Func)();

  CHIP8Func table[0xF + 1u] = {0};
  CHIP8Func table0[0xE + 1u] = {0};
  CHIP8Func table8[0xE + 1u] = {0};
  CHIP8Func tableE[0xE + 1u] = {0};
  CHIP8Func tableF[0x65 + 1u] = {0};

  std::default_random_engine randGen;
  std::uniform_int_distribution<uint8_t> randByte;

  CHIP8();

  void LoadROM(const char *filename);

  void OP_00E0();
  void OP_00EE();
  void OP_0nnn();
  void OP_1nnn();
  void OP_2nnn();
  void OP_3xkk();
  void OP_4xkk();
  void OP_5xy0();
  void OP_6xkk();
  void OP_7xkk();
  void OP_8xy0();
  void OP_8xy1();
  void OP_8xy2();
  void OP_8xy3();
  void OP_8xy4();
  void OP_8xy5();
  void OP_8xy6();
  void OP_8xy7();
  void OP_8xyE();
  void OP_9xy0();
  void OP_Annn();
  void OP_Bnnn();
  void OP_Cxkk();
  void OP_Dxyn();
  void OP_Ex9E();
  void OP_ExA1();
  void OP_Fx07();
  void OP_Fx0A();
  void OP_Fx15();
  void OP_Fx18();
  void OP_Fx1E();
  void OP_Fx29();
  void OP_Fx33();
  void OP_Fx55();
  void OP_Fx65();

  void OP_NULL();

  void Table0();
  void Table8();
  void TableE();
  void TableF();

  void Cycle();
};

#endif
