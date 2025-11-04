#pragma once

#include "globals.hpp"
#include "texture.hpp"

#include "SDL3_ttf/SDL_ttf.h"

#include <iomanip>
#include <sstream>

namespace {
uint64_t lcg32(uint64_t counter) {
  return (1664525u * counter + 1013904223u);
}
}  // namespace

struct game_state {
  uint64_t frame_counter;
  uint32_t score;
  bool game_over;
  uint32_t health;

  bool is_eyes_idle = false;
  uint64_t idle_start;

  bool is_eyes_closed = false;
  uint16_t head_id;
  uint16_t head_texture_next;
  uint16_t next_blink_frame = 0;
  uint64_t eyes_closed_start;
};

// entity
// TODO: OOD data base design; handler_id needs only primary key! or handler is a bitmask
struct handler_id {
  uint16_t position_id;  // position id is unique key for handler
  uint16_t obj_size_id;
  uint16_t texture_id;
  uint16_t tex_size_id;
  uint16_t motion_id;
  uint16_t drag_id;
  uint16_t tracker_id;
};

// center of object and object texture
struct position {
  float x;
  float y;
};

struct object_size {
  float width;
  float height;
};

struct texture_size {
  float width;
  float height;
};

struct motion {
  float dx;
  float dy;
  float ax;
  float ay;
};

struct drag {
  float shift_x;
  float shift_y;
};

struct mouse_tracker {
  float anchor_x;
  float anchor_y;
  float target_x;
  float target_y;
  float max_radius;
};

struct movable {
  uint16_t position_id;
  uint16_t motion_id;
};

struct drawable {
  uint16_t position_id;
  uint16_t texture_id;
  uint16_t tex_size_id;
};

struct draggable {
  uint16_t position_id;
  uint16_t obj_size_id;
  uint16_t drag_id;
  bool is_dragged;
};

struct mouse_trackable {
  uint16_t position_id;
  uint16_t tracker_id;
  uint16_t motion_id;
};

struct clickable {
  uint16_t position_id;
  uint16_t obj_size_id;
  bool is_pressed;
  // press_event_id
  uint16_t release_event_id;
  // pressed_texture_id
};

struct trigger_zone {
  uint16_t position_id;
  uint16_t obj_size_id;
  bool is_in_zone;
  // enter_event_id
  // leave_event_id
  // triggered_texture_id
};

// TODO: Try OOD table method to arrange data
struct ECS {
  static constexpr uint16_t kNoId = std::numeric_limits<uint16_t>::max();

  SDL_Renderer* renderer;
  TTF_Font* font;
  texture_manager& manager;

  ECS(SDL_Renderer* r, TTF_Font* f, texture_manager& m) : renderer(r), font(f), manager(m) {}

  // components
  std::vector<position> positions;
  std::vector<object_size> object_sizes;

  std::vector<texture_size> texture_sizes;

  std::vector<motion> motions;
  std::vector<drag> drags;
  std::vector<mouse_tracker> trackers;

  // handlers
  std::vector<handler_id> handlers;

  // systems (component links)
  std::vector<movable> movs;
  std::vector<drawable> draws;
  std::vector<draggable> draggs;
  std::vector<mouse_trackable> tracks;
  std::vector<clickable> buttons;
  std::vector<trigger_zone> zones;

  // to delete
  std::vector<uint16_t> to_delete;

  // register entity
  handler_id register_object(float x, float y) noexcept {
    handler_id h{.position_id = static_cast<uint16_t>(positions.size()),
                 .obj_size_id = kNoId,
                 .texture_id = kNoId,
                 .tex_size_id = kNoId,
                 .motion_id = kNoId,
                 .drag_id = kNoId,
                 .tracker_id = kNoId};
    positions.emplace_back(x, y);
    handlers.emplace_back(h);

    return h;
  }

  void destroy_entity(const handler_id& h) noexcept { to_delete.emplace_back(h.position_id); }
  void destroy_entity(uint16_t position_id) noexcept { to_delete.emplace_back(position_id); }

  void cleanup() noexcept {
    for (const auto pos_id : to_delete) {
      SDL_Log("deleting entt %d\n", pos_id);
      std::erase_if(handlers, [&](const handler_id& h) { return h.position_id == pos_id; });

      std::erase_if(draggs, [&](const draggable& sys) { return sys.position_id == pos_id; });
      std::erase_if(draws, [&](const drawable& sys) { return sys.position_id == pos_id; });
      std::erase_if(tracks, [&](const mouse_trackable& sys) { return sys.position_id == pos_id; });
    }

    to_delete.clear();
  }

  // attach component
  void add_tracker(handler_id& handler, float x, float y, float r) noexcept {
    handler.tracker_id = trackers.size();
    trackers.emplace_back(x, y, x, y, r);

    make_movable(handler);

    tracks.emplace_back(handler.position_id, handler.tracker_id, handler.motion_id);
  }

  void add_dimetions(handler_id& handler, float w, float h) noexcept {
    handler.obj_size_id = object_sizes.size();
    object_sizes.emplace_back(w, h);
  }

  void add_drag(handler_id& h) noexcept {
    h.drag_id = drags.size();
    drags.emplace_back(0.f, 0.f);
  }

  void add_texture(handler_id& handler, uint16_t texture_id) noexcept {
    auto [w, h] = manager.get_texture_sizes(texture_id);

    add_texture(handler, texture_id, w, h);
  }

  void add_texture(handler_id& handler, uint16_t texture_id, float w, float h) noexcept {
    handler.texture_id = texture_id;
    handler.tex_size_id = texture_sizes.size();

    texture_sizes.emplace_back(w, h);
    draws.emplace_back(handler.position_id, handler.texture_id, handler.tex_size_id);
  }

  void make_draggable(handler_id& h) noexcept { draggs.emplace_back(h.position_id, h.obj_size_id, h.drag_id, false); }
  void make_clickable(handler_id& h) noexcept { buttons.emplace_back(h.position_id, h.obj_size_id, false, 0); }
  void make_triggerable(handler_id& h) noexcept { zones.emplace_back(h.position_id, h.obj_size_id, false); }
  void make_movable(handler_id& h) noexcept {
    h.motion_id = motions.size();
    motions.emplace_back(0.f, 0.f, 0.f, 0.f);

    movs.emplace_back(h.position_id, h.motion_id);
  }

  // logic
  void move_dragged() noexcept {
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

  void move_tracked(game_state& state) noexcept {
    static constexpr float stiffness = 400.0f;
    static constexpr float damping = 15.0f;

    float x_target, y_target;

    if (state.frame_counter % (60 * 5) == 0) {
      state.is_eyes_idle = true;
      state.idle_start = state.frame_counter;
    }

    if (state.is_eyes_idle == true && (state.frame_counter - state.idle_start == 60)) {
      state.is_eyes_idle = false;
    }

    for (const auto sys : tracks) {
      auto& pos = positions[sys.position_id];
      auto& anc = trackers[sys.tracker_id];
      auto& vel = motions[sys.motion_id];

      if (state.is_eyes_idle) {
        float x = -1.f, y = -1.f;
        if (state.frame_counter - state.idle_start < 30) {
          anc.target_x = 200;
          anc.target_y = 300;
        } else {
          anc.target_x = 600;
          anc.target_y = 300;
        }
      } else {
        if (SDL_GetMouseFocus() == nullptr) {
          anc.target_x = anc.anchor_x;
          anc.target_y = anc.anchor_y;
        } else {
          SDL_GetMouseState(&anc.target_x, &anc.target_y);
        }
      }

      const float x_anc = static_cast<float>(anc.anchor_x);
      const float y_anc = static_cast<float>(anc.anchor_y);

      float x_vec = anc.target_x - x_anc;
      float y_vec = anc.target_y - y_anc;
      const float z_vec = 300;

      float norm = std::sqrt(x_vec * x_vec + y_vec * y_vec + z_vec * z_vec);

      x_vec /= norm;
      y_vec /= norm;

      x_target = anc.max_radius * x_vec + x_anc;
      y_target = anc.max_radius * y_vec + y_anc;

      float target_dx = x_target - pos.x;
      float target_dy = y_target - pos.y;

      vel.ax = stiffness * target_dx - damping * vel.dx;
      vel.ay = stiffness * target_dy - damping * vel.dy;
    }
  }

  void loop_logic(game_state& state) noexcept {
    if (state.frame_counter >= state.next_blink_frame) {
      state.is_eyes_closed = true;
      state.eyes_closed_start = state.frame_counter;

      auto it = std::find_if(draws.begin(), draws.end(), [&](auto& dr) { return dr.position_id == state.head_id; });
      if (it != draws.end()) {
        std::swap(it->texture_id, state.head_texture_next);
      }

      uint32_t delay = 60 + lcg32(state.frame_counter) % 120;
      state.next_blink_frame = state.frame_counter + delay;
    }

    if (state.is_eyes_closed && (state.frame_counter - state.eyes_closed_start >= 10)) {
      state.is_eyes_closed = false;

      auto it = std::find_if(draws.begin(), draws.end(), [&](auto& dr) { return dr.position_id == state.head_id; });
      if (it != draws.end()) {
        std::swap(it->texture_id, state.head_texture_next);
      }
    }
  }

  void handle_event(SDL_Event& e, game_state& state) noexcept {
    float x = -1.f, y = -1.f;
    SDL_GetMouseState(&x, &y);

    if (e.type == SDL_EVENT_MOUSE_MOTION) {
      // mouse might leave button
      for (auto& click_sys : buttons) {
        if (click_sys.is_pressed) [[unlikely]] {
          const auto& dim = object_sizes[click_sys.obj_size_id];
          const auto& pos = positions[click_sys.position_id];

          if (pos.x - dim.width / 2 > x || x > pos.x + dim.width / 2 || pos.y - dim.height / 2 > y || y > pos.y + dim.height / 2) {
            SDL_Log("Mouse left button %d scope; won't trigger event\n", click_sys.position_id);
            click_sys.is_pressed = false;
            // no event trigger
          }
        }
      }

      // mouse might enter or leave zone
      for (auto& zone_sys : zones) {
        const auto& dim = object_sizes[zone_sys.obj_size_id];
        const auto& pos = positions[zone_sys.position_id];

        const bool is_now_in_zone = (pos.x - dim.width / 2 <= x && x <= pos.x + dim.width / 2 && pos.y - dim.height / 2 <= y && y <= pos.y + dim.height / 2);
        if (is_now_in_zone ^ zone_sys.is_in_zone) {
          if (zone_sys.is_in_zone) {
            SDL_Log("Zone %d is left; trigger leave event\n", zone_sys.position_id);
          } else {
            SDL_Log("Zone %d is entered; trigger enter event\n", zone_sys.position_id);
          }
          zone_sys.is_in_zone = !zone_sys.is_in_zone;
        }
      }
    }

    else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))) {
      state.score++;
      std::ostringstream oss;
      oss << std::setw(6) << std::setfill('0') << state.score;
      std::string new_str = oss.str();
      manager.update_texture_from_text_named(renderer, font, new_str, "score", 0x00, 0x00, 0x00, 0xFF);

      // someone can be dragged!
      for (auto& drag_sys : draggs) {
        auto& dr = drags[drag_sys.drag_id];
        const auto& dim = object_sizes[drag_sys.obj_size_id];
        const auto& pos = positions[drag_sys.position_id];

        if (pos.x - dim.width / 2 <= x && x <= pos.x + dim.width / 2 && pos.y - dim.height / 2 <= y && y <= pos.y + dim.height / 2) {
          drag_sys.is_dragged = true;

          dr.shift_x = pos.x - x;
          dr.shift_y = pos.y - y;
        }
      }

      // or clicked!
      for (auto& click_sys : buttons) {
        const auto& dim = object_sizes[click_sys.obj_size_id];
        const auto& pos = positions[click_sys.position_id];

        if (pos.x - dim.width / 2 <= x && x <= pos.x + dim.width / 2 && pos.y - dim.height / 2 <= y && y <= pos.y + dim.height / 2) {
          SDL_Log("button %d is pressed; trigger press event\n", click_sys.position_id);
          click_sys.is_pressed = true;
        }
      }
    }

    else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && !(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))) {
      // nobody is dragged!
      for (auto& drag_sys : draggs) {
        if (drag_sys.is_dragged == true) {
          SDL_Log("marking entt %d for delete\n", drag_sys.position_id);
          destroy_entity(drag_sys.position_id);
        }
        drag_sys.is_dragged = false;
      }

      // button may be released
      for (auto& click_sys : buttons) {
        const auto& dim = object_sizes[click_sys.obj_size_id];
        const auto& pos = positions[click_sys.position_id];

        if (pos.x - dim.width / 2 <= x && x <= pos.x + dim.width / 2 && pos.y - dim.height / 2 <= y && y <= pos.y + dim.height / 2) {
          if (click_sys.is_pressed == true) {
            click_sys.is_pressed = false;
            SDL_Log("button %d is released; trigger release event\n", click_sys.position_id);
            // trigger some event
            if (click_sys.release_event_id == 0 && !state.is_eyes_closed) {
              state.is_eyes_closed = true;
              state.eyes_closed_start = state.frame_counter;

              auto it = std::find_if(draws.begin(), draws.end(), [&](auto& dr) { return dr.position_id == state.head_id; });
              if (it != draws.end()) {
                std::swap(it->texture_id, state.head_texture_next);
              }

              uint32_t delay = 60 + lcg32(state.frame_counter) % 120;
              state.next_blink_frame = state.frame_counter + delay;
            }
          }
        }
      }
    }
  }

  void render() noexcept {
    for (auto dr : draws) {
      const auto& pos = positions[dr.position_id];
      const auto& dim = texture_sizes[dr.tex_size_id];

      manager.render(renderer, dr.texture_id, pos.x, pos.y, dim.width, dim.height);
    }
  }

  void move() noexcept {
    static constexpr float dt = 1.0 / kScreenFps;
    for (auto mv : movs) {
      auto& pos = positions[mv.position_id];
      auto& vel = motions[mv.motion_id];

      vel.dx += vel.ax * dt;
      vel.dy += vel.ay * dt;

      pos.x += vel.dx * dt;
      pos.y += vel.dy * dt;
    }
  }
};