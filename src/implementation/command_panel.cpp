#include<sfe/command_panel.hpp>
#include<cppp/rtl.hpp>
namespace sfe{
    void CommandSelectorPanel::render(const GraphicsContext& gc,cppp::fvec2 pos,float width,float text_scale){
        const float padding = 2.0f * text_scale;
        float line_height = gc.font().font().line_height_px() * text_scale;
        float row_height = line_height + 2.0f * padding;
        float ascender = gc.font().font().ascender_px() * text_scale + padding;
        
        pos.x() -= width/2.0f;
        gc.rect(pos,cppp::fvec2{width,row_height},DIM_ORANGE);
        gc.draw_text(_buffer,cppp::rtl<cppp::fvec2>(pos+cppp::fvec2(0,ascender)),text_scale,WHITE);
        
        for(std::size_t i=0uz;i<candidates.size();++i){
            pos.y() += line_height;
            gc.rect(pos,cppp::fvec2{width,row_height},i==selection?WHITE:DIM_ORANGE);
            gc.draw_text(candidates[i]->first,cppp::rtl<cppp::fvec2>(pos+cppp::fvec2(0,ascender)),text_scale,i==selection?BLACK:WHITE);
        }
    }
}
