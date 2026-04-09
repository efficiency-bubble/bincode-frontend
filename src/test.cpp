#include<sfe/sfe.hpp>
#include<sgl/sgl.hpp>
#include<bbe/bbe.hpp>
#include<bbe/targets/yasbepl.hpp>
#include<cppp/static-functor.hpp>
#include<bbe/formats/elf.hpp>
#include<bbe/targets/x86.hpp>
#include<bbe/inter/dfg.hpp>
#include<bbe/inter/rtl.hpp>
#include<cppp/tostring.hpp>
#include<cppp/bfile.hpp>
#include<cppp/swap.hpp>
#include<cppp/rtl.hpp>
#include<cppp/int.hpp>
#include<stdio.h>
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
void steal_lhs(sfe::VisualASTNode& ui,bbe::ASTNode&& node){
    cppp::swap(node,ui.node());
    // node is now old
    sfe::VisualASTNode old_ui{ui.node(),sfe::VisualASTNode::no_populate};
    cppp::swap(old_ui,ui);
    
    old_ui.repoint(ui.node().children()[0] = std::move(node));
    old_ui.rerender_children();
    ui.populate(std::move(old_ui));
}
void builtin_unary(sfe::VisualASTNode& sel,std::uint32_t prim,sfe::Editor& ed){
    steal_lhs(sel,{bbe::NodeType::CALL_BUILTIN,prim,1});
    ed.subst_sel(sel);
    ed.set_select_after(true);
}
void builtin_binary(sfe::VisualASTNode& sel,std::uint32_t prim,sfe::Editor& ed){
    bool second = sel.type() != bbe::NodeType::NTYPE;
    steal_lhs(sel,{bbe::NodeType::CALL_BUILTIN,prim,2});
    sel.node().children()[1uz] = {bbe::NodeType::NTYPE};
    sel.rerender_except_first();
    ed.subst_sel(sel);
    ed.enter(second,false);
}
bool keydown(sfe::Toast& toast,sfe::Editor& ed,const SDL_KeyboardEvent& ke){
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
                                if((static_cast<std::uint64_t>(sel->p32())*10+static_cast<std::uint64_t>(ke.key-SDLK_0))>static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())){
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
using hrc = std::chrono::high_resolution_clock;
using µs = std::chrono::duration<std::uint64_t,std::micro>;
template<typename F>
µs time_execution(const F& f){
    auto begin = hrc::now();
    f();
    auto dur = hrc::now()-begin;
    return std::chrono::duration_cast<µs>(dur);
}
sfe::Toast toast;
void save(sfe::Editor& ed){
    cppp::bytes save;
    static_cast<const sfe::VisualFunctionNode&>(ed.root()).func().ast().serialize(save);
    cppp::BinaryFile bf{u8"testprog"s,std::ios_base::out|std::ios_base::trunc|std::ios_base::binary};
    bf.write(save);
    toast.reset(cppp::format<u8"Saved {} bytes"_ts>(save.size()),1s);
}
void load(sfe::Editor& ed){
    cppp::BinaryFile bf{u8"testprog"s,std::ios_base::in|std::ios_base::binary};
    cppp::bytes save;
    std::array<std::byte,1024uz> buf;
    std::size_t nread;
    do{
        nread = bf.read(buf);
        save.append(std::span{buf.data(),nread});
    }while(nread);
    static_cast<sfe::VisualFunctionNode&>(ed.root()).func().ast() = bbe::ASTNode(cppp::rtl<cppp::frozen_byte_view>(save));
    ed.root().rerender_children();
    ed.home();
}
cppp::str strtype(bbe::type_id tid,const bbe::TypeDatabase& tdb){
    if(tid == tdb.T_ERROR){
        return u8"error-type"s;
    }
    cppp::str tag;
    const bbe::TypeInfo& t = tdb[tid];
    switch(t.type()){
        using enum bbe::FundamentalTypeType;
        case VOID:
            return u8"void"s;
        case UNSIGNED_INTEGRAL:
            if(t.size() == 1){
                tag = u8"bool"s; // TODO: special case bool better
            }else{
                tag = cppp::format<u8"uint{}_t"_ts>(t.size()*8);
            }
            break;
        case FUNCTION: {
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
            if(tdb[tid].type() == bbe::FundamentalTypeType::FUNCTION){
                return {0.6274509803921569f,0.9568627450980393f,0.2235294117647059f};
            }
            return {0.8588235294117647f, 0.23137254901960785f, 0.9215686274509803f};
    }
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
    bbe::ErrorDatabase edb;
    
    bbe::type_id b_uint32{proj.types().T_UINT32};
    bbe::Function& fn = proj.functions()[proj.functions().emplace(bbe::FunctionSignature{b_uint32,b_uint32})];
    fn.ast() = {bbe::NodeType::NTYPE};
    sfe::GraphicsContext gc{code_font(),{1200,600},1.0f};
    sfe::Editor ed{{fn,0}};
    
    sfe::CommandSet cmds;
    constexpr bool(*TRUE_FN)(sfe::Editor&) = [](sfe::Editor&)static{return true;};
    cmds.add(u8"exit"s,{TRUE_FN,[](sfe::Editor&)static{
        SDL_Event ev{.quit={
            .type=SDL_EVENT_QUIT,
            .reserved=0,
            .timestamp=SDL_GetTicksNS()
        }};
        SDL_PushEvent(&ev);
    }});
    cmds.add(u8"save"s,{TRUE_FN,save});
    cmds.add(u8"load"s,{TRUE_FN,load});
    sfe::CommandSelectorPanel csp{cmds,ed};
    bool csp_open = false;
    
    fn.ast().recursively_recalculate_result_type(proj,edb,fn.signature());
    while(true){
        for(const auto& e : sgl::events()){
            switch(e.type){
                case SDL_EVENT_QUIT: goto cleanup;
                case SDL_EVENT_WINDOW_RESIZED:
                    glViewport(0,0,e.window.data1,e.window.data2);
                    gc.update_window(static_cast<std::uint32_t>(e.window.data1),static_cast<std::uint32_t>(e.window.data2));
                    break;
                case SDL_EVENT_TEXT_INPUT:
                    csp.append(e.text.text);
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if(e.key.mod & SDL_KMOD_CTRL) switch(e.key.key){
                        case SDLK_S:
                            save(ed);
                            break;
                        case SDLK_O:
                            load(ed);
                            break;
                    }else if(csp_open) switch(e.key.key){
                        case SDLK_RETURN:
                            csp.exec();
                            [[fallthrough]];
                        case SDLK_ESCAPE:
                            csp_open = false;
                            csp.clear();
                            win.stop_input();
                            break;
                        case SDLK_BACKSPACE:
                            csp.backspace();
                            break;
                        case SDLK_DOWN:
                            csp.next();
                            break;
                        case SDLK_UP:
                            csp.prev();
                            break;
                    }else switch(e.key.key){
                        case SDLK_F1:
                            csp_open = true;
                            win.start_input();
                            break;
                        case SDLK_F5:
                            try{
                                cppp::str rbuf;
                                {
                                    bbe::inter::dfg::CompiledFunctionPool compiled{proj};
                                    µs delta = time_execution([&]{
                                        bbe::inter::stringify(compiled.call(0,{bbe::inter::uint32v{30}}),rbuf);
                                    });
                                    std::span<int> a;
                                    if(delta.count() < 1000){
                                        cppp::format_to<u8" in {} µs (dfg inter); "_ts>(rbuf,delta.count());
                                    }else{
                                        cppp::format_to<u8" in {:.2f} ms (dfg inter); "_ts>(rbuf,static_cast<float>(delta.count())/1000.0f);
                                    }
                                }
                                {
                                    bbe::inter::rtl::CompiledFunctionPool compiled{proj};
                                    µs delta = time_execution([&]{
                                        bbe::inter::stringify(compiled.call(0,{bbe::inter::uint32v{30}}),rbuf);
                                    });
                                    if(delta.count() < 1000){
                                        cppp::format_to<u8" in {} µs (rtl inter)"_ts>(rbuf,delta.count());
                                    }else{
                                        cppp::format_to<u8" in {:.2f} ms (rtl inter)"_ts>(rbuf,static_cast<float>(delta.count())/1000.0f);
                                    }
                                }
                                toast.reset(std::move(rbuf),3s);
                            }catch(const std::exception& e){
                                toast.reset(cppp::tou8(std::string_view(e.what())),3s);
                            }
                            break;
                        case SDLK_F6:
                            try{
                                cppp::str rbuf;
                                {
                                    bbe::formats::elf::Elf elf;
                                    bbe::targets::x86::Program prog;
                                    {
                                        bbe::targets::x86::Function fn{ed.root().func(),proj.types()};
                                        cppp::format_to<u8"{} bytes; "_ts>(rbuf,fn.instructions().size());
                                        prog.export_function(u8"example"s,std::move(fn));
                                    }
                                    elf.add_text(prog);
                                    cppp::BinaryFile outf{u8"testprog_c.o"s,std::ios_base::out|std::ios_base::binary|std::ios_base::trunc};
                                    
                                    outf.write(elf.encode());
                                }
                                if(int ret=std::system("g++ -O3 -std=c++26 -s timing_helper.cpp testprog_c.o -o testprog_c")){
                                    throw std::runtime_error(std::format("GCC failed with: {}"sv,ret));
                                }
                                if(std::unique_ptr<std::FILE,cppp::static_functor<pclose>> dl{popen(
                                    reinterpret_cast<const char*>(cppp::format<u8"./testprog_c {}"_ts>(30).c_str())
                                    ,"r")}){
                                    std::array<char8_t,1024uz> buf;
                                    while(std::size_t nr = std::fread(buf.data(),1,buf.size(),dl.get())){
                                        rbuf.append(buf.data(),nr);
                                    }
                                    if(std::ferror(dl.get())) rbuf.append(u8"<READ ERROR>"s);
                                    rbuf.append(u8" (x86_64)"s);
                                }else{
                                    throw std::runtime_error("can't start testprog"s);
                                }
                                toast.reset(std::move(rbuf),3s);
                            }catch(const std::exception& e){
                                toast.reset(cppp::tou8(std::string_view(e.what())),3s);
                            }
                            break;
                        case SDLK_F7: {
                            cppp::str rbuf;
                            if(auto* vn=dynamic_cast<sfe::VisualASTNode*>(&ed.selected())){
                                cppp::format_to<u8"{:p} = {}"_ts>(rbuf,static_cast<void*>(vn),std::to_underlying(vn->type()));
                            }
                            std::println("{}"sv,cppp::cview(rbuf));
                            break;
                        }
                        case SDLK_F8:
                            ed.home();
                            ed.root().rerender_children();
                            break;
                        default:
                            if(!keydown(toast,ed,e.key)){
                                ed.keydown(e.key);
                            }
                            edb.clear();
                            fn.ast().recursively_recalculate_result_type(proj,edb,fn.signature());
                            break;
                    }
                    break;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        ed.render_full(gc,edb,cppp::rtl<cppp::fvec2>({10.0f,10.0f+gc.line_height()*0.65f+gc.ascender()}));
        if(toast.alive()){
            gc.draw_text(toast.message(),{10.0f,10.0f+gc.ascender()*0.6f},0.6f,{1.0f,0.0f,0.0f});
        }
        if(auto van=dynamic_cast<const sfe::VisualASTNode*>(&ed.selected())){
            constexpr static float DIAG_TEXT_SCALE = 0.4f;
            float winheight_f = static_cast<float>(gc.cmap().win_size().y());
            float y = winheight_f * 0.75f + gc.ascender()*DIAG_TEXT_SCALE;
            for(const auto& em : edb.query(&van->node())){
                gc.draw_text(em.reason(),{10.0f,y},DIAG_TEXT_SCALE,{1.0f,0.0f,0.0f});
            }
            gc.draw_text(strtype(van->node().result_type(),proj.types()),{10.0f,winheight_f-8.0f+gc.descender()*DIAG_TEXT_SCALE},DIAG_TEXT_SCALE,coltype(van->node().result_type(),proj.types()));
        }
        if(csp_open){
            float winwidth_f = static_cast<float>(gc.cmap().win_size().x());
            csp.render(gc,{winwidth_f/2.0f,10.0f},winwidth_f*0.6f,0.5f);
        }
        win.flip();
    }
    }
    cleanup:
    SDL_Quit();
    return 0;
}
