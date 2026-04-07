#pragma once
#include<cppp/string.hpp> // command names
#include<cppp/strmap.hpp>
#include<numeric>
#include<vector>
#include"graphics.hpp"
#include"editor.hpp"
namespace sfe{
    class Command{
        bool(*_valid)(Editor&);
        void(*_exec)(Editor&);
        public:
            Command(bool(*v)(Editor&),void(*e)(Editor&)) : _valid(v), _exec(e){}
            bool valid(Editor& ed) const{
                return _valid(ed);
            }
            void exec(Editor& ed) const{
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
            const cppp::ordered_strmap<Command>& commands() const{
                return cmdv;
            }
    };
    class CommandSelectorPanel{
        const CommandSet* commands;
        Editor* ed;
        cppp::str _buffer;
        std::vector<const CommandSet::entry_type*> candidates;
        std::size_t selection;
        void _populate(){
            for(const auto& el : commands->commands()){
                if(el.second.valid(*ed)){
                    candidates.emplace_back(&el);
                }
            }
        }
        static char8_t casefold(char8_t c){
            return static_cast<char8_t>(std::char_traits<char>::to_char_type(tolower(std::char_traits<char>::to_int_type(static_cast<char>(c)))));
        }
        static bool has_prefix(cppp::sv s,cppp::sv prefix,std::size_t known){
            if(prefix.size() > s.size()) return false;
            for(std::size_t i=known;i<prefix.size();++i){
                if(casefold(s[i]) != casefold(prefix[i])) return false;
            }
            return true;
        }
        public:
            CommandSelectorPanel(const CommandSet& c,Editor& e) : commands(&c), ed(&e), selection(0uz){
                _populate();
            }
            void append(std::string_view data){
                std::size_t match_begin = _buffer.size();
                _buffer.append_range(data);
                if(!candidates.empty()){
                    std::vector<const CommandSet::entry_type*> refined_candidates;
                    std::size_t current_selection = std::exchange(selection,0uz);
                    for(std::size_t i=0uz;i<candidates.size();++i){
                        if(has_prefix(candidates[i]->first,_buffer,match_begin) && candidates[i]->second.valid(*ed)){
                            if(i == current_selection){
                                selection = refined_candidates.size();
                            }
                            refined_candidates.emplace_back(candidates[i]);
                        }
                    }
                    candidates = std::move(refined_candidates);
                }
            }
            void backspace(){
                if(!_buffer.empty()){
                    _buffer.pop_back();
                    candidates.clear();
                    for(const auto& el : commands->commands()){
                        if(has_prefix(el.first,_buffer,0uz) && el.second.valid(*ed)){
                            candidates.emplace_back(&el);
                        }
                    }
                    selection = 0uz;
                }
            }
            void exec() const{
                if(!candidates.empty()){
                    candidates[selection]->second.exec(*ed);
                }
            }
            void next(){
                if(!candidates.empty() && (++selection) == candidates.size()){
                    selection = 0;
                }
            }
            void prev(){
                if(selection){
                    --selection;
                }else{
                    selection = std::sub_sat(candidates.size(),1uz);
                }
            }
            void clear(){
                _buffer.clear();
                candidates.clear();
                selection = 0uz;
                _populate();
            }
            void render(const GraphicsContext& gc,cppp::fvec2 pos,float width,float text_scale);
    };
}
