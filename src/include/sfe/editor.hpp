#pragma once
#include<SDL3/SDL_events.h>
#include"uitree.hpp"
namespace sfe{
    class Editor{
        UICursor cursor;
        // Owns the root for perf reasons (no need to store an extra pointer)
        void navigate(bool right,bool fast);
        public:
            Editor(VisualFunctionNode&& root) : cursor(std::move(root)){}
            const VisualFunctionNode& root() const{
                return cursor.trail().root();
            }
            VisualFunctionNode& root(){
                return cursor.trail().root();
            }
            void render_full(const GraphicsContext& gc,cppp::fvec2& pos) const{
                root().draw(gc,cursor,pos);
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
            void keydown(const SDL_KeyboardEvent&);
    };
}
