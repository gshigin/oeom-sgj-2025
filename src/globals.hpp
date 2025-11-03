#pragma once

#include <cstdint>

constexpr uint64_t kScreenWidth{800};
constexpr uint64_t kScreenHeight{600};

constexpr uint64_t kScreenFps{60};
constexpr uint64_t kNsPerFrame = 1'000'000'000 / kScreenFps;