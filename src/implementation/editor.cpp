#include<sfe/editor.hpp>
#include<cppp/tostring.hpp>
namespace sfe{
    void Editor::navigate(bool right,bool fast){
        bool nochildren = cursor.trail().top().children().empty();
        if(nochildren && (cursor.trail().top().fast() || fast)){
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
            cursor.trail().enter(right?0:static_cast<std::uint32_t>(cursor.trail().top().children().size()-1));
        }else{
            cursor.set_after(right);
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
                cursor.trail().home();
                break;
            case SDLK_BACKSPACE:
                if(auto van=dynamic_cast<VisualASTNode*>(&selected())){
                    if(van->type() == bbe::NodeType::NTYPE){
                        if(!cursor.trail().has_nesting()){
                            break; // can't delete root node
                        }
                        cursor.trail().leave();
                    }
                    van->node() = {bbe::NodeType::NTYPE,0};
                    selected().rerender_children();
                }
                break;
        }
    }
}
