#include<sfe/style.hpp>
#include<cppp/format.hpp>
#include<cppp/binary.hpp>
#include<cppp/uleb.hpp>
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
    static void serialize_color(cppp::bytes& dst,cppp::fvec3 color){
        dst.append(static_cast<std::uint8_t>(color.x()*255.0f));
        dst.append(static_cast<std::uint8_t>(color.y()*255.0f));
        dst.append(static_cast<std::uint8_t>(color.z()*255.0f));
    }
    static cppp::fvec3 deserialize_color(cppp::frozen_byte_view& fbv){
        float r = static_cast<float>(fbv.pop_front())/255.0f;
        float g = static_cast<float>(fbv.pop_front())/255.0f;
        float b = static_cast<float>(fbv.pop_front())/255.0f;
        return {r,g,b};
    }
    NameDatabase::NameDatabase(cppp::frozen_byte_view& v){
        // see TODO above
        std::uint64_t nentries = cppp::muleb128_r<std::uint64_t>(v);
        while(nentries--){
            bbe::func_id fid = cppp::muleb128_r<bbe::func_id>(v);
            std::uint64_t ns = cppp::muleb128_r<std::uint64_t>(v);
            const char8_t* namebuf = std::start_lifetime_as_array<char8_t>(v.read(ns),ns);
            fnames.try_emplace(fid,cppp::str(namebuf,ns),deserialize_color(v));
        }
    }
    void NameDatabase::serialize(cppp::bytes& b,const bbe::SCM& scm) const{
        cppp::muleb128_w<std::uint64_t>(b,fnames.size());
        for(const auto& [k,v] : fnames){
            cppp::muleb128_w<bbe::func_id>(b,scm.fcmap.at(k));
            cppp::muleb128_w<std::uint64_t>(b,v.identifier().size());
            b.append(std::as_bytes(std::span{v.identifier()}));
            serialize_color(b,v.color());
        }
    }
}
