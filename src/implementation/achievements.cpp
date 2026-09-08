#ifdef SFE_ENABLE_ACHIEVEMENTS
#include<sfe/achievements.hpp>
namespace sfe{
    constexpr static cppp::fvec3 BG_COLOR{1.0f,0.9019607843137255f,0.0f};
    constexpr static cppp::fvec3 BG_COLOR_2{0.7450980392156863f, 0.8666666666666667f, 0.6f};
    constexpr static cppp::fvec3 HEADER_COLOR{0.21176470588235294f, 0.12941176470588237f, 1.0f};
    void AchievementPopup::render(const GraphicsContext& gc,cppp::fvec2 pos,float brightness,float alpha){
        constexpr float width = 450.0f;
        constexpr float height = 200.0f;
        constexpr float padding = 10.0f;
        constexpr float border = 2.5f;
        gc.rect(pos,{width,height},{BG_COLOR*(1.0f-brightness) + brightness,alpha});
        gc.rect(pos+border,{width-border*2.0f,height-border*2.0f},{BG_COLOR_2*(1.0f-brightness) + brightness,alpha});
        pos.x() += padding;
        const float left = pos.x();
        const float right = left + width - padding * 2;
        pos.y() += gc.ascender() * 0.84f;
        gc.draw_wrapped_text_at_cursor(u8"Achievement obtained!"sv,pos,right,left,0.7f,{cppp::fvec3(brightness),alpha});
        pos.y() += gc.line_height() * 0.84f;
        pos.x() = left;
        gc.draw_wrapped_text_at_cursor(ach->name(),pos,right,left,0.7f,{HEADER_COLOR*(1.0f-brightness) + brightness,alpha});
        pos.y() += gc.line_height() * 0.94f;
        pos.x() = left;
        gc.draw_wrapped_text_at_cursor(ach->description(),pos,right,left,0.5f,{cppp::fvec3(brightness),alpha});
    }
}
#endif
