#pragma once

#include <map>
#include <vector>
#include "strategy_factory.h"
#include <string>
#include <memory>
#include <exception>
#include <fstream>

using namespace std;

enum class tuner_type { 
    none,
    random,
    opt_grid,
    opt_random,
    opt_tpe,
    opt_cma,
    opt_partial_fixed,
    opt_nsga2,
    opt_qmc,
    all
};

std::string tuner_type_to_string(tuner_type type);

class optimizer { 
public:
    void set_type(tuner_type type, std::string ip, int port, int repeat, int evals);

    string get_hash_name(int guid, 
                         int layer_guid, 
                         int conv_id, 
                         string kernel_name, 
                         string layer_name) const;

    void tuner_reset(bool tuner_sleep_delete);
    bool create_tuner(string name, int max_window_size);

    void next_param();
    shared_ptr<tune_engine> get_tuner(string name);


public:
    std::string export_best() const;
    void import_best(std::string data);
    void import_best(std::ifstream& data);

    void run_best();

private:
    bool is_first = true;
    map<string, shared_ptr<tune_engine>> tune;
    tuner_type type;
    std::string ip;
    int port;
    int repeat;
    int evals;
    bool fix_mode = false;
};
