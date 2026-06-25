#include<print>
#include<cstdlib>
#include<string>
#include<chrono>
extern "C"{
    std::uint32_t fn_0(std::uint32_t);
}
using hrc = std::chrono::high_resolution_clock;
using namespace std::literals;
using µs = std::chrono::duration<std::uint64_t,std::micro>;
int main(int argc,char* argv[]){
    if(argc == 1) return 1;
    std::uint32_t num = static_cast<std::uint32_t>(std::strtoul(argv[1],nullptr,10));
    if(errno == ERANGE) return 2;
    hrc::time_point bgn = hrc::now();
    std::uint32_t result = fn_0(num);
    hrc::time_point end = hrc::now();
    µs delta = std::chrono::duration_cast<µs>(end-bgn);
    std::print("{} in "sv,result);
    if(delta.count() < 1000){
        std::print("{} µs"sv,delta.count());
    }else{
        std::print("{:.2f} ms"sv,static_cast<float>(delta.count())/1000.0f);
    }
    return 0;
}
