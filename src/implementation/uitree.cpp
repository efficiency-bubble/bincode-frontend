#include<cppp/tostring.hpp>
#include<sfe/uitree.hpp>
namespace sfe{
    using namespace std::literals;
    constexpr static cppp::fvec3 RED{1.0f,0.0f,0.0f};
    constexpr static cppp::fvec3 WHITE{1.0f};
    constexpr static cppp::fvec3 GRAY{0.7f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_1{0.5f,0.0f,0.0f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_2{0.8f,1.0f,1.0f};
    void VisualASTNode::draw(const GraphicsContext& gc,const UICursor& cursor,cppp::fvec2& pos) const{
        using namespace cppp::literals;
        cppp::fvec2 cpos = pos;
        bool selected = (&cursor.trail().top() == this);
        switch(nd->type()){
            using enum bbe::NodeType;
            case UINT32:
                gc.draw_text(cppp::format<u8"{}"_ts>(p32()),pos,1.0f,WHITE);
                if(cursor.is_after()) cpos = pos;
                gc.draw_text(u8"dw"sv,pos,0.5f,GRAY);
                break;
            case CALL_BUILTIN:
                switch(p32()){
                    case 10:
                        gc.draw_text(u8"("sv,pos,1.0f,WHITE);
                        _children[0uz].draw(gc,cursor,pos);
                        gc.draw_text(u8")+("s,pos,1.0f,WHITE);
                        _children[1uz].draw(gc,cursor,pos);
                        gc.draw_text(u8")"sv,pos,1.0f,WHITE);
                        break;
                    default:
                        gc.draw_text(cppp::format<u8"BUILTIN[{}]("_ts>(p32()),pos,1.0f,WHITE);
                        _children.front().draw(gc,cursor,pos);
                        gc.draw_text(u8")"sv,pos,1.0f,WHITE);
                        break;
                }
                if(cursor.is_after()) cpos = pos;
                break;
            case PACK: {
                for(std::uint32_t i=0;i<_children.size();++i){
                    if(i) gc.draw_text(u8","sv,pos,1.0f,WHITE);
                    _children[i].draw(gc,cursor,pos);
                }
                if(cursor.is_after()) cpos = pos;
                break;
            }
            case NTYPE:
                gc.draw_text(u8"_"s,pos,1.0f,selected?RED:WHITE);
                return;
            default:
                std::unreachable();
        }
        if(selected){
            gc.line(cpos-cppp::fvec2(0.0f,gc.font().ascender_px()),CURSOR_ACCENT_1,cpos-cppp::fvec2(0.0f,gc.font().descender_px()),CURSOR_ACCENT_2);
        }
    }
    void VisualFunctionNode::draw(const GraphicsContext& gc,const UICursor& cursor,cppp::fvec2& pos) const{
        float top_y = pos.y()-gc.font().ascender_px();
        float left_x = pos.x();
        float right_x;
        {
            cppp::fvec2 line_1{pos};
            gc.draw_text(u8"function:"sv,line_1,1.0f,WHITE);
            right_x = line_1.x();
        }
        pos += cppp::fvec2(static_cast<float>(gc.font_cache().query(gc.font().char_to_glyph_id(u8'0')).advance())/16.0f,gc.font().line_height_px());
        child.draw(gc,cursor,pos);
        
        if(&cursor.trail().top() == this){
            float bottom_y = pos.y()-gc.font().descender_px();
            
            float x = cursor.is_after()?std::max(right_x,pos.x()):left_x;
            gc.line({x,top_y},CURSOR_ACCENT_1,{x,bottom_y},CURSOR_ACCENT_2);
        }
    }
}
