#include <SDL2/SDL.h>
#include <SDL_pixels.h>
#include <SDL_render.h>
#include <SDL_video.h>
#include <cstdint>

class Platform {
public:
  // what does any of this mean?
  Platform(const char* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight){
    SDL_Init(SDL_INIT_VIDEO); 
    
    window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
  }

  ~Platform(){
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  void Update(const void* buffer, int pitch){
    SDL_UpdateTexture(texture, nullptr, buffer, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
  }

  bool ProcessInput(std::uint8_t* keys){
    bool quit = false;
    SDL_Event event;
    
    // continue here
  }

private:
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
};
