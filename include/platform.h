#ifndef PLATFORM_H
#define PLATFORM_H

#include <SDL2/SDL.h>
#include <SDL_events.h>
#include <SDL_keycode.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <cstdint>

class Platform {
private:
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;

public:
  Platform(const char *title, int windowWidth, int windowHeight, int textureWidth, int textureHeight);
  ~Platform();

  void Update(const void *buffer, int pitch);
  bool ProcessInput(std::uint8_t *keys);
};

#endif
