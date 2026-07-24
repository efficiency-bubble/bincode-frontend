#include<sfe/keys.hpp>
#include<sfe/uitree.hpp>
#include<sfe/editor.hpp>
#include<concepts>
namespace sfe{
    static void steal_lhs(VisualNode& ui,bbe::ASTNode&& node){
        std::ranges::swap(node,ui.a());
        // node is now old
        VisualNode old_ui{ui.a(),VisualNode::no_populate};
        std::ranges::swap(old_ui,ui);
        
        old_ui.repoint(ui.a().children()[0] = std::move(node));
        old_ui.arerender();
        ui.apopulate(std::move(old_ui));
    }
    static void builtin_n_ary(VisualNode& sel,bbe::NodeType nt,std::uint32_t prim,CodeEntry& ed,std::uint32_t arity){
        sel.assert_a();
        bool second = (arity > 1) && (sel.a().type() != bbe::NodeType::NTYPE);
        steal_lhs(sel,{nt,prim,arity,bbe::null_initialize});
        sel.apopulate_butfirst();
        ed.cursor().enter(second,false);
    }
    bool NodeKeyConfig::handle(CodeEntry& e,Keypress k) const{
        if(e.cursor().selected().type() == VisualNodeType::A){
            bbe::ASTNode& n = e.cursor().selected().a();
            if(n.type() == bbe::NodeType::NTYPE){
                if(auto it=replace.find(k);it!=replace.end()){
                    if(it->second.arity){
                        n = {it->second.nt,it->second.prim,it->second.arity,bbe::null_initialize};
                        e.cursor().selected().arerender();
                        e.cursor().enter(0,false);
                    }else{
                        n = {it->second.nt,it->second.prim};
                        e.cursor().selected().arerender();
                        e.cursor().set_after(true);
                    }
                    return true;
                }
            }
            if(auto it=suffix.find(k);it!=suffix.end()){
                builtin_n_ary(e.cursor().selected(),it->second.nt,it->second.prim,e,it->second.arity);
                return true;
            }
        }
        return false;
    }
}
