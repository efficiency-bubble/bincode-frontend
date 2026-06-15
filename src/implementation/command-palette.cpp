#include<sfe/command-palette.hpp>
#include<cppp/assert.hpp>
namespace sfe{
    void CommandPalette::append(cppp::sv more){
        std::size_t previous_length = buffer.size();
        buffer.append(more);
        if(!candidates.empty()){
            std::vector<const CommandSet::entry_type*> refined_candidates;
            std::size_t current_selection = std::exchange(selection,0uz);
            for(std::size_t i=0uz;i<candidates.size();++i){
                if(has_prefix(candidates[i]->first,buffer,previous_length)){
                    if(i == current_selection){
                        selection = refined_candidates.size();
                    }
                    refined_candidates.emplace_back(candidates[i]);
                }
            }
            candidates = std::move(refined_candidates);
        }
    }
    void CommandPalette::backspace(){
        buffer.pop_back();
        candidates.clear();
        for(const auto& el : commands->commands()){
            if(has_prefix(el.first,buffer,0uz)){
                candidates.emplace_back(&el);
            }
        }
        selection = 0uz;
    }
    void CommandPalette::render(const GraphicsContext& gc,cppp::fvec2 pos,float width,float text_scale) const{
        const float padding = 2.0f * text_scale;
        float line_height = gc.font().font().line_height_px() * text_scale;
        float row_height = line_height + 2.0f * padding;
        float ascender = gc.font().font().ascender_px() * text_scale + padding;
        
        gc.rect(pos,cppp::fvec2{width,row_height},DIM_ORANGE);
        
        for(std::size_t i=0uz;i<candidates.size();++i){
            pos.y() += line_height;
            gc.rect(pos,cppp::fvec2{width,row_height},i==selection?WHITE:DIM_ORANGE);
            gc.draw_text(candidates[i]->first,pos+cppp::fvec2(0,ascender),text_scale,i==selection?BLACK:WHITE);
        }
    }
}
