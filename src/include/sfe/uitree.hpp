#pragma once
#include<bbe/project_entity_pool.hpp>
#include<bbe/function.hpp>
#include<cppp/variant.hpp>
#include<cppp/assert.hpp>
#include<bbe/ast.hpp>
#include<bbe/gc.hpp>
#include<type_traits>
#include<concepts>
#include<cstdint>
#include<ranges>
#include<vector>
#include"commons.hpp"
#include"graphics.hpp"
#include"name.hpp"
namespace sfe{
    #ifdef __INTELLISENSE__
    #define SFE_ANNOT(t)
    #else
    // GCC please stop doing this to me
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=127427
    #define SFE_ANNOT(t) [[=^^std::type_identity_t<t>]]
    #endif
    class UICursor;
    class ASTVisualNode{
        std::vector<ASTVisualNode> _children;
        mutable bbe::ASTNode* node;
        void populate(){
            for(bbe::ASTNode& c : node->children()){
                _children.emplace_back(c);
            }
        }
        void populate_reusing_first_child(ASTVisualNode&& vn){
            _children.emplace_back(std::move(vn));
            for(std::uint32_t i=1_u32;i<node->children().size();++i){
                _children.emplace_back(node->children()[i]);
            }
        }
        public:
            struct no_populate_t{};
            constexpr static no_populate_t no_populate{};
            ASTVisualNode(no_populate_t,bbe::ASTNode& n) : node(&n){}
            ASTVisualNode(bbe::ASTNode& n) : ASTVisualNode(no_populate,n){
                populate();
            }
            std::span<const ASTVisualNode> children() const{
                return _children;
            }
            std::span<ASTVisualNode> children(){
                return _children;
            }
            const bbe::ASTNode& get() const{
                return *node;
            }
            class NodeHandle{
                ASTVisualNode* vn;
                friend ASTVisualNode;
                void _destroy(){
                    if(vn){
                        vn->_children.clear();
                        vn->populate();
                    }
                }
                public:
                    NodeHandle(ASTVisualNode& vn) : vn(&vn){}
                    bbe::ASTNode& operator*() const{
                        return *vn->node;
                    }
                    bbe::ASTNode* operator->() const{
                        return vn->node;
                    }
                    bbe::ASTNode& release(){
                        return *std::exchange(vn,nullptr)->node;
                    }
                    NodeHandle(const NodeHandle&) = delete;
                    NodeHandle(NodeHandle&& other) : vn(std::exchange(other.vn,nullptr)){}
                    NodeHandle& operator=(const NodeHandle&) = delete;
                    NodeHandle& operator=(NodeHandle&& other){
                        ASTVisualNode* tmp = std::exchange(other.vn,nullptr);
                        _destroy();
                        vn = tmp;
                        return *this;
                    }
                    ~NodeHandle(){
                        _destroy();
                    }
            };
            NodeHandle modify(){
                return *this;
            }
            void erase(std::uint32_t i){
                node->children().erase(i);
                _children.erase(_children.begin()+i);
                for(std::uint32_t i=0;i<cppp::assume_cast<std::uint32_t>(_children.size());++i){
                    _children[i].moved(node->children()[i]);
                }
            }
            void moved(bbe::ASTNode& newloc) const{
                node = &newloc;
            }
            void repoint(bbe::ASTNode& nd){
                moved(nd);
                _children.clear();
                populate();
            }
            void repoint_reusing_first_child(bbe::ASTNode& nd,ASTVisualNode&& fchl){
                moved(nd);
                _children.clear();
                populate_reusing_first_child(std::move(fchl));
            }
            void update(bbe::ASTNode&& an){
                *node = std::move(an);
                _children.clear();
                populate();
            }
            void update_fromnone(bbe::ASTNode&& an){
                CPPP_ASSERT(node->type() == bbe::NodeType::NTYPE);
                *node = std::move(an);
                populate();
            }
            void clear(){
                *node = {bbe::NodeType::NTYPE,0};
                _children.clear();
            }
            void promote(std::uint32_t indx){
                bbe::ASTNode tmp = std::move(node->children()[indx]);
                update(std::move(tmp));
            }
            void insert(std::uint32_t i,bbe::ASTNode&& nd){
                node->children().insert(i,std::move(nd));
                _children.emplace(_children.begin()+i,node->children()[i]);
                for(std::uint32_t i=0;i<cppp::assume_cast<std::uint32_t>(_children.size());++i){
                    _children[i].moved(node->children()[i]);
                }
            }
            void append(bbe::ASTNode&& nd){
                node->children().emplace(std::move(nd));
                _children.emplace_back(node->children().back());
                for(std::uint32_t i=0;i<cppp::assume_cast<std::uint32_t>(_children.size()-1uz);++i){
                    _children[i].moved(node->children()[i]);
                }
            }
            void setp32(std::uint32_t p32){
                node->setp32(p32);
            }
            void draw(const GraphicsContext&,const bbe::ErrorDatabase&,const NameDatabase&,const UICursor&,cppp::fvec2& pos,float right,float left,bool altmode) const;
    };
    struct vn_from_dt_t{} constexpr inline vn_from_dt;
    struct vn_from_unknown_t{} constexpr inline vn_from_unknown;
    enum class MTSCompoundTypeCategory{
        PACK,
        FUNCTION_POINTER,
        POINTER
    };
    enum class TypeVisualNodeType{
        DT SFE_ANNOT(const bbe::TypeInfo*),
        CT SFE_ANNOT(MTSCompoundTypeCategory)
    };
    class TypeVisualNode{
        std::vector<TypeVisualNode> _children;
        cppp::variant<TypeVisualNodeType> data;
        const bbe::TypeInfo*& m_dt(){
            return data.get<TypeVisualNodeType::DT>();
        }
        MTSCompoundTypeCategory& m_ct(){
            return data.get<TypeVisualNodeType::CT>();
        }
        void populate(const bbe::TypeInfo& inf){
            switch(inf.type()){
                case bbe::TypeCategory::DTYPE:
                    data.emplace<TypeVisualNodeType::DT>(&inf);
                    break;
                case bbe::TypeCategory::PACK:
                    data.emplace<TypeVisualNodeType::CT>(MTSCompoundTypeCategory::PACK);
                    for(const auto& c : inf.pack_contents()){
                        _children.emplace_back(vn_from_unknown,c);
                    }
                    break;
                case bbe::TypeCategory::POINTER:
                    data.emplace<TypeVisualNodeType::CT>(MTSCompoundTypeCategory::POINTER);
                    _children.emplace_back(vn_from_unknown,inf.pointee());
                    break;
                case bbe::TypeCategory::FUNCTION_POINTER:
                    data.emplace<TypeVisualNodeType::CT>(MTSCompoundTypeCategory::FUNCTION_POINTER);
                    _children.emplace_back(vn_from_unknown,inf.function_signature().return_type());
                    for(const bbe::TypeInfo& par : inf.function_signature().parameters()){
                        _children.emplace_back(vn_from_unknown,par);
                    }
                    break;
            }
        }
        void populate_uninit(const bbe::TypeInfo& inf){
            switch(inf.type()){
                case bbe::TypeCategory::DTYPE:
                    data.initialize<TypeVisualNodeType::DT>(&inf);
                    break;
                case bbe::TypeCategory::PACK:
                    data.initialize<TypeVisualNodeType::CT>(MTSCompoundTypeCategory::PACK);
                    for(const auto& c : inf.pack_contents()){
                        _children.emplace_back(vn_from_unknown,c);
                    }
                    break;
                case bbe::TypeCategory::POINTER:
                    data.initialize<TypeVisualNodeType::CT>(MTSCompoundTypeCategory::POINTER);
                    _children.emplace_back(vn_from_unknown,inf.pointee());
                    break;
                case bbe::TypeCategory::FUNCTION_POINTER:
                    data.initialize<TypeVisualNodeType::CT>(MTSCompoundTypeCategory::FUNCTION_POINTER);
                    _children.emplace_back(vn_from_unknown,inf.function_signature().return_type());
                    for(const bbe::TypeInfo& par : inf.function_signature().parameters()){
                        _children.emplace_back(vn_from_unknown,par);
                    }
                    break;
            }
        }
        public:
            TypeVisualNode(vn_from_dt_t) : data(cppp::in_place_etor<TypeVisualNodeType::DT>,nullptr){}
            TypeVisualNode(vn_from_dt_t,const bbe::TypeInfo& t) : data(cppp::in_place_etor<TypeVisualNodeType::DT>,&t){}
            TypeVisualNode(MTSCompoundTypeCategory mctc) : data(cppp::in_place_etor<TypeVisualNodeType::CT>,mctc){}
            TypeVisualNode(vn_from_unknown_t,const bbe::TypeInfo& t) : data(cppp::uninitialize){
                populate_uninit(t);
            }
            std::span<const TypeVisualNode> children() const{
                return _children;
            }
            std::span<TypeVisualNode> children(){
                return _children;
            }
            TypeVisualNodeType type() const{
                return data.tag();
            }
            const bbe::TypeInfo* dt() const{
                return data.get<TypeVisualNodeType::DT>();
            }
            MTSCompoundTypeCategory ct() const{
                return data.get<TypeVisualNodeType::CT>();
            }
            void cttodblank(){
                CPPP_ASSERT(data.tag() == TypeVisualNodeType::CT);
                data.emplace<TypeVisualNodeType::DT>(nullptr);
                _children.clear();
            }
            void dtblank(){
                m_dt() = nullptr;
                _children.clear();
            }
            void ctappend(TypeVisualNode&& nd){
                _children.emplace_back(std::move(nd));
            }
            void dtset(const bbe::TypeInfo& dt){
                CPPP_ASSERT(dt.type() == bbe::TypeCategory::DTYPE);
                m_dt() = &dt;
            }
            void update(const bbe::TypeInfo& inf){
                _children.clear();
                populate(inf);
            }
            const bbe::TypeInfo* compute(const bbe::TypeDatabase& tdb) const{
                switch(data.tag()){
                    case TypeVisualNodeType::CT:
                        switch(ct()){
                            case MTSCompoundTypeCategory::FUNCTION_POINTER: {
                                if(const bbe::TypeInfo* rt = _children[0uz].compute(tdb)){
                                    if(const bbe::TypeInfo* pt = _children[1uz].compute(tdb)){
                                        return &tdb.function_of({*rt,{*pt}});
                                    }else return nullptr;
                                }else return nullptr;
                            }
                            case MTSCompoundTypeCategory::PACK: {
                                bbe::TypePackBuilder tpb{_children.size()};
                                for(std::size_t i=0uz;i<_children.size();++i){
                                    if(const bbe::TypeInfo* t = _children[i].compute(tdb)){
                                        tpb.emplace(i,*t);
                                    }else{
                                        tpb.abandon(i);
                                        return nullptr;
                                    }
                                }
                                return &tdb.pack_of(std::move(tpb));
                            }
                            case MTSCompoundTypeCategory::POINTER: {
                                if(const bbe::TypeInfo* c0 = _children[0uz].compute(tdb)){
                                    return &tdb.pointer_to(*c0);
                                }else return nullptr;
                            }
                        }
                        cppp::unreachable();
                    case TypeVisualNodeType::DT:
                        return dt();
                    default: cppp::unreachable();
                }
            }
            void cterase(std::uint32_t i){
                CPPP_ASSERT(data.tag() == TypeVisualNodeType::CT);
                _children.erase(_children.begin()+i);
            }
            void ctpromote(std::uint32_t indx){
                CPPP_ASSERT(data.tag() == TypeVisualNodeType::CT);
                TypeVisualNode tmp{std::move(_children[indx])};
                *this = std::move(tmp);
            }
            void draw(const GraphicsContext&,const bbe::ErrorDatabase&,const NameDatabase&,const UICursor&,cppp::fvec2& pos,float right,float left,bool altmode) const;
    };
    class FunctionVisualNode{
        TypeVisualNode retv;
        ASTVisualNode _body;
        std::vector<TypeVisualNode> parv;
        mutable bbe::Function* func;
        public:
            FunctionVisualNode(bbe::Function& f) : retv(vn_from_unknown,f.signature().return_type()), _body(f.ast()), func(&f){
                for(const bbe::TypeInfo& par : f.signature().parameters()){
                    parv.emplace_back(vn_from_unknown,par);
                }
            }
            std::span<const TypeVisualNode> parameters() const{
                return parv;
            }
            std::span<TypeVisualNode> parameters(){
                return parv;
            }
            const TypeVisualNode& return_type() const{
                return retv;
            }
            TypeVisualNode& return_type(){
                return retv;
            }
            const ASTVisualNode& body() const{
                return _body;
            }
            ASTVisualNode& body(){
                return _body;
            }
            const bbe::Function& get() const{
                return *func;
            }
            void twriteback(const bbe::TypeDatabase& tdb){
                if(const bbe::TypeInfo* rt=retv.compute(tdb)){
                    func->signature().set_return(*rt);
                }
                bbe::TypePackBuilder ptvb{parv.size()};
                for(std::size_t i=0uz;i<parv.size();++i){
                    if(const bbe::TypeInfo* pt=parv[i].compute(tdb)){
                        ptvb.emplace(i,*pt);
                    }else{
                        ptvb.abandon(i);
                        return;
                    }
                }
                func->signature().parameters() = std::move(ptvb);
            }
            void setp(std::size_t i,const bbe::TypeInfo& r){
                parv[i].update(r);
            }
            void addp(const bbe::TypeInfo& r){
                parv.emplace_back(vn_from_unknown,r);
            }
            void setr(const bbe::TypeInfo& r){
                func->signature().set_return(r);
                retv.update(r);
            }
            void traceself(bbe::EntitySweeper& swp) const{
                swp.trace_function(func);
                _body.moved(func->ast());
            }
            cppp::str& cname(){
                return func->cname();
            }
            void draw(const GraphicsContext&,const bbe::ErrorDatabase&,const NameDatabase&,const UICursor&,cppp::fvec2& pos,bool altmode) const;
    };
    class OverloadSetVisualNode{
        std::vector<FunctionVisualNode> _children;
        bbe::OverloadSet* os;
        public:
            OverloadSetVisualNode(bbe::OverloadSet& o) : os(&o){
                for(const bbe::Function& f : o){
                    _children.emplace_back(const_cast<bbe::Function&>(f));
                }
            }
            std::span<const FunctionVisualNode> children() const{
                return _children;
            }
            std::span<FunctionVisualNode> children(){
                return _children;
            }
            const bbe::OverloadSet& get() const{
                return *os;
            }
            void addf(bbe::Function& fn){
                os->add(fn);
                _children.emplace_back(fn);
            }
            void erasef(const bbe::Function& fn){
                os->remove(fn);
                #if __cpp_lib_parallel_algorithm >= 202506L
                #warning GCC updated! change this to use std::ranges::find_if
                #endif
                _children.erase(std::find_if(std::execution::unseq,_children.begin(),_children.end(),[p=&fn](const FunctionVisualNode& vn){
                    return &vn.get() == p;
                }));
            }
            void tracef(bbe::EntitySweeper& swp) const{
                for(const FunctionVisualNode& f : _children){
                    f.traceself(swp);
                }
            }
            void draw(const GraphicsContext&,const bbe::ErrorDatabase&,const NameDatabase&,const UICursor&,cppp::fvec2& pos,bool altmode) const;
    };
    class ProjectVisualNode{
        std::vector<OverloadSetVisualNode> _children;
        bbe::ProjectEntitiesPool* proj;
        public:
            ProjectVisualNode(bbe::ProjectEntitiesPool& p) : proj(&p){
                for(bbe::OverloadSet& os : p.overloads()){
                    _children.emplace_back(os);
                }
            }
            std::span<const OverloadSetVisualNode> children() const{
                return _children;
            }
            std::span<OverloadSetVisualNode> children(){
                return _children;
            }
            const bbe::ProjectEntitiesPool& get() const{
                return *proj;
            }
            void ptracef(bbe::EntitySweeper& swp) const{
                for(const OverloadSetVisualNode& o : _children){
                    o.tracef(swp);
                }
            }
            void addo(bbe::OverloadSet& os){
                _children.emplace_back(os);
            }
            void eraseo(const bbe::OverloadSet& os){
                #if __cpp_lib_parallel_algorithm >= 202506L
                #warning GCC updated! change this to use std::ranges::find_if
                #endif
                _children.erase(std::find_if(std::execution::unseq,_children.begin(),_children.end(),[p=&os](const OverloadSetVisualNode& vn){
                    return &vn.get() == p;
                }));
            }
            void repopulate(){
                _children.clear();
                for(bbe::OverloadSet& os : proj->overloads()){
                    _children.emplace_back(os);
                }
            }
            void draw(const GraphicsContext&,const bbe::ErrorDatabase&,const NameDatabase&,const UICursor&,cppp::fvec2& pos,bool altmode) const;
    };
    enum class VisualNodeType{
        A, T, O, F, P
    };
    namespace detail{
        template<bool const_>
        class VisualNodeReference{
            template<typename T>
            using qualify = std::conditional_t<const_,const T,T>;
            qualify<void>* data;
            VisualNodeType tag;
            VisualNodeReference(qualify<void>* d,VisualNodeType t) : data(d), tag(t){}
            friend VisualNodeReference<false>;
            public:
                VisualNodeReference(qualify<ASTVisualNode>& avn) : data(&avn), tag(VisualNodeType::A){}
                VisualNodeReference(qualify<TypeVisualNode>& tvn) : data(&tvn), tag(VisualNodeType::T){}
                VisualNodeReference(qualify<FunctionVisualNode>& fvn) : data(&fvn), tag(VisualNodeType::F){}
                VisualNodeReference(qualify<OverloadSetVisualNode>& ovn) : data(&ovn), tag(VisualNodeType::O){}
                VisualNodeReference(qualify<ProjectVisualNode>& pvn) : data(&pvn), tag(VisualNodeType::P){}
                VisualNodeType type() const{
                    return tag;
                }
                operator VisualNodeReference<true>() const requires(!const_){
                    return {data,tag};
                }
                qualify<ASTVisualNode>& a() const{
                    CPPP_ASSERT(tag == VisualNodeType::A);
                    return *static_cast<qualify<ASTVisualNode>*>(data);
                }
                qualify<TypeVisualNode>& t() const{
                    CPPP_ASSERT(tag == VisualNodeType::T);
                    return *static_cast<qualify<TypeVisualNode>*>(data);
                }
                qualify<OverloadSetVisualNode>& o() const{
                    CPPP_ASSERT(tag == VisualNodeType::O);
                    return *static_cast<qualify<OverloadSetVisualNode>*>(data);
                }
                qualify<FunctionVisualNode>& f() const{
                    CPPP_ASSERT(tag == VisualNodeType::F);
                    return *static_cast<qualify<FunctionVisualNode>*>(data);
                }
                qualify<ProjectVisualNode>& p() const{
                    CPPP_ASSERT(tag == VisualNodeType::P);
                    return *static_cast<qualify<ProjectVisualNode>*>(data);
                }
                bool is(const ASTVisualNode& n) const{ return tag == VisualNodeType::A && data == &n; }
                bool is(const TypeVisualNode& n) const{ return tag == VisualNodeType::T && data == &n; }
                bool is(const FunctionVisualNode& n) const{ return tag == VisualNodeType::F && data == &n; }
                bool is(const OverloadSetVisualNode& n) const{ return tag == VisualNodeType::O && data == &n; }
                bool is(const ProjectVisualNode& n) const{ return tag == VisualNodeType::P && data == &n; }
                std::size_t num_children() const{
                    switch(tag){
                        case VisualNodeType::A:
                            return a().children().size();
                        case VisualNodeType::T:
                            return t().children().size();
                        case VisualNodeType::F:
                            return f().parameters().size() + 2uz;
                        case VisualNodeType::O:
                            return o().children().size();
                        case VisualNodeType::P:
                            return p().children().size();
                    }
                    cppp::unreachable();
                }
                VisualNodeReference nth_child(std::size_t i) const{
                    switch(tag){
                        case VisualNodeType::A:
                            return a().children()[i];
                        case VisualNodeType::T:
                            return t().children()[i];
                        case VisualNodeType::F: {
                            std::size_t npar = f().parameters().size();
                            if(i < npar) return f().parameters()[i];
                            if(i == npar) return f().return_type();
                            CPPP_ASSERT(i == npar+1uz);
                            return f().body();
                        }
                        case VisualNodeType::O:
                            return o().children()[i];
                        case VisualNodeType::P:
                            return p().children()[i];
                    }
                    cppp::unreachable();
                }
                bool is_placeholder() const{
                    return (tag == VisualNodeType::A && static_cast<ASTVisualNode*>(data)->get().type() == bbe::NodeType::NTYPE)
                        || (tag == VisualNodeType::T && static_cast<TypeVisualNode*>(data)->type() == TypeVisualNodeType::DT
                                                     && !static_cast<TypeVisualNode*>(data)->dt());
                }
        };
    }
    using VisualNodeReference = detail::VisualNodeReference<true>;
    using MutableVisualNodeReference = detail::VisualNodeReference<false>;
    struct PathEntry{
        MutableVisualNodeReference p;
        std::uint32_t index;
    };
    class Breadcrumbs{
        ProjectVisualNode _root;
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
            Breadcrumbs(ProjectVisualNode&& root) : _root(std::move(root)){}
            ProjectVisualNode& root(){
                return _root;
            }
            const ProjectVisualNode& root() const{
                return _root;
            }
            void home(){
                path.clear();
            }
            bool has_nesting() const{
                return !path.empty();
            }
            VisualNodeReference top() const{
                return has_nesting()?VisualNodeReference(etop().p):_root;
            }
            MutableVisualNodeReference top(){
                return has_nesting()?etop().p:_root;
            }
            std::uint32_t top_index() const{
                return etop().index;
            }
            VisualNodeReference below_top() const{
                CPPP_ASSERT(has_nesting());
                return path.size()>1uz?VisualNodeReference(path[path.size()-2uz].p):_root;
            }
            MutableVisualNodeReference below_top(){
                CPPP_ASSERT(has_nesting());
                return path.size()>1uz?path[path.size()-2uz].p:_root;
            }
            void leave(){
                path.pop_back();
            }
            void leave_opt(){
                if(has_nesting()) path.pop_back();
            }
            void enter(std::uint32_t index){
                path.emplace_back(top().nth_child(index),index);
            }
            bool is_first_child() const{
                return etop().index == 0_u32;
            }
            bool is_last_child() const{
                return etop().index+1_u32 == below_top().num_children();
            }
            void prev_sibling(){
                etop().p = below_top().nth_child(--etop().index);
            }
            void next_sibling(){
                etop().p = below_top().nth_child(++etop().index);
            }
    };
    class UICursor{
        Breadcrumbs crumbs;
        bool after;
        public:
            UICursor(ProjectVisualNode&& root) : crumbs(std::move(root)), after(false){}
            const std::vector<PathEntry>& path_elements() const{
                return crumbs.elements();
            }
            const ProjectVisualNode& root() const{
                return crumbs.root();
            }
            ProjectVisualNode& root(){
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
            VisualNodeReference selected() const{
                return crumbs.top();
            }
            MutableVisualNodeReference selected(){
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
            VisualNodeReference selected2() const{
                return crumbs.below_top();
            }
            MutableVisualNodeReference selected2(){
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
