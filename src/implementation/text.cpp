#include<sfe/text.hpp>
namespace sfe{
    static cppp::fvec2 yf(cppp::fvec2 x){
        return {x.x(),-x.y()};
    }
    void SDFTextRenderer::draw_text(cppp::sv text,cppp::fvec2& pos,float sca,cppp::fvec3 color,sgl::CachedFont& cf,const sgl::CoordinateMap& cm) const{
        for(auto it = sh.shape(text,cf.font());it;++it){
            auto& gl = cf.query(it.glyph());
            if(gl.bitmap()){
                sr.draw(gl.bitmap(),cm.cvt_abs(pos+yf(cppp::fvec2(it.bearing()+gl.bearing())*sca)),cm.pixel_size()*sca,color);
            }
            pos += cppp::fvec2(it.advance())*sca/64.0f;
        }
    }
}
