#pragma once
#include<sgl/ext/freetype.hpp>
#include<sgl/draw/line.hpp>
#include<sgl/draw/rect.hpp>
#include"color-picker-drawer.hpp"
namespace sfe{
    constexpr inline cppp::fvec3 COMPAL_COLOR{0.2549019607843137f,0.2549019607843137f,0.2549019607843137f};
    constexpr inline cppp::fvec3 WHITE{1.0f};
    constexpr inline cppp::fvec3 BLACK{0.0f};
    class GraphicsContext{
        SVPickerSquareDrawer rb;
        sgl::LineDrawer ld;
        sgl::SDFTextRenderer tr;
        sgl::CachedFont cf;
        sgl::CoordinateMap cm;
        sgl::MonochromeRectDrawer mrd;
        float scale;
        public:
            GraphicsContext(sgl::CachedFont&& f,sgl::CoordinateMap cm,float scale) : cf(std::move(f)), cm(cm), scale(scale){}
            void update_window(float w,float h){
                cm.update(w,h);
            }
            void draw_text_at_cursor(cppp::sv text,cppp::fvec2& pos,float sca,cppp::fvec3 color) const{
                tr.draw_text(text,pos,scale*sca,color,cf,cm);
            }
            void rainbow(cppp::fvec2 start,cppp::fvec2 dims,cppp::fvec3 hsv) const{
                rb.rainbow(cm,start,dims,hsv);
            }
            void draw_text(cppp::sv text,cppp::fvec2 pos,float sca,cppp::fvec3 color) const{
                draw_text_at_cursor(text,pos,sca,color);
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
