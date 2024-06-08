#include "optimizer/random_search.h"

#include <iostream>
#include <array>

random_search::random_search(int max, int repeat, std::vector<std::vector<int>> core) : tune_engine(max, repeat, core) { 
}

long long random_search::get_score() const { 
    return -1;
}
tuner_data random_search::get_max_score() const {
    return best_data;
}

long long random_search::get_best_score() const {
    return best_score;
}

tuner_data random_search::get_current() {
    req_next = true;
    return next_data;
}
void random_search::next(int elem) { 
    if (this->fix_mode) { 
        return;
    }
    if (req_next) { 
        req_next = false;
        if (search_space.size() == 0) {
            std::cout << "손나 바카나!\n";
            throw "error!"; 
        }
        int core_selection = eng() % search_space.size();
        int idx = eng() % search_space[core_selection].size();

        this->next_data = search_space[core_selection][idx];
    }
}


void random_search::set_score(tuner_data data, long long score) { 
    if (score < best_score) {
        best_score = score;
        best_data = data;
    }
    
    return;
}

bool random_search::is_next() { 
    return search_space.size() > 0;
}

void random_search::reset() {
    search_space.clear();
}

void random_search::setup(void* data, int window_size) {
    reset();
    this->max_window_size = window_size;
    this->create_search_space();
    eng = std::mt19937_64(rd());
    next_data = search_space[0][1];
}
