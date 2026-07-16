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
cppp::fvec3 new_chroma(){
    constexpr static std::array chromas{
        cppp::fvec3{0.11764705882352941f,0.6470588235294118f,0.8980392156862745f},
        cppp::fvec3{0.5137254901960784f,0.9019607843137255f,0.11764705882352941f},
        cppp::fvec3{0.9254901960784314f,0.2901960784313726f,0.7529411764705882f},
        cppp::fvec3{0.9294117647058824f,0.47843137254901963f,0.2901960784313726f}
    };
    static std::size_t i = 0uz;
    cppp::fvec3 ret = chromas[i++];
    if(i == chromas.size()) i = 0uz;
    return ret;
}
bool keydown(sfe::Toast& toast,sfe::Project& proj,sfe::CodeEntry& ed,const sfe::NodeKeyConfig& kc,sfe::Keypress ke){
    if(kc.handle(ed,ke)) return true;
    switch(ed.cursor().selected().type()){
        case sfe::VisualNodeType::A: {
            bbe::ASTNode& a = ed.cursor().selected().a();
            switch(a.type()){
                using enum bbe::NodeType;
                case BOOL:
                    if(ke.key() == SDLK_RETURN){
                        a.setp32(1-a.getp32());
                        return true;
                    }
                    break;
                case UINT32: case FNSYM: case COMMA: case PACKIND: case GETVAR: case HAVEVAR:
                    if(ed.cursor().is_after()){
                        if(!(ke.mods()&(sfe::KeyModifiers::CTRL|sfe::KeyModifiers::SHIFT|sfe::KeyModifiers::ALT))){
                            switch(ke.key()){
                                case SDLK_BACKSPACE:
                                    if(a.getp32()){
                                        a.setp32(a.getp32() / 10);
                                        return true;
                                    }
                                    break;
                                case SDLK_0: case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4:
                                case SDLK_5: case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9: {
                                    std::uint64_t new_num = static_cast<std::uint64_t>(a.getp32())*10+static_cast<std::uint64_t>(ke.key()-SDLK_0);
                                    if(std::in_range<std::uint32_t>(new_num)){
                                        a.setp32(static_cast<std::uint32_t>(new_num));
                                    }else{
                                        using namespace std::chrono_literals;
                                        toast.reset(u8"Overflow!"s,810ms);
                                    }
                                    return true;
                                }
                            }
                        }
                    }
                    break;
                default:;
            }
            if(ed.cursor().selected2().type() == sfe::VisualNodeType::A && ed.cursor().selected2().a().type() == bbe::NodeType::COMMA){
                if(!(ke.mods()&(sfe::KeyModifiers::CTRL|sfe::KeyModifiers::SHIFT|sfe::KeyModifiers::ALT))&&ke.key()==SDLK_RETURN){
                    std::uint32_t indx = ed.cursor().index_of_selection();
                    bool aft = ed.cursor().is_after();
                    ed.cursor().selected2().a().children().insert(indx+aft,bbe::NodeType::NTYPE);
                    ed.cursor().leave();
                    ed.cursor().selected().arerender();
                    ed.cursor().enter(indx+1uz,aft);
                }
            }
            break;
        }
        case sfe::VisualNodeType::F: {
            if(!(ke.mods()&(sfe::KeyModifiers::CTRL|sfe::KeyModifiers::SHIFT|sfe::KeyModifiers::ALT))){
                switch(ke.key()){
                    case SDLK_BACKSPACE: {
                        bbe::Function& f = ed.cursor().selected().f();
                        proj.entities().functions().erase(f.index());
                        ed.cursor().leave();
                        ed.cursor().selected().perasef(f);
                        ed.cursor().selected().prerender();
                        break;
                    }
                    case SDLK_RETURN: {
                        if(bbe::type_id tid=ed.cursor().selected().f().ast().result_type();tid != bbe::TypeDatabase::T_ERROR){
                            ed.cursor().selected().f().signature().set_return(&proj.entities().types()[tid]);
                        }
                        break;
                    }
                }
            }
            break;
        }
        case sfe::VisualNodeType::P: {
            if(!(ke.mods()&(sfe::KeyModifiers::CTRL|sfe::KeyModifiers::SHIFT|sfe::KeyModifiers::ALT))){
                switch(ke.key()){
                    case SDLK_RETURN: {
                        const bbe::TypeInfo& b_uint32{proj.entities().types()[bbe::TypeDatabase::T_UINT32]};
                        bbe::Function& fn = proj.entities().functions().emplace(bbe::FunctionSignature{&b_uint32,&b_uint32});
                        fn.set_cname(cppp::format<u8"fn{}"_ts>(fn.index()));
                        proj.names().name_function(fn.index(),{u8"_unnamed"s,new_chroma()});
                        ed.root().paddf(fn);
                        ed.root().prerender();
                        break;
                    }
                }
            }
            break;
        }
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
    if(!std::filesystem::exists(u8"timing_helper.o"sv) || std::filesystem::last_write_time(u8"timing_helper.cpp"sv) > std::filesystem::last_write_time(u8"timing_helper.o"sv)){
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
    kc.register_node(SDLK_S,{bbe::NodeType::SINT32,0,0});
    kc.register_node(SDLK_F,{bbe::NodeType::FNSYM,0,0});
    kc.register_node(SDLK_V,{bbe::NodeType::GETVAR,0,0});
    kc.register_node(SDLK_L,{bbe::NodeType::HAVEVAR,0,2});
    kc.register_node(SDLK_Q,{bbe::NodeType::IMPORT_STUB,0,0});
    kc.register_node({sfe::KeyModifiers::SHIFT,SDLK_8},{bbe::NodeType::PACK,0,1});
    
    const bbe::TypeInfo& b_uint32{proj.entities().types()[bbe::TypeDatabase::T_UINT32]};
    bbe::Function& fn = proj.entities().functions().emplace(u8"example"s,bbe::FunctionSignature{&b_uint32,&b_uint32});
    proj.names().name_function(fn.index(),{u8"testfn"s,new_chroma()});
    fn.ast() = {bbe::NodeType::NTYPE};
    
    SDL_SetHint(SDL_HINT_INVALID_PARAM_CHECKS,"1");
    SDL_SetAppMetadata("edBCC (SGL)",nullptr,"edbcc.cpp");
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    { // scope for all GL objects. Their dtors must run before we destroy everything with SDL_Quit().
    sfe::Window ed{proj,{u8"edBCC (SGL)"s,1200,600},proj.entities(),{code_font(),{1200,600},1.0f}};
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glEnable(GL_CULL_FACE);
    glLineWidth(3.0f);
    sgl::init_gl_for_text();
    
    if(!SDL_GL_SetSwapInterval(-1)) SDL_GL_SetSwapInterval(1);
    
    ed.add_command(u8"open command palette"s,SDLK_F1,sfe::commands::open_command_palette);
    ed.add_command(u8"rename selection"s,SDLK_F2,sfe::commands::rename_selection);
    ed.add_command(u8"change function cname"s,{sfe::KeyModifiers::CTRL,SDLK_F2},sfe::commands::re_cname_selection);
    ed.add_command(u8"recolor"s,SDLK_F3,sfe::commands::recolor_selection);
    ed.add_command(u8"save"s,{sfe::KeyModifiers::CTRL,SDLK_S},sfe::commands::save);
    ed.add_command(u8"load"s,{sfe::KeyModifiers::CTRL,SDLK_O},sfe::commands::load);
    ed.add_command(u8"reset cursor"s,SDLK_F8,sfe::commands::reset_cursor);
    ed.add_command(u8"inline function"s,sfe::commands::inline_function);
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
                    if(ed.is_textbox_open() || ed.color_picker().is_open() || !keydown(ed.toast(),proj,ed.code(),kc,kp)){
                        ed.keydown(kp);
                    }
                    edb.clear();
                    for(auto& f: proj.entities().functions()){
                        f.ast().recursively_recalculate_result_type(proj.entities(),edb,f.signature());
                    }
                    break;
            }
        }
        const bool* keys = SDL_GetKeyboardState(nullptr);
        if(ed.color_picker().is_open()){
            cppp::fvec3 hsv = ed.color_picker().get_hsv();
            hsv.x() += (keys[SDL_SCANCODE_LSHIFT]-keys[SDL_SCANCODE_LCTRL]) * 0.02f;
            if(hsv.x() < 0.0f) hsv.x() += 6.0f;
            else if(hsv.x() >= 6.0f) hsv.x() -= 6.0f;
            hsv.y() = std::clamp(hsv.y() + (keys[SDL_SCANCODE_D]-keys[SDL_SCANCODE_A]) * 0.01f,0.0f,1.0f);
            hsv.z() = std::clamp(hsv.z() + (keys[SDL_SCANCODE_W]-keys[SDL_SCANCODE_S]) * 0.01f,0.0f,1.0f);
            ed.color_picker().set_hsv(hsv);
        }
        glClear(GL_COLOR_BUFFER_BIT);
        ed.render(edb,proj.names(),keys[SDL_SCANCODE_LALT]);
        if(ed.toast().alive()){
            ed.graphics_context().draw_text(ed.toast().message(),{10.0f,10.0f+ed.graphics_context().ascender()*0.6f},0.6f,{1.0f,0.0f,0.0f});
        }
        if(ed.code().cursor().selected().type() == sfe::VisualNodeType::A){
            bbe::ASTNode& an = ed.code().cursor().selected().a();
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
