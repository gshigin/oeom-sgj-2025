#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include "ecs.hpp"
#include "globals.hpp"
#include "texture.hpp"
#include "timer.hpp"

// Texture loading
bool load_assets(SDL_Renderer* renderer, texture_manager& texman) noexcept {
  auto dot_id = texman.load_texture_with_color_key_named(renderer, "assets/dot.png", "dot", 0xFF, 0xFF, 0xFF);
  auto sprite_id = texman.load_texture_named(renderer, "assets/jokr.png", "jokr");
  auto tomato_id = texman.load_texture_named(renderer, "assets/tomato.png", "tomato");
  auto bear_id = texman.load_texture_named(renderer, "assets/bear.png", "bear");
  auto zone_id = texman.load_texture_named(renderer, "assets/zone.png", "zone");

  return true;
}

void game_loop(ECS& ecs, SDL_Renderer* renderer) noexcept {
  bool quit = false;
  SDL_Event e;
  timer cap_timer;

  while (!quit) {
    cap_timer.start();

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT)
        quit = true;
      else
        ecs.handle_event(e);
    }
    ecs.cleanup();

    ecs.move_dragged();
    ecs.move_tracked();

    ecs.move();

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    ecs.render();

    SDL_RenderPresent(renderer);

    uint64_t frameNs = cap_timer.get_ticks_ns();
    if (frameNs < kNsPerFrame) {
      SDL_DelayNS(kNsPerFrame - frameNs);
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

  if (SDL_CreateWindowAndRenderer("I want to be a japaneese girl", kScreenWidth, kScreenHeight, 0, &window, &renderer) == false) {
    SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
    SDL_Quit();
    return 2;
  }

  texture_manager manager;
  if (load_assets(renderer, manager) == false) {
    SDL_Log("Unable to load assets!");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 3;
  }

  // init ecs
  ECS ecs(renderer, manager);

  float center_x = 1.f * kScreenWidth / 2;
  float center_y = 1.f * kScreenHeight / 2;

  handler_id dot1 = ecs.create_dot(1.f * kScreenWidth / 2 - 35, 1.f * kScreenHeight / 2 - 115, 20, 20);
  handler_id dot2 = ecs.create_dot(1.f * kScreenWidth / 2 + 128, 1.f * kScreenHeight / 2 - 115, 20, 20);
  handler_id jokr = ecs.create_bg();

  game_loop(ecs, renderer);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
