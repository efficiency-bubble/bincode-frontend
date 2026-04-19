#pragma once
#include<cppp/string.hpp> // command names
#include<cppp/strmap.hpp>
#include<numeric>
#include<vector>
#include"graphics.hpp"
namespace sfe{
    using namespace std::literals;
    class Window;
    class Command{
        void(*_exec)(Window&);
        public:
            Command(void(*e)(Window&)) : _exec(e){}
            void exec(Window& ed) const{
                return _exec(ed);
            }
    };
    class CommandSet{
        cppp::ordered_strmap<Command> cmdv;
        public:
            using entry_type = cppp::ordered_strmap<Command>::value_type;
            void add(cppp::str&& n,Command c){
                cmdv.try_emplace(std::move(n),c);
            }
            Command get(cppp::sv k) const{
                // TODO: replace with C++26 heterogenous at
                if(auto it=cmdv.find(k);it!=cmdv.end()){
                    return it->second;
                }
                throw std::logic_error("CommandSet::at: no such command"s);
            }
            const cppp::ordered_strmap<Command>& commands() const{
                return cmdv;
            }
    };
}
