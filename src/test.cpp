#include<sfe/toast.hpp>
#include<sfe/editor.hpp>
#include<sfe/style.hpp>
#include<sfe/builtin-commands.hpp>
#include<cppp/format.hpp>
#include<cppp/swap.hpp>
#include<cppp/int.hpp>
#include<sgl/sgl.hpp>
#include<bbe/bbe.hpp>
#include<filesystem>
#include<cstring>
using namespace std::literals;
using namespace cppp::literals;
sgl::FreeType ftlib;
sgl::CachedFont code_font(){
    sgl::CachedFont gc{ftlib.load_font_from_fc(u8"Consolas"s),sgl::SdfMode::DIRECT};
    gc.font().init_width_pt(15<<6uz,191,191);
    return gc;
}
bool keydown(sfe::Toast& toast,sfe::CodeEntry& ed,const sfe::NodeKeyConfig& kc,sfe::Keypress ke){
    if(kc.handle(ed,ke)) return true;
    switch(ed.selected().type()){
        case sfe::VisualNodeType::A: {
            bbe::ASTNode& a = ed.selected().a();
            switch(a.type()){
                using enum bbe::NodeType;
                case BOOL:
                    if(ke.key() == SDLK_RETURN){
                        a.setp32(1-a.getp32());
                    }
                    break;
                case UINT32: case FNSYM: case COMMA: case PACKIND:
                    if(ed.selected_after()){
                        if(!(ke.mods()&(sfe::KeyModifiers::CTRL|sfe::KeyModifiers::SHIFT|sfe::KeyModifiers::ALT))){
                            switch(ke.key()){
                                case SDLK_BACKSPACE:
                                    if(a.getp32()){
                                        a.setp32(a.getp32() / 10);
                                        return true;
                                    }
                                    break;
                                case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
                                case SDLK_5: case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9:
                                    if((static_cast<std::uint64_t>(a.getp32())*10+static_cast<std::uint64_t>(ke.key()-SDLK_0))>std::numeric_limits<std::uint32_t>::max()){
                                        using namespace std::chrono_literals;
                                        toast.reset(u8"Overflow!"s,810ms);
                                    }else{
                                        a.setp32(a.getp32()*10 + static_cast<std::uint32_t>(ke.key()-SDLK_0));
                                    }
                                    return true;
                            }
                        }
                    }
                    break;
                default:;
            }
        }
        case sfe::VisualNodeType::F:;
    }
    return false;
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
    if(std::filesystem::last_write_time(u8"timing_helper.cpp"sv) > std::filesystem::last_write_time(u8"timing_helper.o"sv)){
        if(int ret=std::system("g++ -O3 -m64 -std=c++26 -s -c timing_helper.cpp -o timing_helper.o")){
            throw std::runtime_error(std::format("precompiling timing_helper failed with: {}"sv,ret));
        }
    }
    sfe::Project proj;
    bbe::ErrorDatabase edb;
    sfe::NodeKeyConfig kc;
    kc.register_key({sfe::KeyModifiers::SHIFT,SDLK_EQUALS},{bbe::NodeType::CALL_BUILTIN,10,2});
    kc.register_key(SDLK_MINUS,{bbe::NodeType::CALL_BUILTIN,20,2});
    kc.register_key({sfe::KeyModifiers::SHIFT,SDLK_8},{bbe::NodeType::CALL_BUILTIN,30,2});
    kc.register_key(SDLK_EQUALS,{bbe::NodeType::CALL_BUILTIN,50,2});
    kc.register_key({sfe::KeyModifiers::SHIFT,SDLK_9},{bbe::NodeType::CALL_BUILTIN,0,2});
    kc.register_key({sfe::KeyModifiers::SHIFT,SDLK_COMMA},{bbe::NodeType::CALL_BUILTIN,51,2});
    kc.register_key(SDLK_COMMA,{bbe::NodeType::COMMA,0,2});
    kc.register_key(SDLK_LEFTBRACKET,{bbe::NodeType::PACKIND,0,1});
    kc.register_key({sfe::KeyModifiers::SHIFT,SDLK_SLASH},{bbe::NodeType::FORK,0,3});
    
    kc.register_node(SDLK_A,{bbe::NodeType::ARG,0,0});
    kc.register_node(SDLK_E,{bbe::NodeType::BOOL,0,0});
    kc.register_node(SDLK_D,{bbe::NodeType::UINT32,0,0});
    kc.register_node(SDLK_F,{bbe::NodeType::FNSYM,0,0});
    kc.register_node({sfe::KeyModifiers::SHIFT,SDLK_8},{bbe::NodeType::PACK,0,1});
    
    const bbe::TypeInfo& b_uint32{proj.entities().types()[bbe::TypeDatabase::T_UINT32]};
    bbe::Function& fn = proj.entities().functions().emplace(bbe::FunctionSignature{&b_uint32,&b_uint32});
    proj.names().name_function(fn.index(),u8"testfn"s);
    fn.ast() = {bbe::NodeType::NTYPE};
    
    
    SDL_SetHint(SDL_HINT_INVALID_PARAM_CHECKS,"1");
    SDL_SetAppMetadata("edBCC (SGL)",nullptr,"edbcc.cpp");
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    sfe::Window ed{proj,{u8"edBCC (SGL)"s,1200,600},fn,{code_font(),{1200,600},1.0f}};
    { // scope for all GL objects. Their dtors must run before we destroy everything with SDL_Quit().
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glEnable(GL_CULL_FACE);
    glLineWidth(3.0f);
    sgl::init_gl_for_text();
    
    if(!SDL_GL_SetSwapInterval(-1)) SDL_GL_SetSwapInterval(1);
    
    ed.add_command(u8"open command palette"s,SDLK_F1,sfe::commands::open_command_palette);
    ed.add_command(u8"rename selection"s,SDLK_F2,sfe::commands::rename_selection);
    ed.add_command(u8"save"s,{sfe::KeyModifiers::CTRL,SDLK_S},sfe::commands::save);
    ed.add_command(u8"load"s,{sfe::KeyModifiers::CTRL,SDLK_O},sfe::commands::load);
    ed.add_command(u8"reset cursor"s,SDLK_F8,sfe::commands::reset_cursor);
    ed.add_command(u8"exit"s,sfe::commands::quit);
    ed.add_command(u8"debug selection"s,SDLK_F7,sfe::commands::debug_selection);
    ed.add_command(u8"compile code for x86"s,SDLK_F6,{sfe::commands::compile_and_run,&edb});
    ed.add_command(u8"interpret code"s,SDLK_F5,{sfe::commands::interpret,&edb});
    fn.ast().recursively_recalculate_result_type(proj.entities(),edb,fn.signature());
    while(true){
        for(const auto& e : sgl::events()){
            switch(e.type){
                case SDL_EVENT_QUIT: goto cleanup;
                case SDL_EVENT_WINDOW_RESIZED:
                    glViewport(0,0,e.window.data1,e.window.data2);
                    ed.graphics_context().update_window(static_cast<std::uint32_t>(e.window.data1),static_cast<std::uint32_t>(e.window.data2));
                    break;
                case SDL_EVENT_TEXT_EDITING:
                    ed.preedit_set(static_cast<std::uint32_t>(e.edit.start),static_cast<std::uint32_t>(e.edit.length),e.edit.text);
                    break;
                case SDL_EVENT_TEXT_INPUT: {
                    std::size_t len = std::strlen(e.text.text);
                    ed.textinput({std::start_lifetime_as_array<char8_t>(e.text.text,len),len});
                    break;
                }
                case SDL_EVENT_KEY_DOWN:
                    sfe::Keypress kp{e.key};
                    if(ed.is_command_palette_open() || !keydown(ed.toast(),ed.code(),kc,kp)){
                        ed.keydown(kp);
                    }
                    edb.clear();
                    fn.ast().recursively_recalculate_result_type(proj.entities(),edb,fn.signature());
                    break;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        ed.render(edb,proj.names());
        if(ed.toast().alive()){
            ed.graphics_context().draw_text(ed.toast().message(),{10.0f,10.0f+ed.graphics_context().ascender()*0.6f},0.6f,{1.0f,0.0f,0.0f});
        }
        if(ed.code().selected().type() == sfe::VisualNodeType::A){
            bbe::ASTNode& an = ed.code().selected().a();
            constexpr static float DIAG_TEXT_SCALE = 0.65f;
            float winheight_f = static_cast<float>(ed.graphics_context().cmap().win_size().y());
            float y = winheight_f * 0.75f + ed.graphics_context().ascender()*DIAG_TEXT_SCALE;
            for(const auto& em : edb.query(&an)){
                ed.graphics_context().draw_text(em.reason(),{10.0f,y},DIAG_TEXT_SCALE,{1.0f,0.0f,0.0f});
            }
            ed.graphics_context().draw_text(proj.names().display_type_name(proj.entities().types().getopt(an.result_type())),{10.0f,winheight_f-8.0f+ed.graphics_context().descender()*DIAG_TEXT_SCALE},DIAG_TEXT_SCALE,coltype(an.result_type(),proj.entities().types()));
        }
        ed.render_overlay();
        ed.system_window().flip();
    }
    }
    cleanup:
    SDL_Quit();
    return 0;
}
