#include<sfe/keys.hpp>
#include<sfe/uitree.hpp>
#include<sfe/editor.hpp>
#include<concepts>
namespace sfe{
    static void steal_lhs(ASTVisualNode& outervn,bbe::ASTNode&& node){
        ASTVisualNode oldvn{std::move(outervn)};
        bbe::ASTNode& outeran = oldvn.modify().release();
        bbe::ASTNode oldan{std::exchange(outeran,std::move(node))};
        outeran.children()[0_u32].initialize(std::move(oldan));
        oldvn.moved(outeran.children()[0_u32]);
        outervn.repoint_reusing_first_child(outeran,std::move(oldvn));
    }
    static void builtin_n_ary(ASTVisualNode& sel,bbe::NodeType nt,std::uint32_t prim,CodeEntry& ed,std::uint32_t arity){
        bool second = (arity > 1) && (sel.get().type() != bbe::NodeType::NTYPE);
        steal_lhs(sel,{nt,prim,arity,bbe::null_initialize});
        ed.cursor().enter(second,false);
    }
    bool NodeKeyConfig::handle(CodeEntry& e,Keypress k) const{
        if(e.cursor().selected().type() == VisualNodeType::A){
            if(e.cursor().selected().a().get().type() == bbe::NodeType::NTYPE){
                if(auto it=replace.find(k);it!=replace.end()){
                    if(it->second.arity){
                        e.cursor().selected().a().update_fromnone({it->second.nt,it->second.prim,it->second.arity,bbe::null_initialize});
                        e.cursor().enter(0,false);
                    }else{
                        e.cursor().selected().a().update_fromnone({it->second.nt,it->second.prim});
                        e.cursor().set_after(true);
                    }
                    return true;
                }
            }
            if(auto it=suffix.find(k);it!=suffix.end()){
                builtin_n_ary(e.cursor().selected().a(),it->second.nt,it->second.prim,e,it->second.arity);
                return true;
            }
        }
        return false;
    }
}
