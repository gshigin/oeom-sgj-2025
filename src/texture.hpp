#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <cstdint>
#include <string>
#include <unordered_map>
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

  SDL_Texture* ptr() const noexcept { return texture_ptr_; }

 private:
  SDL_Texture* texture_ptr_ = nullptr;
};

// TODO: Fallback texture
// TODO: Tetutes get canvas size from caller!
class texture_manager {
  struct dim {
    uint16_t width;
    uint16_t height;
  };

 public:
  static constexpr uint32_t kNoImage = std::numeric_limits<uint32_t>::max();

  uint32_t load_texture(SDL_Renderer* renderer, const std::string& path) noexcept { return load_texture_named(renderer, path, ""); }

  uint32_t load_texture_with_color_key(SDL_Renderer* renderer, const std::string& path, uint8_t red, uint8_t green, uint8_t blue) noexcept {
    return load_texture_with_color_key_named(renderer, path, "", red, green, blue);
  }

  uint32_t
  load_texture_from_text(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) noexcept {
    return load_texture_from_text_named(renderer, font, "", text, red, green, blue, alpha);
  }

  // named
  uint32_t load_texture_named(SDL_Renderer* renderer, const std::string& path, const std::string& name) noexcept {
    if (SDL_Surface* loadedSurface = IMG_Load(path.c_str()); loadedSurface == nullptr) {
      SDL_Log("Unable to load image %s! SDL_image error: %s\n", path.c_str(), SDL_GetError());
      return kNoImage;
    } else {
      if (SDL_Texture* internal_texture = SDL_CreateTextureFromSurface(renderer, loadedSurface); internal_texture == nullptr) {
        SDL_Log("Unable to create texture from loaded pixels! SDL error: %s\n", SDL_GetError());
        return kNoImage;
      } else {
        textures_.emplace_back(internal_texture);
        dimentions_.emplace_back(loadedSurface->w, loadedSurface->h);

        if (!name.empty()) {
          name_to_id_[name] = textures_.size() - 1;
        }
      }

      SDL_DestroySurface(loadedSurface);
    }

    return textures_.size() - 1;
  }

  uint32_t load_texture_with_color_key_named(SDL_Renderer* renderer,
                                             const std::string& path,
                                             const std::string& name,
                                             uint8_t red,
                                             uint8_t green,
                                             uint8_t blue) noexcept {
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

          if (!name.empty()) {
            name_to_id_[name] = textures_.size() - 1;
          }
        }
      }

      SDL_DestroySurface(loadedSurface);
    }

    return textures_.size() - 1;
  }

  uint32_t load_texture_from_text_named(SDL_Renderer* renderer,
                                        TTF_Font* font,
                                        const std::string& text,
                                        const std::string& name,
                                        uint8_t red,
                                        uint8_t green,
                                        uint8_t blue,
                                        uint8_t alpha) noexcept {
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

        if (!name.empty()) {
          name_to_id_[name] = textures_.size() - 1;
        }
      }

      SDL_DestroySurface(textSurface);
    }

    return textures_.size() - 1;
  }

  void render(SDL_Renderer* renderer, uint32_t tex_id, float center_x, float center_y, float width, float height) noexcept {
    if (tex_id >= textures_.size()) {
      return;
    }

    SDL_FRect dst_rect{center_x - static_cast<float>(width) / 2, center_y - static_cast<float>(height) / 2, static_cast<float>(width),
                       static_cast<float>(height)};

    SDL_RenderTexture(renderer, textures_[tex_id].ptr(), nullptr, &dst_rect);
  }

  void set_name(uint32_t tex_id, std::string name) noexcept {
    if (tex_id >= textures_.size()) {
      return;
    }
    name_to_id_[name] = tex_id;
  }

  uint32_t get_texture_id(const std::string& name) const noexcept {
    if (auto it = name_to_id_.find(name); it != name_to_id_.end()) {
      return it->second;
    }
    return kNoImage;
  }

  std::pair<uint16_t, uint16_t> get_texture_sizes(const std::string& name) const noexcept { return get_texture_sizes(get_texture_id(name)); }
  std::pair<uint16_t, uint16_t> get_texture_sizes(uint32_t tex_id) const noexcept {
    if (tex_id == kNoImage) {
      return {0, 0};
    }
    return {dimentions_[tex_id].width, dimentions_[tex_id].height};
  }

 private:
  std::vector<texture> textures_;
  std::vector<dim> dimentions_;
  std::unordered_map<std::string, uint32_t> name_to_id_;
};