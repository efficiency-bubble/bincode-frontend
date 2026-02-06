#pragma once
#include<sgl/gl.hpp>
#include<sgl/draw/sdf.hpp>
#include<sgl/ext/freetype.hpp>
namespace sfe{
    class SDFTextRenderer{
        mutable sgl::Shaper sh;
        sgl::SDFRenderer sr;
        public:
            sgl::Shaper& shaper() const{
                return sh;
            }
            void draw_text(cppp::sv text,cppp::fvec2& pos,float sca,cppp::fvec3 color,sgl::CachedFont& cf,const sgl::CoordinateMap& cm) const;
    };
}
