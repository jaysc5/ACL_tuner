#pragma once

#include "tuner_data.h"

#include <vector>
#include <map>
#include <tuple>
#include <limits>

class tune_engine { 
public:
    tune_engine(int max, int repeat, std::vector<std::vector<int>> core)
        : max_search_size(max), repeat_count(repeat), core(core) {}

    virtual tuner_data get_current() = 0;  
    void set_current(tuner_data data);
    void best_current();

    virtual void next(int elem) = 0;
    virtual void set_score(tuner_data data, long long score) = 0;
    virtual long long get_best_score() const = 0;
    virtual long long get_score() const = 0;
    virtual tuner_data get_max_score() const = 0;
    
    virtual bool is_next() = 0;
    virtual void reset() = 0;
    virtual void setup(void*, int window_size) = 0;
    int get_search_space() const;
    
    std::vector<int> cluster_to_core(tuner_data t);
    std::tuple<int, int, int> index_to_work_size(tuner_data current, 
                                                    int idx, int max_idx, 
                                                    int max_window, 
                                                    int start, int end, int step);

    bool fix_mode = false;
protected:

    tuner_data best_data;
    long long best_score = INT64_MAX;
    tuner_data next_data;

    void create_search_space();
    std::vector<std::vector<tuner_data>> search_space;
    int max_window_size;
    std::vector<std::vector<int>> core;
    int max_search_size;
    int repeat_count;

private:
    std::vector<std::vector<int>> convert_core(tuner_data t);

};
