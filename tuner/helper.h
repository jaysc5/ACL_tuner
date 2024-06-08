#include <vector>
#include <string>
#include <utility>

std::vector<int> get_available_frequency(int core_pin);
void set_cpu_frequency(int core_pin, int freq);
std::vector<int> get_current_frequency(int max_cpu);
std::vector<std::string> get_available_governors(int core_pin);
void set_governors(int core_pin, std::string governors);
std::string get_governors(int core_pin);
void set_thread_affinity(int core_id);
std::pair<std::vector<std::vector<int>>, std::vector<std::vector<int>>>  partition_core_freq(int core_count);

void setup(int core_count);
void remove_cache();
