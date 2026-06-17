#include<sfe/style.hpp>
#include<cppp/format.hpp>
#include<cppp/binary.hpp>
#include<span>
namespace sfe{
    // cppp::str NameDatabase::display_function_name(bbe::func_id id) const{
    //     if(auto it=fnames.find(id);it!=fnames.end()){
    //         return it->second.identifier();
    //     }else{
    //         using namespace cppp::literals;
    //         return cppp::format<u8"[unknown function {}]"_ts>(id);
    //     }
    // }
    std::optional<const Name&> NameDatabase::optget_function_name(bbe::func_id fid) const{
        if(auto it=fnames.find(fid);it!=fnames.end()){
            return it->second;
        }
        return std::nullopt;
    }
    cppp::str NameDatabase::display_type_name(const bbe::TypeInfo* ti) const{
        using namespace cppp::literals;
        using namespace std::literals;
        if(!ti) return u8"[error-type]"s;
        switch(ti->type()){
            using enum bbe::TypeCategory;
            case VOID:
                return u8"void"s;
            case SIGNED_INTEGRAL:
                if(ti->size() == 1){
                    return u8"bool"s; // TODO: special case bool better
                }else{
                    return cppp::format<u8"int{}_t"_ts>(ti->size()*8);
                }
            case UNSIGNED_INTEGRAL:
                return cppp::format<u8"uint{}_t"_ts>(ti->size()*8);
            case FUNCTION_POINTER: {
                const bbe::FunctionSignature& sig = ti->function_signature();
                return cppp::format<u8"{} => {}"_ts>(display_type_name(sig.parameter()),display_type_name(sig.return_type()));
            }
            case PACK: {
                cppp::str name{u8"pack["s};
                for(const bbe::TypeInfo* t : ti->pack_contents().types()){
                    name.append(display_type_name(t));
                    name.append(u8", "sv);
                }
                name.pop_back();
                name.back() = u8']';
                return name;
            }
            default:
                if(auto it=dtnames.find(ti->index());it!=dtnames.end()){
                    return it->second.identifier();
                }else{
                    return cppp::format<u8"[unknown type {}]"_ts>(ti->index());
                }
        }
    }
    NameDatabase::NameDatabase(cppp::frozen_byte_view& v){
        std::uint64_t nentries = cppp::read<std::uint64_t>(v);
        while(nentries--){
            bbe::func_id fid = cppp::read<bbe::func_id>(v);
            std::uint64_t ns = cppp::read<std::uint64_t>(v);
            // TODO: deserialize colors
            fnames.try_emplace(fid,cppp::str(std::start_lifetime_as_array<char8_t>(v.read(ns),ns),ns),cppp::fvec3{1.0f});
        }
    }
    void NameDatabase::serialize(cppp::bytes& b) const{
        b.appendl<std::uint64_t>(fnames.size());
        for(const auto& [k,v] : fnames){
            b.appendl<bbe::func_id>(k);
            b.appendl<std::uint64_t>(v.identifier().size());
            b.append(std::as_bytes(std::span{v.identifier()}));
        }
    }
}
