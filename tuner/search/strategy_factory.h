#pragma once

#include <memory>
#include <string>
#include "../helper.h"
#include <iostream>

#include "../optimizer/random_search.h"
#include "../optimizer/optuna_search.h"

class strategy_factory { 
public:
    static std::shared_ptr<strategy_factory> instance() { 
        static std::shared_ptr<strategy_factory> self = std::make_shared<strategy_factory>(6);
        // std::cout << "CPU Core SIZE : " << 6 << "\n";
        return self;
    }

    // 1개의 레이어별 최적화
    std::shared_ptr<tune_engine> random(int repeat);
    std::shared_ptr<tune_engine> optuna_grid(int repeat);
    std::shared_ptr<tune_engine> optuna_random(int repeat);
    std::shared_ptr<tune_engine> optuna_tpe(int repeat);
    std::shared_ptr<tune_engine> optuna_cma(int repeat);
    std::shared_ptr<tune_engine> optuna_partial_fixed(int repeat);
    std::shared_ptr<tune_engine> optuna_nsga2(int repeat);
    std::shared_ptr<tune_engine> optuna_qmc(int repeat);

    // core, frequency
    strategy_factory(int core_count) { 
        partition = partition_core_freq(core_count);
    }
protected: 
    std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> partition;

private:

};
