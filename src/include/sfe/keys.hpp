#pragma once
#include"commands.hpp"
#include<SDL3/SDL_keycode.h>
#include<SDL3/SDL_events.h>
#include<cppp/string.hpp> // stringifying keybinds
#include<utility>
namespace sfe{
    enum class KeyModifiers : std::uint8_t{
        NONE = 0,
        CTRL = 1,
        SHIFT = 2,
        ALT = 4
    };
    // findable by ADL
    inline KeyModifiers operator|(KeyModifiers lhs,KeyModifiers rhs){
        return static_cast<KeyModifiers>(std::to_underlying(lhs) | std::to_underlying(rhs));
    }
    inline KeyModifiers& operator|=(KeyModifiers& lhs,KeyModifiers rhs){
        return lhs = (lhs | rhs);
    }
    inline bool operator&(KeyModifiers lhs,KeyModifiers rhs){
        return std::to_underlying(lhs) & std::to_underlying(rhs);
    }
    inline KeyModifiers keymods_from_sdl(SDL_Keymod sk){
        KeyModifiers result = KeyModifiers::NONE;
        if(sk & SDL_KMOD_CTRL) result |= KeyModifiers::CTRL;
        if(sk & SDL_KMOD_ALT) result |= KeyModifiers::ALT;
        if(sk & SDL_KMOD_SHIFT) result |= KeyModifiers::SHIFT;
        return result;
    }
    class Keypress{
        SDL_Keycode _key;
        KeyModifiers _mods;
        public:
            Keypress(KeyModifiers m,SDL_Keycode k) : _key(k), _mods(m){}
            Keypress(SDL_Keycode k) : Keypress(KeyModifiers::NONE,k){}
            Keypress(const SDL_KeyboardEvent& ke) : Keypress(keymods_from_sdl(ke.mod),ke.key){}
            cppp::str name() const{
                cppp::str s;
                if(_mods & KeyModifiers::CTRL){
                    s.append(u8"Ctrl-"s);
                }
                if(_mods & KeyModifiers::SHIFT){
                    s.append(u8"Shift-"s);
                }
                if(_mods & KeyModifiers::ALT){
                    s.append(u8"Alt-"s);
                }
                const char* p = SDL_GetKeyName(_key);
                while(char c=*p++){
                    s.push_back(static_cast<char8_t>(c));
                }
                return s;
            }
            SDL_Keycode key() const{
                return _key;
            }
            KeyModifiers mods() const{
                return _mods;
            }
            bool operator==(const Keypress& other) const{
                return _key == other._key && _mods == other._mods;
            }
    };
}
namespace std{
    template<>
    struct hash<sfe::Keypress>{
        static size_t operator()(sfe::Keypress rec){
            return static_cast<size_t>(rec.key()) ^ static_cast<size_t>(rec.mods());
        }
    };
}
namespace sfe{
    class Window;
    class HotkeyRecords{
        std::unordered_map<Keypress,Command> cmd;
        public:
            void add(Keypress k,Command c){
                cmd.insert_or_assign(k,c);
            }
            bool handle(Window& e,Keypress k) const{
                if(auto it=cmd.find(k);it!=cmd.end()){
                    it->second.exec(e);
                    return true;
                }
                return false;
            }
    };
    struct NodeProperties{
        constexpr static std::uint32_t N_ARY = std::numeric_limits<std::uint32_t>::max();
        std::uint32_t id;
        std::uint32_t arity;
    };
    class CodeEntry;
    class NodeKeyConfig{
        std::unordered_map<Keypress,NodeProperties> suffix;
        public:
            void register_key(Keypress kc,NodeProperties prop){
                suffix.try_emplace(kc,prop);
            }
            bool handle(CodeEntry& e,Keypress k) const;
    };
}
