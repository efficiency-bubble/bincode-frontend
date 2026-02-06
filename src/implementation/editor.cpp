#include<sfe/editor.hpp>
#include<cppp/tostring.hpp>
namespace sfe{
    constexpr static cppp::fvec3 RED{1.0f,0.0f,0.0f};
    constexpr static cppp::fvec3 WHITE{1.0f};
    constexpr static cppp::fvec3 GRAY{0.7f};
    void Editor::draw_node(const VisualNode& nd,cppp::fvec2& pos,sgl::CachedFont& cf,sgl::CoordinateMap& cm){
        using namespace std::literals;
        using namespace cppp::literals;
        cppp::fvec2 cpos = pos;
        bool selected = (&nd==&cursor.top());
        switch(nd.type()){
            using enum bbe::NodeType;
            case UINT32:
                tr.draw_text(cppp::format<u8"{}"_ts>(nd.prim()),pos,1.0f,WHITE,cf,cm);
                if(cursor_after) cpos = pos;
                tr.draw_text(u8"dw"sv,pos,0.5f,GRAY,cf,cm);
                break;
            case CALL_BUILTIN:
                switch(nd.prim()){
                    case 10:
                        tr.draw_text(u8"("sv,pos,1.0f,WHITE,cf,cm);
                        draw_node(nd.children().front(),pos,cf,cm);
                        tr.draw_text(u8")+("s,pos,1.0f,WHITE,cf,cm);
                        draw_node(nd.children()[1uz],pos,cf,cm);
                        tr.draw_text(u8")"sv,pos,1.0f,WHITE,cf,cm);
                        break;
                    default:
                        tr.draw_text(cppp::format<u8"BUILTIN[{}]("_ts>(nd.prim()),pos,1.0f,WHITE,cf,cm);
                        draw_node(nd.children().front(),pos,cf,cm);
                        tr.draw_text(u8")"sv,pos,1.0f,WHITE,cf,cm);
                        break;
                }
                if(cursor_after) cpos = pos;
                break;
            case PACK: {
                for(std::uint32_t i=0;i<nd.children().size();++i){
                    if(i) tr.draw_text(u8","sv,pos,1.0f,WHITE,cf,cm);
                    draw_node(nd.children()[i],pos,cf,cm);
                }
                if(cursor_after) cpos = pos;
                break;
            }
            case NTYPE:
                tr.draw_text(u8"￼"s,pos,1.0f,selected?RED:WHITE,cf,cm);
                return;
            default:
                std::unreachable();
        }
        if(selected){
            ld.line(cm.cvt_abs(cpos-cppp::ivec2(0,cf.font().ascender()>>6)),{0.5f,0.0f,0.0f},cm.cvt_abs(cpos-cppp::ivec2(0,cf.font().descender()>>6)),{0.8f,1.0f,1.0f});
        }
    };
    void Editor::navigate(bool right,bool fast){
        bool nochildren = cursor.top().children().empty();
        if(nochildren && (cursor.top().fast() || fast)){
            cursor_after = right;
        }
        if(cursor_after == right){
            if(cursor.has_nesting()){
                if((right && cursor.is_last_child()) || (!right && cursor.is_first_child())){
                    cursor.leave();
                    cursor_after = right;
                }else{
                    if(right){
                        cursor.next_sibling();
                        cursor_after = false;
                    }else{
                        cursor.prev_sibling();
                        cursor_after = true;
                    }
                }
            }else{
                // wrap around whole project
                cursor_after = !right;
            } 
        }else if(!nochildren){
            cursor.enter(right?0:static_cast<std::uint32_t>(cursor.top().children().size()-1));
        }else{
            cursor_after = right;
        }
    }
    void Editor::keydown(const SDL_KeyboardEvent& key){
        switch(key.key){
            case SDLK_LEFT:
                navigate(false,true);
                break;
            case SDLK_RIGHT:
                navigate(true,true);
                break;
            case SDLK_TAB:
                navigate(!(key.mod&SDL_KMOD_SHIFT),false);
                break;
            case SDLK_ESCAPE:
                cursor.home();
                break;
            case SDLK_BACKSPACE:
                if(selected().type() == bbe::NodeType::NTYPE){
                    if(!cursor.has_nesting()){
                        break; // can't delete root node
                    }
                    cursor.leave();
                }
                selected().node() = {bbe::NodeType::NTYPE,0};
                selected().rerender();
                break;
        }
    }
}
