#include<sfe/graphics.hpp>
#include<sgl/draw/impl/buffer_creation.hpp>
#include<string>
namespace sfe{
    using namespace std::literals;
    SDFRendererWithTorch::SDFRendererWithTorch() : p(u8"#version 460 core\nlayout(location=0)in vec2 p;layout(location=1)in vec2 t;layout(location=0)out vec2 T;void main(){T=t;gl_Position=vec4(p,0.,1.);}"sv,u8"#version 460 core\nout vec4 C;layout(location=0)in vec2 t;layout(location=0)uniform sampler2D i;layout(location=1)uniform vec4 c;layout(location=2)uniform vec2 m;layout(location=3)uniform float I;layout(origin_upper_left)in vec4 gl_FragCoord;void main(){float v=texture(i,t).r,E=fwidth(v)*.55;C=vec4(c.xyz,c.w*smoothstep(.5-E,.5+E,v)*I/max(1.1,.0009*dot(gl_FragCoord.xy-m,gl_FragCoord.xy-m)));}"sv){
        sgl::detail::create_buffer_2_2_4(vao,vbo);
    }
    void SDFRendererWithTorch::draw_stretched(const sgl::Texture<sgl::TextureType::TWO_D>& sdf,cppp::fvec2 pos,cppp::fvec2 size,cppp::fvec4 color,SpookyFlashlight fl) const{
        p.set_uniform(1,color);
        p.set_uniform(2,fl.position());
        p.set_uniform(3,fl.intensity());
        float l = pos.x();
        float r = l + size.x();
        float t = pos.y();
        float b = t - size.y();
        std::array vertices{
            r,b,1.0f,1.0f,
            r,t,1.0f,0.0f,
            l,b,0.0f,1.0f,
            l,t,0.0f,0.0f
        };
        vbo.upload(vertices.data(),vertices.size()*sizeof(float));
        vao.use();
        p.use();
        glActiveTexture(GL_TEXTURE0);
        sdf.bind();
        glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    }
    void WrappingSDFTextRenderer::draw_text(cppp::sv text,cppp::fvec2& pos,float sca,cppp::fvec4 color,const sgl::CachedFont& cf,const sgl::CoordinateMap& cm,SpookyFlashlight fl) const{
        for(auto it = sh.shape(text,cf.font());it;++it){
            auto& gl = cf.query(it.glyph());
            if(gl.bitmap()){
                sr.draw(gl.bitmap(),cm.cvt_abs(pos+yf(cppp::fvec2(it.bearing()+gl.bearing())*sca)),cm.pixel_size()*sca,color,fl);
            }
            pos += cppp::fvec2(it.advance())*sca/64.0f;
        }
    }
    void WrappingSDFTextRenderer::draw_wrapped_text(cppp::sv text,cppp::fvec2& pos,float right,float left,float gap,float sca,cppp::fvec4 color,const sgl::CachedFont& cf,const sgl::CoordinateMap& cm,SpookyFlashlight fl) const{
        for(auto it = sh.shape(text,cf.font());it;++it){
            auto& gl = cf.query(it.glyph());
            cppp::fvec2 nextpos = pos + cppp::fvec2(it.advance())*sca/64.0f;
            if(nextpos.x() >= right){
                pos.x() = left;
                pos.y() += gap;
                nextpos = pos + cppp::fvec2(it.advance())*sca/64.0f;
            }
            if(gl.bitmap()){
                sr.draw(gl.bitmap(),cm.cvt_abs(pos+yf(cppp::fvec2(it.bearing()+gl.bearing())*sca)),cm.pixel_size()*sca,color,fl);
            }
            pos = nextpos;
        }
    }
}
