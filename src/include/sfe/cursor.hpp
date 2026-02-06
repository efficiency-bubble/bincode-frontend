#pragma once
#include<concepts>
#include<ranges>
#include<vector>
namespace sfe{
    namespace detail{
        template<typename T,typename E>
        concept random_access_container_of = std::ranges::random_access_range<T> && std::same_as<std::ranges::range_value_t<T>,std::remove_cv_t<E>> && requires(T& t,std::size_t ind){
            {t[ind]} -> std::same_as<E&>;
        };
        template<typename T>
        concept NodeLike = requires(const T& t,T& mt){
            {t.children()} -> random_access_container_of<const T>;
            {mt.children()} -> random_access_container_of<T>;
        };
        template<typename T,typename V>
        concept CPropPointerTo = requires(const T& t,T& mt){
            {*t} -> std::same_as<const V&>;
            {*mt} -> std::same_as<V&>;
        };
    }
    template<detail::NodeLike T,detail::CPropPointerTo<T> Root>
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
            T& root(){
                return *_root;
            }
            const T& root() const{
                return *_root;
            }
            void home(){
                path.clear();
            }
            bool has_nesting() const{
                return !path.empty();
            }
            const T& top() const{
                return has_nesting()?*etop().node:*_root;
            }
            T& top(){
                return has_nesting()?*etop().node:*_root;
            }
            const T& top2() const{
                return path.size()>1?*path[path.size()-2].node:*_root;
            }
            T& top2(){
                return path.size()>1?*path[path.size()-2].node:*_root;
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
}
