#pragma once
#include"graphics.hpp"
#include<cppp/vector.hpp>
#include<cppp/colorconv.hpp>
namespace sfe{
    class ColorPicker{
        cppp::fvec3* ptr;
        cppp::fvec3 hsv;
        public:
            ColorPicker() : ptr(nullptr){}
            void open(cppp::fvec3& obj){
                ptr = &obj;
                hsv = cppp::rgb_to_hsv(obj);
            }
            void close(){
                ptr = nullptr;
            }
            bool is_open() const{
                return ptr;
            }
            void render(const GraphicsContext& gc,cppp::fvec2 pos) const{
                gc.rainbow(pos,{300.0f,300.0f},hsv);
            }
            cppp::fvec3 get_hsv() const{
                return hsv;
            }
            void set_hsv(cppp::fvec3 nv){
                *ptr = cppp::hsv_to_rgb_parallel(nv);
                hsv = nv;
            }
    };
}
