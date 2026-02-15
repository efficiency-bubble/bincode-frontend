#pragma once
#include<concepts>
#include<vector>
namespace sfe{
    template<typename T,typename Root>
    class Breadcrumbs{
        Root _root;
        struct Entry{
            T* node;
            std::uint32_t index;
        };
        // forward_list::clear is very slow
        std::vector<Entry> path;
        const Entry& etop() const{
            return path.back();
        }
        Entry& etop(){
            return path.back();
        }
        public:
            Breadcrumbs(Root&& root) : _root(std::move(root)){}
            Root& root(){
                return _root;
            }
            const Root& root() const{
                return _root;
            }
            void home(){
                path.clear();
            }
            bool has_nesting() const{
                return !path.empty();
            }
            const T& top() const{
                return has_nesting()?*etop().node:static_cast<const T&>(_root);
            }
            T& top(){
                return has_nesting()?*etop().node:static_cast<T&>(_root);
            }
            const T& top2() const{
                return path.size()>1?*path[path.size()-2].node:static_cast<const T&>(_root);
            }
            T& top2(){
                return path.size()>1?*path[path.size()-2].node:static_cast<T&>(_root);
            }
            void leave(){
                path.pop_back();
            }
            void leave_opt(){
                if(has_nesting()) path.pop_back();
            }
            void enter(std::uint32_t index){
                path.emplace_back(&top().children()[index],index);
            }
            bool is_first_child() const{
                return etop().index == 0;
            }
            bool is_last_child() const{
                return etop().index+1 == top2().children().size();
            }
            void prev_sibling(){
                etop().node = &top2().children()[--etop().index];
            }
            void next_sibling(){
                etop().node = &top2().children()[++etop().index];
            }
    };
    template<typename T,typename Root>
    class Cursor{
        Breadcrumbs<T,Root> crumbs;
        bool after;
        public:
            Cursor(Root&& root) : crumbs(std::move(root)), after(false){}
            const Breadcrumbs<T,Root>& trail() const{
                return crumbs;
            }
            Breadcrumbs<T,Root>& trail(){
                return crumbs;
            }
            bool is_after() const{
                return after;
            }
            void set_after(bool new_after){
                after = new_after;
            }
    };
}
