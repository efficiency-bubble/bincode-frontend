#include<cppp/tostring.hpp>
#include<sfe/uitree.hpp>
namespace sfe{
    using namespace std::literals;
    using namespace cppp::literals;
    constexpr static cppp::fvec3 RED{1.0f,0.0f,0.0f};
    constexpr static cppp::fvec3 GRAY{0.7f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_1{0.5f,0.0f,0.0f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_2{0.8f,1.0f,1.0f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_WEAK{0.3f,0.7f,0.7f};
    static void draw_operand(const VisualASTNode& operand,const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const UICursor& cursor,cppp::fvec2& pos,std::uint32_t my_priority){
        bool parenthesize = operand.priority() < my_priority;
        if(parenthesize){
            gc.draw_text_at_cursor(u8"("sv,pos,1.0f,WHITE);
        }
        operand.draw(gc,errors,cursor,pos);
        if(parenthesize){
            gc.draw_text_at_cursor(u8")"sv,pos,1.0f,WHITE);
        }
    }
    static void draw_binop(std::u8string_view op,const std::vector<VisualASTNode>& children,const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const UICursor& cursor,cppp::fvec2& pos,std::uint32_t my_priority){
        draw_operand(children[0uz],gc,errors,cursor,pos,my_priority);
        gc.draw_text_at_cursor(op,pos,1.0f,WHITE);
        draw_operand(children[1uz],gc,errors,cursor,pos,my_priority);
    }
    std::uint32_t VisualASTNode::priority() const{
        switch(type()){
            case bbe::NodeType::CALL_BUILTIN:
                switch(p32()){
                    case 10: // +
                    case 11: // -
                        return 3;
                    case 50: // =
                    case 51: // <=
                        return 2;
                    case 80: // []
                        return 5;
                }
                break;
            default:;
        }
        return 1984;
    }
    void VisualASTNode::draw(const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const UICursor& cursor,cppp::fvec2& pos) const{
        cppp::fvec2 start_pos = pos;
        cppp::fvec2 cursor_pos;
        bool selected = (&cursor.trail().top() == this);
        switch(type()){
            using enum bbe::NodeType;
            case ARG:
                gc.draw_text_at_cursor(u8"arg"sv,pos,0.75f,WHITE);
                cursor_pos = pos;
                break;
            case FNSYM:
                gc.draw_text_at_cursor(u8"fn"sv,pos,0.75f,GRAY);
                gc.draw_text_at_cursor(cppp::format<u8"{}"_ts>(p32()),pos,1.0f,WHITE);
                cursor_pos = pos;
                break;
            case BOOL:
                gc.draw_text_at_cursor(p32()?u8"true"s:u8"false"s,pos,1.0f,WHITE);
                cursor_pos = pos;
                break;
            case UINT32:
                gc.draw_text_at_cursor(cppp::format<u8"{}"_ts>(p32()),pos,1.0f,WHITE);
                cursor_pos = pos;
                gc.draw_text_at_cursor(u8"dw"sv,pos,0.5f,GRAY);
                break;
            case CALL_BUILTIN:
                switch(p32()){
                    case 0:
                        _children[0uz].draw(gc,errors,cursor,pos);
                        gc.draw_text_at_cursor(u8"("sv,pos,1.0f,WHITE);
                        _children[1uz].draw(gc,errors,cursor,pos);
                        gc.draw_text_at_cursor(u8")"sv,pos,1.0f,WHITE);
                        break;
                    case 10:
                        draw_binop(u8"+"sv,_children,gc,errors,cursor,pos,priority());
                        break;
                    case 11:
                        draw_binop(u8"-"sv,_children,gc,errors,cursor,pos,priority());
                        break;
                    case 25:
                        gc.draw_text_at_cursor(u8"print("sv,pos,1.0f,WHITE);
                        _children.front().draw(gc,errors,cursor,pos);
                        gc.draw_text_at_cursor(u8")"sv,pos,1.0f,WHITE);
                        break;
                    case 50:
                        draw_binop(u8"="sv,_children,gc,errors,cursor,pos,priority());
                        break;
                    case 51:
                        draw_binop(u8"<="sv,_children,gc,errors,cursor,pos,priority());
                        break;
                    case 60:
                        gc.draw_text_at_cursor(u8"!"s,pos,1.0f,WHITE);
                        draw_operand(_children.front(),gc,errors,cursor,pos,priority());
                        break;
                    default:
                        gc.draw_text_at_cursor(cppp::format<u8"BUILTIN[{}]("_ts>(p32()),pos,1.0f,WHITE);
                        for(std::uint32_t i=0;i<_children.size();++i){
                            if(i) gc.draw_text_at_cursor(u8","sv,pos,1.0f,WHITE);
                            _children[i].draw(gc,errors,cursor,pos);
                        }
                        gc.draw_text_at_cursor(u8")"sv,pos,1.0f,WHITE);
                        break;
                }
                cursor_pos = pos;
                break;
            case PACK:
                gc.draw_text_at_cursor(u8"("sv,pos,1.0f,WHITE);
                for(std::uint32_t i=0;i<_children.size();++i){
                    if(i) gc.draw_text_at_cursor(u8","sv,pos,1.0f,WHITE);
                    _children[i].draw(gc,errors,cursor,pos);
                }
                gc.draw_text_at_cursor(u8")"sv,pos,1.0f,WHITE);
                cursor_pos = pos;
                break;
            case PACKIND:
                _children[0uz].draw(gc,errors,cursor,pos);
                gc.draw_text_at_cursor(cppp::format<u8"[{}]"_ts>(p32()),pos,1.0f,WHITE);
                cursor_pos = pos;
                break;
            case FORK:
                _children[0uz].draw(gc,errors,cursor,pos);
                gc.draw_text_at_cursor(u8"?"sv,pos,1.0f,WHITE);
                _children[1uz].draw(gc,errors,cursor,pos);
                gc.draw_text_at_cursor(u8":"sv,pos,1.0f,WHITE);
                _children[2uz].draw(gc,errors,cursor,pos);
                cursor_pos = pos;
                break;
            case NTYPE:
                gc.draw_text_at_cursor(u8"_"sv,pos,1.0f,selected?RED:WHITE);
                goto end;
            default:
                std::unreachable();
        }
        if(selected){
            if(cursor.is_after()){
                gc.line(start_pos-cppp::fvec2(0.0f,gc.ascender()),CURSOR_ACCENT_WEAK,start_pos-cppp::fvec2(0.0f,gc.descender()),CURSOR_ACCENT_WEAK);
                gc.line(cursor_pos-cppp::fvec2(0.0f,gc.ascender()),CURSOR_ACCENT_1,cursor_pos-cppp::fvec2(0.0f,gc.descender()),CURSOR_ACCENT_2);
            }else{
                gc.line(start_pos-cppp::fvec2(0.0f,gc.ascender()),CURSOR_ACCENT_1,start_pos-cppp::fvec2(0.0f,gc.descender()),CURSOR_ACCENT_2);
                gc.line(cursor_pos-cppp::fvec2(0.0f,gc.ascender()),CURSOR_ACCENT_WEAK,cursor_pos-cppp::fvec2(0.0f,gc.descender()),CURSOR_ACCENT_WEAK);
            }
        }
        end:
        if(!errors.query(&node()).empty()){
            gc.line(start_pos-cppp::fvec2(0.0f,gc.descender()+gc.line_height()/2.0f),RED,pos-cppp::fvec2(0.0f,gc.descender()+gc.line_height()/2.0f),RED);
        }
    }
    void VisualFunctionNode::draw(const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const UICursor& cursor,cppp::fvec2& pos) const{
        float top_y = pos.y()-gc.ascender();
        float left_x = pos.x();
        float right_x;
        {
            cppp::fvec2 line_1{pos};
            gc.draw_text_at_cursor(cppp::format<u8"function {}:"_ts>(id),line_1,1.0f,WHITE);
            right_x = line_1.x();
        }
        pos += cppp::fvec2(gc.indentation()*4.0f,gc.line_height());
        _child.draw(gc,errors,cursor,pos);
        
        if(&cursor.trail().top() == this){
            float bottom_y = pos.y()-gc.descender();
            
            float x = cursor.is_after()?std::max(right_x,pos.x()):left_x;
            gc.line({x,top_y},CURSOR_ACCENT_1,{x,bottom_y},CURSOR_ACCENT_2);
        }
    }
}
