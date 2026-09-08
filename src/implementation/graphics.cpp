#include<sfe/graphics.hpp>
namespace sfe{
    void WrappingSDFTextRenderer::draw_text(cppp::sv text,cppp::fvec2& pos,float sca,cppp::fvec4 color,const sgl::CachedFont& cf,const sgl::CoordinateMap& cm) const{
        for(auto it = sh.shape(text,cf.font());it;++it){
            auto& gl = cf.query(it.glyph());
            if(gl.bitmap()){
                sr.draw(gl.bitmap(),cm.cvt_abs(pos+yf(cppp::fvec2(it.bearing()+gl.bearing())*sca)),cm.pixel_size()*sca,color);
            }
            pos += cppp::fvec2(it.advance())*sca/64.0f;
        }
    }
    void WrappingSDFTextRenderer::draw_wrapped_text(cppp::sv text,cppp::fvec2& pos,float right,float left,float gap,float sca,cppp::fvec4 color,const sgl::CachedFont& cf,const sgl::CoordinateMap& cm) const{
        for(auto it = sh.shape(text,cf.font());it;++it){
            auto& gl = cf.query(it.glyph());
            cppp::fvec2 nextpos = pos + cppp::fvec2(it.advance())*sca/64.0f;
            if(nextpos.x() >= right){
                pos.x() = left;
                pos.y() += gap;
                nextpos = pos + cppp::fvec2(it.advance())*sca/64.0f;
            }
            if(gl.bitmap()){
                sr.draw(gl.bitmap(),cm.cvt_abs(pos+yf(cppp::fvec2(it.bearing()+gl.bearing())*sca)),cm.pixel_size()*sca,color);
            }
            pos = nextpos;
        }
    }
}
