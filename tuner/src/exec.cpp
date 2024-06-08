#include "include/tuner.h"
#include "include/helper.h"
#include <nlohmann/json.hpp>
#include <fstream>

#define IP "192.168.0.7"
#define PORT 9666

namespace exec { 

void log_extract_inference(const std::vector<extract_inference>& data, std::string logfile, int save_step = 1) { 
    nlohmann::json result = nlohmann::json::array();
    for (int i = 0; i < data.size(); i += save_step) { 
        // if (data.size() == i) { 
            // i = data.size() - 1;
        // }
        nlohmann::json infer;
        double ttime = 0;
        infer["kernels"] = nlohmann::json::array();

        for (const auto& k : data[i].kernels) {
            nlohmann::json kernel;
            kernel["kernel_name"] = k.kernel_name;
            kernel["conv_id"] = k.convolution_id;
            kernel["layer_name"] = k.layer_name;
            kernel["layer_id"] = k.guid;
            kernel["kernel_compute_time"] = k.kernel_time;
            kernel["core"] = nlohmann::json::array();
            for (const auto& c : k.core_time) { 
                nlohmann::json core;
                core["time"] = c.compute_time;
                core["id"] = c.core_id;
                kernel["core"].push_back(core);
            }
            kernel["work"] = nlohmann::json::array();
            for (const auto& t : k.tune_data) { 
                nlohmann::json core;
                core["cluster"] = t.core_cluster;
                core["size"] = t.window_size;
                kernel["work"].push_back(core);
            }

            // cout << "Layer Name : " << k.layer_name << ", Kernel Name : " << k.kernel_name << ", Is Conv : " << k.convolution_id << endl; 
            // cout << "ID : " << k.guid << ", Kernel Time : " << k.kernel_time / 1000.0 / 1000.0 << endl;
            ttime += k.kernel_time / 1000.0 / 1000.0;
            // for (const auto& core : k.core_time) { 
            //     cout << "\tCore Id : " << core.core_id << ", Core Time : " << core.compute_time / 1000.0 / 1000.0 << endl;
            // }
            infer["kernels"].push_back(kernel);
        }
        infer["step"] = i;
        infer["blackbox"] = data[i].black_box_time / 1000.0 / 1000.0;
        infer["total_inference"] = data[i].inference_time / 1000.0 / 1000.0;
        infer["search_inference"] = (data[i].black_box_time / 1000.0 / 1000.0) - ttime;
        infer["compute_inference"] = ttime;
        // cout << "inference time : " << item.inference_time / 1000.0 / 1000.0 << ", black time : " << item.black_box_time / 1000.0 / 1000.0 << ", profile : " << ttime << endl;
        result.push_back(infer);
    }

    
    std::ofstream output(logfile);
    output << result.dump();
    output.close();
}

void log_extract_inference_console(extract_inference data) { 
    // cout << endl<< endl;
    double ttime = 0;
    for (const auto& k : data.kernels) { 
        // cout << "Layer Name : " << k.layer_name << ", Kernel Name : " << k.kernel_name << ", Is Conv : " << k.convolution_id << endl; 
        // cout << "ID : " << k.guid << ", Kernel Time : " << k.kernel_time / 1000.0 / 1000.0 << endl;
        ttime += k.kernel_time / 1000.0 / 1000.0;
        // for (const auto& core : k.core_time) { 
        //     cout << "\tCore Id : " << core.core_id << ", Core Time : " << core.compute_time / 1000.0 / 1000.0 << endl;
        // }
    }
    
    cout << "inference time : " << data.inference_time / 1000.0 / 1000.0 << ", black time : " << data.black_box_time / 1000.0 / 1000.0 << ", profile : " << ttime << endl;
    // cout << ttime << endl;
}


bool optimize(std::string model, std::string save_opt_file, int repeat, int evals, int conv_mode, std::string logfile, std::string ip, int port) { 
    tuner_manager<unsigned char> tun;
    
    tun.set_tuner(tuner_option { 
        conv_mode,
        true,
        tuner_type::all,
    }, ip, port, evals, repeat);
    
    std::cout << "init start, real run" << std::endl;
    int max_conv_size, max_work_size;
    std::tie(max_conv_size, max_work_size) = tun.init(model, "", false);
    std::cout << max_conv_size << "conv_max_size" << std::endl;
    if (max_conv_size <= conv_mode) { 
        printf("%d <= %d, finish!!!\n", max_conv_size, conv_mode);
        printf("%d <= %d, finish!!!\n", max_conv_size, conv_mode);
        return false;
    }
    
    std::vector<extract_inference> inference;
    
    int save_id = 0;
    for (int i = 0; i < max_work_size + 1; i++) { 
        auto infer = tun.inference();
        if (logfile != "") { 
            inference.push_back(infer);
            if (i % 100 == 0) { 
                log_extract_inference(inference, logfile + "-" + std::to_string(save_id) + ".json");
                save_id += 1;
                inference.clear();
            }
        }
        log_extract_inference_console(infer);
        std::cout << "(" << i + 1 << "/" << max_work_size + 1 << ") 반복함 " << std::endl;
        // if (REMOVE_CACHE) {
        remove_cache();
        // }
    }

    std::cout << " 최종적인 결과를 반환함. " << std::endl;
    tun.export_best(save_opt_file);
    if (logfile != "") { 
        log_extract_inference(inference, logfile + std::to_string(save_id) + ".json");
    }
    return true;
}
void exec_log(std::string model, std::string opt_file, int repeat, int evals, std::string logfile, std::string ip, int port) { 
    tuner_manager<float> tun;
    
    tun.set_tuner(tuner_option { 
        0,
        true,
        tuner_type::all,
    }, ip, port, evals, repeat);
    
    std::cout << "init start, real run" << std::endl;
    int max_conv_size, max_work_size;
    std::tie(max_conv_size, max_work_size) = tun.init(model, opt_file, true);
    std::cout << max_conv_size << "conv_max_size" << std::endl;
    tun.execute_log(opt_file);

    std::vector<extract_inference> inference;
    for (int i = 0; i < evals; i++) { 
        auto infer = tun.inference();
        if (logfile != "") { 
             inference.push_back(infer);
        }   
        log_extract_inference_console(infer);
        std::cout << i + 1 << " 최적화한 성능 벤치마킹 반복함 " << std::endl;
        remove_cache();
    }
    if (logfile != "") { 
        log_extract_inference(inference, logfile);
    }
}


void exec_default(std::string mode, int repeat, int evals, std::string logfile, std::string ip, int port) { 
    tuner_manager<float> tun;
    std::cout << "test" << std::endl;
    
    tun.set_tuner(tuner_option { 
        -1,
        true,
        tuner_type::all,
    }, ip, port, 1000, repeat);
    std::cout << "test5" << std::endl;

    std::cout << "init start, real run" << std::endl;
    int max_conv_size, max_work_size;
    std::tie(max_conv_size, max_work_size) = tun.init(mode, "", true);
    std::cout << max_conv_size << "conv_max_size" << std::endl;

    tun.set_default(true);
    std::vector<extract_inference> inference;
    for (int i = 0; i < evals; i++) { 
        auto infer = tun.inference();
        if (logfile != "") { 
             inference.push_back(infer);
        }   
        log_extract_inference_console(infer);
        std::cout << i + 1 << " 기본 성능 벤치마킹 반복함 " << std::endl;
        remove_cache();
    }
    if (logfile != "") { 
        log_extract_inference(inference, logfile);
    }
}
}
