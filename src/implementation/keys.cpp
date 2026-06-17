#include<sfe/keys.hpp>
#include<sfe/uitree.hpp>
#include<sfe/editor.hpp>
namespace sfe{
    static void steal_lhs(VisualNode& ui,bbe::ASTNode&& node){
        ui.assert_a();
        cppp::swap(node,ui.a());
        // node is now old
        VisualNode old_ui{ui.a(),VisualNode::no_populate};
        cppp::swap(old_ui,ui);
        
        old_ui.repoint(ui.a().children()[0] = std::move(node));
        old_ui.arerender();
        ui.apopulate(std::move(old_ui));
    }
    static void builtin_n_ary(VisualNode& sel,bbe::NodeType nt,std::uint32_t prim,CodeEntry& ed,std::uint32_t arity){
        sel.assert_a();
        bool second = (arity > 1) && (sel.a().type() != bbe::NodeType::NTYPE);
        (void)ed;
        steal_lhs(sel,{nt,prim,arity});
        for(std::uint32_t i=1;i<arity;++i){
            sel.a().children()[i] = {bbe::NodeType::NTYPE};
        }
        sel.apopulate_butfirst();
        ed.enter(second,false);
    }
    bool NodeKeyConfig::handle(CodeEntry& e,Keypress k) const{
        if(e.selected().type() == VisualNodeType::A){
            bbe::ASTNode& n = e.selected().a();
            if(n.type() == bbe::NodeType::NTYPE){
                if(auto it=replace.find(k);it!=replace.end()){
                    n = {it->second.nt,it->second.prim,it->second.arity};
                    for(std::uint32_t i=0;i<it->second.arity;++i){
                        n.children()[i] = {bbe::NodeType::NTYPE};
                    }
                    e.selected().arerender();
                    e.set_select_after(true);
                    return true;
                }
            }
            if(auto it=suffix.find(k);it!=suffix.end()){
                builtin_n_ary(e.selected(),it->second.nt,it->second.prim,e,it->second.arity);
                return true;
            }
        }
        return false;
    }
}
