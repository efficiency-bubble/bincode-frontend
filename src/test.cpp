#include<sfe/sfe.hpp>
#include<sfe/builtin-commands.hpp>
#include<cppp/tostring.hpp>
#include<cppp/swap.hpp>
#include<cppp/int.hpp>
#include<sgl/sgl.hpp>
#include<bbe/bbe.hpp>
using namespace std::literals;
using namespace cppp::literals;
sgl::FreeType ftlib;
sgl::CachedFont code_font(){
    sgl::CachedFont gc{ftlib.load_font_from_fc(u8"Consolas"s),sgl::SdfMode::DIRECT};
    gc.font().init_width_pt(19<<6uz,191,191);
    return gc;
}
void steal_lhs(sfe::VisualASTNode& ui,bbe::ASTNode&& node){
    cppp::swap(node,ui.node());
    // node is now old
    sfe::VisualASTNode old_ui{ui.node(),sfe::VisualASTNode::no_populate};
    cppp::swap(old_ui,ui);
    
    old_ui.repoint(ui.node().children()[0] = std::move(node));
    old_ui.rerender_children();
    ui.populate(std::move(old_ui));
}
void builtin_unary(sfe::VisualASTNode& sel,std::uint32_t prim,sfe::CodeEntry& ed){
    steal_lhs(sel,{bbe::NodeType::CALL_BUILTIN,prim,1});
    ed.subst_sel(sel);
    ed.set_select_after(true);
}
void builtin_binary(sfe::VisualASTNode& sel,std::uint32_t prim,sfe::CodeEntry& ed){
    bool second = sel.type() != bbe::NodeType::NTYPE;
    steal_lhs(sel,{bbe::NodeType::CALL_BUILTIN,prim,2});
    sel.node().children()[1uz] = {bbe::NodeType::NTYPE};
    sel.rerender_except_first();
    ed.subst_sel(sel);
    ed.enter(second,false);
}
bool keydown(sfe::Toast& toast,sfe::CodeEntry& ed,const SDL_KeyboardEvent& ke){
    bool shift = (ke.mod & SDL_KMOD_SHIFT);
    if(auto sel=dynamic_cast<sfe::VisualASTNode*>(&ed.selected())){
        switch(ke.key){
            case SDLK_EQUALS:
                builtin_binary(*sel,shift?10_u32:50_u32,ed);
                return true;
            case SDLK_MINUS: if(!shift){
                builtin_binary(*sel,11,ed);
                return true;
            }else break;
            case SDLK_COMMA: if(shift){
                builtin_binary(*sel,51,ed);
                return true;
            }else if(sel->type() == bbe::NodeType::PACK && ed.selected_after()){
                sel->node().children().emplace({bbe::NodeType::NTYPE});
                sel->rerender_children();
                return true;
            }else break;
            case SDLK_1: if(shift && !ed.selected_after()){
                builtin_unary(*sel,60,ed);
                return true;
            }else break;
            case SDLK_3: if(shift){
                builtin_unary(*sel,25,ed);
                return true;
            }else break;
            case SDLK_9: if(shift){
                builtin_binary(*sel,0,ed);
                return true;
            }else break;
            case SDLK_SLASH: if(shift){
                bool second = sel->type() != bbe::NodeType::NTYPE;
                steal_lhs(*sel,{bbe::NodeType::FORK,3});
                sel->node().children()[1uz] = {bbe::NodeType::NTYPE};
                sel->node().children()[2uz] = {bbe::NodeType::NTYPE};
                sel->rerender_except_first();
                ed.enter(second,false);
                return true;
            }else break;
            case SDLK_LEFTBRACKET:
                steal_lhs(*sel,{bbe::NodeType::PACKIND,0,1});
                ed.set_select_after(true);
                break;
            default:;
        }
        switch(sel->type()){
            using enum bbe::NodeType;
            case BOOL:
                if(ke.key == SDLK_RETURN){
                    sel->setp32(1-sel->p32());
                }
                break;
            case UINT32: case FNSYM: case PACKIND:
                if(ed.selected_after()){
                    if(!(ke.mod&(SDL_KMOD_SHIFT|SDL_KMOD_CTRL|SDL_KMOD_ALT))){
                        switch(ke.key){
                            case SDLK_BACKSPACE:
                                if(sel->p32()){
                                    sel->setp32(sel->p32() / 10);
                                    return true;
                                }
                                break;
                            case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
                            case SDLK_5: case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
                                if((static_cast<std::uint64_t>(sel->p32())*10+static_cast<std::uint64_t>(ke.key-SDLK_0))>std::numeric_limits<std::uint32_t>::max()){
                                    using namespace std::chrono_literals;
                                    toast.reset(u8"Overflow!"s,810ms);
                                }else{
                                    sel->setp32(sel->p32()*10 + static_cast<std::uint32_t>(ke.key-SDLK_0));
                                }
                                return true;
                        }
                    }
                }
                break;
            case NTYPE:
                switch(ke.key){
                    case SDLK_A:
                        sel->node() = {bbe::NodeType::ARG};
                        sel->rerender_children();
                        ed.set_select_after(true);
                        break;
                    case SDLK_E:
                        sel->node() = {bbe::NodeType::BOOL};
                        sel->rerender_children();
                        ed.set_select_after(true);
                        break;
                    case SDLK_D:
                        sel->node() = {bbe::NodeType::UINT32};
                        sel->rerender_children();
                        ed.set_select_after(true);
                        break;
                    case SDLK_F:
                        sel->node() = {bbe::NodeType::FNSYM};
                        sel->rerender_children();
                        ed.set_select_after(true);
                        break;
                    case SDLK_X:
                        sel->node() = {bbe::NodeType::PACKIND,0,1};
                        sel->node().children().front() = {bbe::NodeType::ARG};
                        sel->rerender_children();
                        ed.set_select_after(true);
                        break;
                    case SDLK_8: if(shift){
                        sel->node() = {bbe::NodeType::PACK,0,1};
                        sel->node().children().front() = {bbe::NodeType::NTYPE};
                        sel->rerender_children();
                        ed.enter(0,false);
                        break;
                    }else break;
                    default:;
                }
            default:;
        }
    }
    return false;
}
cppp::str strtype(bbe::type_id tid,const bbe::TypeDatabase& tdb){
    if(tid == tdb.T_ERROR){
        return u8"error-type"s;
    }
    cppp::str tag;
    const bbe::TypeInfo& t = tdb[tid];
    switch(t.type()){
        using enum bbe::TypeCategory;
        case VOID:
            return u8"void"s;
        case UNSIGNED_INTEGRAL:
            if(t.size() == 1){
                tag = u8"bool"s; // TODO: special case bool better
            }else{
                tag = cppp::format<u8"uint{}_t"_ts>(t.size()*8);
            }
            break;
        case FUNCTION_POINTER: {
            const bbe::FunctionSignature& sig = t.function_signature();
            tag = cppp::format<u8"{} => {}"_ts>(strtype(sig.parameter(),tdb),strtype(sig.return_type(),tdb));
            break;
        }
        case PACK:
            tag = u8"pack["s;
            for(const auto& c : t.pack_contents().types()){
                tag.append(strtype(c,tdb));
                tag.append(u8", "sv);
            }
            tag.pop_back();
            tag.back() = u8']';
            break;
        default:
            tag = u8"unknown"s;
            break;
    }
    return cppp::format<u8"{} ({}/{}:{})"_ts>(tag,tdb[tid].size(),tdb[tid].alignment(),tdb[tid].stride());
}
cppp::fvec3 coltype(bbe::type_id tid,const bbe::TypeDatabase& tdb){
    switch(tid){
        case bbe::TypeDatabase::T_VOID:
            return {0.7f};
        case bbe::TypeDatabase::T_UINT32:
            return {0.23137254901960785f,0.8509803921568627f,0.9215686274509803f};
        case bbe::TypeDatabase::T_UINT64:
            return {0.9215686274509803f,0.7647058823529411f,0.23137254901960785f};
        case bbe::TypeDatabase::T_BOOL:
            return {0.23529411764705882f,0.9176470588235294f,0.47058823529411764f};
        case bbe::TypeDatabase::T_ERROR:
            return {1.0f,0.0f,0.0f};
        default:
            if(tdb[tid].type() == bbe::TypeCategory::FUNCTION_POINTER){
                return {0.6274509803921569f,0.9568627450980393f,0.2235294117647059f};
            }
            return {0.8588235294117647f, 0.23137254901960785f, 0.9215686274509803f};
    }
}
int main(){
    bbe::ProjectEntitiesPool proj;
    bbe::ErrorDatabase edb;
    
    bbe::type_id b_uint32{proj.types().T_UINT32};
    bbe::Function& fn = proj.functions()[proj.functions().emplace(bbe::FunctionSignature{b_uint32,b_uint32})];
    fn.ast() = {bbe::NodeType::NTYPE};
    
    SDL_SetHint(SDL_HINT_INVALID_PARAM_CHECKS,"1");
    SDL_SetAppMetadata("edBCC (SGL)",nullptr,"edbcc.cpp");
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    sfe::Window ed{proj,{u8"edBCC (SGL)"s,1200,600},{fn,0},{code_font(),{1200,600},1.0f}};
    { // scope for all GL objects. Their dtors must run before we destroy everything with SDL_Quit().
    // gldbg();
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glEnable(GL_CULL_FACE);
    glLineWidth(3.0f);
    sgl::init_gl_for_text();
    
    if(!SDL_GL_SetSwapInterval(-1)){SDL_GL_SetSwapInterval(1);}
    
    ed.add_command(u8"open command palette"s,SDLK_F1,sfe::commands::open_command_palette);
    ed.add_command(u8"save"s,{sfe::KeyModifiers::CTRL,SDLK_S},sfe::commands::save);
    ed.add_command(u8"load"s,{sfe::KeyModifiers::CTRL,SDLK_O},sfe::commands::load);
    ed.add_command(u8"reset cursor"s,SDLK_F8,sfe::commands::reset_cursor);
    ed.add_command(u8"exit"s,sfe::commands::quit);
    ed.add_command(u8"debug selection"s,SDLK_F7,sfe::commands::debug_selection);
    ed.add_command(u8"compile code for x86"s,SDLK_F6,sfe::commands::compile_and_run);
    ed.add_command(u8"interpret code"s,SDLK_F5,sfe::commands::interpret);
    fn.ast().recursively_recalculate_result_type(proj,edb,fn.signature());
    while(true){
        for(const auto& e : sgl::events()){
            switch(e.type){
                case SDL_EVENT_QUIT: goto cleanup;
                case SDL_EVENT_WINDOW_RESIZED:
                    glViewport(0,0,e.window.data1,e.window.data2);
                    ed.graphics_context().update_window(static_cast<std::uint32_t>(e.window.data1),static_cast<std::uint32_t>(e.window.data2));
                    break;
                case SDL_EVENT_TEXT_INPUT:
                    CPPP_ASSERT(ed.is_command_palette_open());
                    ed.command_palette().append(e.text.text);
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if(ed.is_command_palette_open() || !keydown(ed.toast(),ed.code(),e.key)){
                        ed.keydown(e.key);
                    }
                    edb.clear();
                    fn.ast().recursively_recalculate_result_type(proj,edb,fn.signature());
                    break;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        ed.render(edb);
        if(ed.toast().alive()){
            ed.graphics_context().draw_text(ed.toast().message(),{10.0f,10.0f+ed.graphics_context().ascender()*0.6f},0.6f,{1.0f,0.0f,0.0f});
        }
        if(auto van=dynamic_cast<const sfe::VisualASTNode*>(&ed.code().selected())){
            constexpr static float DIAG_TEXT_SCALE = 0.4f;
            float winheight_f = static_cast<float>(ed.graphics_context().cmap().win_size().y());
            float y = winheight_f * 0.75f + ed.graphics_context().ascender()*DIAG_TEXT_SCALE;
            for(const auto& em : edb.query(&van->node())){
                ed.graphics_context().draw_text(em.reason(),{10.0f,y},DIAG_TEXT_SCALE,{1.0f,0.0f,0.0f});
            }
            ed.graphics_context().draw_text(strtype(van->node().result_type(),proj.types()),{10.0f,winheight_f-8.0f+ed.graphics_context().descender()*DIAG_TEXT_SCALE},DIAG_TEXT_SCALE,coltype(van->node().result_type(),proj.types()));
        }
        ed.render_overlay();
        ed.system_window().flip();
    }
    }
    cleanup:
    SDL_Quit();
    return 0;
}
