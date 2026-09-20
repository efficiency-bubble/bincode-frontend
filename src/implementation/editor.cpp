#include<sfe/editor.hpp>
#include<cppp/format.hpp>
#include<cppp/int.hpp>
namespace sfe{
    void CodeEntry::navigate(bool right,bool fast){
        bool nochildren = _cursor.selected().children().empty();
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
            _cursor.enter(right?0:static_cast<std::uint32_t>(_cursor.selected().children().size()-1));
        }else{
            _cursor.set_after(right);
        }
    }
    void CodeEntry::keydown(Keypress kp){
        using namespace cppp::literals;
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
                        const bbe::ASTNode& an = _cursor.selected().a();
                        if(an.type() == bbe::NodeType::NTYPE){
                            if(_cursor.selected2().type() != VisualNodeType::A){
                                break; // trying to delete an already-blank root node; do nothing
                            }
                            std::uint32_t ti = _cursor.index_of_selection();
                            _cursor.leave();
                            _cursor.set_after(false);
                            if(bbe::nchld_of(an.type()) == bbe::VARIABLE){
                                _cursor.selected().aerase(ti);
                                if(ti) _cursor.enter(ti-1,true);
                            }else if(_cursor.selected().a().children().size() == 2){
                                [[assume(ti <= 1)]];
                                _cursor.selected().apromote(1_u32-ti);
                            }else{
                                // can't drop down multiple nodes, just delete them
                                _cursor.selected().aclear();
                            }
                        }else{
                            _cursor.selected().aclear();
                        }
                        break;
                    }
                    case VisualNodeType::T: {
                        // const bbe::TypeInfo*& t = _cursor.selected().t();
                        // if(t){
                        //     _cursor.selected().treset();
                        // }else{
                        //     if(_cursor.selected2().type() != VisualNodeType::T){
                        //         break; // trying to delete an already-blank root node; do nothing
                        //     }
                        //     std::uint32_t ti = _cursor.index_of_selection();
                        //     _cursor.leave();
                        //     _cursor.set_after(false);
                        //     if(t->type() == bbe::TypeCategory::PACK){
                        //         cppp::uninitialized_memory<const bbe::TypeInfo*> np{t->pack_contents().size()-1uz};
                        //         auto middle = t->pack_contents().array().begin() + ti;
                        //         std::uninitialized_copy(middle+1,t->pack_contents().array().end(),
                        //             std::uninitialized_copy(t->pack_contents().array().begin(),middle,np.data())
                        //         );
                        //         t = &proj().types().pack_of(std::move(np));
                        //         _cursor.selected().popi(ti);
                        //         if(ti) _cursor.enter(ti-1,true);
                        //     }else if(t->type() == bbe::TypeCategory::FUNCTION_POINTER){
                        //         [[assume(ti <= 1)]];
                        //         bbe::ASTNode tmp = std::move(_cursor.selected().a().children()[1-ti]);
                        //         _cursor.selected().a() = std::move(tmp);
                        //         _cursor.selected().arerender();
                        //     }else{
                        //         // can't drop down multiple nodes, just delete them
                        //         _cursor.selected().a() = {bbe::NodeType::NTYPE,0};
                        //         break;
                        //     }
                        // }
                        break;
                    }
                    default:;
                }
                break;
        }
    }
}
