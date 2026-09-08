#pragma once
#ifdef SFE_ENABLE_ACHIEVEMENTS
#include"graphics.hpp"
#include<cppp/string.hpp> // achievement names
#include<cppp/vector.hpp>
#include<numeric>
#include<chrono>
#include<queue>
#include<cmath>
#include<meta>
namespace sfe{
    using namespace std::literals;
    class Achievement{
        cppp::sv aname;
        cppp::sv adesc;
        bool completed;
        friend class AchievementQueue;
        public:
            Achievement(cppp::sv n,cppp::sv d) : aname(n), adesc(d), completed(false){}
            void revoke(){
                completed = false;
            }
            cppp::sv name() const{
                return aname;
            }
            cppp::sv description() const{
                return adesc;
            }
    };
    class AchievementPopup{
        const Achievement* ach;
        public:
            AchievementPopup(const Achievement& ach) : ach(&ach){}
            void render(const GraphicsContext& gc,cppp::fvec2 pos,float brightness,float alpha);
    };
    class AchievementQueue{
        std::queue<AchievementPopup> popups;
        using µs = std::chrono::duration<std::uint64_t,std::micro>;
        using µsts = std::chrono::time_point<std::chrono::steady_clock,µs>;
        µsts spawn_time;
        constexpr static float DECAY_RATE = -0.000005f;
        constexpr static float FADE_TIME = 3'150'000.0f;
        constexpr static float FADE_LENGTH = 550'000.0f;
        public:
            void complete(Achievement& ach){
                if(ach.completed) return;
                ach.completed = true;
                if(popups.empty()) spawn_time = std::chrono::time_point_cast<µs>(std::chrono::steady_clock::now());
                popups.emplace(ach);
            }
            void render(const GraphicsContext& gc,cppp::fvec2 pos){
                while(!popups.empty()){
                    float elapsed = static_cast<float>((std::chrono::time_point_cast<µs>(std::chrono::steady_clock::now()) - spawn_time).count());
                    float alpha = std::min(1.0f,(FADE_TIME-elapsed) / FADE_LENGTH);
                    if(alpha < 0.0f){
                        popups.pop();
                        spawn_time = std::chrono::time_point_cast<µs>(std::chrono::steady_clock::now());
                    }else{
                        float brightness = std::exp(elapsed * DECAY_RATE);
                        popups.front().render(gc,pos,brightness,alpha);
                        break;
                    }
                }
            }
    };
    namespace achievements{
        struct GarbageCollected : Achievement{
            GarbageCollected() : Achievement(u8"Taking out the trash"sv,u8"Ran GC on the type database"sv){}
        };
    }
    consteval cppp::str uncamelcaseify(cppp::sv ident){
        cppp::str idt;
        bool consequ = true;
        for(const char8_t c : ident){
            if(c >= u8'A' && c <= u8'Z'){
                if(!consequ){
                    idt.push_back(u8'_');
                    consequ = true;
                }
                idt.push_back(static_cast<char8_t>(static_cast<std::uint8_t>(c) | 0b00100'000));
            }else{
                consequ = false;
                idt.push_back(c);
            }
        }
        return idt;
    }
    class AchievementDB;
    consteval{
        std::vector<std::meta::info> specs{data_member_spec(^^AchievementQueue,{.name=u8"queue"sv})};
        for(std::meta::info mem : members_of(^^achievements,std::meta::access_context::unprivileged())){
            if(!is_type(mem)) continue;
            specs.emplace_back(data_member_spec(mem,{.name=uncamelcaseify(u8identifier_of(mem))}));
        }
        define_aggregate(^^AchievementDB,specs);
    }
}
#endif
