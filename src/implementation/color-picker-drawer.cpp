#include<sfe/color-picker-drawer.hpp>
#include<array>
namespace sfe{
    using namespace std::literals;
    // HSV to RGB algorithm from https://stackoverflow.com/a/17897228
    SVPickerSquareDrawer::SVPickerSquareDrawer() : prog(u8"#version 460 core\nlayout(location=0)in vec2 t;layout(location=0)out vec2 T;layout(location=0)uniform vec2 p;layout(location=1)uniform vec2 s;void main(){T=t;gl_Position=vec4(p+t*s,0.,1.);}"sv,u8"#version 460 core\nout vec4 c;layout(location=0)in vec2 t;layout(location=2)uniform vec3 i;vec3 F(vec3 j,vec2 q){return q.y*(1.-q.x*clamp(min(j,4.-j),0.,1.));}vec3 I(vec3 P,vec3 Q,float R,float S,float e){return mix(P,Q,smoothstep(S-e,S+e,R));}void main(){vec3 k=mod(i.x+vec3(5.,3.,1.),6.),C=F(k,i.yz);float l=length(i.yz-t),E=fwidth(l)*.6;c=vec4(I(C,I(vec3(dot(C,vec3(.2126,.7152,.0722))<.5?1.:0.),F(k,t),l,.034,E),l,.0219,E),1.);}"sv){
        constexpr static std::array data{
            1.0f,0.0f,
            1.0f,1.0f,
            0.0f,0.0f,
            0.0f,1.0f
        };
        vbo.allocate_static(sizeof(data),{},data.data());
        vao.add_buffer(vbo,0,0,2*sizeof(float));
        vao.set_attr<float>(0,0,2,0,false);
    }
    void SVPickerSquareDrawer::rainbow(const sgl::CoordinateMap& cm,cppp::fvec2 start,cppp::fvec2 dims,cppp::fvec3 hsv) const{
        vao.use();
        prog.set_uniform(0,cm.cvt_abs(start+dims.yproj()));
        prog.set_uniform(1,dims*cm.pixel_size());
        prog.set_uniform(2,hsv);
        prog.use();
        glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    }
}
