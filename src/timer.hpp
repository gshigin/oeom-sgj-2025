#pragma once

#include <SDL3/SDL.h>

class LTimer {
 public:
  // Initializes variables
  LTimer();

  // The various clock actions
  void start();
  void stop();
  void pause();
  void unpause();

  // Gets the timer's time
  uint64_t getTicksNS();

  // Checks the status of the timer
  bool isStarted();
  bool isPaused();

 private:
  // The clock time when the timer started
  uint64_t mStartTicks;

  // The ticks stored when the timer was paused
  uint64_t mPausedTicks;

  // The timer status
  bool mPaused;
  bool mStarted;
};

LTimer::LTimer()
    : mStartTicks{0},
      mPausedTicks{0},

      mPaused{false},
      mStarted{false} {}

void LTimer::start() {
  // Start the timer
  mStarted = true;

  // Unpause the timer
  mPaused = false;

  // Get the current clock time
  mStartTicks = SDL_GetTicksNS();
  mPausedTicks = 0;
}

void LTimer::stop() {
  // Stop the timer
  mStarted = false;

  // Unpause the timer
  mPaused = false;

  // Clear tick variables
  mStartTicks = 0;
  mPausedTicks = 0;
}

void LTimer::pause() {
  // If the timer is running and isn't already paused
  if (mStarted && !mPaused) {
    // Pause the timer
    mPaused = true;

    // Calculate the paused ticks
    mPausedTicks = SDL_GetTicksNS() - mStartTicks;
    mStartTicks = 0;
  }
}

void LTimer::unpause() {
  // If the timer is running and paused
  if (mStarted && mPaused) {
    // Unpause the timer
    mPaused = false;

    // Reset the starting ticks
    mStartTicks = SDL_GetTicksNS() - mPausedTicks;

    // Reset the paused ticks
    mPausedTicks = 0;
  }
}

uint64_t LTimer::getTicksNS() {
  // The actual timer time
  uint64_t time{0};

  // If the timer is running
  if (mStarted) {
    // If the timer is paused
    if (mPaused) {
      // Return the number of ticks when the timer was paused
      time = mPausedTicks;
    } else {
      // Return the current time minus the start time
      time = SDL_GetTicksNS() - mStartTicks;
    }
  }

  return time;
}

bool LTimer::isStarted() {
  // Timer is running and paused or unpaused
  return mStarted;
}

bool LTimer::isPaused() {
  // Timer is running and paused
  return mPaused && mStarted;
}