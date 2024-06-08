#include "search/strategy_factory.h"

std::shared_ptr<tune_engine> strategy_factory::random(int repeat) { 
    return std::static_pointer_cast<tune_engine>(std::make_shared<random_search>(0, repeat, partition.first));
}

std::shared_ptr<tune_engine> strategy_factory::optuna_random(int repeat) { 
    return std::static_pointer_cast<tune_engine>(std::make_shared<optuna_search>(0, repeat, partition.first, "random"));
}
std::shared_ptr<tune_engine> strategy_factory::optuna_grid(int repeat) { 
    return std::static_pointer_cast<tune_engine>(std::make_shared<optuna_search>(0, repeat, partition.first, "grid"));
}
std::shared_ptr<tune_engine> strategy_factory::optuna_tpe(int repeat) { 
    return std::static_pointer_cast<tune_engine>(std::make_shared<optuna_search>(0, repeat, partition.first, "tpe"));
}
std::shared_ptr<tune_engine> strategy_factory::optuna_cma(int repeat) { 
    return std::static_pointer_cast<tune_engine>(std::make_shared<optuna_search>(0, repeat, partition.first, "cma"));
}
std::shared_ptr<tune_engine> strategy_factory::optuna_partial_fixed(int repeat) { 
    return std::static_pointer_cast<tune_engine>(std::make_shared<optuna_search>(0, repeat, partition.first, "pf"));
}
std::shared_ptr<tune_engine> strategy_factory::optuna_nsga2(int repeat) { 
    return std::static_pointer_cast<tune_engine>(std::make_shared<optuna_search>(0, repeat, partition.first, "nsga2"));
}
std::shared_ptr<tune_engine> strategy_factory::optuna_qmc(int repeat) { 
    return std::static_pointer_cast<tune_engine>(std::make_shared<optuna_search>(0, repeat, partition.first, "qmc"));
}
