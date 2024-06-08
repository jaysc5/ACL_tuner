#pragma once

#include "../search/tune_engine.h"
#include <random>

class random_search : public tune_engine { 
public:
    random_search(int max, int repeat, std::vector<std::vector<int>> core);
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
    std::random_device rd;     //Get a random seed from the OS entropy device, or whatever
    std::mt19937_64 eng; //Use the 64-bit Mersenne Twister 19937 generator
};
