#include<sfe/sfe.hpp>
#include<sgl/sgl.hpp>
#include<bbe/bbe.hpp>
#include<bbe/targets/dfg.hpp>
#include<bbe/inter/dfg.hpp>
#include<cppp/tostring.hpp>
#include<cppp/bfile.hpp>
#include<cppp/rtl.hpp>
#include<cppp/int.hpp>
#include<chrono>
#include<print>
using namespace std::literals;
using namespace cppp::literals;
sgl::FreeType ftlib;
sgl::CachedFont code_font(){
    sgl::CachedFont gc{ftlib.load_font_from_fc(u8"Consolas"s),sgl::SdfMode::DIRECT};
    gc.font().init_width_pt(19<<6uz,191,191);
    return gc;
}
bbe::ASTNode u32(std::uint32_t val){
    return {bbe::NodeType::UINT32,val,0};
}
template<typename ...T>
bbe::ASTNode pack(T&& ...children){
    bbe::ASTNode x{bbe::NodeType::PACK,sizeof...(T)};
    [&]<std::size_t ...i>(std::index_sequence<i...>){
        (... , x.emplace(i,std::forward<T>(children)));
    }(std::index_sequence_for<T...>());
    return x;
}
bbe::ASTNode cmag(std::uint32_t magic,bbe::ASTNode&& arg){
    bbe::ASTNode x{bbe::NodeType::CALL_BUILTIN,magic,1};
    x.emplace(0,std::move(arg));
    return x;
}
template<std::size_t pack>
void steal_lhs(sfe::VisualASTNode& outer_ui,bbe::ASTNode&& new_node){
    bbe::ASTNode& outer = outer_ui.node();
    bbe::ASTNode old_node{std::exchange(outer,std::move(new_node))};

    sfe::VisualASTNode old_ui{std::exchange(outer_ui,outer)};
    
    bbe::ASTNode* inner;
    if constexpr(pack){
        inner = &(outer.children()[0uz] = {bbe::NodeType::PACK,pack});
    }else{
        inner = &outer;
    }
    old_ui.repoint(inner->children()[0uz] = std::move(old_node));
    outer_ui.clear();
    outer_ui.populate(std::move(old_ui));
}
bool keydown(sfe::Toast& err,sfe::Editor& ed,const SDL_KeyboardEvent& ke){
    if(auto sel=dynamic_cast<sfe::VisualASTNode*>(&ed.selected())){
        switch(ke.key){
            case SDLK_EQUALS: {
                bool second = sel->type() != bbe::NodeType::NTYPE;
                steal_lhs<2uz>(*sel,{bbe::NodeType::CALL_BUILTIN,(ke.mod & SDL_KMOD_SHIFT)?10_u32:50_u32,1});
                sel->node().children().front().children()[1uz] = {bbe::NodeType::NTYPE,0};
                sel->populate<true>(1);
                ed.enter(second,false);
                return true;
            }
            case SDLK_MINUS: if(!(ke.mod & SDL_KMOD_SHIFT)){
                bool second = sel->type() != bbe::NodeType::NTYPE;
                steal_lhs<2uz>(*sel,{bbe::NodeType::CALL_BUILTIN,11,1});
                sel->node().children().front().children()[1uz] = {bbe::NodeType::NTYPE,0};
                sel->populate<true>(1);
                ed.enter(second,false);
                return true;
            }else break;
            case SDLK_COMMA: if(ke.mod & SDL_KMOD_SHIFT){
                bool second = sel->type() != bbe::NodeType::NTYPE;
                steal_lhs<2uz>(*sel,{bbe::NodeType::CALL_BUILTIN,51,1});
                sel->node().children().front().children()[1uz] = {bbe::NodeType::NTYPE,0};
                sel->populate<true>(1);
                ed.enter(second,false);
                return true;
            }else break;
            case SDLK_9: if(ke.mod & SDL_KMOD_SHIFT){
                bool second = sel->type() != bbe::NodeType::NTYPE;
                steal_lhs<2uz>(*sel,{bbe::NodeType::CALL_BUILTIN,0,1});
                sel->node().children().front().children()[1uz] = {bbe::NodeType::NTYPE,0};
                sel->populate<true>(1);
                ed.enter(second,false);
                return true;
            }else break;
            case SDLK_LEFTBRACKET: if(!(ke.mod & SDL_KMOD_SHIFT)){
                bool second = sel->type() != bbe::NodeType::NTYPE;
                steal_lhs<2uz>(*sel,{bbe::NodeType::CALL_BUILTIN,80,1});
                sel->node().children().front().children()[1uz] = {bbe::NodeType::NTYPE,0};
                sel->populate<true>(1);
                ed.enter(second,false);
                return true;
            }else break;
            case SDLK_SLASH: if(ke.mod & SDL_KMOD_SHIFT){
                bool second = sel->type() != bbe::NodeType::NTYPE;
                steal_lhs<0uz>(*sel,{bbe::NodeType::FORK,3});
                sel->node().children()[1uz] = {bbe::NodeType::NTYPE,0};
                sel->node().children()[2uz] = {bbe::NodeType::NTYPE,0};
                sel->populate<false>(1);
                sel->populate<false>(2);
                ed.enter(second,false);
                return true;
            }else break;
            default:;
        }
        switch(sel->type()){
            using enum bbe::NodeType;
            case BOOL:
                if(ke.key == SDLK_RETURN){
                    sel->setp32(1-sel->p32());
                }
                break;
            case UINT32:
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
                                if((static_cast<std::uint64_t>(sel->p32())*10+static_cast<std::uint64_t>(ke.key-SDLK_0))>static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())){
                                    using namespace std::chrono_literals;
                                    err.reset(u8"Overflow!"s,810ms);
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
                        sel->node() = {bbe::NodeType::ARGV,0};
                        sel->rerender();
                        ed.set_select_after(true);
                        break;
                    case SDLK_E:
                        sel->node() = {bbe::NodeType::BOOL,0};
                        sel->rerender();
                        ed.set_select_after(true);
                        break;
                    case SDLK_D:
                        sel->node() = {bbe::NodeType::UINT32,0};
                        sel->rerender();
                        ed.set_select_after(true);
                        break;
                    case SDLK_F:
                        sel->node() = {bbe::NodeType::FNSYM,0};
                        sel->rerender();
                        ed.set_select_after(true);
                        break;
                    default:;
                }
            default:;
        }
    }
    return false;
}
void vdebug(const sfe::VisualASTNode& vn,cppp::str& dst){
    dst.append(cppp::format<u8"{}"_ts>(std::to_underlying(vn.type())));
}
int main(){
    SDL_SetAppMetadata("edBCC (SGL)",nullptr,"edbcc.cpp");
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    sgl::Window win{u8"edBCC (SGL)"s,1200,600};
    { // scope for all GL objects. Their dtors must run before we destroy everything with SDL_Quit().
    // gldbg();
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glEnable(GL_CULL_FACE);
    glLineWidth(3.0f);
    sgl::init_gl_for_text();
    
    SDL_GL_SetSwapInterval(-1);
    
    bbe::ProjectEntitiesPool proj;
    bbe::Function& fn = proj.function_pool()[proj.function_pool().emplace(nullptr)];
    fn.ast() = {bbe::NodeType::NTYPE,0};
    sfe::Editor ed{{fn,0},sfe::GraphicsContext{code_font(),{1200,600},1.0f}};
    sfe::Toast err;
    while(true){
        for(const auto& e : sgl::events()){
            switch(e.type){
                case SDL_EVENT_QUIT: goto cleanup;
                case SDL_EVENT_WINDOW_RESIZED:
                    glViewport(0,0,e.window.data1,e.window.data2);
                    ed.update_window(static_cast<std::uint32_t>(e.window.data1),static_cast<std::uint32_t>(e.window.data2));
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if(e.key.mod & SDL_KMOD_CTRL){
                        switch(e.key.key){
                            case SDLK_S: {
                                cppp::bytes save;
                                fn.ast().serialize(save);
                                cppp::BinaryFile bf{u8"testprog"s,std::ios_base::out|std::ios_base::trunc|std::ios_base::binary};
                                bf.write(save);
                                err.reset(cppp::format<u8"Saved {} bytes"_ts>(save.size()),1s);
                                break;
                            }
                            case SDLK_O: {
                                cppp::BinaryFile bf{u8"testprog"s,std::ios_base::in|std::ios_base::binary};
                                cppp::bytes save;
                                std::array<std::byte,1024uz> buf;
                                std::size_t nread;
                                do{
                                    nread = bf.read(buf);
                                    save.append(std::span{buf.data(),nread});
                                }while(nread);
                                fn.ast() = bbe::ASTNode(cppp::rtl<cppp::frozen_byte_view>(save));
                                ed.root().rerender();
                            }
                        }
                    }else{
                        if(e.key.key == SDLK_F5){
                            try{
                                using hrc = std::chrono::high_resolution_clock;
                                using µs = std::chrono::duration<std::uint64_t,std::micro>;
                                bbe::inter::dfg::CompiledFunctionPool compiled{proj};
                                cppp::str rbuf;
                                auto begin = hrc::now();
                                bbe::inter::stringify(compiled.call(0,{bbe::inter::uint32v{20}}),rbuf);
                                µs delta = std::chrono::duration_cast<µs>(hrc::now()-begin);
                                if(delta.count() < 1000){
                                    rbuf.append(cppp::format<u8" in {} µs"_ts>(delta.count()));
                                }else{
                                    rbuf.append(cppp::format<u8" in {:.2f} µs"_ts>(static_cast<float>(delta.count())/1000.0f));
                                }
                                err.reset(std::move(rbuf),3s);
                            }catch(const std::exception& e){
                                err.reset(cppp::tou8(std::string_view(e.what())),3s);
                            }
                        }else if(e.key.key == SDLK_F7){
                            cppp::str rbuf;
                            vdebug(ed.root().child(),rbuf);
                            std::println("{}"sv,cppp::cview(rbuf));
                        }else if(!keydown(err,ed,e.key)){
                            ed.keydown(e.key);
                        }
                    }
                    break;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        ed.render_full(cppp::rtl<cppp::fvec2>({10.0f,10.0f+ed.graphics_context().line_height()*0.65f+ed.graphics_context().ascender()}));
        if(err.alive()){
            ed.graphics_context().draw_text(err.message(),cppp::rtl<cppp::fvec2>({10.0f,10.0f+ed.graphics_context().ascender()*0.6f}),0.6f,{1.0f,0.0f,0.0f});
        }
        win.flip();
    }
    }
    cleanup:
    SDL_Quit();
    return 0;
}
