#include "include/helper.h"
#include <iostream>
#include <fstream>

std::vector<int> get_available_frequency(int core_pin) { 
    std::vector<int> result;
    char buffer[256];
    sprintf(buffer, "%s%d%s", "/sys/devices/system/cpu/cpu", core_pin, "/cpufreq/scaling_available_frequencies");
    std::ifstream file(buffer, std::ios::in);
    int mem;
    while (!file.eof()) { 
        file >> mem;
        result.push_back(mem);
    }
    file.close();
    return result;
}

void set_cpu_frequency(int core_pin, int freq) { 
    std::vector<int> result;
    char buffer[256];

    sprintf(buffer, "%s%d%s", "/sys/devices/system/cpu/cpu", core_pin, "/cpufreq/scaling_setspeed");
    
    std::ofstream file(buffer, std::ios::out);
    file << freq;
    file.close();
}

std::vector<int> get_current_frequency(int max_cpu) {
    int cpu_num = max_cpu;
    char buffer[256];
    std::vector<int> result;
    for (int num = 0; num < cpu_num; num++) {
        sprintf(buffer, "%s%d%s", "/sys/devices/system/cpu/cpu", num, "/cpufreq/scaling_cur_freq");
        std::ifstream file(buffer, std::ios::in);
        int mem;
        if (file.is_open()) { 
            file >> mem;
        }
        result.push_back(mem);
    }
    return result;
}

std::vector<std::string> get_available_governors(int core_pin) { 
    char buffer[256];
    std::vector<std::string> result;
    sprintf(buffer, "%s%d%s", "/sys/devices/system/cpu/cpu", core_pin, "/cpufreq/scaling_available_governors");
    std::ifstream file(buffer, std::ios::in);

    std::string data;
    while (!file.eof()) { 
        file >> data;
        result.push_back(data);
    }
    return result;
}

void set_governors(int core_pin, std::string governors) { 
    char buffer[256];
    sprintf(buffer, "%s%d%s", "/sys/devices/system/cpu/cpu", core_pin, "/cpufreq/scaling_governor");
    
    std::ofstream file(buffer, std::ios::out);
    file << governors;
    file.close();
}

std::string get_governors(int core_pin) { 
    char buffer[256];
    sprintf(buffer, "%s%d%s", "/sys/devices/system/cpu/cpu", core_pin, "/cpufreq/scaling_governor");
    std::string result;
    std::ifstream file(buffer, std::ios::in);
    file >> result;
    file.close();
    return result;
}

void set_thread_affinity(int core_id)
{
    if(core_id < 0)
    {
        return;
    }

#if !defined(_WIN64) && !defined(__APPLE__) && !defined(__OpenBSD__)
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(core_id, &set);
    sched_setaffinity(0, sizeof(set), &set);
#endif /* !defined(__APPLE__) && !defined(__OpenBSD__) */
}

std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>> partition_core_freq(int core_count)
{
    std::vector<std::vector<int>> cluster;
    std::vector<std::vector<int>> cluster_frequncy;

    std::vector<int> cluster_tmp;
    std::vector<int> freq_tmp;

    for (int i = 0; i < core_count; i++) { 
        auto freq = get_available_frequency(i);
        if (freq == freq_tmp || freq_tmp.empty()) { 
            freq_tmp = freq;
            cluster_tmp.push_back(i);
        }else { 
            cluster.push_back(cluster_tmp);
            freq_tmp = freq;
            cluster_frequncy.push_back(freq_tmp);
            cluster_tmp = std::vector<int>();
            cluster_tmp.push_back(i);
        }   
    }

    if (!cluster_tmp.empty()) { 
        cluster.push_back(cluster_tmp);
    }
    if (!freq_tmp.empty()) { 
        cluster_frequncy.push_back(freq_tmp);
    }

    printf("\n");
    for (int i = 0; i < cluster.size() ; i++) { 
        printf("[%02d] : ", i);
        for (int c = 0; c < cluster[i].size(); c++) { 
            printf("%d, ", cluster[i][c]);
        }
        printf("\n");
    }

    return std::make_pair(cluster, cluster_frequncy);
}

void setup(int core)
{
    printf("%d   ]]]\n", core);
    int cpu_core = core;

    auto cf = partition_core_freq(cpu_core);
    for (int i = 0; i < cf.first.size(); i++)
    {
        for (const auto &core : cf.first[i])
        {
            set_governors(core, "userspace");
            set_cpu_frequency(core, cf.second[i].back());
        }
    }

    auto cur_freq = get_current_frequency(cpu_core);
    std::cout << "Available Governors\n";
    for (int i = 0; i < cpu_core; i++)
    {
        auto freq = get_available_frequency(i);
        auto gov = get_available_governors(i);
        auto cur_gov = get_governors(i);
        std::cout << "Core Pin : [ " << i + 1 << " ] \n";
        std::cout << "Current Freq : [ " << cur_freq[i] << " ] \n";
        std::cout << "Current Gov : [ " << cur_gov << " ] \n";
        std::cout << "Available Freq : [ ";
        for (const auto &data : freq)
        {
            std::cout << data << ", ";
        }
        std::cout << "]\n";

        std::cout << "Available Governors : [ ";
        for (const auto &data : gov)
        {
            std::cout << data << ", ";
        }
        std::cout << "]\n\n\n";
    }

    return;
}

void remove_cache() {
    system("sync; echo 3 > /proc/sys/vm/drop_caches");
}

