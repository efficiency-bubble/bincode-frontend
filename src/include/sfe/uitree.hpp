#pragma once
#include<bbe/ast.hpp>
#include<vector>
namespace sfe{
    class VisualNode{
        bbe::ASTNode* nd;
        std::vector<VisualNode> _children;
        bbe::impl::ASTChildren& find_child_array(){
            switch(nd->type()){
                using enum bbe::NodeType;
                case CALL_BUILTIN:
                    return nd->children().front().children();
                default:
                    return nd->children();
            }
        }
        public:
            VisualNode(bbe::ASTNode& nd) : nd(&nd){
                populate();
            }
            void repoint(bbe::ASTNode& other){
                nd = &other;
            }
            void clear(){
                _children.clear();
            }
            void populate(){
                for(auto& c : find_child_array()){
                    _children.emplace_back(c);
                }
            }
            // hopefully CSE can eliminate all the calls to find_child_array()
            void populate(std::uint32_t i){
                _children.emplace_back(find_child_array()[i]);
            }
            void populate(VisualNode&& vn){
                _children.emplace_back(std::move(vn));
            }
            void rerender(){
                clear();
                populate();
            }
            bool fast() const{
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
            std::uint32_t prim() const{
                return nd->getp32();
            }
            void setp32(std::uint32_t prim){
                nd->setp32(prim);
            }
            const std::vector<VisualNode>& children() const{
                return _children;
            }
            std::vector<VisualNode>& children(){
                return _children;
            }
    };
}
