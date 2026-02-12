#pragma once
#include<cppp/type-erasure.hpp>
#include<sgl/ext/freetype.hpp>
#include<sgl/draw/line.hpp>
#include<bbe/function.hpp>
#include<cppp/vector.hpp>
#include<bbe/ast.hpp>
#include<concepts>
#include<cstdint>
#include<vector>
#include"cursor.hpp"
#include"text.hpp"
namespace sfe{
    class GraphicsContext{
        sgl::LineDrawer ld;
        sfe::SDFTextRenderer tr;
        sgl::CachedFont cf;
        sgl::CoordinateMap cm;
        public:
            GraphicsContext(sgl::CachedFont&& f,sgl::CoordinateMap cm) : cf(std::move(f)), cm(cm){}
            void draw_text(cppp::sv text,cppp::fvec2& pos,float scale,cppp::fvec3 color) const{
                tr.draw_text(text,pos,scale,color,cf,cm);
            }
            void line(cppp::fvec2 spos,cppp::fvec3 scolor,cppp::fvec2 tpos,cppp::fvec3 tcolor) const{
                ld.line(cm.cvt_abs(spos),scolor,cm.cvt_abs(tpos),tcolor);
            }
            const sgl::CachedFont& font_cache() const{
                return cf;
            }
            const sgl::Font& font() const{
                return cf.font();
            }
    };
    using UICursor = Cursor<class VisualNode,class VisualFunctionNode>;
    class VisualNode{
        public:
            template<typename Self>
            cppp::erased_span<Self> children(this Self& v){
                return v.do_children().template downcast<Self>();
            }
            virtual void draw(const GraphicsContext& gc,const UICursor& cursor,cppp::fvec2& pos) const = 0;
            virtual bool fast() const = 0;
            virtual void rerender() = 0;
        protected:
            using children_t = cppp::erased_span<VisualNode>;
            using const_children_t = cppp::erased_span<const VisualNode>;
            virtual children_t do_children() = 0;
            virtual const_children_t do_children() const = 0;
            ~VisualNode(){}
    };
    class VisualASTNode : public VisualNode{
        bbe::ASTNode* nd;
        std::vector<VisualASTNode> _children;
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
                for(auto& c : find_child_array()){
                    _children.emplace_back(c);
                }
            }
            // hopefully CSE can eliminate all the calls to find_child_array()
            void populate(std::uint32_t i){
                _children.emplace_back(find_child_array()[i]);
            }
            void populate(VisualASTNode&& vn){
                _children.emplace_back(std::move(vn));
            }
            void rerender() override{
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
            const_children_t do_children() const{
                return std::span{_children};
            }
            children_t do_children(){
                return std::span{_children};
            }
    };
    class VisualFunctionNode : public VisualNode{
        bbe::Function* fn;
        VisualASTNode child;
        public:
            VisualFunctionNode(bbe::Function& f) : fn(&f), child(f.ast()){}
            void draw(const GraphicsContext& gc,const UICursor& cursor,cppp::fvec2& pos) const override;
            bool fast() const{
                return false;
            }
            void rerender() override{
                child.rerender();
            }
        protected:
            children_t do_children() override{
                return std::span<VisualASTNode,1uz>{&child,1uz};
            }
            const_children_t do_children() const override{
                return std::span<const VisualASTNode,1uz>{&child,1uz};
            }
    };
}
