#include<sfe/style.hpp>
#include<cppp/format.hpp>
#include<cppp/binary.hpp>
#include<span>
namespace sfe{
    cppp::str NameDatabase::display_function_name(std::uint32_t id) const{
        if(auto it=fnames.find(id);it!=fnames.end()){
            return it->second;
        }else{
            using namespace cppp::literals;
            return cppp::format<u8"[unknown function {}]"_ts>(id);
        }
    }
    NameDatabase::NameDatabase(cppp::frozen_byte_view& v){
        std::uint64_t nentries = cppp::read<std::uint64_t>(v);
        while(nentries--){
            bbe::func_id fid = cppp::read<bbe::func_id>(v);
            std::uint64_t ns = cppp::read<std::uint64_t>(v);
            fnames.try_emplace(fid,cppp::sv{std::start_lifetime_as_array<char8_t>(v.read(ns),ns),ns});
        }
    }
    void NameDatabase::serialize(cppp::bytes& b) const{
        b.appendl<std::uint64_t>(fnames.size());
        for(const auto& [k,v] : fnames){
            b.appendl<bbe::func_id>(k);
            b.appendl<std::uint64_t>(v.size());
            b.append(std::as_bytes(std::span{v}));
        }
    }
}
