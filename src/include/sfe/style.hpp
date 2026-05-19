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
        public:
            NameDatabase(){}
            NameDatabase(cppp::frozen_byte_view&);
            void name_function(bbe::func_id fid,cppp::str s){
                fnames.try_emplace(fid,std::move(s));
            }
            const cppp::str& get_function_name(bbe::func_id fid) const{
                return fnames.at(fid);
            }
            cppp::str display_function_name(bbe::func_id id) const;
            void serialize(cppp::bytes&) const;
    };
}
