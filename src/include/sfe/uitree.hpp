#pragma once
#include<cppp/type-erasure.hpp>
#include<bbe/function.hpp>
#include<cppp/vector.hpp>
#include<cppp/swap.hpp>
#include<bbe/ast.hpp>
#include<concepts>
#include<cstdint>
#include<ranges>
#include<vector>
#include"graphics.hpp"
#include"cursor.hpp"
namespace sfe{
    using UICursor = Cursor<class VisualNode,class VisualFunctionNode>;
    class VisualNode{
        public:
            template<typename Self>
            cppp::erased_span<Self> children(this Self& v){
                return static_cast<cppp::copy_const_t<Self,VisualNode>&>(v).do_children().template downcast<Self>();
            }
            virtual void draw(const GraphicsContext& gc,const UICursor& cursor,cppp::fvec2& pos) const = 0;
            virtual bool fast() const = 0;
            virtual void rerender_children() = 0;
        protected:
            using children_t = cppp::erased_span<VisualNode>;
            using const_children_t = cppp::erased_span<const VisualNode>;
            virtual children_t do_children() = 0;
            virtual const_children_t do_children() const = 0;
            ~VisualNode(){}
    };
    class VisualASTNode final : public VisualNode{
        bbe::ASTNode* nd;
        std::vector<VisualASTNode> _children;
        friend void swap(VisualASTNode& lhs,VisualASTNode& rhs){
            cppp::swap(lhs.nd,rhs.nd);
            cppp::swap(lhs._children,rhs._children);
        }
        struct no_populate_t{};
        public:
            constexpr static no_populate_t no_populate{};
            VisualASTNode(bbe::ASTNode& nd,no_populate_t) : nd(&nd){}
            VisualASTNode(bbe::ASTNode& nd) : nd(&nd){
                populate();
            }
            void draw(const GraphicsContext& gc,const UICursor& cursor,cppp::fvec2& pos) const override;
            void repoint(bbe::ASTNode& other){
                nd = &other;
            }
            void clear(){
                _children.clear();
            }
            void populate(){
                for(auto& c : nd->children()){
                    _children.emplace_back(c);
                }
            }
            // for lhs stealing
            void populate(VisualASTNode&& vn){
                _children.emplace_back(std::move(vn));
            }
            // for lhs stealing
            void rerender_except_first(){
                for(auto& c : nd->children() | std::views::drop(1uz)){
                    _children.emplace_back(c);
                }
            }
            std::uint32_t priority() const;
            void rerender_children() override{
                clear();
                populate();
            }
            bool fast() const override{
                return nd->type() == bbe::NodeType::NTYPE;
            }
            const bbe::ASTNode& node() const{
                return *nd;
            }
            bbe::ASTNode& node(){
                return *nd;
            }
            bbe::NodeType type() const{
                return nd->type();
            }
            std::uint32_t p32() const{
                return nd->getp32();
            }
            void setp32(std::uint32_t prim){
                nd->setp32(prim);
            }
        protected:
            const_children_t do_children() const override{
                return std::span{_children};
            }
            children_t do_children() override{
                return std::span{_children};
            }
    };
    class VisualFunctionNode final : public VisualNode{
        bbe::Function* fn;
        VisualASTNode _child;
        std::uint32_t id;
        public:
            VisualFunctionNode(bbe::Function& f,std::uint32_t id) : fn(&f), _child(f.ast()), id(id){}
            void draw(const GraphicsContext& gc,const UICursor& cursor,cppp::fvec2& pos) const override;
            const bbe::Function& func() const{
                return *fn;
            }
            bbe::Function& func(){
                return *fn;
            }
            const VisualASTNode& child() const{
                return _child;
            }
            bool fast() const{
                return false;
            }
            void rerender_children() override{
                _child.rerender_children();
            }
        protected:
            children_t do_children() override{
                return std::span<VisualASTNode,1uz>{&_child,1uz};
            }
            const_children_t do_children() const override{
                return std::span<const VisualASTNode,1uz>{&_child,1uz};
            }
    };
}
