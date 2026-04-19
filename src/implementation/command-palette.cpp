#include<sfe/command-palette.hpp>
namespace sfe{
    void CommandPalette::append(std::string_view data){
        std::size_t match_begin = _buffer.size();
        _buffer.append_range(data);
        if(!candidates.empty()){
            std::vector<const CommandSet::entry_type*> refined_candidates;
            std::size_t current_selection = std::exchange(selection,0uz);
            for(std::size_t i=0uz;i<candidates.size();++i){
                if(has_prefix(candidates[i]->first,_buffer,match_begin)){
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
        if(!_buffer.empty()){
            _buffer.pop_back();
            candidates.clear();
            for(const auto& el : commands->commands()){
                if(has_prefix(el.first,_buffer,0uz)){
                    candidates.emplace_back(&el);
                }
            }
            selection = 0uz;
        }
    }
    void CommandPalette::render(const GraphicsContext& gc,cppp::fvec2 pos,float width,float text_scale) const{
        const float padding = 2.0f * text_scale;
        float line_height = gc.font().font().line_height_px() * text_scale;
        float row_height = line_height + 2.0f * padding;
        float ascender = gc.font().font().ascender_px() * text_scale + padding;
        
        pos.x() -= width/2.0f;
        gc.rect(pos,cppp::fvec2{width,row_height},DIM_ORANGE);
        gc.draw_text(_buffer,pos+cppp::fvec2(0,ascender),text_scale,WHITE);
        
        for(std::size_t i=0uz;i<candidates.size();++i){
            pos.y() += line_height;
            gc.rect(pos,cppp::fvec2{width,row_height},i==selection?WHITE:DIM_ORANGE);
            gc.draw_text(candidates[i]->first,pos+cppp::fvec2(0,ascender),text_scale,i==selection?BLACK:WHITE);
        }
    }
}
