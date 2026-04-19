#pragma once
#include<bbe/project_entity_pool.hpp>
#include<SDL3/SDL_events.h>
#include<sgl/window.hpp>
#include<bbe/error.hpp>
#include<cppp/rtl.hpp>
#include<optional>
#include"uitree.hpp"
#include"toast.hpp"
#include"command-palette.hpp"
#include"keys.hpp"
namespace sfe{
    // Owns the root for perf reasons (no need to store an extra pointer)
    class CodeEntry{
        UICursor cursor;
        void navigate(bool right,bool fast);
        public:
            CodeEntry(VisualFunctionNode&& root) : cursor(std::move(root)){}
            const VisualFunctionNode& root() const{
                return cursor.trail().root();
            }
            VisualFunctionNode& root(){
                return cursor.trail().root();
            }
            void render_full(const GraphicsContext& gc,const bbe::ErrorDatabase& errors,cppp::fvec2& pos) const{
                root().draw(gc,errors,cursor,pos);
            }
            void leave(){
                cursor.trail().leave();
            }
            void subst_sel(VisualASTNode& other){
                cursor.trail().subst(other);
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
                return cursor.trail().top();
            }
            VisualNode& selected(){
                return cursor.trail().top();
            }
            void set_select_after(bool after){
                cursor.set_after(after);
            }
            bool selected_after() const{
                return cursor.is_after();
            }
            void keydown(Keypress);
    };
    class Window{
        bbe::ProjectEntitiesPool* pr;
        sgl::Window w;
        CodeEntry ce;
        GraphicsContext gc;
        CommandSet cs;
        HotkeyRecords hr;
        std::optional<CommandPalette> cp;
        Toast _toast;
        public:
            Window(bbe::ProjectEntitiesPool& proj,sgl::Window&& w,VisualFunctionNode&& root,GraphicsContext&& gc) : pr(&proj), w(std::move(w)), ce(std::move(root)), gc(std::move(gc)){}
            const bbe::ProjectEntitiesPool& project() const{
                return *pr;
            }
            bbe::ProjectEntitiesPool& project(){
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
                system_window().start_input();
                cp.emplace(cs);
            }
            void close_command_palette(){
                system_window().stop_input();
                cp.reset();
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
            void keydown(Keypress kp){
                if(is_command_palette_open() && kp.mods() == KeyModifiers::NONE) switch(kp.key()){
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
                    case SDLK_BACKSPACE:
                        cp->backspace();
                        return;
                    case SDLK_DOWN:
                        cp->next();
                        return;
                    case SDLK_UP:
                        cp->prev();
                        return;
                }else if(!hr.handle(*this,kp)){
                    ce.keydown(kp);
                }
            }
            void render(bbe::ErrorDatabase& edb) const{
                ce.render_full(gc,edb,cppp::rtl<cppp::fvec2>({10.0f,10.0f+gc.line_height()*0.65f+gc.ascender()}));
            }
            void render_overlay() const{
                if(cp.has_value()){
                    float winwidth_f = static_cast<float>(gc.cmap().win_size().x());
                    cp->render(gc,{winwidth_f/2.0f,10.0f},winwidth_f*0.6f,0.5f);
                }
            }
    };
}
