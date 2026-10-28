#include<sfe/editor.hpp>
#include<cppp/format.hpp>
#include<cppp/int.hpp>
namespace sfe{
    void CodeEntry::navigate(bool right,bool fast){
        bool nochildren = !_cursor.selected().num_children();
        if(nochildren && (_cursor.selected().is_placeholder() || fast)){
            _cursor.set_after(right);
        }
        if(_cursor.is_after() == right){
            if(_cursor.is_nested()){
                if((right && _cursor.is_last_child()) || (!right && _cursor.is_first_child())){
                    _cursor.leave();
                    _cursor.set_after(right);
                }else{
                    if(right){
                        _cursor.next_sibling();
                        _cursor.set_after(false);
                    }else{
                        _cursor.prev_sibling();
                        _cursor.set_after(true);
                    }
                }
            }else{
                // wrap around whole project
                _cursor.set_after(!right);
            } 
        }else if(!nochildren){
            _cursor.enter(right?0:static_cast<std::uint32_t>(_cursor.selected().num_children()-1));
        }else{
            _cursor.set_after(right);
        }
    }
    void CodeEntry::keydown(Keypress kp){
        switch(kp.key()){
            case SDLK_LEFT:
            case SDLK_UP:
                navigate(false,true);
                break;
            case SDLK_RIGHT:
            case SDLK_DOWN:
                navigate(true,true);
                break;
            case SDLK_TAB:
                navigate(!(kp.mods()&KeyModifiers::SHIFT),false);
                break;
            case SDLK_ESCAPE:
                _cursor.home();
                break;
            case SDLK_BACKSPACE:
                switch(_cursor.selected().type()){
                    case VisualNodeType::A: {
                        const bbe::ASTNode& an = _cursor.selected().a().get();
                        if(an.type() == bbe::NodeType::NTYPE){
                            if(_cursor.selected2().type() != VisualNodeType::A){
                                break; // trying to delete an already-blank root node; do nothing
                            }
                            std::uint32_t ti = _cursor.index_of_selection();
                            _cursor.leave();
                            _cursor.set_after(false);
                            if(bbe::can_pop_back_from(_cursor.selected().a().get())){
                                _cursor.selected().a().erase(ti);
                                if(ti) _cursor.enter(ti-1,true);
                            }else if(_cursor.selected().a().get().children().size() == 2){
                                [[assume(ti <= 1)]];
                                _cursor.selected().a().promote(1_u32-ti);
                            }else{
                                // can't drop down multiple nodes, just delete them
                                _cursor.selected().a().clear();
                            }
                        }else{
                            _cursor.selected().a().clear();
                        }
                        break;
                    }
                    case VisualNodeType::T:
                        if(_cursor.selected().t().type() == TypeVisualNodeType::CT){
                            _cursor.selected().t().cttodblank();
                            break;
                        }else{
                            if(_cursor.selected().t().dt()){
                                _cursor.selected().t().dtblank();
                            }else{
                                // parent can't be DT or else something's really, really wrong, so we just check for CT here
                                if(_cursor.selected2().t().type() != TypeVisualNodeType::CT){
                                    break; // trying to delete an already-blank root node; do nothing
                                }
                                std::uint32_t ti = _cursor.index_of_selection();
                                _cursor.leave();
                                _cursor.set_after(false);
                                if(_cursor.selected().t().ct() == MTSCompoundTypeCategory::PACK){
                                    _cursor.selected().t().cterase(ti);
                                    if(ti) _cursor.enter(ti-1,true);
                                }else if(_cursor.selected().t().ct() == MTSCompoundTypeCategory::FUNCTION_POINTER){
                                    [[assume(ti <= 1)]];
                                    _cursor.selected().t().ctpromote(1_u32-ti);
                                }else{
                                    _cursor.selected().t().cttodblank();
                                }
                            }
                            break;
                        }
                    default:;
                }
                break;
        }
    }
}
