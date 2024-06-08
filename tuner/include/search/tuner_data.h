#pragma once

#include <vector>
#include <chrono>
#include <tuple>

struct tune { 
    int frequency;
    int core_cluster;
    int window_size;
};

typedef std::vector<tune> tuner_data;

