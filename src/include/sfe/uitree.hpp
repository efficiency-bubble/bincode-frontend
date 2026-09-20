#pragma once
#include<cppp/type-erasure.hpp>
#include<bbe/project_entity_pool.hpp>
#include<bbe/function.hpp>
#include<cppp/variant.hpp>
#include<bbe/ast.hpp>
#include<type_traits>
#include<concepts>
#include<cstdint>
#include<ranges>
#include<vector>
#include"graphics.hpp"
#include"style.hpp"
namespace sfe{
    #ifdef __INTELLISENSE__
    #define SFE_ANNOT(t)
    #else
    // GCC please stop doing this to me
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=127427
    #define SFE_ANNOT(t) [[=^^std::type_identity_t<t>]]
    #endif
    enum class VisualNodeType{
        A SFE_ANNOT(bbe::ASTNode*),
        F SFE_ANNOT(bbe::Function*),
        P SFE_ANNOT(bbe::ProjectEntitiesPool*),
        T SFE_ANNOT(const bbe::TypeInfo*)
    };
    class UICursor;
    class VisualNode{
        std::vector<VisualNode> _children;
        cppp::variant<VisualNodeType> data;
        friend void swap(VisualNode& lhs,VisualNode& rhs){
            std::ranges::swap(lhs._children,rhs._children);
            std::ranges::swap(lhs.data,rhs.data);
        }
        struct no_populate_t{};
        bbe::ASTNode& m_a(){
            return *data.get<VisualNodeType::A>();
        }
        bbe::Function& m_f(){
            return *data.get<VisualNodeType::F>();
        }
        bbe::ProjectEntitiesPool& m_p(){
            return *data.get<VisualNodeType::P>();
        }
        const bbe::TypeInfo*& m_t(){
            return data.get<VisualNodeType::T>();
        }
        void apopulate(){
            for(auto& c : m_a().children()){
                _children.emplace_back(c);
            }
        }
        void apopulate_reusing_first_child(VisualNode&& vn){
            _children.emplace_back(std::move(vn));
            for(std::uint32_t i=1;i<m_a().children().size();++i){
                _children.emplace_back(m_a().children()[i]);
            }
        }
        public:
            constexpr static no_populate_t no_populate{};
            VisualNode(bbe::ASTNode& nd,no_populate_t) : data(cppp::in_place_etor<VisualNodeType::A>,&nd){}
            VisualNode(bbe::ASTNode& nd) : VisualNode(nd,no_populate){
                apopulate();
            }
            VisualNode(bbe::Function& f) : data(cppp::in_place_etor<VisualNodeType::F>,&f){
                _children.emplace_back(f.signature().parameter());
                _children.emplace_back(f.signature().return_type());
                _children.emplace_back(f.ast());
            }
            VisualNode(bbe::ProjectEntitiesPool& f) : data(cppp::in_place_etor<VisualNodeType::P>,&f){
                for(bbe::Function& fn : f.functions()){
                    _children.emplace_back(fn);
                }
            }
            VisualNode(const bbe::TypeInfo& t) : data(cppp::in_place_etor<VisualNodeType::T>,&t){}
            VisualNode(VisualNode&& other) = default;
            VisualNode(const VisualNode&) = delete;
            VisualNode& operator=(VisualNode&& other) = default;
            VisualNode& operator=(const VisualNode&) = delete;
            const std::vector<VisualNode>& children() const{
                return _children;
            }
            std::vector<VisualNode>& children(){
                return _children;
            }
            void adraw(const GraphicsContext&,const bbe::ErrorDatabase&,const NameDatabase&,const UICursor&,cppp::fvec2& pos,float right,float left,bool altmode) const;
            void tdraw(const GraphicsContext&,const bbe::ErrorDatabase&,const NameDatabase&,const UICursor&,cppp::fvec2& pos,float right,float left,bool altmode) const;
            void fdraw(const GraphicsContext&,const bbe::ErrorDatabase&,const NameDatabase&,const UICursor&,cppp::fvec2& pos,bool altmode) const;
            void pdraw(const GraphicsContext&,const bbe::ErrorDatabase&,const NameDatabase&,const UICursor&,cppp::fvec2& pos,bool altmode) const;
            void clear(){
                _children.clear();
            }
            const bbe::ASTNode& a() const{
                return *data.get<VisualNodeType::A>();
            }
            const bbe::Function& f() const{
                return *data.get<VisualNodeType::F>();
            }
            cppp::str& fcname(){
                return m_f().cname();
            }
            const bbe::ProjectEntitiesPool& p() const{
                return *data.get<VisualNodeType::P>();
            }
            const bbe::TypeInfo* t() const{
                return data.get<VisualNodeType::T>();
            }
            void treset(){
                m_t() = nullptr;
                clear();
            }
            void tupdate(const bbe::TypeInfo& inf){
                m_t() = &inf;
                clear();
                switch(inf.type()){
                    case bbe::TypeCategory::VOID:
                    case bbe::TypeCategory::SIGNED_INTEGRAL:
                    case bbe::TypeCategory::UNSIGNED_INTEGRAL:
                        break;
                    case bbe::TypeCategory::PACK:
                        for(const auto& c : t()->pack_contents()){
                            _children.emplace_back(c);
                        }
                        break;
                    case bbe::TypeCategory::POINTER:
                        _children.emplace_back(t()->pointee());
                        break;
                    case bbe::TypeCategory::FUNCTION_POINTER:
                        _children.emplace_back(t()->function_signature().return_type());
                        _children.emplace_back(t()->function_signature().parameter());
                        break;
                }
            }
            void ftwriteback(){
                if(const bbe::TypeInfo* at=_children[0uz].t()){
                    m_f().signature().set_param(*at);
                }
                if(const bbe::TypeInfo* rt=_children[1uz].t()){
                    m_f().signature().set_param(*rt);
                }
            }
            void fsetp(const bbe::TypeInfo& r){
                m_f().signature().set_param(r);
                _children[0uz].tupdate(r);
            }
            void fsetr(const bbe::TypeInfo& r){
                m_f().signature().set_return(r);
                _children[1uz].tupdate(r);
            }
            void paddf(bbe::Function& fn){
                CPPP_ASSERT(data.tag() == VisualNodeType::P);
                _children.emplace_back(fn);
            }
            void perasef(const bbe::Function& fn){
                CPPP_ASSERT(data.tag() == VisualNodeType::P);
                #if __cpp_lib_parallel_algorithm >= 202506L
                #warning GCC updated! change this to use std::ranges::find_if
                #endif
                _children.erase(std::find_if(std::execution::unseq,_children.begin(),_children.end(),[p=&fn](const VisualNode& vn){
                    return &vn.f() == p;
                }));
            }
            std::uint32_t apriority() const;
            class ANodeHandle{
                VisualNode* vn;
                friend VisualNode;
                void _destroy(){
                    if(vn){
                        vn->clear();
                        vn->apopulate();
                    }
                }
                public:
                    ANodeHandle(VisualNode& vn) : vn(&vn){}
                    bbe::ASTNode& operator*() const{
                        return vn->m_a();
                    }
                    bbe::ASTNode* operator->() const{
                        return &vn->m_a();
                    }
                    bbe::ASTNode& release(){
                        return std::exchange(vn,nullptr)->m_a();
                    }
                    ANodeHandle(const ANodeHandle&) = delete;
                    ANodeHandle(ANodeHandle&& other) : vn(std::exchange(other.vn,nullptr)){}
                    ANodeHandle& operator=(const ANodeHandle&) = delete;
                    ANodeHandle& operator=(ANodeHandle&& other){
                        VisualNode* tmp = std::exchange(other.vn,nullptr);
                        _destroy();
                        vn = tmp;
                        return *this;
                    }
                    ~ANodeHandle(){
                        _destroy();
                    }
            };
            ANodeHandle amodify(){
                return *this;
            }
            void aerase(std::uint32_t i){
                m_a().children().erase(i);
                _children.erase(_children.begin()+i);
            }
            void ainsert(std::uint32_t i,bbe::ASTNode&& nd){
                m_a().children().insert(i,std::move(nd));
                _children.emplace(_children.begin()+i,m_a().children()[i]);
            }
            void amoved(bbe::ASTNode& newloc){
                data.emplace<VisualNodeType::A>(&newloc);
            }
            void arepoint(bbe::ASTNode& nd){
                amoved(nd);
                clear();
                apopulate();
            }
            void arepoint_reusing_first_child(bbe::ASTNode& nd,VisualNode&& fchl){
                amoved(nd);
                clear();
                apopulate_reusing_first_child(std::move(fchl));
            }
            void aupdate(bbe::ASTNode&& an){
                m_a() = std::move(an);
                clear();
                apopulate();
            }
            void aupdate_fromnone(bbe::ASTNode&& an){
                CPPP_ASSERT(a().type() == bbe::NodeType::NTYPE);
                m_a() = std::move(an);
                apopulate();
            }
            void aclear(){
                m_a() = {bbe::NodeType::NTYPE,0};
                clear();
            }
            void apromote(std::uint32_t indx){
                bbe::ASTNode tmp = std::move(m_a().children()[indx]);
                aupdate(std::move(tmp));
            }
            void asetp32(std::uint32_t p32){
                m_a().setp32(p32);
            }
            // TODO: remove this
            void prepopulate(){
                _children.clear();
                for(auto& fn : m_p().functions()){
                    _children.emplace_back(fn);
                }
            }
            bool is_placeholder() const{
                return (data.tag() == VisualNodeType::A && a().type() == bbe::NodeType::NTYPE)
                    || (data.tag() == VisualNodeType::T && !t());
            }
            VisualNodeType type() const{
                return data.tag();
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
