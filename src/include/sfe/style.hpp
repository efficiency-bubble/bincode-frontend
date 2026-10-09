#pragma once
#include<cppp/string.hpp> // for entity names
#include<cppp/vector.hpp>
#include<cppp/object-view.hpp>
#include<cppp/bytearray.hpp>
#include<bbe/project_entity_pool.hpp>
#include<unordered_map>
#include<optional>
#include<cstdint>
namespace sfe{
    class Name{
        cppp::str name;
        cppp::fvec3 chroma;
        public:
            Name(cppp::str name,cppp::fvec3 chroma) : name(std::move(name)), chroma(chroma){}
            const cppp::str& identifier() const{
                return name;
            }
            cppp::str& identifier(){
                return name;
            }
            cppp::fvec3 color() const{
                return chroma;
            }
            cppp::fvec3& color(){
                return chroma;
            }
    };
    class NameDatabase{
        struct eq_mtk{
            bool operator()(bbe::TraceableReference<bbe::TypeInfo> tr,bbe::TraceableReference<bbe::TypeInfo> tr2) const{
                return tr == tr2;
            }
            bool operator()(bbe::TraceableReference<bbe::TypeInfo> tr,const bbe::TypeInfo& ti) const{
                return tr.get() == &ti;
            }
            using is_transparent = void;
        };
        struct hash_mtk{
            constexpr std::size_t operator()(bbe::TraceableReference<bbe::TypeInfo> r) const noexcept{
                return static_cast<std::size_t>(r->hash().value());
            }
            constexpr std::size_t operator()(const bbe::TypeInfo& i) const noexcept{
                return static_cast<std::size_t>(i.hash().value());
            }
            using is_transparent = void;
        };
        std::unordered_map<bbe::func_id,Name> fnames;
        std::unordered_map<bbe::TraceableReference<bbe::TypeInfo>,Name,hash_mtk,eq_mtk> dtnames;
        public:
            NameDatabase() = default;
            NameDatabase(const bbe::TypeDatabase&,cppp::frozen_byte_view&);
            void update(const bbe::EntitySweeper& swp){
                {
                    auto it = dtnames.begin();
                    const auto done = dtnames.end();
                    while(it != done){
                        if(swp.is_type_marked(*it->first)){
                            swp.update_type_ref_to_new_location(it->first);
                            ++it;
                        }else{
                            it = dtnames.erase(it);
                        }
                    }
                }
                std::unordered_map<bbe::func_id,Name> fnames2;
                {
                    auto it = fnames.begin();
                    const auto done = fnames.end();
                    while(it != done){
                        auto node{fnames.extract(it++)};
                        if(swp.is_function_marked(node.key())){
                            node.key() = swp.new_function_location(node.key());
                            fnames2.insert(std::move(node));
                        }
                    }
                }
                fnames = std::move(fnames2);
            }
            void name_function(bbe::func_id fid,Name n){
                fnames.try_emplace(fid,std::move(n));
            }
            void name_defined_type(const bbe::TypeInfo& ti,Name n){
                dtnames.try_emplace(ti,std::move(n));
            }
            const Name& get_function_name(bbe::func_id fid) const{
                return fnames.at(fid);
            }
            Name& get_function_name(bbe::func_id fid){
                return fnames.at(fid);
            }
            std::optional<const Name&> optget_function_name(bbe::func_id fid) const;
            const Name& get_defined_type_name(const bbe::TypeInfo& t) const{
                return dtnames.at(t);
            }
            cppp::str display_type_name(const bbe::TypeInfo& ti) const;
            void serialize(cppp::bytes&) const;
    };
}
