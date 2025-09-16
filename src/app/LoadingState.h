#pragma once

#include <atomic>
#include <string>

struct LoadingState {
    std::atomic<float> progress = 0.0f;
    std::atomic<const char *> message = "Loading...";
};