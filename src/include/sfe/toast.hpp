#pragma once
#include<cppp/string.hpp>
#include"time.hpp"
namespace sfe{
    class Toast{
        cppp::str _message;
        timer_point_t until;
        mutable bool possibly_alive;
        public:
            Toast() : possibly_alive(false){}
            Toast(cppp::str msg,timer_point_t expiry) : _message(std::move(msg)),  until(std::move(expiry)){}
            Toast(cppp::str msg,microseconds dur) : Toast(std::move(msg),now()+dur){}
            void reset(cppp::str msg,microseconds dur){
                reset(std::move(msg),now()+dur);
            }
            const cppp::str& message() const{
                return _message;
            }
            void reset(cppp::str msg,timer_point_t expiry){
                _message = std::move(msg);
                until = expiry;
                possibly_alive = true;
            }
            bool alive() const{
                if(possibly_alive){
                    possibly_alive = (now() <= until);
                }
                return possibly_alive;
            }
    };
}
