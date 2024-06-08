#include "optimizer/optuna_search.h"

optuna_search::optuna_search(int max, int repeat, std::vector<std::vector<int>> core, std::string tune_mode) 
    : tune_engine(max, repeat, core), tune_mode(tune_mode) { 
}
tuner_data optuna_search::get_current()  {
    req_next = true;
    return next_data;
}

long long optuna_search::get_score() const { 
    return cur_score;
}
tuner_data optuna_search::get_max_score() const {
    return best_data;
}

long long optuna_search::get_best_score() const {
    return best_score;
}

void optuna_search::set_score(tuner_data data, long long score) { 
    cur_score = score;
    if (score < best_score) {
        best_score = score;
        best_data = data;
    }
}
bool optuna_search::is_next() { 
    return work_index != 0;
}
void optuna_search::reset() {
    search_space.clear();
}

void optuna_search::next(int elem) { 
    if (this->fix_mode) { 
        return;
    }
    // simple simple GET
    if (!req_next) { 
        return;
    }
   int ss = 0;
    for (int i = 0; i < search_space.size(); i++) { 
        if (elem < search_space[i].size()) { 
            ss = i;
            break;
        }
        elem -= search_space[i].size();
    }

    this->next_data = search_space[ss][elem];
}


void optuna_search::setup(void* data, int window_size) {
    reset();
    this->max_window_size = window_size;
    this->create_search_space();
    next_data = search_space[0][1];
}
