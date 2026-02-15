#pragma once
#include<SDL3/SDL_events.h>
#include"uitree.hpp"
namespace sfe{
    class Editor{
        UICursor cursor;
        GraphicsContext gc;
        // Owns the root for perf reasons (no need to store an extra pointer)
        void navigate(bool right,bool fast);
        public:
            Editor(VisualFunctionNode&& root,GraphicsContext&& gc) : cursor(std::move(root)), gc(std::move(gc)){}
            void update_window(std::uint32_t w,std::uint32_t h){
                gc.update_window(w,h);
            }
            const VisualFunctionNode& root() const{
                return cursor.trail().root();
            }
            VisualFunctionNode& root(){
                return cursor.trail().root();
            }
            void render_full(cppp::fvec2& pos) const{
                root().draw(gc,cursor,pos);
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
            void keydown(const SDL_KeyboardEvent&);
            const GraphicsContext& graphics_context() const{
                return gc;
            }
    };
}
