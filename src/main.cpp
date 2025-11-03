/*This source code copyrighted by Lazy Foo' Productions 2004-2025
and may not be redistributed without written permission.*/

/* Headers */
// Using SDL, SDL_image, and STL string
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include "ecs.hpp"
#include "globals.hpp"
#include "texture.hpp"
#include "timer.hpp"

// Texture loading
bool loadMedia(SDL_Renderer* renderer, texture_manager& texman) {
  auto dot_id = texman.load_texture_with_color_key_named(renderer, "assets/dot.png", "dot", 0xFF, 0xFF, 0xFF);

  auto sprite_id = texman.load_texture_named(renderer, "assets/jokr.png", "jokr");
  texman.resize(sprite_id, kScreenWidth, kScreenHeight);

  auto tomato_id = texman.load_texture_named(renderer, "assets/tomato.png", "tomato");

  return true;
}

void game_loop(ECS& ecs, SDL_Renderer* renderer) {
  bool quit = false;
  SDL_Event e;
  LTimer capTimer;

  while (!quit) {
    capTimer.start();

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT)
        quit = true;
      else
        ecs.handle_event(e);
    }

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    ecs.render();

    SDL_RenderPresent(renderer);

    constexpr Uint64 nsPerFrame = 1'000'000'000 / kScreenFps;
    Uint64 frameNs = capTimer.getTicksNS();
    if (frameNs < nsPerFrame) {
      SDL_DelayNS(nsPerFrame - frameNs);
    }
  }
}

int main(int argc, char* args[]) {
  if (SDL_Init(SDL_INIT_VIDEO) == false) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
  }

  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;

  if (SDL_CreateWindowAndRenderer("ЩЩЩЩЩЩЩЩЩЩЩЩ", kScreenWidth, kScreenHeight, 0, &window, &renderer) == false) {
    SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
    SDL_Quit();
    return 2;
  }

  texture_manager manager;
  if (loadMedia(renderer, manager) == false) {
    SDL_Log("Unable to load media!");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 3;
  }

  // init ecs
  ECS ecs(renderer, manager);

  handler_id dot1 = ecs.create_dot(kScreenWidth / 2 - 35, kScreenHeight / 2 - 115, 20, 20);
  handler_id dot2 = ecs.create_dot(kScreenWidth / 2 + 128, kScreenHeight / 2 - 115, 20, 20);
  handler_id jokr = ecs.create_bg();
  handler_id tomato = ecs.create_tomato(kScreenWidth / 4, kScreenHeight / 4);

  game_loop(ecs, renderer);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
