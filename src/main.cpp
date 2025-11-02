/*This source code copyrighted by Lazy Foo' Productions 2004-2025
and may not be redistributed without written permission.*/

/* Headers */
// Using SDL, SDL_image, and STL string
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

#include "texture.hpp"
#include "timer.hpp"

#include <cmath>
#include <limits>
#include <string>

/* Constants */
// Screen dimension constants
constexpr int kScreenWidth{640};
constexpr int kScreenHeight{480};
constexpr int kScreenFps{60};

constexpr uint16_t kNoId = std::numeric_limits<uint16_t>::max();

// globals
SDL_Window* gWindow{nullptr};

// The renderer used to draw to the window
SDL_Renderer* gRenderer{nullptr};

// Texture manager
texture_manager gManager;

// Ids
uint32_t gTomatoId;
uint32_t gDotId;
uint32_t gSpriteId;

// Texture loading
bool loadMedia() {
  gDotId = gManager.load_texure_with_color_key(gRenderer, "assets/dot.png", 0xFF, 0xFF, 0xFF);

  gSpriteId = gManager.load_texure(gRenderer, "assets/jokr.png");
  gManager.resize(gSpriteId, kScreenWidth, kScreenHeight);

  gTomatoId = gManager.load_texure(gRenderer, "assets/tomato.png");

  return true;
}

// entity
struct handler_id {
  uint16_t position_id;
  uint16_t dimention_id;
  uint16_t texture_id;
  uint16_t motion_id;
  uint16_t drag_id;
  uint16_t tracker_id;
};

// center of object and object texture
struct position {
  float x;
  float y;
};

struct dimention {
  float width;
  float height;
};

struct motion {
  float dx;
  float dy;
  float ddx;
  float ddy;

  // uint16_t position_id;
};

struct drag {
  float shift_x;
  float shift_y;

  // uint16_t position_id;
  // uint16_t dimention_id;
};

struct mouse_tracker {
  float anchor_x;
  float anchor_y;
  float max_radius;

  // uint16_t position_id;
};

// TODO: Try OOD table method to arrange data
// components
std::vector<handler_id> handlers;

std::vector<position> positions;
std::vector<dimention> dimentions;
std::vector<motion> motions;

std::vector<drag> drags;  // bruh

std::vector<mouse_tracker> trackers;

// systems
struct movable {
  uint16_t position_id;
  uint16_t motion_id;
};

struct drawable {
  uint16_t position_id;
  uint16_t texture_id;
};

struct draggable {
  uint16_t position_id;
  uint16_t dimention_id;
  uint16_t drag_id;
  bool is_dragged;
};

struct mouse_trackable {
  uint16_t position_id;
  uint16_t tracker_id;
};

std::vector<movable> movs;
std::vector<drawable> draws;
std::vector<draggable> draggs;
std::vector<mouse_trackable> tracks;

handler_id register_object(float x, float y) {
  handler_id h{.position_id = static_cast<uint16_t>(positions.size()),
               .dimention_id = kNoId,
               .texture_id = kNoId,
               .motion_id = kNoId,
               .drag_id = kNoId,
               .tracker_id = kNoId};
  positions.emplace_back(x, y);

  return h;
}

void add_tracker(handler_id& handler, float x, float y, float r) {
  handler.tracker_id = trackers.size();
  trackers.emplace_back(x, y, r);

  tracks.emplace_back(handler.position_id, handler.tracker_id);
}

void add_dimetions(handler_id& handler, float w, float h) {
  handler.dimention_id = dimentions.size();
  dimentions.emplace_back(w, h);
}

void add_drag(handler_id& h) {
  h.drag_id = drags.size();
  drags.emplace_back(0.f, 0.f);
}

void add_texture(handler_id& h, uint16_t texture_id) {
  h.texture_id = texture_id;

  draws.emplace_back(h.position_id, h.texture_id);
}

void make_draggable(handler_id& h) {
  draggs.emplace_back(h.position_id, h.dimention_id, h.drag_id, false);
}

void move_dragged() {
  float x = -1.f, y = -1.f;
  SDL_GetMouseState(&x, &y);

  for (const auto& sys : draggs) {
    if (sys.is_dragged) {
      auto& pos = positions[sys.position_id];
      const auto& dr = drags[sys.drag_id];

      pos.x = x + dr.shift_x;
      pos.y = y + dr.shift_y;
    }
  }
}
void move_tracked() {
  for (const auto sys : tracks) {
    auto& pos = positions[sys.position_id];
    const auto& anc = trackers[sys.tracker_id];

    if (SDL_GetMouseFocus() == nullptr) {
      pos.x = anc.anchor_x;
      pos.y = anc.anchor_y;
    } else {
      // Get mouse position
      float x = -1.f, y = -1.f;
      SDL_GetMouseState(&x, &y);

      const float x_anc = static_cast<float>(anc.anchor_x);
      const float y_anc = static_cast<float>(anc.anchor_y);

      float x_vec = x - x_anc;
      float y_vec = y - y_anc;
      const float z_vec = 200;

      float norm = std::sqrt(x_vec * x_vec + y_vec * y_vec + z_vec * z_vec);

      x_vec /= norm;
      y_vec /= norm;

      pos.x = anc.max_radius * x_vec + x_anc;
      pos.y = anc.max_radius * y_vec + y_anc;
    }
  }
}

void handle_event(SDL_Event& e) {
  if (e.type == SDL_EVENT_MOUSE_MOTION) {
    move_tracked();
    move_dragged();
  }

  else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && !(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LEFT)) {
    // nobody is dragged!
    for (auto& drag_sys : draggs) {
      drag_sys.is_dragged = false;
    }
  }

  else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LEFT)) {
    // someone can be dragged!
    float x = -1.f, y = -1.f;
    SDL_GetMouseState(&x, &y);
    for (auto& drag_sys : draggs) {
      auto& dr = drags[drag_sys.drag_id];
      const auto& dim = dimentions[drag_sys.dimention_id];
      const auto& pos = positions[drag_sys.position_id];

      if (pos.x - dim.width / 2 <= x && x <= pos.x + dim.width / 2 && pos.y - dim.height / 2 <= y && y <= pos.y + dim.height / 2) {
        drag_sys.is_dragged = true;

        dr.shift_x = pos.x - x;
        dr.shift_y = pos.y - y;
      }
    }
  }
}

// hight order builders
handler_id create_dot(float x, float y, float dot_r, float anchor_r) {
  auto h = register_object(x, y);

  add_texture(h, gDotId);
  // gManager.resize(h.texture_id, 2 * dot_r, 2 * dot_r);

  add_tracker(h, x, y, anchor_r);

  return h;
}

handler_id create_tomato(float x, float y) {
  auto h = register_object(x, y);

  add_texture(h, gTomatoId);

  add_dimetions(h, 30, 30);
  add_drag(h);

  make_draggable(h);

  return h;
}

handler_id create_bg() {
  auto h = register_object(kScreenWidth / 2, kScreenHeight / 2);
  add_texture(h, gSpriteId);
  gManager.resize(h.texture_id, kScreenWidth, kScreenHeight);

  return h;
}

void render() {
  for (auto dr : draws) {
    const auto& pos = positions[dr.position_id];

    gManager.render(gRenderer, dr.texture_id, pos.x, pos.y);
  }
}

bool init() {
  bool success{true};

  if (SDL_Init(SDL_INIT_VIDEO) == false) {
    SDL_Log("SDL could not initialize! SDL error: %s\n", SDL_GetError());
    success = false;
  } else {
    if (SDL_CreateWindowAndRenderer("ЩЩЩЩЩЩЩЩЩЩЩЩ", kScreenWidth, kScreenHeight, 0, &gWindow, &gRenderer) == false) {
      SDL_Log("Window could not be created! SDL error: %s\n", SDL_GetError());
      success = false;
    }
  }

  return success;
}

void close() {
  // Destroy window
  SDL_DestroyRenderer(gRenderer);
  gRenderer = nullptr;
  SDL_DestroyWindow(gWindow);
  gWindow = nullptr;

  // Quit SDL subsystems
  SDL_Quit();
}

int main(int argc, char* args[]) {
  // Final exit code
  int exitCode{0};

  // Initialize
  if (init() == false) {
    SDL_Log("Unable to initialize program!\n");
    exitCode = 1;

    close();

    return exitCode;
  }

  // Load media
  if (loadMedia() == false) {
    SDL_Log("Unable to load media!\n");
    exitCode = 2;

    close();

    return exitCode;
  }

  // The quit flag
  bool quit{false};

  // The event data
  SDL_Event e;
  SDL_zero(e);

  // Timer to cap frame rate
  LTimer capTimer;

  handler_id dot1 = create_dot(kScreenWidth / 2 - 35, kScreenHeight / 2 - 115, 20, 20);
  handler_id dot2 = create_dot(kScreenWidth / 2 + 128, kScreenHeight / 2 - 115, 20, 20);

  handler_id jokr = create_bg();

  handler_id tomato = create_tomato(kScreenWidth / 4, kScreenHeight / 4);

  // The main loop
  while (quit == false) {
    // Start frame time
    capTimer.start();

    // Get event data
    while (SDL_PollEvent(&e) == true) {
      if (e.type == SDL_EVENT_QUIT) {
        quit = true;
      }

      handle_event(e);
    }

    // Fill the background
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(gRenderer);

    render();

    // Update screen
    SDL_RenderPresent(gRenderer);

    // Cap frame rate
    constexpr Uint64 nsPerFrame = 1000000000 / kScreenFps;
    Uint64 frameNs{capTimer.getTicksNS()};
    if (frameNs < nsPerFrame) {
      SDL_DelayNS(nsPerFrame - frameNs);
    }
  }

  // Clean up
  close();

  return exitCode;
}
