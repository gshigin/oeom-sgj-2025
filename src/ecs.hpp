#pragma once

#include "globals.hpp"
#include "texture.hpp"

// entity
struct handler_id {
  uint16_t position_id;  // position id is unique key for handler
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
};

struct drag {
  float shift_x;
  float shift_y;
};

struct mouse_tracker {
  float anchor_x;
  float anchor_y;
  float max_radius;
};

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

// TODO: Try OOD table method to arrange data
struct ECS {
  static constexpr uint16_t kNoId = std::numeric_limits<uint16_t>::max();

  SDL_Renderer* renderer;
  texture_manager& manager;

  ECS(SDL_Renderer* r, texture_manager& m) : renderer(r), manager(m) {}

  // components
  std::vector<position> positions;
  std::vector<dimention> dimentions;
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

  // to delete
  std::vector<uint16_t> to_delete;

  // register entity
  handler_id register_object(float x, float y) noexcept {
    handler_id h{.position_id = static_cast<uint16_t>(positions.size()),
                 .dimention_id = kNoId,
                 .texture_id = kNoId,
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
    trackers.emplace_back(x, y, r);

    tracks.emplace_back(handler.position_id, handler.tracker_id);
  }

  void add_dimetions(handler_id& handler, float w, float h) noexcept {
    handler.dimention_id = dimentions.size();
    dimentions.emplace_back(w, h);
  }

  void add_drag(handler_id& h) noexcept {
    h.drag_id = drags.size();
    drags.emplace_back(0.f, 0.f);
  }

  void add_texture(handler_id& h, uint16_t texture_id) noexcept {
    h.texture_id = texture_id;

    draws.emplace_back(h.position_id, h.texture_id);
  }

  void make_draggable(handler_id& h) noexcept { draggs.emplace_back(h.position_id, h.dimention_id, h.drag_id, false); }

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

  void move_tracked() noexcept {
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

  void handle_event(SDL_Event& e) noexcept {
    if (e.type == SDL_EVENT_MOUSE_MOTION) {
      move_tracked();
      move_dragged();
    }

    else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && !(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LEFT)) {
      // nobody is dragged!
      for (auto& drag_sys : draggs) {
        if (drag_sys.is_dragged == true) {
          SDL_Log("marking entt %d for delete\n", drag_sys.position_id);
          destroy_entity(drag_sys.position_id);
        }
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

  // factories
  handler_id create_dot(float x, float y, float dot_r, float anchor_r) noexcept {
    auto h = register_object(x, y);

    add_texture(h, manager.get_id("dot"));

    add_tracker(h, x, y, anchor_r);

    return h;
  }

  handler_id create_tomato(float x, float y) noexcept {
    auto h = register_object(x, y);

    add_texture(h, manager.get_id("tomato"));

    add_dimetions(h, 30, 30);
    add_drag(h);

    make_draggable(h);

    return h;
  }

  handler_id create_bg() noexcept {
    auto h = register_object(kScreenWidth / 2, kScreenHeight / 2);
    add_texture(h, manager.get_id("jokr"));
    manager.resize(h.texture_id, kScreenWidth, kScreenHeight);

    return h;
  }

  void render() noexcept {
    for (auto dr : draws) {
      const auto& pos = positions[dr.position_id];

      manager.render(renderer, dr.texture_id, pos.x, pos.y);
    }
  }
};