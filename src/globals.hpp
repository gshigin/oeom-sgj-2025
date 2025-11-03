#pragma once

#include <cstdint>

constexpr uint64_t kScreenWidth{640};
constexpr uint64_t kScreenHeight{480};

constexpr uint64_t kScreenFps{60};
constexpr uint64_t kNsPerFrame = 1'000'000'000 / kScreenFps;