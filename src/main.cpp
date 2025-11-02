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
#include <string>

/* Constants */
// Screen dimension constants
constexpr int kScreenWidth{640};
constexpr int kScreenHeight{480};
constexpr int kScreenFps{60};

/* Class Prototypes */

class Dot {
 public:
  // The dimensions of the dot
  static constexpr int kDotWidth = 20;
  static constexpr int kDotHeight = 20;

  // Maximum axis velocity of the dot
  static constexpr int kDotVel = 10;

  // Initializes the variables
  Dot(int, int);

  // Takes key presses and adjusts the dot's velocity
  void handleEvent(SDL_Event& e);

  // Moves the dot
  void move();

  // Shows the dot on the screen
  void render();

 private:
  // The X and Y offsets of the dot
  int mPosX, mPosY;

  // The velocity of the dot
  int mVelX, mVelY;

  // Dot anchor
  int mAncX, mAncY;
};

class Tomato {
 public:
  // The dimensions of the dot
  static constexpr int kTomatoWidth = 64;
  static constexpr int kTomatoHeight = 64;

  // Initializes the variables
  Tomato(int, int);

  // Takes key presses and adjusts the dot's velocity
  void handleEvent(SDL_Event& e);

  // Moves the dot
  void move();

  // Shows the dot on the screen
  void render();

 private:
  // The X and Y offsets of the dot
  int mPosX, mPosY;

  // The velocity of the dot
  int mVelX, mVelY;

  // Is tomato grabbed
  bool is_dragged = false;

  // relative vector to mouse
  float dx, dy;
};

/* Global Variables */
// The window we'll be rendering to
SDL_Window* gWindow{nullptr};

// The renderer used to draw to the window
SDL_Renderer* gRenderer{nullptr};

// Texture manager
texture_manager gManager;

// Ids
uint32_t gDotId;
uint32_t gTomatoId;
uint32_t gSpriteId;

/* Class Implementations */

// Tomato Implementation
Tomato::Tomato(int x, int y) : mPosX{x}, mPosY{y}, mVelX{0}, mVelY{0} {}

void Tomato::handleEvent(SDL_Event& e) {
  float x = -1.f, y = -1.f;
  SDL_GetMouseState(&x, &y);

  if (e.type == SDL_EVENT_MOUSE_MOTION && is_dragged) {
    mPosX = x + dx;
    mPosY = y + dy;
  } else if (e.type == SDL_EVENT_MOUSE_BUTTON_UP && !(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LEFT) && is_dragged) {
    is_dragged = false;
  } else if (mPosX - kTomatoWidth / 2 <= x && x <= mPosX + kTomatoWidth / 2 && mPosY - kTomatoHeight / 2 <= y && y <= mPosY + kTomatoHeight / 2) {
    if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN && (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LEFT) && !is_dragged) {
      is_dragged = true;

      dx = mPosX - x;
      dy = mPosY - y;
    }
  }
}

void Tomato::move() {
  // placeholder
}

void Tomato::render() {
  // Show the dot
  gManager.render(gRenderer, gTomatoId, static_cast<float>(mPosX - kTomatoWidth / 2), static_cast<float>(mPosY - kTomatoHeight / 2));
}

// Dot Implementation
Dot::Dot(int x, int y) : mPosX{x}, mPosY{y}, mVelX{0}, mVelY{0}, mAncX{mPosX}, mAncY{mPosY} {}

void Dot::handleEvent(SDL_Event& e) {
  if (e.type == SDL_EVENT_MOUSE_MOTION) {
    if (SDL_GetMouseFocus() == nullptr) {
      mPosX = mAncX;
      mPosY = mAncY;
    } else {
      // Get mouse position
      float x = -1.f, y = -1.f;
      SDL_GetMouseState(&x, &y);

      const float x_anc = static_cast<float>(mAncX);
      const float y_anc = static_cast<float>(mAncY);

      float x_vec = x - x_anc;
      float y_vec = y - y_anc;
      const float z_vec = 200;

      float norm = std::sqrt(x_vec * x_vec + y_vec * y_vec + z_vec * z_vec);

      x_vec /= norm;
      y_vec /= norm;

      mPosX = 20 * x_vec + x_anc;
      mPosY = 20 * y_vec + y_anc;
    }
  }
}

void Dot::move() {
  // Move the dot left or right
  mPosX += mVelX;

  // If the dot went too far to the left or right
  if ((mPosX < 0) || (mPosX + kDotWidth > kScreenWidth)) {
    // Move back
    mPosX -= mVelX;
  }

  // Move the dot up or down
  mPosY += mVelY;

  // If the dot went too far up or down
  if ((mPosY < 0) || (mPosY + kDotHeight > kScreenHeight)) {
    // Move back
    mPosY -= mVelY;
  }
}

void Dot::render() {
  // Show the dot
  gManager.render(gRenderer, gDotId, static_cast<float>(mPosX - kDotWidth / 2), static_cast<float>(mPosY - kDotHeight / 2));
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

bool loadMedia() {
  gDotId = gManager.load_texure_with_color_key(gRenderer, "assets/dot.png", 0xFF, 0xFF, 0xFF);

  gSpriteId = gManager.load_texure(gRenderer, "assets/jokr.png");
  gManager.resize(gSpriteId, kScreenWidth, kScreenHeight);

  gTomatoId = gManager.load_texure(gRenderer, "assets/tomato.png");

  return true;
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

  // Dot we will be moving around on the screen
  Dot dot1(kScreenWidth / 2 - 35, kScreenHeight / 2 - 115);
  Dot dot2(kScreenWidth / 2 + 128, kScreenHeight / 2 - 115);

  // Tomato
  Tomato tomato(kScreenWidth / 4, kScreenHeight / 4);

  // The main loop
  while (quit == false) {
    // Start frame time
    capTimer.start();

    // Get event data
    while (SDL_PollEvent(&e) == true) {
      // If event is quit type
      if (e.type == SDL_EVENT_QUIT) {
        // End the main loop
        quit = true;
      }

      // Process dot events
      dot1.handleEvent(e);
      dot2.handleEvent(e);
      tomato.handleEvent(e);
    }

    // Update dot
    dot1.move();
    dot2.move();
    tomato.move();

    // Fill the background
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(gRenderer);

    // Init sprite clip
    constexpr float kSpriteSize = 100.f;
    SDL_FRect spriteClip{0.f, 0.f, kSpriteSize, kSpriteSize};

    // Render dot
    dot1.render();
    dot2.render();

    // Draw original sized sprite
    // gSpriteSheetTexture.render(0.f, 0.f, nullptr, kScreenWidth, kScreenHeight);
    gManager.render(gRenderer, gSpriteId, 0, 0);

    tomato.render();

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
