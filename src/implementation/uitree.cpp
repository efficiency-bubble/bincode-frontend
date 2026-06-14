#include<cppp/format.hpp>
#include<sfe/uitree.hpp>
namespace sfe{
    using namespace std::literals;
    using namespace cppp::literals;
    constexpr static cppp::fvec3 RED{1.0f,0.0f,0.0f};
    constexpr static cppp::fvec3 GRAY{0.7f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_1{0.5f,0.0f,0.0f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_2{0.8f,1.0f,1.0f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_WEAK{0.3f,0.7f,0.7f};
    static void draw_operand(const VisualNode& operand,const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const sfe::NameDatabase& names,const UICursor& cursor,cppp::fvec2& pos,std::uint32_t my_priority){
        bool parenthesize = operand.apriority() < my_priority;
        if(parenthesize){
            gc.draw_text_at_cursor(u8"("sv,pos,1.0f,WHITE);
        }
        operand.draw(gc,errors,names,cursor,pos);
        if(parenthesize){
            gc.draw_text_at_cursor(u8")"sv,pos,1.0f,WHITE);
        }
    }
    static void draw_binop(std::u8string_view op,const std::vector<VisualNode>& children,const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const sfe::NameDatabase& names,const UICursor& cursor,cppp::fvec2& pos,std::uint32_t my_priority){
        draw_operand(children[0uz],gc,errors,names,cursor,pos,my_priority);
        gc.draw_text_at_cursor(op,pos,1.0f,WHITE);
        draw_operand(children[1uz],gc,errors,names,cursor,pos,my_priority);
    }
    std::uint32_t VisualNode::apriority() const{
        assert_a();
        switch(a().type()){
            case bbe::NodeType::CALL_BUILTIN:
                switch(a().getp32()){
                    case 10: // +
                    case 20: // -
                        return 3;
                    case 30: // *
                        return 4;
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
    void VisualNode::draw(const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const sfe::NameDatabase& names,const UICursor& cursor,cppp::fvec2& pos) const{
        switch(type()){
            case VisualNodeType::A: {
                cppp::fvec2 start_pos = pos;
                cppp::fvec2 cursor_pos;
                bool selected = (&cursor.trail().top() == this);
                switch(a().type()){
                    using enum bbe::NodeType;
                    case ARG:
                        gc.draw_text_at_cursor(u8"arg"sv,pos,0.75f,WHITE);
                        cursor_pos = pos;
                        break;
                    case FNSYM:
                        gc.draw_text_at_cursor(names.display_function_name(a().getp32()),pos,1.0f,WHITE);
                        cursor_pos = pos;
                        break;
                    case BOOL:
                        gc.draw_text_at_cursor(a().getp32()?u8"true"s:u8"false"s,pos,1.0f,WHITE);
                        cursor_pos = pos;
                        break;
                    case UINT32:
                        gc.draw_text_at_cursor(cppp::format<u8"{}"_ts>(a().getp32()),pos,1.0f,WHITE);
                        cursor_pos = pos;
                        gc.draw_text_at_cursor(u8"dw"sv,pos,0.5f,GRAY);
                        break;
                    case CALL_BUILTIN:
                        switch(a().getp32()){
                            case 0:
                                _children[0uz].draw(gc,errors,names,cursor,pos);
                                gc.draw_text_at_cursor(u8"("sv,pos,1.0f,WHITE);
                                _children[1uz].draw(gc,errors,names,cursor,pos);
                                gc.draw_text_at_cursor(u8")"sv,pos,1.0f,WHITE);
                                break;
                            case 10:
                                draw_binop(u8"+"sv,_children,gc,errors,names,cursor,pos,apriority());
                                break;
                            case 20:
                                draw_binop(u8"-"sv,_children,gc,errors,names,cursor,pos,apriority());
                                break;
                            case 30:
                                draw_binop(u8"*"sv,_children,gc,errors,names,cursor,pos,apriority());
                                break;
                            case 25:
                                gc.draw_text_at_cursor(u8"print("sv,pos,1.0f,WHITE);
                                _children.front().draw(gc,errors,names,cursor,pos);
                                gc.draw_text_at_cursor(u8")"sv,pos,1.0f,WHITE);
                                break;
                            case 50:
                                draw_binop(u8"="sv,_children,gc,errors,names,cursor,pos,apriority());
                                break;
                            case 51:
                                draw_binop(u8"<="sv,_children,gc,errors,names,cursor,pos,apriority());
                                break;
                            case 60:
                                gc.draw_text_at_cursor(u8"!"s,pos,1.0f,WHITE);
                                draw_operand(_children.front(),gc,errors,names,cursor,pos,apriority());
                                break;
                            default:
                                gc.draw_text_at_cursor(cppp::format<u8"BUILTIN[{}]("_ts>(a().getp32()),pos,1.0f,WHITE);
                                for(std::uint32_t i=0;i<_children.size();++i){
                                    if(i) gc.draw_text_at_cursor(u8","sv,pos,1.0f,WHITE);
                                    _children[i].draw(gc,errors,names,cursor,pos);
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
                            _children[i].draw(gc,errors,names,cursor,pos);
                        }
                        gc.draw_text_at_cursor(u8")"sv,pos,1.0f,WHITE);
                        cursor_pos = pos;
                        break;
                    case COMMA:
                        gc.draw_text_at_cursor(u8"("sv,pos,1.0f,WHITE);
                        for(std::uint32_t i=0;i<_children.size();++i){
                            if(i) gc.draw_text_at_cursor(u8"; "sv,pos,1.0f,WHITE);
                            _children[i].draw(gc,errors,names,cursor,pos);
                        }
                        gc.draw_text_at_cursor(cppp::format<u8")"_ts>(),pos,1.0f,WHITE);
                        cursor_pos = pos;
                        break;
                    case PACKIND:
                        _children[0uz].draw(gc,errors,names,cursor,pos);
                        gc.draw_text_at_cursor(cppp::format<u8"[{}]"_ts>(a().getp32()),pos,1.0f,WHITE);
                        cursor_pos = pos;
                        break;
                    case FORK:
                        _children[0uz].draw(gc,errors,names,cursor,pos);
                        gc.draw_text_at_cursor(u8"?"sv,pos,1.0f,WHITE);
                        _children[1uz].draw(gc,errors,names,cursor,pos);
                        gc.draw_text_at_cursor(u8":"sv,pos,1.0f,WHITE);
                        _children[2uz].draw(gc,errors,names,cursor,pos);
                        cursor_pos = pos;
                        break;
                    case NTYPE:
                        gc.draw_text_at_cursor(u8"_"sv,pos,1.0f,selected?RED:WHITE);
                        goto anodrawsel;
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
                anodrawsel:
                if(!errors.query(&a()).empty()){
                    gc.line(start_pos-cppp::fvec2(0.0f,gc.descender()+gc.line_height()/2.0f),RED,pos-cppp::fvec2(0.0f,gc.descender()+gc.line_height()/2.0f),RED);
                }
                break;
            }
            case VisualNodeType::F: {
                float top_y = pos.y()-gc.ascender();
                float left_x = pos.x();
                float right_x;
                {
                    cppp::fvec2 line_1{pos};
                    gc.draw_text_at_cursor(cppp::format<u8"{}({}) -> {}:"_ts>(names.display_function_name(f().index()),names.display_type_name(f().signature().parameter()),names.display_type_name(f().signature().return_type())),line_1,1.0f,WHITE);
                    right_x = line_1.x();
                }
                pos += cppp::fvec2(gc.indentation()*4.0f,gc.line_height());
                _children.front().draw(gc,errors,names,cursor,pos);
                
                if(&cursor.trail().top() == this){
                    float bottom_y = pos.y()-gc.descender();
                    
                    float x = cursor.is_after()?std::max(right_x,pos.x()):left_x;
                    gc.line({x,top_y},CURSOR_ACCENT_1,{x,bottom_y},CURSOR_ACCENT_2);
                }
                break;
            }
        }
    }
}
