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
  texman.load_texture_named(renderer, "assets/head0_256.png", "head0_256");
  texman.load_texture_named(renderer, "assets/head1_256.png", "head1_256");
  texman.load_texture_named(renderer, "assets/left_eye.png", "left_eye");
  texman.load_texture_named(renderer, "assets/right_eye.png", "right_eye");
  texman.load_texture_named(renderer, "assets/table.png", "table");
  texman.load_texture_named(renderer, "assets/room.png", "room");
  texman.load_texture_from_text_named(renderer, font, "000000", "score", 0x00, 0x00, 0x00, 0xFF);

  return true;
}

void game_loop(ECS& ecs, game_state& state, SDL_Renderer* renderer) noexcept {
  bool quit = false;
  SDL_Event e;
  timer cap_timer;

  while (!quit) {
    cap_timer.start();

    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_EVENT_QUIT)
        quit = true;
      else
        ecs.handle_event(e, state);
    }

    ecs.cleanup();

    ecs.move_dragged();
    ecs.move_tracked(state);
    ecs.loop_logic(state);

    ecs.move();

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(renderer);

    ecs.render();

    SDL_RenderPresent(renderer);

    uint64_t frameNs = cap_timer.get_ticks_ns();
    if (frameNs < kNsPerFrame) {
      SDL_DelayNS(kNsPerFrame - frameNs);
    }
    ++state.frame_counter;
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

  if (SDL_CreateWindowAndRenderer("All Eyes On Me", kScreenWidth, kScreenHeight, 0, &window, &renderer) == false) {
    SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
    SDL_Quit();
    return 3;
  }

  if (SDL_Surface* icon = IMG_Load("assets/icon.png"); icon == nullptr) {
    SDL_Log("Unable to load image %s! SDL_image error: %s\n", "assets/icon.png", SDL_GetError());
  } else {
    SDL_SetWindowIcon(window, icon);
    SDL_DestroySurface(icon);
  }

  TTF_Font* font = nullptr;
  std::string fontPath{"assets/press_start.ttf"};
  if (font = TTF_OpenFont(fontPath.c_str(), 56); font == nullptr) {
    SDL_Log("Could not load %s! SDL_ttf Error: %s\n", fontPath.c_str(), SDL_GetError());
    return 4;
  }

  texture_manager manager;
  if (load_assets(renderer, font, manager) == false) {
    SDL_Log("Unable to load assets!");
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    SDL_Quit();
    TTF_Quit();
    return 5;
  }

  // init ecs
  ECS ecs(renderer, font, manager);
  game_state state{0, 0, false, 100};

  float center_x = 1.f * kScreenWidth / 2;
  float center_y = 1.f * kScreenHeight / 2;

  auto room_id = ecs.register_object(center_x, center_y);
  ecs.add_texture(room_id, manager.get_texture_id("room"), kScreenWidth, kScreenHeight);

  auto leye_id = ecs.register_object(335, 330 + 70);
  ecs.add_texture(leye_id, manager.get_texture_id("left_eye"), 100, 100);
  ecs.add_tracker(leye_id, 335, 330, 23);

  auto reye_id = ecs.register_object(462, 335 + 70);
  ecs.add_texture(reye_id, manager.get_texture_id("right_eye"), 100, 100);
  ecs.add_tracker(reye_id, 462, 335, 23);

  auto head_id = ecs.register_object(center_x, center_y + 180);
  ecs.add_texture(head_id, manager.get_texture_id("head0_256"), 512, 512);
  ecs.add_tracker(head_id, center_x, center_y + 80, 10);
  state.head_texture_next = manager.get_texture_id("head1_256");
  state.head_id = head_id.position_id;

  auto head_trigger_id = ecs.register_object(center_x, center_y);
  ecs.add_dimetions(head_trigger_id, 230, 200);
  ecs.make_clickable(head_trigger_id);

  auto table_id = ecs.register_object(center_x, center_y + 200);
  ecs.add_texture(table_id, manager.get_texture_id("table"), 800, 200);

  auto score_id = ecs.register_object(122, 38);
  ecs.add_texture(score_id, manager.get_texture_id("score"), 224, 56);

  game_loop(ecs, state, renderer);

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  TTF_CloseFont(font);
  SDL_Quit();
  TTF_Quit();

  return 0;
}
