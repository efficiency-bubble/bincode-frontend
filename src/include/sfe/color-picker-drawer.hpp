#pragma once
#include<cppp/vector.hpp>
#include<sgl/gl.hpp>
namespace sfe{
    class SVPickerSquareDrawer{
        sgl::GLBuffer vbo;
        sgl::VAO vao;
        mutable sgl::Program prog;
        public:
            SVPickerSquareDrawer();
            void rainbow(const sgl::CoordinateMap& cm,cppp::fvec2 start,cppp::fvec2 dims,cppp::fvec3 hue) const;
    };
}
