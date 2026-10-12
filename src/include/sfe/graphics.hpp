#pragma once
#include<sgl/ext/freetype.hpp>
#include<sgl/draw/line.hpp>
#include<sgl/draw/rect.hpp>
#include"color-picker-drawer.hpp"
namespace sfe{
    constexpr inline cppp::fvec3 COMPAL_COLOR{0.09f,0.09f,0.09f};
    constexpr inline cppp::fvec3 WHITE{1.0f};
    constexpr inline cppp::fvec3 BLACK{0.0f};
    class SpookyFlashlight{
        cppp::fvec2 _position;
        float _intensity;
        public:
            void set_position(cppp::fvec2 pos){
                _position = pos;
            }
            void set_intensity(float intens){
                _intensity = intens;
            }
            cppp::fvec2 position() const{
                return _position;
            }
            float intensity() const{
                return _intensity;
            }
    };
    class SDFRendererWithTorch{
        mutable sgl::GLBuffer vbo;
        sgl::VAO vao;
        mutable sgl::Program p;
        public:
            SDFRendererWithTorch();
            void draw_stretched(const sgl::Texture<sgl::TextureType::TWO_D>& sdf,cppp::fvec2 pos,cppp::fvec2 size,cppp::fvec4 color,SpookyFlashlight fl) const;
            void draw_stretched(const sgl::Texture<sgl::TextureType::TWO_D>& sdf,cppp::fvec2 pos,cppp::fvec2 size,cppp::fvec3 color,SpookyFlashlight fl) const{
                draw_stretched(sdf,pos,size,{color,1.0f},fl);
            }
            void draw(const sgl::Texture<sgl::TextureType::TWO_D>& sdf,cppp::fvec2 pos,cppp::fvec2 size_per_px,cppp::fvec4 color,SpookyFlashlight fl) const{
                draw_stretched(sdf,pos,size_per_px*cppp::fvec2(sdf.size()),color,fl);
            }
            void draw(const sgl::Texture<sgl::TextureType::TWO_D>& sdf,cppp::fvec2 pos,cppp::fvec2 size_per_px,cppp::fvec3 color,SpookyFlashlight fl) const{
                draw(sdf,pos,size_per_px,{color,1.0f},fl);
            }
    };
    class WrappingSDFTextRenderer{
        mutable sgl::Shaper sh;
        SDFRendererWithTorch sr;
        static cppp::fvec2 yf(cppp::fvec2 x){
            return {x.x(),-x.y()};
        }
        public:
            sgl::Shaper& shaper() const{
                return sh;
            }
            void draw_text(cppp::sv text,cppp::fvec2& pos,float sca,cppp::fvec4 color,const sgl::CachedFont& cf,const sgl::CoordinateMap& cm,SpookyFlashlight fl) const;
            void draw_wrapped_text(cppp::sv text,cppp::fvec2& pos,float right,float left,float gap,float sca,cppp::fvec4 color,const sgl::CachedFont& cf,const sgl::CoordinateMap& cm,SpookyFlashlight fl) const;
    };
    class GraphicsContext{
        SVPickerSquareDrawer rb;
        sgl::LineDrawer ld;
        WrappingSDFTextRenderer tr;
        sgl::CachedFont cf;
        sgl::CoordinateMap cm;
        sgl::MonochromeRectDrawer mrd;
        float scale;
        SpookyFlashlight fl;
        public:
            GraphicsContext(cppp::fvec2 ws,sgl::CachedFont&& f,float scale) : cf(std::move(f)), cm(ws), scale(scale){}
            SpookyFlashlight& flashlight(){
                return fl;
            }
            const SpookyFlashlight& flashlight() const{
                return fl;
            }
            void update_window(cppp::fvec2 ws){
                cm.update(ws);
            }
            void draw_wrapped_text_at_cursor(cppp::sv text,cppp::fvec2& pos,float right,float left,float sca,cppp::fvec4 color) const{
                tr.draw_wrapped_text(text,pos,right,left,line_height()*sca,scale*sca,color,cf,cm,fl);
            }
            void draw_wrapped_text_at_cursor(cppp::sv text,cppp::fvec2& pos,float right,float left,float sca,cppp::fvec3 color) const{
                draw_wrapped_text_at_cursor(text,pos,right,left,sca,{color,1.0f});
            }
            void draw_wrapped_text(cppp::sv text,cppp::fvec2 pos,float right,float left,float sca,cppp::fvec4 color) const{
                draw_wrapped_text_at_cursor(text,pos,right,left,sca,color);
            }
            void draw_wrapped_text(cppp::sv text,cppp::fvec2 pos,float right,float left,float sca,cppp::fvec3 color) const{
                draw_wrapped_text(text,pos,right,left,sca,{color,1.0f});
            }
            void draw_text_at_cursor(cppp::sv text,cppp::fvec2& pos,float sca,cppp::fvec4 color) const{
                tr.draw_text(text,pos,scale*sca,color,cf,cm,fl);
            }
            void draw_text_at_cursor(cppp::sv text,cppp::fvec2& pos,float sca,cppp::fvec3 color) const{
                draw_text_at_cursor(text,pos,sca,{color,1.0f});
            }
            void draw_text(cppp::sv text,cppp::fvec2 pos,float sca,cppp::fvec4 color) const{
                draw_text_at_cursor(text,pos,sca,color);
            }
            void draw_text(cppp::sv text,cppp::fvec2 pos,float sca,cppp::fvec3 color) const{
                draw_text(text,pos,sca,{color,1.0f});
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
            void rect(cppp::fvec2 pos,cppp::fvec2 size,cppp::fvec4 color) const{
                mrd.rect(cm.cvt_abs(pos),cm.cvt_rel(size),color);
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
            float get_zoom() const{
                return scale;
            }
            void set_zoom(float zm){
                scale = zm;
            }
            float charadvance(char8_t forchar=u8'0') const{
                return scale*static_cast<float>(cf.query(cf.font().char_to_glyph_id(forchar)).advance())/64.0f;
            }
            float line_height() const{
                return scale*cf.font().line_height_px();
            }
    };
}
