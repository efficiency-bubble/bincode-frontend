#pragma once
#include<sgl/draw/line.hpp>
#include<SDL3/SDL_events.h>
#include"cursor.hpp"
#include"uitree.hpp"
#include"text.hpp"
namespace sfe{
    namespace detail{
        template<typename T>
        class Owned{
            T v;
            public:
                template<typename ...A>
                Owned(A&& ...a) : v(std::forward<A>(a)...){}
                T& operator*(){
                    return v;
                }
                const T& operator*() const{
                    return v;
                }
                T* operator->(){
                    return &v;
                }
                const T* operator->() const{
                    return &v;
                }
        };
    }
    class Editor{
        sgl::LineDrawer ld;
        sfe::SDFTextRenderer tr;
        // Owns the root for perf reasons (no need to store an extra pointer)
        sfe::Breadcrumbs<VisualNode,detail::Owned<VisualNode>> cursor;
        bool cursor_after;
        void draw_node(const VisualNode& nd,cppp::fvec2& pos,sgl::CachedFont& cf,sgl::CoordinateMap& cm);
        void navigate(bool right,bool fast);
        public:
            Editor(VisualNode&& root) : cursor(std::move(root)), cursor_after(false){}
            void render_full(cppp::fvec2& pos,sgl::CachedFont& cf,sgl::CoordinateMap& cm){
                draw_node(cursor.root(),pos,cf,cm);
            }
            void enter(std::uint32_t c,bool from_right){
                cursor.enter(c);
                cursor_after = from_right;
            }
            const VisualNode& selected() const{
                return cursor.top();
            }
            VisualNode& selected(){
                return cursor.top();
            }
            void set_select_after(bool after){
                cursor_after = after;
            }
            bool selected_after() const{
                return cursor_after;
            }
            void keydown(const SDL_KeyboardEvent&);
            sfe::SDFTextRenderer& text_renderer(){
                return tr;
            }
    };
}
