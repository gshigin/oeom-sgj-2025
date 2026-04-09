#pragma once

#include <SDL3/SDL.h>
#include <cstdint>

class timer {
 public:
  timer() = default;

  void start() noexcept {
    started_ = true;
    paused_ = false;
    start_ticks_ = SDL_GetTicksNS();
    paused_ticks_ = 0;
  }

  void stop() noexcept {
    started_ = false;
    paused_ = false;
    start_ticks_ = 0;
    paused_ticks_ = 0;
  }

  void pause() noexcept {
    if (started_ && !paused_) {
      paused_ = true;
      paused_ticks_ = SDL_GetTicksNS() - start_ticks_;
      start_ticks_ = 0;
    }
  }

  void unpause() noexcept {
    if (started_ && paused_) {
      paused_ = false;
      start_ticks_ = SDL_GetTicksNS() - paused_ticks_;
      paused_ticks_ = 0;
    }
  }

  [[nodiscard]] uint64_t get_ticks_ns() const noexcept {
    if (!started_)
      return 0;
    return paused_ ? paused_ticks_ : (SDL_GetTicksNS() - start_ticks_);
  }

  [[nodiscard]] bool is_started() const noexcept { return started_; }
  [[nodiscard]] bool is_paused() const noexcept { return started_ && paused_; }

 private:
  uint64_t start_ticks_{0};
  uint64_t paused_ticks_{0};

  bool paused_{false};
  bool started_{false};
};