#include<sfe/editor.hpp>
#include<cppp/format.hpp>
namespace sfe{
    void CodeEntry::navigate(bool right,bool fast){
        bool nochildren = cursor.selected().children().empty();
        if(nochildren && (cursor.selected().is_placeholder() || fast)){
            cursor.set_after(right);
        }
        if(cursor.is_after() == right){
            if(cursor.trail().has_nesting()){
                if((right && cursor.trail().is_last_child()) || (!right && cursor.trail().is_first_child())){
                    cursor.trail().leave();
                    cursor.set_after(right);
                }else{
                    if(right){
                        cursor.trail().next_sibling();
                        cursor.set_after(false);
                    }else{
                        cursor.trail().prev_sibling();
                        cursor.set_after(true);
                    }
                }
            }else{
                // wrap around whole project
                cursor.set_after(!right);
            } 
        }else if(!nochildren){
            cursor.trail().enter(right?0:static_cast<std::uint32_t>(cursor.selected().children().size()-1));
        }else{
            cursor.set_after(right);
        }
    }
    void CodeEntry::keydown(Keypress kp){
        switch(kp.key()){
            case SDLK_LEFT:
                navigate(false,true);
                break;
            case SDLK_RIGHT:
                navigate(true,true);
                break;
            case SDLK_TAB:
                navigate(!(kp.mods()&KeyModifiers::SHIFT),false);
                break;
            case SDLK_ESCAPE:
                cursor.trail().home();
                break;
            case SDLK_BACKSPACE:
                if(selected().type() == VisualNodeType::A){
                    bbe::ASTNode& an = selected().a();
                    if(an.type() == bbe::NodeType::NTYPE){
                        if(cursor.trail().below_top().type() != VisualNodeType::A){
                            break; // trying to delete an already-blank root node; do nothing
                        }
                        std::uint32_t ti = cursor.trail().top_index();
                        cursor.trail().leave();
                        cursor.set_after(false);
                        if(selected().a().children().size() == 2 && selected().a().type() != bbe::NodeType::PACK){
                            [[assume(ti <= 1)]];
                            bbe::ASTNode tmp = std::move(selected().a().children()[1-ti]);
                            selected().a() = std::move(tmp);
                        }else{
                            switch(selected().a().type()){
                                case bbe::NodeType::COMMA:
                                case bbe::NodeType::PACK:
                                    printf("Popping\n");
                                    selected().a().children().pop(ti);
                                    break;
                                default:
                                    // can't drop down multiple nodes, just delete them
                                    selected().a() = {bbe::NodeType::NTYPE,0};
                                    break;
                            }
                        }
                    }else{
                        an = {bbe::NodeType::NTYPE,0};
                    }
                    selected().arerender();
                }
                break;
        }
    }
}
