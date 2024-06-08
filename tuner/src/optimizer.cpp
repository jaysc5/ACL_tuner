#pragma once

#include "search/optimizer.h"
#include <sstream>
#include <iostream>
#include "nlohmann/json.hpp"
#include "network/network.hpp"
#include <unistd.h>

using namespace std;

vector<string> split(string input, char delimiter) {
    vector<string> answer;
    stringstream ss(input);
    string temp;
 
    while (getline(ss, temp, delimiter)) {
        answer.push_back(temp);
    }
 
    return answer;
}

std::string tuner_type_to_string(tuner_type type) { 
    switch (type) { 
    case tuner_type::opt_grid:
        return "grid";
    case tuner_type::opt_random:
        return "random";
    case tuner_type::opt_tpe:
        return "tpe";
    case tuner_type::opt_cma:
        return "cma";
    case tuner_type::opt_partial_fixed:
        return "pf";
    case tuner_type::opt_nsga2:
        return "nsga2";
    case tuner_type::opt_qmc:
        return "qmc";
    case tuner_type::random:
    default:
        throw std::exception();
    }
}


std::string optimizer::export_best() const{
    std::vector<std::pair<std::string, std::shared_ptr<tune_engine>>> conv;
    for (auto& item : tune) { 
        conv.push_back(std::make_pair(item.first, item.second));
    }

    nlohmann::json result;
    for (const auto& item : conv) { 
        result[item.first]["score"] = item.second->get_best_score();
        auto max = item.second->get_max_score();
        // std::cout << "힝힝힝이 : " << max.size() << std::endl; 
        
        result[item.first]["data"] = nlohmann::json::array();
        for (const auto& t : max) {
            nlohmann::json core;
            core["core_cluster"] = t.core_cluster;
            core["window_size"] = t.window_size;
            result[item.first]["data"].push_back(core);
        }
    }
    return result.dump(4);
}
void optimizer::import_best(std::ifstream& data){ 
    nlohmann::json result = nlohmann::json::parse(data);
    for (const auto& t : tune) {
        std::cout << t.first << std::endl;

        auto idx = split(t.first, '_')[0];
        tuner_data tun_data;
        for (const auto& idata : result[idx][t.first]["data"]) {
            int cluster = idata["core_cluster"].get<int>();
            int window = idata["window_size"].get<int>();
            tun_data.push_back({ 
                0, cluster, window
            });
        }
        tune[t.first]->set_current(tun_data);
        this->tune[t.first]->fix_mode = true;
    }
    fix_mode = true;
}

void optimizer::import_best(std::string data){ 
    nlohmann::json result = nlohmann::json::parse(data);

    for (const auto& t : tune) {
        std::cout << t.first << std::endl;

        auto idx = split(t.first, '_')[0];
        tuner_data tun_data;
        for (const auto& idata : result[idx][t.first]["data"]) {
            int cluster = idata["core_cluster"].get<int>();
            int window = idata["window_size"].get<int>();
            tun_data.push_back({ 
                0, cluster, window
            });
        }
        tune[t.first]->set_current(tun_data);
        this->tune[t.first]->fix_mode = true;
    }

    fix_mode = true;
}

void optimizer::run_best() { 
     for (auto& item : tune) { 
        item.second->best_current();
        item.second->fix_mode = true;
    }
    fix_mode = true;
}

void optimizer::set_type(tuner_type type, std::string ip, int port, int repeat, int evals) { 
    this->type = type;
    this->ip = ip;
    this->port = port;
    this->repeat = repeat;
    this->evals = evals;
}

string optimizer::get_hash_name(int guid, 
                         int layer_guid, 
                         int conv_id, 
                         string kernel_name, 
                         string layer_name) const { 

    stringstream ss;
    if (guid == layer_guid) { 
        ss << guid << "_" << conv_id << "_" << kernel_name << "_" << layer_name; 
    }else { 
        ss << guid << "_" << kernel_name << "_" << layer_name; 
    }
    return ss.str();

}

void optimizer::tuner_reset(bool tuner_sleep_delete) { 
    if (tuner_sleep_delete == true) { 
        return;
    }
    
    if (this->type == tuner_type::random || this->type == tuner_type::all) { 
        return;
    }
    // {
    //     netowrk nwtp(this->ip, this->port);
    //     nwtp.request("/refresh", "");
    //     nwtp.wait();
    // }


    nlohmann::json req_next;
    req_next["ports"] = 9001;
    req_next["layers"] = tune.size();
    req_next["opt"] = tuner_type_to_string(this->type);

    std::cout << req_next.dump() << std::endl;

    netowrk nwtp(this->ip, this->port);
    nwtp.request("/optimizer_setup", req_next.dump());
    nwtp.wait();

    std::cout << "sleep start" << std::endl;
    sleep(30);
    std::cout << "sleep finish" << std::endl;

    req_next = nlohmann::json();
    netowrk nwtp2(this->ip, this->port);
    req_next["ports"] = 9001;
    req_next["evals"] = this->evals;
    std::vector<std::pair<std::string, std::shared_ptr<tune_engine>>> conv;
    for (auto& item : tune) { 
        conv.push_back(std::make_pair(item.first, item.second));
    }
    std::sort(conv.begin(), conv.end());
    std::vector<long long> scores;
    for (auto& item : tune) { 
        scores.push_back(item.second->get_search_space());
    }
    req_next["search"] = scores;
    std::cout << req_next.dump() << std::endl;
    nwtp2.request("/setup", req_next.dump());
    nwtp2.wait();
}

int one_cluster_mode_priority = 0;

void optimizer::next_param() { 
    if (one_cluster_mode_priority < 3) { 
        for (auto& item : tune) { 
// #if DEBUG_LOG == 1
//                 printf("------------------------------------------\n");
//                 printf("[    %d    ]\n", one_cluster_mode_priority);
//                 printf("------------------------------------------\n");
// #endif
            item.second->next(one_cluster_mode_priority);
        }
        one_cluster_mode_priority += 1;
        return;
    }
    if (this->fix_mode) { 
        return;
    }

    if (type == tuner_type::random || type == tuner_type::all) { 
        for (auto& item : tune) { 
            item.second->next(0);
        }
    } else { 
        netowrk nwtp(this->ip, this->port);
        nlohmann::json req_next;
        std::vector<std::pair<std::string, std::shared_ptr<tune_engine>>> conv;
        for (auto& item : tune) { 
            conv.push_back(std::make_pair(item.first, item.second));
        }
        std::sort(conv.begin(), conv.end());

        if (!is_first) { 
            std::vector<long long> scores;
            for (auto& item : tune) { 
                scores.push_back(item.second->get_score());
            }
            req_next["ports"] = 9001;
            req_next["scores"] = scores;
            nwtp.request("/next", req_next.dump());
            nwtp.wait();
        }
        
        // sleep(1);

        is_first = false;
        netowrk nwtp2(this->ip, this->port);
        req_next = nlohmann::json{};
        req_next["ports"] = 9001;
        req_next["layers"] = tune.size();
        nwtp2.request("/get", req_next.dump());
        nwtp2.wait();
        req_next = nlohmann::json::parse(nwtp2.recv_body());
        // req_next.operator[]
        for (int i = 0; i < conv.size(); i++) { 
            int next = std::stoi(req_next[i].get<std::string>());
            conv[i].second->next(next);
        }
    }
}

bool optimizer::create_tuner(string name, int max_window_size) { 
    // 만약 없다면! 생성.
    if (tune.find(name) == tune.end()) { 
        switch (this->type) { 
        case tuner_type::random:
            tune[name] = strategy_factory::instance()->random(max_window_size);
            break;
        case tuner_type::all:
            tune[name] = strategy_factory::instance()->all(max_window_size);
            break;
        case tuner_type::opt_grid:
            tune[name] = strategy_factory::instance()->optuna_grid(max_window_size);
            break;
        case tuner_type::opt_random:
            tune[name] = strategy_factory::instance()->optuna_random(max_window_size);
            break;
        case tuner_type::opt_tpe:
            tune[name] = strategy_factory::instance()->optuna_tpe(max_window_size);
            break;
        case tuner_type::opt_cma:
            tune[name] = strategy_factory::instance()->optuna_cma(max_window_size);
            break;
        case tuner_type::opt_partial_fixed:
            tune[name] = strategy_factory::instance()->optuna_partial_fixed(max_window_size);
            break;
        case tuner_type::opt_nsga2:
            tune[name] = strategy_factory::instance()->optuna_nsga2(max_window_size);
            break;
        case tuner_type::opt_qmc:
            tune[name] = strategy_factory::instance()->optuna_qmc(max_window_size);
            break;
        default:
            return false;
        }
        tune[name]->setup(nullptr, max_window_size);
        return true;
    }
    return false;
}



shared_ptr<tune_engine> optimizer::get_tuner(string name) { 
    return tune[name];
}
