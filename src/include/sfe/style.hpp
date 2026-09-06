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
        template<typename Kt>
        using name_map_t = std::unordered_map<Kt,Name>;
        name_map_t<bbe::func_id> fnames;
        name_map_t<bbe::type_id> dtnames;
        public:
            NameDatabase() = default;
            NameDatabase(cppp::frozen_byte_view&);
            void garbage_collect(const bbe::LinearMovingGarbageCollectedPool<bbe::TypeInfo>::Sweeper& swp){
                const auto end = dtnames.end();
                for(auto it=dtnames.begin();it!=end;){
                    name_map_t<bbe::type_id>::node_type node{dtnames.extract(it)};
                    const bbe::TypeInfo& type = swp.query(node.key());
                    if(type.marked()){
                        node.key() = type.index();
                        dtnames.insert(std::move(node));
                    }
                }
            }
            void name_function(bbe::func_id fid,Name n){
                fnames.try_emplace(fid,std::move(n));
            }
            void name_defined_type(bbe::type_id tid,Name n){
                fnames.try_emplace(tid,std::move(n));
            }
            const Name& get_function_name(bbe::func_id fid) const{
                return fnames.at(fid);
            }
            Name& get_function_name(bbe::func_id fid){
                return fnames.at(fid);
            }
            std::optional<const Name&> optget_function_name(bbe::func_id fid) const;
            const Name& get_defined_type_name(bbe::type_id tid) const{
                return dtnames.at(tid);
            }
            cppp::str display_type_name(const bbe::TypeInfo* ti) const;
            void serialize(cppp::bytes&,const bbe::SCM&) const;
    };
}
