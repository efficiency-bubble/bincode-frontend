#pragma once
#include<sgl/ext/freetype.hpp>
#include<sgl/draw/line.hpp>
#include"color-picker-drawer.hpp"
namespace sfe{
    constexpr inline cppp::fvec3 DIM_ORANGE{0.5529f,0.4353f,0.0627f};
    constexpr inline cppp::fvec3 WHITE{1.0f};
    constexpr inline cppp::fvec3 BLACK{0.0f};
    class GraphicsContext{
        SVPickerSquareDrawer rb;
        sgl::LineDrawer ld;
        sgl::SDFTextRenderer tr;
        sgl::CachedFont cf;
        sgl::CoordinateMap cm;
        float scale;
        public:
            GraphicsContext(sgl::CachedFont&& f,sgl::CoordinateMap cm,float scale) : cf(std::move(f)), cm(cm), scale(scale){}
            void update_window(std::uint32_t w,std::uint32_t h){
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
            void rect(cppp::uvec2 pos,cppp::uvec2 size,cppp::fvec3 color) const{
                std::array<float,4uz> old_clearcolor;
                glGetFloatv(GL_COLOR_CLEAR_VALUE,old_clearcolor.data());
                pos.y() = cm.win_size().y()-(pos.y()+size.y());
                glScissor(static_cast<GLint>(pos.x()),static_cast<GLint>(pos.y()),static_cast<GLsizei>(size.x()),static_cast<GLsizei>(size.y()));
                glEnable(GL_SCISSOR_TEST);
                glClearColor(color.x(),color.y(),color.z(),1.0f);
                glClear(GL_COLOR_BUFFER_BIT);
                glDisable(GL_SCISSOR_TEST);
                glClearColor(old_clearcolor[0uz],old_clearcolor[1uz],old_clearcolor[2uz],old_clearcolor[3uz]);
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
