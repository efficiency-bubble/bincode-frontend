#pragma once
#include<chrono>
namespace sfe{
    using microseconds = std::chrono::duration<std::int64_t,std::micro>;
    using timer_point_t = std::chrono::time_point<std::chrono::steady_clock,microseconds>;
    inline timer_point_t now(){
        return std::chrono::time_point_cast<microseconds>(std::chrono::steady_clock::now());
    }
}
