#pragma once

#include "search/tune_engine.h"
#include <random>

class optuna_search : public tune_engine { 
public:
    optuna_search(int max, int repeat, std::vector<std::vector<int>> core, std::string tune_mode);
    virtual tuner_data get_current() override;
    virtual void next(int elem) override;
    virtual bool is_next() override;
    virtual void set_score(tuner_data data, long long score) override;
    virtual long long get_best_score() const override;
    virtual void reset() override;
    virtual void setup(void*, int window_size) override;

    virtual long long get_score() const override;
    virtual tuner_data get_max_score() const override;
    

protected:
    bool req_next = false;    
    long long cur_score = -1;
    std::string tune_mode;
    
    int work_index;
    std::string optimizer_ip;
    int port;
};
