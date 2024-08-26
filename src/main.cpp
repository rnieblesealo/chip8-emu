#include <chrono>
#include <cstdlib>
#include <iostream>

#include "core.h"
#include "platform.h"

int main(int argc, const char **argv) {
  // gather arguments, pretty straightforward stuff
  if (argc != 4) {
    std::cerr << "Usage: " << argv[0] << "<scale> <delay> <rom>\n";
    std::exit(EXIT_FAILURE);
  }

  int videoScale = std::stoi(argv[1]);
  int cycleDelay = std::stoi(argv[2]);
  const char *romFilename = argv[3];

  // start up platform
  Platform platform("CHIP-8 Emulator", VIDEO_WIDTH * videoScale,
                    VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);

  // start core, load rom
  CHIP8 core;
  core.LoadROM(romFilename);

  // pitch is length of scanline; amt. of pixels to get to the pixel below it
  // makes sense; we're storing display data as an 1-d array
  int videoPitch = sizeof(core.video[0]) * VIDEO_WIDTH;

  // store program time this cycle
  auto lastCycleTime = std::chrono::high_resolution_clock::now();

  bool quit = false;
  while (!quit) {
    quit = platform.ProcessInput(core.keypad);

    // take difference in time between last and curr. frame; deltatime
    auto currentTime = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

    // if deltatimer exceeds cycle delay, well, cycle!
    // you've seen this before
    if (dt > cycleDelay) {
      lastCycleTime = currentTime;

      core.Cycle();

      platform.Update(core.video, videoPitch);
    }
  }

  return 0;
}
