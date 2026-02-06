#include<sfe/sfe.hpp>
#include<sgl/sgl.hpp>
#include<bbe/bbe.hpp>
#include<bbe/targets/dfg.hpp>
#include<bbe/inter/dfg.hpp>
#include<cppp/rtl.hpp>
#include<stack>
using namespace std::literals;
sgl::FreeType ftlib;
sgl::CachedFont noto_sans_19pt(){
    sgl::CachedFont gc{ftlib.load_font_from_fc(u8"Noto Sans:lang=zh-cn"s),sgl::SdfMode::DIRECT};
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
bool keydown(sfe::Toast& err,sfe::Editor& ed,const SDL_KeyboardEvent& ke){
    auto& sel = ed.selected();
    if(!ke.mod) switch(ke.key){
        case SDLK_P: {
            auto old_anode{std::exchange(sel.node(),{bbe::NodeType::CALL_BUILTIN,10,1})};
            auto old_vnode{std::exchange(sel,sel.node())};
            
            auto& pack = (old_vnode.node().children().front() = {bbe::NodeType::PACK,2});
            old_vnode.repoint(pack.children()[0uz] = std::move(old_anode));
            pack.children()[1uz] = {bbe::NodeType::NTYPE,0};
            
            sel.populate(std::move(old_vnode));
            sel.populate(1);
            
            ed.enter(old_vnode.type() != bbe::NodeType::NTYPE,false);
            break;
        }
        default:;
    }
    switch(sel.type()){
        using enum bbe::NodeType;
        case UINT32:
            if(ed.selected_after()){
                if(!(ke.mod&(SDL_KMOD_SHIFT|SDL_KMOD_CTRL|SDL_KMOD_ALT))){
                    switch(ke.key){
                        case SDLK_BACKSPACE:
                            sel.setp32(sel.prim() / 10);
                            return true;
                        case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
                        case SDLK_5: case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
                            if((static_cast<std::uint64_t>(sel.prim())*10+static_cast<std::uint64_t>(ke.key-SDLK_0))>static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())){
                                using namespace std::chrono_literals;
                                err.reset(u8"Overflow!"s,810ms);
                            }else{
                                sel.setp32(sel.prim()*10 + static_cast<std::uint32_t>(ke.key-SDLK_0));
                            }
                            return true;
                    }
                }
            }
            break;
        case NTYPE:
            switch(ke.key){
                case SDLK_D:
                    sel.node() = {bbe::NodeType::UINT32,0};
                    sel.rerender();
                    ed.set_select_after(true);
                    break;
                default:;
            }
        default:;
    }
    return false;
}
int main(){
    SDL_SetAppMetadata("edBCC (SGL)",nullptr,"edbcc.cpp");
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    sgl::Window win{u8"edBCC (SGL)"s,1200,600};
    { // scope for all GL objects. Their dtors must run before we destroy everything with SDL_Quit().
    // gldbg();
    sgl::CoordinateMap cm{1200,600};
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glEnable(GL_CULL_FACE);
    glLineWidth(3.0f);
    sgl::init_gl_for_text();
    
    sgl::CachedFont cf{noto_sans_19pt()};
    SDL_GL_SetSwapInterval(-1);
    
    bbe::ProjectEntitiesPool proj;
    auto fn = proj.function_pool().emplace(nullptr);
    sfe::Editor ed{proj.function_pool()[fn].ast() = {bbe::NodeType::NTYPE,0}};
    sfe::Toast err;
    while(true){
        for(const auto& e : sgl::events()){
            switch(e.type){
                case SDL_EVENT_QUIT: goto cleanup;
                case SDL_EVENT_WINDOW_RESIZED:
                    glViewport(0,0,e.window.data1,e.window.data2);
                    cm.update(static_cast<std::uint32_t>(e.window.data1),static_cast<std::uint32_t>(e.window.data2));
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if(!keydown(err,ed,e.key)){
                        ed.keydown(e.key);
                    }
                    break;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        ed.render_full(cppp::rtl<cppp::fvec2>({10.0f,100.0f}),cf,cm);
        if(err.alive()){
            ed.text_renderer().draw_text(err.message(),cppp::rtl<cppp::fvec2>({10.0f,10.0f+static_cast<float>(cf.font().ascender())/64.0f*0.3f}),0.3f,{1.0f,0.0f,0.0f},cf,cm);
        }
        win.flip();
    }
    }
    cleanup:
    SDL_Quit();
    return 0;
}
