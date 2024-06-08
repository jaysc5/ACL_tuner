#include "exec.h"
#include "arm_compute/runtime/Scheduler.h"
#include <thread>
#include <fstream>
#include "helper.h"
#include <asm/hwcap.h>
#include <sys/auxv.h>
#include "cxxopts/cxxopts.hpp"

#define ARM_COMPUTE_CPU_FEATURE_HWCAP_ASIMD (1 << 1)
#define ARM_COMPUTE_CPU_FEATURE_HWCAP_FPHP (1 << 9)
#define ARM_COMPUTE_CPU_FEATURE_HWCAP_ASIMDHP (1 << 10)
#define ARM_COMPUTE_CPU_FEATURE_HWCAP_ASIMDDP (1 << 20)
#define ARM_COMPUTE_CPU_FEATURE_HWCAP_SVE (1 << 22)
#define ARM_COMPUTE_CPU_FEATURE_HWCAP2_SVE2 (1 << 1)
#define ARM_COMPUTE_CPU_FEATURE_HWCAP2_SVEI8MM (1 << 9)
#define ARM_COMPUTE_CPU_FEATURE_HWCAP2_SVEF32MM (1 << 10)
#define ARM_COMPUTE_CPU_FEATURE_HWCAP2_SVEBF16 (1 << 12)
#define ARM_COMPUTE_CPU_FEATURE_HWCAP2_I8MM (1 << 13)
#define ARM_COMPUTE_CPU_FEATURE_HWCAP2_BF16 (1 << 14)
inline bool is_feature_supported(uint64_t features, uint64_t feature_mask)
{
    return (features & feature_mask);
}

/** Get the maximim number of CPUs in the system by parsing /sys/devices/system/cpu/present
 *
 * @return int Maximum number of CPUs
 */
int get_max_cpus()
{
    int           max_cpus = 1;
    std::ifstream CPUspresent;
    CPUspresent.open("/sys/devices/system/cpu/present", std::ios::in);
    bool success = false;

    if(CPUspresent.is_open())
    {
        std::string line;

        if(bool(getline(CPUspresent, line)))
        {
            /* The content of this file is a list of ranges or single values, e.g.
                 * 0-5, or 1-3,5,7 or similar.  As we are interested in the
                 * max valid ID, we just need to find the last valid
                 * delimiter ('-' or ',') and parse the integer immediately after that.
                 */
            auto startfrom = line.begin();

            for(auto i = line.begin(); i < line.end(); ++i)
            {
                if(*i == '-' || *i == ',')
                {
                    startfrom = i + 1;
                }
            }

            line.erase(line.begin(), startfrom);

            max_cpus = std::stoi(line) + 1;
            success  = true;
        }
    }

    // Return std::thread::hardware_concurrency() as a fallback.
    if(!success)
    {
        max_cpus = std::thread::hardware_concurrency();
    }
    return max_cpus;
}

struct CpuIsaInfos
{
    /* SIMD extension support */
    bool neon{ false };
    bool sve{ false };
    bool sve2{ false };

    /* Data-type extensions support */
    bool fp16{ false };
    bool bf16{ false };
    bool svebf16{ false };

    /* Instruction support */
    bool dot{ false };
    bool i8mm{ false };
    bool svei8mm{ false };
    bool svef32mm{ false };
};

void decode_hwcaps(CpuIsaInfos &isa, const uint32_t hwcaps, const uint32_t hwcaps2)
{
    // High-level SIMD support
    isa.neon = is_feature_supported(hwcaps, ARM_COMPUTE_CPU_FEATURE_HWCAP_ASIMD);
    isa.sve  = is_feature_supported(hwcaps, ARM_COMPUTE_CPU_FEATURE_HWCAP_SVE);
    isa.sve2 = is_feature_supported(hwcaps2, ARM_COMPUTE_CPU_FEATURE_HWCAP2_SVE2);

    // Data-type support
    isa.fp16    = is_feature_supported(hwcaps, ARM_COMPUTE_CPU_FEATURE_HWCAP_FPHP | ARM_COMPUTE_CPU_FEATURE_HWCAP_ASIMDHP);
    isa.bf16    = is_feature_supported(hwcaps2, ARM_COMPUTE_CPU_FEATURE_HWCAP2_BF16);
    isa.svebf16 = is_feature_supported(hwcaps2, ARM_COMPUTE_CPU_FEATURE_HWCAP2_SVEBF16);

    // Instruction extensions
    isa.dot      = is_feature_supported(hwcaps, ARM_COMPUTE_CPU_FEATURE_HWCAP_ASIMDDP);
    isa.i8mm     = is_feature_supported(hwcaps2, ARM_COMPUTE_CPU_FEATURE_HWCAP2_I8MM);
    isa.svei8mm  = is_feature_supported(hwcaps2, ARM_COMPUTE_CPU_FEATURE_HWCAP2_SVEI8MM);
    isa.svef32mm = is_feature_supported(hwcaps2, ARM_COMPUTE_CPU_FEATURE_HWCAP2_SVEF32MM);
}

int main(int argc, const char *argv[])
{
    for (int q = 0; q < 6; q++) { 
        printf("Core : %d\n\n", q);
        // set_thread_affinity(0);
    #if defined(__arm__)
        printf("is arm\n");
    #elif defined(__aarch64__)
        printf("is aarch64\n");
    #endif

        const uint32_t hwcaps   = getauxval(AT_HWCAP);
        const uint32_t hwcaps2  = getauxval(AT_HWCAP2);
        const uint32_t max_cpus = get_max_cpus();
        CpuIsaInfos r;
        decode_hwcaps(r, hwcaps, hwcaps2);

        printf("hwcaps : %d\nhwcaps2 : %d\nmax_cpus : %d\n", hwcaps, hwcaps2, max_cpus);

        printf("GCC VERSION : %d.%d.%d\n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);

        printf("%s %s\n", __DATE__, __TIME__);
        printf("%s %s\n", __DATE__, __TIME__);
        printf("%s %s\n", __DATE__, __TIME__);

        arm_compute::CPUInfo* info = &arm_compute::Scheduler::get().cpu_info();
        // dummy isa = *(dummy*)(void*)();
        // arm_compute::cpuinfo::CpuIsaInfo* p = (arm_compute::cpuinfo::CpuIsaInfo*)(&info->get_isa());

        std::cout << "has_bf16\t:" << info->has_bf16() << "\n";
        std::cout << "has_sve\t:" << info->has_sve() << "\n";
        std::cout << "has_sve2\t:" << info->has_sve2() << "\n";
        std::cout << "has_svebf16\t:" << info->has_svebf16() << "\n";
        std::cout << "has_svef32mm\t:" << info->has_svef32mm() << "\n";
        std::cout << "has_svei8mm\t:" << info->has_svei8mm() << "\n";
        std::cout << "has_dotprod\t:" << info->has_dotprod() << "\n";
        std::cout << "has_i8mm\t:" << info->has_i8mm() << "\n";

        std::cout << "\n";

        std::cout << "neon\t:" << r.neon << "\n";
        std::cout << "i8mm\t:" << r.i8mm << "\n";
        std::cout << "dot\t:" << r.dot << "\n";
        std::cout << "fp16\t:" << r.fp16 << "\n";
        std::cout << "bf16\t:" << r.bf16 << "\n";
        std::cout << "sve2\t:" << r.sve2 << "\n";
        std::cout << "sve\t:" << r.sve << "\n";
        std::cout << "svebf16\t:" << r.svebf16 << "\n";
        std::cout << "svef32mm\t:" << r.svef32mm << "\n";
        std::cout << "svei8mm\t:" << r.svei8mm << "\n";

        std::cout << "\n";
        std::cout << "\n";
        // return 0;   
    }
    cxxopts::Options options("test", "A brief description");
    options.add_options()
            ("m,mode", "mode, [default, log, optimize]", cxxopts::value<std::string>()->default_value("default"))
            ("o,model", "model [./vgg16.tflite]", cxxopts::value<std::string>())
            ("r,repeat", "repeat [8]", cxxopts::value<int>()->default_value("8"))
            ("e,evals", "evals [1000]", cxxopts::value<int>()->default_value("1000"))
            ("t,tune", "tune [./result.json]", cxxopts::value<std::string>()->default_value("./result.json"))
            ("c,conv", "convolution mode [0-16, 0]", cxxopts::value<int>()->default_value("0"))
            ("l,log", "export log-file", cxxopts::value<std::string>()->default_value(""))
            ("s,host", "tune-server host", cxxopts::value<std::string>())
            ("p,port", "tune-server port", cxxopts::value<int>())
            ("h,help", "Print usage")
        ;

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
      std::cout << options.help() << std::endl;
      exit(0);
    }

    auto mode = result["mode"].as<std::string>();

    /*if (mode == "default") { 
        std::cout << "test1" << std::endl;
        exec::exec_default(result["model"].as<std::string>(), 
                            result["repeat"].as<int>(), 
                            result["evals"].as<int>(), 
                            result["log"].as<std::string>(),
                            result["host"].as<std::string>(),
                            result["port"].as<int>());
        std::cout << "test2" << std::endl;
    }else if (mode == "log") {
        exec::exec_log(result["model"].as<std::string>(), 
                        result["tune"].as<std::string>(), 
                        result["repeat"].as<int>(), 
                        result["evals"].as<int>(), 
                        result["log"].as<std::string>(),
                        result["host"].as<std::string>(),
                        result["port"].as<int>());
    }else if (mode == "optimize") { 
        exec::optimize(result["model"].as<std::string>(), 
                        result["tune"].as<std::string>(), 
                        result["repeat"].as<int>(), 
                        result["evals"].as<int>(), 
                        result["conv"].as<int>(), 
                        result["log"].as<std::string>(),
                        result["host"].as<std::string>(),
                        result["port"].as<int>());
    }else { 
        std::cout << options.help() << std::endl;
        exit(0);
    }*/

    return 0;
}
