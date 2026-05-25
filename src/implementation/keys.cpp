#include<sfe/keys.hpp>
#include<sfe/uitree.hpp>
#include<sfe/editor.hpp>
namespace sfe{
    static void steal_lhs(VisualASTNode& ui,bbe::ASTNode&& node){
        cppp::swap(node,ui.node());
        // node is now old
        VisualASTNode old_ui{ui.node(),VisualASTNode::no_populate};
        cppp::swap(old_ui,ui);
        
        old_ui.repoint(ui.node().children()[0] = std::move(node));
        old_ui.rerender_children();
        ui.populate(std::move(old_ui));
    }
    static void builtin_n_ary(VisualASTNode& sel,bbe::NodeType nt,std::uint32_t prim,CodeEntry& ed,std::uint32_t arity){
        bool second = (prim > 1) && (sel.type() != bbe::NodeType::NTYPE);
        steal_lhs(sel,{nt,prim,arity});
        for(std::uint32_t i=1;i<arity;++i){
            sel.node().children()[i] = {bbe::NodeType::NTYPE};
        }
        sel.rerender_except_first();
        ed.subst_sel(sel);
        ed.enter(second,false);
    }
    bool NodeKeyConfig::handle(CodeEntry& e,Keypress k) const{
        if(auto* n=dynamic_cast<VisualASTNode*>(&e.selected())){
            if(n->node().type() == bbe::NodeType::NTYPE){
                if(auto it=replace.find(k);it!=replace.end()){
                    n->node() = {it->second.nt,it->second.prim,it->second.arity};
                    for(std::uint32_t i=0;i<it->second.arity;++i){
                        n->node().children()[i] = {bbe::NodeType::NTYPE};
                    }
                    n->rerender_children();
                    e.set_select_after(true);
                    return true;
                }
            }
            if(auto it=suffix.find(k);it!=suffix.end()){
                builtin_n_ary(*n,it->second.nt,it->second.prim,e,it->second.arity);
                return true;
            }
        }
        return false;
    }
}
