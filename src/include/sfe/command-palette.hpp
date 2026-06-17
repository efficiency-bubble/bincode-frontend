#pragma once
#include"commands.hpp"
#include<numeric>
namespace sfe{
    class CommandPalette{
        cppp::str buffer;
        friend class Textbox;
        
        const CommandSet* commands;
        std::vector<const CommandSet::entry_type*> candidates;
        std::size_t selection;
        void populate(){
            for(const auto& el : commands->commands()){
                candidates.emplace_back(&el);
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
            CommandPalette(const CommandSet& c) : commands(&c), selection(0uz){
                populate();
            }
            void append(cppp::sv more);
            void backspace();
            void reset(){
                buffer.clear();
                candidates.clear();
                populate();
            }
            const Command* selected() const{
                if(candidates.empty()) return nullptr;
                return &candidates[selection]->second;
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
                    selection = std::saturating_sub(candidates.size(),1uz);
                }
            }
            void render(const GraphicsContext& gc,cppp::fvec2 pos,float width,float text_scale) const;
    };
}
