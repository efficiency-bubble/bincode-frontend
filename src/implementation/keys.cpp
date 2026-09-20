#include<sfe/keys.hpp>
#include<sfe/uitree.hpp>
#include<sfe/editor.hpp>
#include<concepts>
namespace sfe{
    static void steal_lhs(VisualNode& outervn,bbe::ASTNode&& node){
        VisualNode oldvn{std::move(outervn)};
        bbe::ASTNode& outeran = oldvn.amodify().release();
        bbe::ASTNode oldan{std::exchange(outeran,std::move(node))};
        outeran.children()[0uz].initialize(std::move(oldan));
        oldvn.amoved(outeran.children()[0uz]);
        outervn.arepoint_reusing_first_child(outeran,std::move(oldvn));
    }
    static void builtin_n_ary(VisualNode& sel,bbe::NodeType nt,std::uint32_t prim,CodeEntry& ed,std::uint32_t arity){
        bool second = (arity > 1) && (sel.a().type() != bbe::NodeType::NTYPE);
        steal_lhs(sel,{nt,prim,arity,bbe::null_initialize});
        ed.cursor().enter(second,false);
    }
    bool NodeKeyConfig::handle(CodeEntry& e,Keypress k) const{
        if(e.cursor().selected().type() == VisualNodeType::A){
            if(e.cursor().selected().a().type() == bbe::NodeType::NTYPE){
                if(auto it=replace.find(k);it!=replace.end()){
                    if(it->second.arity){
                        e.cursor().selected().aupdate_fromnone({it->second.nt,it->second.prim,it->second.arity,bbe::null_initialize});
                        e.cursor().enter(0,false);
                    }else{
                        e.cursor().selected().aupdate_fromnone({it->second.nt,it->second.prim});
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
