#pragma once
#include<cppp/string.hpp> // for entity names
#include<cppp/object-view.hpp>
#include<cppp/bytearray.hpp>
#include<bbe/function.hpp>
#include<unordered_map>
#include<cstdint>
namespace sfe{
    class NameDatabase{
        template<typename Kt>
        using name_map_t = std::unordered_map<Kt,cppp::str>;
        name_map_t<bbe::func_id> fnames;
        name_map_t<bbe::type_id> dtnames;
        public:
            NameDatabase(){}
            NameDatabase(cppp::frozen_byte_view&);
            void name_function(bbe::func_id fid,cppp::str s){
                fnames.try_emplace(fid,std::move(s));
            }
            void name_defined_type(bbe::type_id tid,cppp::str s){
                fnames.try_emplace(tid,std::move(s));
            }
            const cppp::str& get_function_name(bbe::func_id fid) const{
                return fnames.at(fid);
            }
            cppp::str& get_function_name(bbe::func_id fid){
                return fnames.at(fid);
            }
            const cppp::str& get_defined_type_name(bbe::type_id tid) const{
                return dtnames.at(tid);
            }
            cppp::str display_function_name(bbe::func_id id) const;
            cppp::str display_type_name(const bbe::TypeInfo* ti) const;
            void serialize(cppp::bytes&) const;
    };
}
