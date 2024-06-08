#include "search/tune_engine.h"
#include <iostream>

std::vector<int> tune_engine::cluster_to_core(tuner_data t) {
    std::vector<int> result;
    // printf("core : [");
    for (const auto& clust : t) { 
        for (int i = 0; i < core[clust.core_cluster].size(); i++) {
            // window size == 3
            // core size == 4
            // core_idx == 3 인 경우 실행하면 안댐.
            // core_idx >= window_size 인 경우 삽입 ㄴㄴ
            if (i < clust.window_size) { 
                result.push_back(core[clust.core_cluster][i]);
                // printf("%d, ", core[clust.core_cluster][i]);
            } 
        }
    }
    // printf("]\n");
    return result;
}

std::vector<std::vector<int>> tune_engine::convert_core(tuner_data t) { 
    std::vector<std::vector<int>> result;

    for (int i = 0; i < core.size(); i++)  {
        std::vector<int> cores;
        for (const auto& clust : t) {
            if (clust.core_cluster != i) { 
                continue;
            } 
            
            for (int i = 0; i < core[clust.core_cluster].size(); i++) {
                if (i < clust.window_size) { 
                    cores.push_back(core[clust.core_cluster][i]);
                }
            }
        }
        result.push_back(cores);
    }
    return result;
}

std::tuple<int, int, int> tune_engine::index_to_work_size(tuner_data current, int idx, int max_idx, int max_window, int start, int end, int step) { 
    // 튜닝 데이터가 없다면 초기 데이터로 ㄱㄱ
    if (current.size() == 0) { 
        return std::make_tuple(-1,-1,-1);
    }else { 
        // idx -> cluster, core_idx in cluster
        auto core_list = convert_core(current);

        int set_cluster_idx = -1; 
        int core_idx = idx;

        int prev_cluster_worksize = 0;
        
        for (int i = 0; i < current.size(); i++) {
            int core_size = core_list[current[i].core_cluster].size(); 
            if (core_idx < core_size) {
                set_cluster_idx = i;
                break;
            }
            core_idx -= core_size;
            prev_cluster_worksize += current[i].window_size; 
        }

        if (set_cluster_idx == -1) { 
            throw "set cluster idx is -1";
        }
        
        int avg_size = current[set_cluster_idx].window_size / core_list[current[set_cluster_idx].core_cluster].size();
        int remain_size = current[set_cluster_idx].window_size % core_list[current[set_cluster_idx].core_cluster].size();
        // core_idx = [0, 1,2, 3]
        // remain_size = 2
        // core_idx 0 : 1
        // core_idx 1 : 1
        // core_idx 2 : 0
        // core_idx 3 : 0
        int this_core_alloc = (core_idx < remain_size);
        int current_work_size = avg_size + this_core_alloc;
        // std::min(core_idx - 1, 0)

        int start = prev_cluster_worksize                   // 클러스터 위치 탐색
                            + (avg_size * core_idx)            // 코어 위치 탐색
                            + std::min(remain_size, core_idx); // remain_size 오차 보정
        int end = start + current_work_size;

        // printf("\t[%02d, %02d, %04d, %04d, %04d, %04d]\n", idx, max_idx, start, end, step, max_window);
        // printf("\t[%03d, %03d, %03d, %04d]\n", avg_size, remain_size, this_core_alloc, current[set_cluster_idx].window_size);
        
        // int sum = 0;
        // int _start = 0;
        // int _end = 0;
        // for (int i = 0; i < idx; i++) { 
        //     _start += current.windows[i];
        // }
        // _end = _start;
        // _end += current.windows[idx];
        
        return std::make_tuple(start * step, end * step, step);
    }
}

void tune_engine::create_search_space() {
    // 1개씩 넣기
    std::vector<tuner_data> tmp;
    for (int i = 0; i < core.size(); i++)
    {
        tuner_data d;
        tune t;
        t.core_cluster = i;
        t.window_size = max_window_size;
        t.frequency = -1;
        d.push_back(t);
        tmp.push_back(d);
    }
    if (tmp.size() != 0) { 
        search_space.push_back(tmp);
        tmp.clear();
    }
    
    // 2개씩 비율 넣기
    for (int x = 0; x < core.size(); x++)
    {
        for (int y = x + 1; y < core.size(); y++)
        {
            if (x == y)
            {
                continue;
            }

            for (int w = 1; w < max_window_size; w++)
            {
                tuner_data d;
                tune x1{-1, x, max_window_size - w};
                tune y1{-1, y, w};

                // if (max_window_size - w <= 0 || w <= 0)
                // {
                //     continue;
                // }
                d.push_back(x1);
                d.push_back(y1);
                tmp.push_back(d);
            }
        }
    }
    
    if (tmp.size() != 0) { 
        search_space.push_back(tmp);
        tmp.clear();
    }

    // 3개씩 비율넣기
    // for (int x = 0; x < core.size(); x++)
    // {
    //     for (int y = x + 1; y < core.size(); y++)
    //     {
    //         for (int z = y + 1; z < core.size(); z++)
    //         {
    //             if (x == y || x == z || y == z)
    //             {
    //                 continue;
    //             }

    //             for (int x1 = 1; x1 < max_window_size; x1+=4)
    //             {
    //                 for (int y1 = 1; y1 < max_window_size - x1; y1+=4)
    //                 {
    //                     tuner_data d;
    //                     tune xt{-1, x, x1};
    //                     tune yt{-1, y, y1};
    //                     tune zt{-1, z, max_window_size - x1 - y1};

    //                     if (x1 <= 0 || y1 <= 0 || max_window_size - x1 - y1 <= 0)
    //                     {
    //                         continue;
    //                     }

    //                     d.push_back(xt);
    //                     d.push_back(yt);
    //                     d.push_back(zt);
    //                     tmp.push_back(d);
    //                 }
    //             }
    //         }
    //     }
    // }
    // if (tmp.size() != 0) { 
    //     search_space.push_back(tmp);
    //     tmp.clear();
    // }

    if (search_space.size() == 0) { 
        throw "얘가 왜이러는지 나도 잘 모름 ㅎㅎ;;;";
    }
}

int tune_engine::get_search_space() const {
    int search_spaces = 0;
    for (const auto& c : search_space) { 
        search_spaces += c.size();
    }
    return search_spaces;
}
void tune_engine::set_current(tuner_data data) { 
    this->next_data = data;
}

void tune_engine::best_current() { 
    this->next_data = this->best_data;
}
