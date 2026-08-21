#pragma once
#include<cppp/type-erasure.hpp>
#include<bbe/project_entity_pool.hpp>
#include<bbe/function.hpp>
#include<bbe/ast.hpp>
#include<type_traits>
#include<concepts>
#include<cstdint>
#include<ranges>
#include<vector>
#include"graphics.hpp"
#include"style.hpp"
namespace sfe{
    enum class VisualNodeType{
        A,F,P
    };
    class UICursor;
    class VisualNode{
        std::vector<VisualNode> _children;
        void* nd;
        VisualNodeType _type;
        friend void swap(VisualNode& lhs,VisualNode& rhs){
            std::ranges::swap(lhs._children,rhs._children);
            std::ranges::swap(lhs.nd,rhs.nd);
        }
        struct no_populate_t{};
        public:
            constexpr static no_populate_t no_populate{};
            VisualNode(bbe::ASTNode& nd,no_populate_t) : nd(&nd), _type(VisualNodeType::A){}
            VisualNode(bbe::ASTNode& nd) : nd(&nd), _type(VisualNodeType::A){
                apopulate();
            }
            VisualNode(bbe::Function& f) : nd(&f), _type(VisualNodeType::F){
                _children.emplace_back(f.ast());
            }
            VisualNode(bbe::ProjectEntitiesPool& f) : nd(&f), _type(VisualNodeType::P){
                for(bbe::Function& fn : f.functions()){
                    _children.emplace_back(fn);
                }
            }
            const std::vector<VisualNode>& children() const{
                return _children;
            }
            std::vector<VisualNode>& children(){
                return _children;
            }
            void adraw(const GraphicsContext&,const bbe::ErrorDatabase&,const NameDatabase&,const UICursor&,cppp::fvec2& pos,float indentation,bool altmode) const;
            void fdraw(const GraphicsContext&,const bbe::ErrorDatabase&,const NameDatabase&,const UICursor&,cppp::fvec2& pos,bool altmode) const;
            void pdraw(const GraphicsContext&,const bbe::ErrorDatabase&,const NameDatabase&,const UICursor&,cppp::fvec2& pos,bool altmode) const;
            void repoint(bbe::ASTNode& other){
                nd = &other;
            }
            void clear(){
                _children.clear();
            }
            void assert_a() const{
                CPPP_ASSERT(_type == VisualNodeType::A);
            }
            void assert_f() const{
                CPPP_ASSERT(_type == VisualNodeType::F);
            }
            void assert_p() const{
                CPPP_ASSERT(_type == VisualNodeType::P);
            }
            const bbe::ASTNode& a() const{
                assert_a();
                return *static_cast<const bbe::ASTNode*>(nd);
            }
            bbe::ASTNode& a(){
                assert_a();
                return *static_cast<bbe::ASTNode*>(nd);
            }
            const bbe::Function& f() const{
                assert_f();
                return *static_cast<const bbe::Function*>(nd);
            }
            bbe::Function& f(){
                assert_f();
                return *static_cast<bbe::Function*>(nd);
            }
            const bbe::ProjectEntitiesPool& p() const{
                assert_p();
                return *static_cast<const bbe::ProjectEntitiesPool*>(nd);
            }
            bbe::ProjectEntitiesPool& p(){
                assert_p();
                return *static_cast<bbe::ProjectEntitiesPool*>(nd);
            }
            void apopulate(){
                for(auto& c : a().children()){
                    _children.emplace_back(c);
                }
            }
            // for lhs stealing
            void apopulate(VisualNode&& vn){
                assert_a();
                _children.emplace_back(std::move(vn));
            }
            // for lhs stealing
            void apopulate_butfirst(){
                for(auto& c : a().children() | std::views::drop(1uz)){
                    _children.emplace_back(c);
                }
            }
            void paddf(bbe::Function& fn){
                assert_p();
                _children.emplace_back(fn);
            }
            void perasef(const bbe::Function& fn){
                assert_p();
                #if __cpp_lib_parallel_algorithm >= 202506L
                #warning GCC updated! change this to use std::ranges::find_if
                #endif
                _children.erase(std::find_if(std::execution::unseq,_children.begin(),_children.end(),[p=&fn](const VisualNode& vn){
                    return &vn.f() == p;
                }));
            }
            std::uint32_t apriority() const;
            void arerender(){
                assert_a();
                clear();
                apopulate();
            }
            void frerender(){
                assert_f();
                _children.front().arerender();
            }
            void prepopulate(){
                assert_p();
                _children.clear();
                for(auto& fn : p().functions()){
                    _children.emplace_back(fn);
                }
            }
            void prerender(){
                assert_p();
                for(auto& child : _children) child.frerender();
            }
            bool is_placeholder() const{
                return _type == VisualNodeType::A && a().type() == bbe::NodeType::NTYPE;
            }
            VisualNodeType type() const{
                return _type;
            }
    };
    struct PathEntry{
        VisualNode* p;
        std::uint32_t index;
    };
    class Breadcrumbs{
        VisualNode _root;
        // forward_list::clear is very slow
        std::vector<PathEntry> path;
        const PathEntry& etop() const{
            return path.back();
        }
        PathEntry& etop(){
            return path.back();
        }
        public:
            const std::vector<PathEntry>& elements() const{
                return path;
            }
            Breadcrumbs(VisualNode&& root) : _root(std::move(root)){}
            VisualNode& root(){
                return _root;
            }
            const VisualNode& root() const{
                return _root;
            }
            void home(){
                path.clear();
            }
            bool has_nesting() const{
                return !path.empty();
            }
            const VisualNode& top() const{
                return has_nesting()?*etop().p:_root;
            }
            VisualNode& top(){
                return has_nesting()?*etop().p:_root;
            }
            std::uint32_t top_index() const{
                return etop().index;
            }
            const VisualNode& below_top() const{
                CPPP_ASSERT(has_nesting());
                return path.size()>1?*path[path.size()-2].p:_root;
            }
            VisualNode& below_top(){
                CPPP_ASSERT(has_nesting());
                return path.size()>1?*path[path.size()-2].p:_root;
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
                return etop().index+1 == below_top().children().size();
            }
            void prev_sibling(){
                etop().p = &below_top().children()[--etop().index];
            }
            void next_sibling(){
                etop().p = &below_top().children()[++etop().index];
            }
    };
    class UICursor{
        Breadcrumbs crumbs;
        bool after;
        public:
            UICursor(VisualNode&& root) : crumbs(std::move(root)), after(false){}
            const std::vector<PathEntry>& path_elements() const{
                return crumbs.elements();
            }
            const VisualNode& root() const{
                return crumbs.root();
            }
            VisualNode& root(){
                return crumbs.root();
            }
            void home(){
                crumbs.home();
                after = false;
            }
            void enter(std::uint32_t c){
                crumbs.enter(c);
            }
            void enter(std::uint32_t c,bool from_right){
                enter(c);
                after = from_right;
            }
            void leave(){
                crumbs.leave();
            }
            std::uint32_t index_of_selection() const{
                return crumbs.top_index();
            }
            const VisualNode& selected() const{
                return crumbs.top();
            }
            VisualNode& selected(){
                return crumbs.top();
            }
            bool is_nested() const{
                return crumbs.has_nesting();
            }
            bool is_first_child() const{
                return crumbs.is_first_child();
            }
            bool is_last_child() const{
                return crumbs.is_last_child();
            }
            void next_sibling(){
                crumbs.next_sibling();
            }
            void prev_sibling(){
                crumbs.prev_sibling();
            }
            const VisualNode& selected2() const{
                return crumbs.below_top();
            }
            VisualNode& selected2(){
                return crumbs.below_top();
            }
            bool is_after() const{
                return after;
            }
            void set_after(bool new_after){
                after = new_after;
            }
    };
}
