#pragma once
#include<cppp/type-erasure.hpp>
#include<bbe/function.hpp>
#include<cppp/swap.hpp>
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
        A,F
    };
    class UICursor;
    class VisualNode{
        std::vector<VisualNode> _children;
        void* nd;
        VisualNodeType _type;
        friend void swap(VisualNode& lhs,VisualNode& rhs){
            cppp::swap(lhs._children,rhs._children);
            cppp::swap(lhs.nd,rhs.nd);
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
            const std::vector<VisualNode>& children() const{
                return _children;
            }
            std::vector<VisualNode>& children(){
                return _children;
            }
            void draw(const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const sfe::NameDatabase& names,const UICursor& cursor,cppp::fvec2& pos) const;
            void repoint(bbe::ASTNode& other){
                nd = &other;
            }
            void clear(){
                _children.clear();
            }
            void assert_a() const{
                CPPP_ASSERT(_type == VisualNodeType::A);
            }
            const bbe::ASTNode& a() const{
                return *static_cast<const bbe::ASTNode*>(nd);
            }
            bbe::ASTNode& a(){
                return *static_cast<bbe::ASTNode*>(nd);
            }
            const bbe::Function& f() const{
                return *static_cast<const bbe::Function*>(nd);
            }
            bbe::Function& f(){
                return *static_cast<bbe::Function*>(nd);
            }
            void apopulate(){
                assert_a();
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
                assert_a();
                for(auto& c : a().children() | std::views::drop(1uz)){
                    _children.emplace_back(c);
                }
            }
            std::uint32_t apriority() const;
            void rerender(){
                if(_type == VisualNodeType::A){
                    clear();
                    apopulate();
                }else{
                    _children.front().rerender();
                }
            }
            bool placeholder() const{
                return _type == VisualNodeType::A && a().type() == bbe::NodeType::NTYPE;
            }
            VisualNodeType type() const{
                return _type;
            }
    };
    // Owns the root for perf reasons (no need to store an extra pointer)
    class Breadcrumbs{
        VisualNode _root;
        struct PathEntry{
            VisualNode* p;
            std::uint32_t index;
        };
        // forward_list::clear is very slow
        std::vector<PathEntry> path;
        const PathEntry& etop() const{
            return path.back();
        }
        PathEntry& etop(){
            return path.back();
        }
        public:
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
            const Breadcrumbs& trail() const{
                return crumbs;
            }
            Breadcrumbs& trail(){
                return crumbs;
            }
            const VisualNode& selected() const{
                return crumbs.top();
            }
            VisualNode& selected(){
                return crumbs.top();
            }
            bool is_after() const{
                return after;
            }
            void set_after(bool new_after){
                after = new_after;
            }
    };
}
