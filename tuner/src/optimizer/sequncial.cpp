#include "optimizer/all_search.h"

#include <iostream>
#include <array>

all_search::all_search(int max, int repeat, std::vector<std::vector<int>> core) : tune_engine(max, repeat, core) { 
}

long long all_search::get_score() const { 
    return -1;
}
tuner_data all_search::get_max_score() const {
    return best_data;
}

long long all_search::get_best_score() const {
    return best_score;
}

tuner_data all_search::get_current() {
    req_next = true;
    return next_data;
}
void all_search::next(int elem) { 
    if (this->fix_mode) { 
        return;
    }
    if (req_next) { 
        req_next = false;
        
        if (search_space.size() == 0) {
            std::cout << "손나 바카나!\n";
            throw "error!"; 
        }
        this->next_data = search_space[0][this->idx % search_space[0].size()];
        this->idx += 1;
    }
}


void all_search::set_score(tuner_data data, long long score) { 
    if (score < best_score) {
        best_score = score;
        best_data = data;
    }
    
    return;
}

bool all_search::is_next() { 
    return search_space.size() > 0;
}

void all_search::reset() {
    search_space.clear();
}

void all_search::setup(void* data, int window_size) {
    reset();
    this->max_window_size = window_size;
    this->create_search_space();
    next_data = search_space[0][0];
}