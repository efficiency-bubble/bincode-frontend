#include<cppp/tostring.hpp>
#include<sfe/uitree.hpp>
namespace sfe{
    using namespace std::literals;
    using namespace cppp::literals;
    constexpr static cppp::fvec3 RED{1.0f,0.0f,0.0f};
    constexpr static cppp::fvec3 WHITE{1.0f};
    constexpr static cppp::fvec3 GRAY{0.7f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_1{0.5f,0.0f,0.0f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_2{0.8f,1.0f,1.0f};
    static void draw_binop(std::u8string_view op,const std::vector<VisualASTNode>& children,const GraphicsContext& gc,const UICursor& cursor,cppp::fvec2& pos){
        gc.draw_text(u8"("sv,pos,1.0f,WHITE);
        children[0uz].draw(gc,cursor,pos);
        gc.draw_text(u8")"s+op+u8'(',pos,1.0f,WHITE);
        children[1uz].draw(gc,cursor,pos);
        gc.draw_text(u8")"sv,pos,1.0f,WHITE);
    }
    void VisualASTNode::draw(const GraphicsContext& gc,const UICursor& cursor,cppp::fvec2& pos) const{
        cppp::fvec2 cpos = pos;
        bool selected = (&cursor.trail().top() == this);
        switch(nd->type()){
            using enum bbe::NodeType;
            case ARGV:
                gc.draw_text(u8"argv"sv,pos,0.75f,WHITE);
                if(cursor.is_after()) cpos = pos;
                break;
            case FNSYM:
                gc.draw_text(u8"fn"sv,pos,0.75f,GRAY);
                gc.draw_text(cppp::format<u8"{}"_ts>(p32()),pos,1.0f,WHITE);
                if(cursor.is_after()) cpos = pos;
                break;
            case BOOL:
                gc.draw_text(p32()?u8"true"s:u8"false"s,pos,1.0f,WHITE);
                if(cursor.is_after()) cpos = pos;
                break;
            case UINT32:
                gc.draw_text(cppp::format<u8"{}"_ts>(p32()),pos,1.0f,WHITE);
                if(cursor.is_after()) cpos = pos;
                gc.draw_text(u8"dw"sv,pos,0.5f,GRAY);
                break;
            case CALL_BUILTIN:
                switch(p32()){
                    case 0:
                        _children.front().draw(gc,cursor,pos);
                        gc.draw_text(u8"("sv,pos,1.0f,WHITE);
                        for(std::uint32_t i=1;i<_children.size();++i){
                            if(i>1) gc.draw_text(u8","sv,pos,1.0f,WHITE);
                            _children[i].draw(gc,cursor,pos);
                        }
                        gc.draw_text(u8")"sv,pos,1.0f,WHITE);
                        break;
                    case 10:
                        draw_binop(u8"+"sv,_children,gc,cursor,pos);
                        break;
                    case 11:
                        draw_binop(u8"-"sv,_children,gc,cursor,pos);
                        break;
                    case 50:
                        draw_binop(u8"="sv,_children,gc,cursor,pos);
                        break;
                    case 51:
                        draw_binop(u8"<="sv,_children,gc,cursor,pos);
                        break;
                    case 80:
                        _children[0uz].draw(gc,cursor,pos);
                        gc.draw_text(u8"["s,pos,1.0f,WHITE);
                        _children[1uz].draw(gc,cursor,pos);
                        gc.draw_text(u8"]"sv,pos,1.0f,WHITE);
                        break;
                    default:
                        gc.draw_text(cppp::format<u8"BUILTIN[{}]("_ts>(p32()),pos,1.0f,WHITE);
                        for(std::uint32_t i=0;i<_children.size();++i){
                            if(i) gc.draw_text(u8","sv,pos,1.0f,WHITE);
                            _children[i].draw(gc,cursor,pos);
                        }
                        gc.draw_text(u8")"sv,pos,1.0f,WHITE);
                        break;
                }
                if(cursor.is_after()) cpos = pos;
                break;
            case PACK:
                for(std::uint32_t i=0;i<_children.size();++i){
                    if(i) gc.draw_text(u8","sv,pos,1.0f,WHITE);
                    _children[i].draw(gc,cursor,pos);
                }
                if(cursor.is_after()) cpos = pos;
                break;
            case FORK:
                _children[0uz].draw(gc,cursor,pos);
                gc.draw_text(u8"?"sv,pos,1.0f,WHITE);
                _children[1uz].draw(gc,cursor,pos);
                gc.draw_text(u8":"sv,pos,1.0f,WHITE);
                _children[2uz].draw(gc,cursor,pos);
                if(cursor.is_after()) cpos = pos;
                break;
            case NTYPE:
                gc.draw_text(u8"_"s,pos,1.0f,selected?RED:WHITE);
                return;
            default:
                std::unreachable();
        }
        if(selected){
            gc.line(cpos-cppp::fvec2(0.0f,gc.ascender()),CURSOR_ACCENT_1,cpos-cppp::fvec2(0.0f,gc.descender()),CURSOR_ACCENT_2);
        }
    }
    void VisualFunctionNode::draw(const GraphicsContext& gc,const UICursor& cursor,cppp::fvec2& pos) const{
        float top_y = pos.y()-gc.ascender();
        float left_x = pos.x();
        float right_x;
        {
            cppp::fvec2 line_1{pos};
            gc.draw_text(cppp::format<u8"function {}:"_ts>(id),line_1,1.0f,WHITE);
            right_x = line_1.x();
        }
        pos += cppp::fvec2(gc.indentation()*4.0f,gc.line_height());
        _child.draw(gc,cursor,pos);
        
        if(&cursor.trail().top() == this){
            float bottom_y = pos.y()-gc.descender();
            
            float x = cursor.is_after()?std::max(right_x,pos.x()):left_x;
            gc.line({x,top_y},CURSOR_ACCENT_1,{x,bottom_y},CURSOR_ACCENT_2);
        }
    }
}
