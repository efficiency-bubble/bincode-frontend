#pragma once
#include<sgl/ext/freetype.hpp>
#include<sgl/draw/line.hpp>
#include<sgl/draw/rect.hpp>
#include"color-picker-drawer.hpp"
namespace sfe{
    constexpr inline cppp::fvec3 COMPAL_COLOR{0.2549019607843137f,0.2549019607843137f,0.2549019607843137f};
    constexpr inline cppp::fvec3 WHITE{1.0f};
    constexpr inline cppp::fvec3 BLACK{0.0f};
    class WrappingSDFTextRenderer{
        mutable sgl::Shaper sh;
        sgl::SDFRenderer sr;
        static cppp::fvec2 yf(cppp::fvec2 x){
            return {x.x(),-x.y()};
        }
        public:
            sgl::Shaper& shaper() const{
                return sh;
            }
            void draw_text(cppp::sv text,cppp::fvec2& pos,float sca,cppp::fvec3 color,const sgl::CachedFont& cf,const sgl::CoordinateMap& cm) const{
                for(auto it = sh.shape(text,cf.font());it;++it){
                    auto& gl = cf.query(it.glyph());
                    if(gl.bitmap()){
                        sr.draw(gl.bitmap(),cm.cvt_abs(pos+yf(cppp::fvec2(it.bearing()+gl.bearing())*sca)),cm.pixel_size()*sca,color);
                    }
                    pos += cppp::fvec2(it.advance())*sca/64.0f;
                }
            }
            void draw_wrapped_text(cppp::sv text,cppp::fvec2& pos,float right,float left,float gap,float sca,cppp::fvec3 color,const sgl::CachedFont& cf,const sgl::CoordinateMap& cm) const{
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
    };
    class GraphicsContext{
        SVPickerSquareDrawer rb;
        sgl::LineDrawer ld;
        WrappingSDFTextRenderer tr;
        sgl::CachedFont cf;
        sgl::CoordinateMap cm;
        sgl::MonochromeRectDrawer mrd;
        float scale;
        public:
            GraphicsContext(sgl::CachedFont&& f,sgl::CoordinateMap cm,float scale) : cf(std::move(f)), cm(cm), scale(scale){}
            void update_window(float w,float h){
                cm.update(w,h);
            }
            void draw_wrapped_text_at_cursor(cppp::sv text,cppp::fvec2& pos,float right,float left,float sca,cppp::fvec3 color) const{
                tr.draw_wrapped_text(text,pos,right,left,line_height()*sca,scale*sca,color,cf,cm);
            }
            void draw_wrapped_text(cppp::sv text,cppp::fvec2 pos,float right,float left,float sca,cppp::fvec3 color) const{
                draw_wrapped_text_at_cursor(text,pos,right,left,sca,color);
            }
            void draw_text_at_cursor(cppp::sv text,cppp::fvec2& pos,float sca,cppp::fvec3 color) const{
                tr.draw_text(text,pos,scale*sca,color,cf,cm);
            }
            void draw_text(cppp::sv text,cppp::fvec2 pos,float sca,cppp::fvec3 color) const{
                draw_text_at_cursor(text,pos,sca,color);
            }
            void rainbow(cppp::fvec2 start,cppp::fvec2 dims,cppp::fvec3 hsv) const{
                rb.rainbow(cm,start,dims,hsv);
            }
            void line(cppp::fvec2 spos,cppp::fvec3 scolor,cppp::fvec2 tpos,cppp::fvec3 tcolor) const{
                ld.line(cm.cvt_abs(spos),scolor,cm.cvt_abs(tpos),tcolor);
            }
            void rect(cppp::fvec2 pos,cppp::fvec2 size,cppp::fvec3 color) const{
                mrd.rect(cm.cvt_abs(pos),cm.cvt_rel(size),{color,1.0f});
            }
            const sgl::CoordinateMap cmap() const{
                return cm;
            }
            const sgl::CachedFont& font() const{
                return cf;
            }
            float ascender() const{
                return scale*cf.font().ascender_px();
            }
            float descender() const{
                return scale*cf.font().descender_px();
            }
            float charadvance(char8_t forchar=u8'0') const{
                return scale*static_cast<float>(cf.query(cf.font().char_to_glyph_id(forchar)).advance())/64.0f;
            }
            float line_height() const{
                return scale*cf.font().line_height_px();
            }
    };
}
