#pragma once
#include<bbe/project_entity_pool.hpp>
#include<SDL3/SDL_events.h>
#include<cppp/string.hpp> // textbox buffer
#include<sgl/window.hpp>
#include<bbe/error.hpp>
#include<cppp/rtl.hpp>
#include<optional>
#include<stack>
#include"uitree.hpp"
#include"toast.hpp"
#include"command-palette.hpp"
#include"color-picker.hpp"
#include"keys.hpp"
#include"project.hpp"
namespace sfe{
    class CodeEntry{
        UICursor _cursor;
        void navigate(bool right,bool fast);
        public:
            CodeEntry(VisualNode&& root) : _cursor(std::move(root)){}
            const VisualNode& root() const{
                return _cursor.root();
            }
            VisualNode& root(){
                return _cursor.root();
            }
            void render_full(const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const NameDatabase& names,cppp::fvec2& pos,bool altmode) const{
                root().pdraw(gc,errors,names,_cursor,pos,altmode);
            }
            const UICursor& cursor() const{
                return _cursor;
            }
            UICursor& cursor(){
                return _cursor;
            }
            void keydown(Keypress);
    };
    enum class TextboxTargetType{
        COMMAND_PALETTE, RAW_STRING
    };
    class Textbox{
        const cppp::str* buffer;
        cppp::uvec3 area;
        float font_size;
        float x_end_buf;
        TextboxTargetType tt;
        void* target;
        void update_buffer_ref(){
            switch(tt){
                case TextboxTargetType::COMMAND_PALETTE:
                    buffer = &ctarget().buffer;
                    break;
                case TextboxTargetType::RAW_STRING:
                    buffer = &ntarget();
                    break;
            }
        }
        public:
            Textbox() : target(nullptr){}
            void set(cppp::uvec3 a,float s,TextboxTargetType t,void* tgt){
                area = a;
                font_size = s;
                tt = t;
                target = tgt;
                update_buffer_ref();
            }
            void reset(){
                target = nullptr;
            }
            explicit operator bool() const{
                return target;
            }
            cppp::uvec2 position() const{
                return {area.x(),area.y()};
            }
            const cppp::str& text() const{
                return *buffer;
            }
            void append(cppp::sv more){
                switch(tt){
                    case TextboxTargetType::COMMAND_PALETTE:
                        ctarget().append(more);
                        break;
                    case TextboxTargetType::RAW_STRING:
                        ntarget().append(more);
                        break;
                }
            }
            void backspace(){
                if(buffer->empty()) return;
                switch(tt){
                    case TextboxTargetType::COMMAND_PALETTE:
                        ctarget().backspace();
                        break;
                    case TextboxTargetType::RAW_STRING:
                        ntarget().pop_back();
                        break;
                }
            }
            float scale() const{
                return font_size;
            }
            TextboxTargetType target_type() const{
                return tt;
            }
            const CommandPalette& ctarget() const{
                return *static_cast<const CommandPalette*>(target);
            }
            CommandPalette& ctarget(){
                return *static_cast<CommandPalette*>(target);
            }
            const cppp::str& ntarget() const{
                return *static_cast<const cppp::str*>(target);
            }
            cppp::str& ntarget(){
                return *static_cast<cppp::str*>(target);
            }
            void update(sgl::Window& win,const GraphicsContext& gc) const{
                SDL_Rect rect{
                    .x = static_cast<int>(area.x()),
                    .y = static_cast<int>(area.y()),
                    .w = static_cast<int>(area.z()),
                    .h = static_cast<int>(gc.line_height() * font_size)
                };
                SDL_SetTextInputArea(win.native_handle(),&rect,0 /* TODO: calculate window coordinates offset from HB and pass it in */);
            }
    };
    class Window{
        constexpr static float COMMAND_PALETTE_FONT_SCALE = 0.5f;
        Project* pr;
        sgl::Window w;
        CodeEntry ce;
        GraphicsContext gc;
        CommandSet cs;
        HotkeyRecords hr;
        ColorPicker pk;
        CommandPalette cp;
        Toast _toast;
        Textbox textbox;
        cppp::str preedit;
        public:
            Window(Project& proj,sgl::Window&& w,VisualNode&& root,GraphicsContext&& gc) : pr(&proj), w(std::move(w)), ce(std::move(root)), gc(std::move(gc)), cp(cs){}
            void set_textbox(cppp::uvec3 area,float size,TextboxTargetType tt,void* target){
                if(!textbox){
                    SDL_StartTextInput(w.native_handle());
                }
                textbox.set(area,size,tt,target);
                textbox.update(w,gc);
            }
            void remove_textbox(){
                textbox.reset();
                SDL_StopTextInput(w.native_handle());
            }
            const Project& project() const{
                return *pr;
            }
            Project& project(){
                return *pr;
            }
            const Toast& toast() const{
                return _toast;
            }
            Toast& toast(){
                return _toast;
            }
            const sgl::Window& system_window() const{
                return w;
            }
            sgl::Window& system_window(){
                return w;
            }
            const GraphicsContext& graphics_context() const{
                return gc;
            }
            GraphicsContext& graphics_context(){
                return gc;
            }
            const CodeEntry& code() const{
                return ce;
            }
            CodeEntry& code(){
                return ce;
            }
            void open_command_palette(){
                cp.reset();
                set_textbox(get_overlay_top_edge(),COMMAND_PALETTE_FONT_SCALE,TextboxTargetType::COMMAND_PALETTE,&cp);
            }
            void close_command_palette(){
                CPPP_ASSERT(is_command_palette_open());
                remove_textbox();
                cp.reset();
            }
            void preedit_set(std::uint32_t begin,std::uint32_t span,const char* data){
                CPPP_ASSERT(!begin);
                static_cast<void>(span); // TODO
                preedit.clear();
                while(char c=*data++){
                    preedit.push_back(static_cast<char8_t>(c));
                }
            }
            const ColorPicker& color_picker() const{
                return pk;
            }
            ColorPicker& color_picker(){
                return pk;
            }
            bool is_textbox_open() const{
                return static_cast<bool>(textbox);
            }
            bool is_command_palette_open() const{
                return textbox && textbox.target_type() == TextboxTargetType::COMMAND_PALETTE;
            }
            const CommandPalette& command_palette() const{
                return cp;
            }
            CommandPalette& command_palette(){
                return cp;
            }
            void add_command(cppp::str&& name,Command c){
                cs.add(std::move(name),c);
            }
            void add_command(Keypress hotkey,Command c){
                hr.add(hotkey,c);
            }
            void add_command(cppp::str&& name,Keypress hotkey,Command c){
                add_command(std::move(name),c);
                add_command(hotkey,c);
            }
            void refresh_command_list(){
                cp.reset();
            }
            void textinput(cppp::sv s){
                if(textbox){
                    textbox.append(s);
                }
            }
            void keydown(Keypress kp){
                bool hasmods = kp.mods()&(KeyModifiers::CTRL|KeyModifiers::SHIFT|KeyModifiers::ALT);
                if(textbox){
                    if(!hasmods){
                        switch(kp.key()){
                            case SDLK_BACKSPACE:
                                textbox.backspace();
                                break;
                            case SDLK_ESCAPE:
                                if(textbox.target_type() == TextboxTargetType::COMMAND_PALETTE){
                                    close_command_palette();
                                }else remove_textbox();
                                break;
                        }
                        if(is_command_palette_open()) switch(kp.key()){
                            case SDLK_RETURN: {
                                auto cmd = cp.selected();
                                close_command_palette();
                                if(cmd){
                                    cmd->exec(*this);
                                }
                                return;
                            }
                            case SDLK_DOWN:
                                cp.next();
                                return;
                            case SDLK_UP:
                                cp.prev();
                                return;
                        }
                    }
                    return;
                }else if(pk.is_open()){
                    if(!hasmods){
                        if(kp.key() == SDLK_ESCAPE) pk.close();
                    }
                    return;
                }
                if(!hr.handle(*this,kp)){
                    ce.keydown(kp);
                }
            }
            void render(const bbe::ErrorDatabase& edb,const NameDatabase& ndb,bool altmode) const{
                ce.render_full(gc,edb,ndb,cppp::rtl<cppp::fvec2>({10.0f,10.0f+gc.line_height()*0.65f+gc.ascender()}),altmode);
            }
            cppp::fvec3 get_overlay_top_edge() const{
                float half_winw = static_cast<float>(gc.cmap().win_size().x())/2.0f;
                return {half_winw*0.4f,10.0f,half_winw*1.2f};
            }
            void render_overlay() const{
                if(pk.is_open()){
                    pk.render(gc,{static_cast<float>(gc.cmap().win_size().x())-310.0f,10.0f});
                }
                if(is_command_palette_open()){
                    cppp::fvec3 top_edge = get_overlay_top_edge();
                    cp.render(gc,{top_edge.x(),top_edge.y()},top_edge.z(),COMMAND_PALETTE_FONT_SCALE);
                }
                if(textbox){
                    cppp::fvec2 cursor = textbox.position();
                    cursor.y() += gc.ascender() * textbox.scale();
                    gc.draw_text_at_cursor(textbox.text(),cursor,textbox.scale(),WHITE);
                    gc.draw_text_at_cursor(preedit,cursor,textbox.scale(),WHITE);
                }
            }
    };
}
