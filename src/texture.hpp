#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <string>
#include <utility>
#include <vector>

class texture {
 public:
  texture() = default;
  texture(SDL_Texture* texture_ptr) : texture_ptr_{texture_ptr} {};

  texture(texture&) = delete;
  texture& operator=(texture&) = delete;

  texture(texture&& other) noexcept : texture_ptr_{std::exchange(other.texture_ptr_, nullptr)} {}
  texture& operator=(texture&& other) noexcept {
    texture_ptr_ = std::exchange(other.texture_ptr_, nullptr);
    return *this;
  }

  ~texture() {
    if (texture_ptr_ != nullptr) {
      SDL_DestroyTexture(texture_ptr_);
    }
  }

  SDL_Texture* ptr() const { return texture_ptr_; }

 private:
  SDL_Texture* texture_ptr_ = nullptr;
};

struct dim {
  uint16_t width;
  uint16_t height;
};

// TODO: Fallback texture
class texture_manager {
 public:
  static constexpr uint32_t kNoImage = std::numeric_limits<uint32_t>::max();

  uint32_t load_texure(SDL_Renderer* renderer, std::string path) {
    if (SDL_Surface* loadedSurface = IMG_Load(path.c_str()); loadedSurface == nullptr) {
      SDL_Log("Unable to load image %s! SDL_image error: %s\n", path.c_str(), SDL_GetError());
      return std::numeric_limits<uint32_t>::max();
    } else {
      if (SDL_Texture* internal_texture = SDL_CreateTextureFromSurface(renderer, loadedSurface); internal_texture == nullptr) {
        SDL_Log("Unable to create texture from loaded pixels! SDL error: %s\n", SDL_GetError());
        return std::numeric_limits<uint32_t>::max();
      } else {
        textures_.emplace_back(internal_texture);
        dimentions_.emplace_back(loadedSurface->w, loadedSurface->h);
      }

      SDL_DestroySurface(loadedSurface);
    }

    return textures_.size() - 1;
  }

  uint32_t load_texure_with_color_key(SDL_Renderer* renderer, std::string path, uint8_t red, uint8_t green, uint8_t blue) {
    if (SDL_Surface* loadedSurface = IMG_Load(path.c_str()); loadedSurface == nullptr) {
      SDL_Log("Unable to load image %s! SDL_image error: %s\n", path.c_str(), SDL_GetError());
      return std::numeric_limits<uint32_t>::max();
    } else {
      if (SDL_SetSurfaceColorKey(loadedSurface, true, SDL_MapSurfaceRGB(loadedSurface, red, green, blue)) == false) {
        SDL_Log("Unable to load image %s! SDL_image error: %s\n", path.c_str(), SDL_GetError());
        return std::numeric_limits<uint32_t>::max();
      } else {
        if (SDL_Texture* internal_texture = SDL_CreateTextureFromSurface(renderer, loadedSurface); internal_texture == nullptr) {
          SDL_Log("Unable to create texture from loaded pixels! SDL error: %s\n", SDL_GetError());
          return std::numeric_limits<uint32_t>::max();
        } else {
          textures_.emplace_back(internal_texture);
          dimentions_.emplace_back(loadedSurface->w, loadedSurface->h);
        }
      }

      SDL_DestroySurface(loadedSurface);
    }

    return textures_.size() - 1;
  }

  uint32_t load_texture_from_text(SDL_Renderer* renderer, TTF_Font* font, std::string text, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {
    if (SDL_Surface* textSurface = TTF_RenderText_Blended(font, text.c_str(), 0, SDL_Color{red, green, blue, alpha}); textSurface == nullptr) {
      SDL_Log("Unable to render text surface! SDL_ttf Error: %s\n", SDL_GetError());
      return std::numeric_limits<uint32_t>::max();
    } else {
      if (SDL_Texture* internal_texture = SDL_CreateTextureFromSurface(renderer, textSurface); internal_texture == nullptr) {
        SDL_Log("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        return std::numeric_limits<uint32_t>::max();
      } else {
        textures_.emplace_back(internal_texture);
        dimentions_.emplace_back(textSurface->w, textSurface->h);
      }

      SDL_DestroySurface(textSurface);
    }

    return textures_.size() - 1;
  }

  void render(SDL_Renderer* renderer, uint32_t tex_id, float screen_x, float screen_y) {
    if (tex_id >= textures_.size()) {
      return;
    }

    SDL_FRect dst_rect{screen_x, screen_y, static_cast<float>(dimentions_[tex_id].width), static_cast<float>(dimentions_[tex_id].height)};

    SDL_RenderTexture(renderer, textures_[tex_id].ptr(), nullptr, &dst_rect);
  }

  void resize(uint32_t tex_id, uint16_t new_width, uint16_t new_height) {
    if (tex_id >= textures_.size()) {
      return;
    }

    dimentions_[tex_id] = {.width = new_width, .height = new_height};
  }

 private:
  std::vector<texture> textures_;
  std::vector<dim> dimentions_;
};