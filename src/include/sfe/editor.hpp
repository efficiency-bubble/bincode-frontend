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
#include"keys.hpp"
#include"project.hpp"
namespace sfe{
    class CodeEntry{
        UICursor cursor;
        void navigate(bool right,bool fast);
        public:
            CodeEntry(VisualNode&& root) : cursor(std::move(root)){}
            const VisualNode& root() const{
                return cursor.trail().root();
            }
            VisualNode& root(){
                return cursor.trail().root();
            }
            void render_full(const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const sfe::NameDatabase& names,cppp::fvec2& pos) const{
                root().draw(gc,errors,names,cursor,pos);
            }
            void leave(){
                cursor.trail().leave();
            }
            void home(){
                cursor.trail().home();
                set_select_after(false);
            }
            void enter(std::uint32_t c,bool from_right){
                cursor.trail().enter(c);
                cursor.set_after(from_right);
            }
            const VisualNode& selected() const{
                return cursor.selected();
            }
            VisualNode& selected(){
                return cursor.selected();
            }
            void set_select_after(bool after){
                cursor.set_after(after);
            }
            bool selected_after() const{
                return cursor.is_after();
            }
            void keydown(Keypress);
    };
    enum class TextboxTargetType{
        COMMAND_PALETTE
    };
    class Textbox{
        cppp::str buffer;
        cppp::uvec3 area;
        float font_size;
        float x_end_buf;
        TextboxTargetType tt;
        void* target;
        public:
            Textbox(cppp::uvec3 area,float size,TextboxTargetType tt,void* target) : area(area), font_size(size), tt(tt), target(target){}
            cppp::uvec2 position() const{
                return {area.x(),area.y()};
            }
            const cppp::str& text() const{
                return buffer;
            }
            void append(cppp::sv more){
                std::size_t oldsize = buffer.size();
                buffer.append(more);
                switch(tt){
                    case TextboxTargetType::COMMAND_PALETTE:
                        ctarget().update_refine(oldsize,buffer);
                        break;
                    default: std::unreachable();
                }
            }
            void backspace(){
                if(buffer.empty()) return;
                buffer.pop_back();
                switch(tt){
                    case TextboxTargetType::COMMAND_PALETTE:
                        ctarget().reset(buffer);
                        break;
                    default: std::unreachable();
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
            void update(sgl::Window& win,const sfe::GraphicsContext& gc) const{
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
        std::optional<CommandPalette> cp;
        Toast _toast;
        std::stack<Textbox> textboxes;
        cppp::str preedit;
        public:
            Window(Project& proj,sgl::Window&& w,VisualNode&& root,GraphicsContext&& gc) : pr(&proj), w(std::move(w)), ce(std::move(root)), gc(std::move(gc)){}
            void add_textbox(Textbox tb){
                if(textboxes.empty()) SDL_StartTextInput(w.native_handle());
                textboxes.emplace(tb);
                tb.update(w,gc);
            }
            void pop_textbox(){
                textboxes.pop();
                if(textboxes.empty()) SDL_StopTextInput(w.native_handle());
                else textboxes.top().update(w,gc);
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
                bool open_new = !cp.has_value();
                cp.emplace(cs);
                if(open_new){
                    add_textbox({get_overlay_top_edge(),COMMAND_PALETTE_FONT_SCALE,TextboxTargetType::COMMAND_PALETTE,&*cp});
                }
            }
            void close_command_palette(){
                pop_textbox();
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
            bool is_command_palette_open() const{
                return cp.has_value();
            }
            const CommandPalette& command_palette() const{
                return *cp;
            }
            CommandPalette& command_palette(){
                return *cp;
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
                if(!textboxes.empty()){
                    textboxes.top().append(s);
                }
            }
            void keydown(Keypress kp){
                if(kp.mods() == KeyModifiers::NONE){
                    if(!textboxes.empty() && kp.mods() == KeyModifiers::NONE){
                        auto& tb = textboxes.top();
                        switch(kp.key()){
                            case SDLK_BACKSPACE:
                                tb.backspace();
                                return;
                        }
                    }
                    if(is_command_palette_open()) switch(kp.key()){
                        case SDLK_RETURN: {
                            auto cmd = cp->selected();
                            close_command_palette();
                            if(cmd){
                                cmd->exec(*this);
                            }
                            return;
                        }
                        case SDLK_ESCAPE:
                            close_command_palette();
                            return;
                        case SDLK_DOWN:
                            cp->next();
                            return;
                        case SDLK_UP:
                            cp->prev();
                            return;
                    }
                }
                if(!hr.handle(*this,kp)){
                    ce.keydown(kp);
                }
            }
            void render(const bbe::ErrorDatabase& edb,const sfe::NameDatabase& ndb) const{
                ce.render_full(gc,edb,ndb,cppp::rtl<cppp::fvec2>({10.0f,10.0f+gc.line_height()*0.65f+gc.ascender()}));
            }
            cppp::fvec3 get_overlay_top_edge() const{
                float half_winw = static_cast<float>(gc.cmap().win_size().x())/2.0f;
                return {half_winw*0.4f,10.0f,half_winw*1.2f};
            }
            void render_overlay() const{
                if(cp.has_value()){
                    cppp::fvec3 top_edge = get_overlay_top_edge();
                    cp->render(gc,{top_edge.x(),top_edge.y()},top_edge.z(),COMMAND_PALETTE_FONT_SCALE);
                }
                if(!textboxes.empty()){
                    cppp::fvec2 cursor = textboxes.top().position();
                    cursor.y() += gc.ascender() * textboxes.top().scale();
                    gc.draw_text_at_cursor(textboxes.top().text(),cursor,textboxes.top().scale(),WHITE);
                    gc.draw_text_at_cursor(preedit,cursor,textboxes.top().scale(),WHITE);
                }
            }
    };
}
