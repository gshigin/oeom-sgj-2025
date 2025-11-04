#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "ecs.hpp"
#include "globals.hpp"
#include "texture.hpp"
#include "timer.hpp"

// Texture loading
bool load_assets(SDL_Renderer* renderer, TTF_Font* font, texture_manager& texman) noexcept {
  std::string fontPath{"assets/press_start.ttf"};
  if (font = TTF_OpenFont(fontPath.c_str(), 28); font == nullptr) {
    SDL_Log("Could not load %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError());
    return false;
  }

  texman.load_texture_named(renderer, "assets/banana.png", "banana");
  texman.load_texture_named(renderer, "assets/beer.png", "beer");
  texman.load_texture_named(renderer, "assets/cake.png", "cake");
  texman.load_texture_named(renderer, "assets/glue.png", "glue");
  texman.load_texture_named(renderer, "assets/head0_128.png", "head0_128");
  texman.load_texture_named(renderer, "assets/head0_256.png", "head0_256");
  texman.load_texture_named(renderer, "assets/kormen.png", "kormen");
  texman.load_texture_named(renderer, "assets/krieg.png", "krieg");
  texman.load_texture_named(renderer, "assets/left_eye.png", "left_eye");
  texman.load_texture_named(renderer, "assets/right_eye.png", "right_eye");
  texman.load_texture_named(renderer, "assets/rubik.png", "rubik");
  texman.load_texture_named(renderer, "assets/sirok.png", "sirok");
  texman.load_texture_named(renderer, "assets/table.png", "table");
  texman.load_texture_named(renderer, "assets/trusi.png", "trusi");
  texman.load_texture_named(renderer, "assets/wiskey1.png", "wiskey1");
  texman.load_texture_named(renderer, "assets/wiskey2.png", "wiskey2");
  texman.load_texture_named(renderer, "assets/room.png", "room");

  texman.load_texture_from_text_named(renderer, font, "1234", "score", 0x00, 0x00, 0x00, 0xFF);

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
  if (TTF_Init() == false) {
    SDL_Log("TTF_Init failed: %s\n", SDL_GetError());
    return 2;
  }

  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;

  if (SDL_CreateWindowAndRenderer("All eyes on me", kScreenWidth, kScreenHeight, 0, &window, &renderer) == false) {
    SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
    SDL_Quit();
    return 3;
  }

  texture_manager manager;
  TTF_Font* font = nullptr;
  if (load_assets(renderer, font, manager) == false) {
    SDL_Log("Unable to load assets!");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    SDL_Quit();
    TTF_Quit();
    return 4;
  }

  // init ecs
  ECS ecs(renderer, manager);

  float center_x = 1.f * kScreenWidth / 2;
  float center_y = 1.f * kScreenHeight / 2;

  auto room_id = ecs.register_object(center_x, center_y);
  ecs.add_texture(room_id, manager.get_texture_id("room"), kScreenWidth, kScreenHeight);

  auto leye_id = ecs.register_object(335, 332);
  ecs.add_texture(leye_id, manager.get_texture_id("left_eye"), 100, 100);
  ecs.add_tracker(leye_id, 335, 332, 18);

  auto reye_id = ecs.register_object(462, 337);
  ecs.add_texture(reye_id, manager.get_texture_id("right_eye"), 100, 100);
  ecs.add_tracker(reye_id, 462, 337, 18);

  auto head_id = ecs.register_object(center_x, center_y + 80);
  ecs.add_texture(head_id, manager.get_texture_id("head0_256"), 512, 512);
  ecs.add_tracker(head_id, center_x, center_y + 80, 5);

  auto table_id = ecs.register_object(center_x, center_y + 200);
  ecs.add_texture(table_id, manager.get_texture_id("table"), 800, 200);

  auto score_id = ecs.register_object(center_x, center_y + 200);
  ecs.add_texture(score_id, manager.get_texture_id("score"), 80, 20);

  game_loop(ecs, renderer);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_CloseFont(font);
  SDL_Quit();
  TTF_Quit();

  return 0;
}
