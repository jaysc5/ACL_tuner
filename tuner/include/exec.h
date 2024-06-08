#pragma once

#include <string>

namespace exec { 
    bool optimize(std::string model, std::string save_opt_file, int repeat, int evals, int conv_mode, std::string logfile, std::string ip, int port) ;
    void exec_log(std::string model, std::string opt_file, int repeat, int evals, std::string logfile, std::string ip, int port) ;
    void exec_default(std::string mode, int repeat, int evals, std::string logfile, std::string ip, int port);
}