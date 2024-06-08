#pragma once

#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <tuple>
#include <stdio.h>


// #include <luna/neon.h>
#include "search/optimizer.h"
#include "arm_compute/runtime/Scheduler.h"
#include <fstream>
// #include <nlohmann/json.hpp>

using namespace std;

struct tuner_option { 
    int convolution_optimizer = 0;
    bool workload_optimizer = false;
    // bool print_core_time = false;
    // bool print_workload_time = false;
    // bool print_layer_time = false;
    // bool print_inference_time = false;
    tuner_type type = tuner_type::none;
};

struct extract_kernel_work
{
    int core_id;
    long long compute_time;
};

struct extract_kernel { 
    int guid;
    long long kernel_time;
    string kernel_name;
    string layer_name;
    int convolution_id = -1;
    tuner_data tune_data;
    vector<extract_kernel_work> core_time;
};

struct extract_inference { 
    long long inference_time;
    long long black_box_time;
    vector<extract_kernel> kernels;
};

template<typename T>
class tuner_manager { 
private:
    tuner_option option;
    // neon_engine<T> engine;
    int once_tuner_info = 0;
    extract_inference result_tmp;
    extract_kernel kernel_tmp;
    int layer_guid = -1;
    int conv_id = -1;
    optimizer opt;
    int repeat = 1;
    std::vector<long long> measure_time;

    bool hing = false;
    int max_work_size = 0;


private:
    bool default_runner = false;
    bool fix_mode = false;
    bool is_first_exec = true;
    string cur_hash_name;
    tuner_data cur_tuner_data;

    int tune_id = 0;


public:
    void set_default(bool run) { 
        fix_mode = true;
        this->default_runner = run;
    }

    bool set_tuner(tuner_option option, std::string ip, int port, int evals, int repeat) { 
        this->option = option;
        this->repeat = repeat; 
#if DEBUG_LOG == 1
        std::cout << "test3" << std::endl;
#endif
        opt.set_type(option.type, ip, port, repeat, evals);
#if DEBUG_LOG == 1
        std::cout << "test4" << std::endl;
#endif
        return true;
    }

    std::tuple<int, int> init(std::string model, std::string arch = "", bool tuner_sleep_delete = false) { 
        auto gen = arm_compute::Scheduler::get().generate_core_thread();
#if DEBUG_LOG == 1
        for (const auto model : gen) { 
            cout << "model : " << (int)model << endl; 
        }
#endif
        arm_compute::Scheduler::get().reset_window_result();
        arm_compute::Scheduler::get().reset_extract_feature();
        int max_conv_size = 0;

        // nlohmann::json parsed;
        // if (arch != "") { 
        //     std::ifstream ifs(arch);
        //     parsed = nlohmann::json::parse(ifs);
        // }
        
        int capture_id = 0;
        int fully_capture_id = 0;
        // if (arch != "") { 
        //     arm_compute::Scheduler::get().dense_callback = [parsed, &fully_capture_id]() -> std::string { 
        //         printf("fully capture : %d, %d\n", fully_capture_id, parsed["fully"].size());
        //         // std::cout << "12312312! " << std::endl;
        //         // std::string result = parsed["fully"][0]["kernel"].get<std::string>();
        //         // std::cout << result << "helloworld! " << std::endl;
        //         fully_capture_id += 1;
        //         return "a64_hybrid_fp32_mla_6x16";
        //     };
        // }

        if (option.convolution_optimizer != -1) { 
            arm_compute::Scheduler::get().conv_method_callback = [&, &max_conv_size, &arch, parsed, &capture_id](const auto& direct, const auto& general, const auto& wino) { 
                max_conv_size = std::max(max_conv_size, (int)(direct.size() + general.size() + wino.size()));
                int conv = option.convolution_optimizer;
#if DEBUG_LOG == 1
                std::cout << "asdfasdfasdf" << direct.size() << " " << general.size() << " " << wino.size() << std::endl;
#endif
                
                auto cur_layer = arm_compute::Scheduler::get().get_current_kernel();
                auto name = get<0>(cur_layer);
                auto guid = get<1>(cur_layer);
                int method = 0;
                this->layer_guid = guid;
#if DEBUG_LOG == 1
                printf("~~~~~~~~~~~~~~");
                printf("%d\n", guid);
                printf("%s\n", name.c_str());
                printf("%d\n", guid);
                
                fflush(stdout);
#endif
                if (arch == "") { 
                    int kernel = 0;
                    if (conv < direct.size()) {
                        method = 1;
                        kernel = conv;
                    }else { 
                        conv -= direct.size();
                        if (conv < general.size()) {
                            method = 2;
                            kernel = conv;
                        }else { 
                            conv -= general.size();
                            if (conv < wino.size()) {
                                method = 3;
                                kernel = conv;
                            }
                        }
                    }
                    conv_id = method * 100 + kernel;
                    printf("%d, %d\n", method, kernel);
                    
                    return std::make_pair(method, kernel);
                }else { 
#if DEBUG_LOG == 1
                    printf("capture : %d, %d\n", capture_id, parsed["convolution"].size());
#endif
                    
                    method = parsed["convolution"][capture_id]["method"].get<int>();
                    std::string kernel = parsed["convolution"][capture_id]["kernel"].get<std::string>();
                    int kernel_idx = -1;
#if DEBUG_LOG == 1
                    printf("%d %d %s\n", method, kernel_idx, kernel.c_str());
#endif
                    switch (method)
                    {
                    case 1:
                        kernel_idx = std::distance(direct.begin(), std::find(direct.begin(), direct.end(), kernel));
                        if (kernel_idx == direct.size()) kernel_idx = -1;
                        break;
                    case 2:
                        kernel_idx = std::distance(general.begin(), std::find(general.begin(), general.end(), kernel));
                        if (kernel_idx == general.size()) kernel_idx = -1;
                        break;
                    case 3:
                        kernel_idx = std::distance(wino.begin(),std::find(wino.begin(), wino.end(), kernel));
                        if (kernel_idx == wino.size()) kernel_idx = -1;
                        break;
                    default:
                        kernel_idx = -1;
                        break;
                    }
                    conv_id = method * 100 + kernel_idx;
                    capture_id += 1;
#if DEBUG_LOG == 1
                    printf("conv id : %d\n", conv_id);
                    printf("conv id : %d\n", conv_id);
#endif
                    if (kernel_idx == -1) { 
#if DEBUG_LOG == 1
                        printf("error! not found : %d\n", conv_id);
#endif
                        kernel_idx = 0;
                    }
#if DEBUG_LOG == 1
                    fflush(stdout);
#endif                    
                    return std::make_pair(method, kernel_idx);
                }
            };
        }

        // engine = neon_engine<T>();
        // engine.init(model);

        // guid, layer_guid 비교, -> conv_id
        // kernel_name과, layer_name 전달
        // -> tuner 생성
        if (option.workload_optimizer) { 
            arm_compute::Scheduler::get().set_tuner_info(
                [&](const char *kernel_name, int max_window) {
                    // printf("\t[%d]\n", once_tuner_info);
                    this->tune_id += 1;
                    once_tuner_info += 1; 
                    if (once_tuner_info <= this->repeat) {
                        if (default_runner) { 
                            return true;
                        }
                        auto cur_layer = arm_compute::Scheduler::get().get_current_kernel();
                        auto name = get<0>(cur_layer);
                        auto guid = get<1>(cur_layer);
                        cur_hash_name = this->opt.get_hash_name(guid, this->layer_guid, conv_id, kernel_name, name);
                        // std::cout << cur_hash_name << "\n";
                        // printf(("\t\t" + cur_hash_name + "\n").c_str());
                        this->opt.create_tuner(cur_hash_name, max_window);
                        if (is_first_exec) { 
                            this->max_work_size = std::max(max_window, max_work_size);
                        }
                        // 처음이 아니라면, 데이터 생성해야지!
                        if (!is_first_exec) { 
                            cur_tuner_data = this->opt.get_tuner(cur_hash_name)->get_current();
                            // int test_num = 0;
                            // for(const auto& value : cur_tuner_data) { 
                            //     test_num += value.window_size;
                            // }
                            // if (max_window != test_num) { 
                            //     printf("ERRORRRRRRRRRRRRRRRRRR :: [ %d : %d, %s ]\n", max_window, test_num, cur_hash_name.c_str());
                            //     // fflush(stdout); 
                            //     // cur_tuner_data[cur_tuner_data.size() - 1].window_size = max_window;
                            // }
                        }

                        return true;
                    }else { 
                        once_tuner_info = 0;
                        measure_time.clear();
                        return false;
                    }
                },
                [&]() {
                    if (default_runner) { 
                        return std::vector<int> {DEFAULT_CORE};
                    }

                    if (this->is_first_exec) { 
                        return std::vector<int> {WARM_UP_CORE}; 
                    }else { 
                        return this->opt.get_tuner(this->cur_hash_name)->cluster_to_core(cur_tuner_data);
                    }
                },            
                [&](int idx, int max_idx, int max_window, int start, int end, int step) { 
                    if (default_runner) { 
                        return std::make_tuple(-1,-1,-1);
                    }

                    if (this->is_first_exec) { 
                        return std::make_tuple(-1,-1,-1);
                    }else { 
                        return this->opt.get_tuner(this->cur_hash_name)->index_to_work_size(cur_tuner_data, idx, max_idx, max_window, start, end, step);
                    }
                },
                [&](std::string kernelName, unsigned int measure_speed) { 
                    auto cur_layer = arm_compute::Scheduler::get().get_current_kernel();
                    auto name = get<0>(cur_layer);
                    auto guid = get<1>(cur_layer);
                    
                    this->kernel_tmp.guid = guid;
                    this->kernel_tmp.layer_name = name;
                    this->kernel_tmp.kernel_name = kernelName;
                    this->kernel_tmp.kernel_time = measure_speed;
                    this->kernel_tmp.tune_data = this->cur_tuner_data;

                    measure_time.push_back(measure_speed);

                    // printf("\t[%s, %s], [%d], [ %03.3f ms ]\n", kernelName.c_str(), name.c_str(), guid, measure_speed / 1000.0 / 1000.0);
                    if (guid == layer_guid) { 
                        this->kernel_tmp.convolution_id = conv_id;   
                    }
                    if (!this->is_first_exec && !default_runner) {
                        if (once_tuner_info == this->repeat) { 
                            // 점수 측정 방식, 최악, 최대 삭제 후 평균값
                            long long speed = 0;
                            std::sort(measure_time.begin(), measure_time.end());
                            if (measure_time.size() <= 3) { 
                                speed = std::accumulate(measure_time.begin(), measure_time.end(), 0) / measure_time.size();
                            }else { 
                                speed = std::accumulate(measure_time.begin() + 1, measure_time.end() - 1, 0) / (measure_time.size() - 2);
                            }
                            this->opt.get_tuner(cur_hash_name)->set_score(this->cur_tuner_data, speed);
                        }
                    }

                    this->result_tmp.kernels.push_back(this->kernel_tmp);
                    this->kernel_tmp.convolution_id = -1; 
                    // measure.push_back(measure_speed);
                    return; 
                });
        }
        
        arm_compute::Scheduler::get().set_get_core_current_processing_time([&](std::vector<std::pair<int, long long>> compute) { 
            this->kernel_tmp.core_time.clear();
            extract_kernel_work work;
            // printf("[%d/06] : ", compute.size());
            for (int i = 0; i < compute.size(); i++) {
                work.core_id = std::get<0>(compute[i]);
                work.compute_time = std::get<1>(compute[i]);
                this->kernel_tmp.core_time.push_back(work);
                // printf("[%02d : %.4f] ", std::get<0>(compute[i]), std::get<1>(compute[i]) / 1000.0 / 1000.0);
            }
            // printf("\n");
        });
#if DEBUG_LOG == 1
        printf("AAAA\n");
        printf("AAAA\n");
        printf("AAAA\n");
        fflush(stdout);
#endif        
        this->is_first_exec = true; 
        // engine.inference();
        this->is_first_exec = false;

#if DEBUG_LOG == 1
        printf("2AAAA\n");
        printf("2AAAA\n");
        fflush(stdout);
#endif
        opt.tuner_reset(tuner_sleep_delete);
#if DEBUG_LOG == 1
        printf("3AAAA\n");
        printf("3AAAA\n");
        fflush(stdout);
#endif
        return std::make_tuple(max_conv_size, max_work_size);
    }

    void export_best(std::string file) { 
        std::ofstream of(file);
        of << opt.export_best();
        of.close();
    }
    void execute_log(std::string file) { 
        std::ifstream idata(file);
        opt.import_best(idata);
        fix_mode = true;
    }
    void excute_best() { 
        opt.run_best();
        fix_mode = true;
    }

    extract_inference inference() {
        tune_id = 0;
        this->result_tmp.kernels.clear();
        auto start = high_resolution_clock::now();
        // engine.inference();
        auto end = high_resolution_clock::now();
        if (!fix_mode) { 
            this->opt.next_param();
        }
        this->result_tmp.black_box_time = (high_resolution_clock::now() - end).count();
        this->result_tmp.inference_time = (end - start).count();
        return this->result_tmp;
    }

    // void deinit() {  
    //     engine.deinit();
    // }
};
