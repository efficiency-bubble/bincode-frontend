#include<cppp/format.hpp>
#include<sfe/uitree.hpp>
#include<cppp/rtl.hpp>
namespace sfe{
    using namespace std::literals;
    constexpr static cppp::fvec3 RED{1.0f,0.0f,0.0f};
    constexpr static cppp::fvec3 HIGHLIGHT{0.10588235294117647f,0.9607843137254902f,0.7294117647058823f};
    constexpr static cppp::fvec3 GRAY{0.7f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_1{0.5f,0.0f,0.0f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_2{0.8f,1.0f,1.0f};
    constexpr static cppp::fvec3 CURSOR_ACCENT_WEAK{0.3f,0.7f,0.7f};
    // static void draw_operand(const VisualNode& operand,const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const NameDatabase& names,const UICursor& cursor,cppp::fvec2& pos,float right,float left,std::uint32_t my_priority,bool altmode){
    //     bool parenthesize = altmode || operand.apriority() < my_priority;
    //     if(parenthesize){
    //         gc.draw_wrapped_text_at_cursor(u8"("sv,pos,right,left,1.0f,WHITE);
    //     }
    //     operand.draw(gc,errors,names,cursor,pos,right,left,altmode);
    //     if(parenthesize){
    //         gc.draw_wrapped_text_at_cursor(u8")"sv,pos,right,left,1.0f,WHITE);
    //     }
    // }
    // static void draw_binop(std::u8string_view op,const std::vector<VisualNode>& children,const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const NameDatabase& names,const UICursor& cursor,cppp::fvec2& pos,float right,float left,std::uint32_t my_priority,bool altmode){
    //     draw_operand(children[0uz],gc,errors,names,cursor,pos,right,left,my_priority,altmode);
    //     gc.draw_wrapped_text_at_cursor(op,pos,right,left,1.0f,RED);
    //     draw_operand(children[1uz],gc,errors,names,cursor,pos,right,left,my_priority+1,altmode);
    // }
    // std::uint32_t VisualNode::apriority() const{
    //     switch(node->type()){
    //         case bbe::NodeType::CALL:
    //             switch(node->getp32()){
    //                 case 10: // +
    //                 case 20: // -
    //                     return 3;
    //                 case 30: // *
    //                     return 4;
    //                 case 50: // =
    //                 case 51: // <=
    //                     return 2;
    //                 case 80: // []
    //                     return 5;
    //             }
    //             break;
    //         default:;
    //     }
    //     return 1984;
    // }
    void ASTVisualNode::draw(const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const NameDatabase& names,const UICursor& cursor,cppp::fvec2& pos,float right,float left,bool altmode) const{
        cppp::fvec2 start_pos = pos;
        cppp::fvec2 cursor_pos;
        bool selected = cursor.selected().is(*this);
        switch(node->type()){
            using enum bbe::NodeType;
            case ARG:
                gc.draw_wrapped_text_at_cursor(cppp::format<u8"arg{}"_ts>(node->getp32()),pos,right,left,1.0f,WHITE);
                cursor_pos = pos;
                break;
            case DEREF:
                gc.draw_wrapped_text_at_cursor(u8"*"sv,pos,right,left,1.0f,WHITE);
                _children[0uz].draw(gc,errors,names,cursor,pos,right,left,altmode);
                cursor_pos = pos;
                break;
            case ADDROF:
                gc.draw_wrapped_text_at_cursor(u8"&"sv,pos,right,left,1.0f,WHITE);
                _children[0uz].draw(gc,errors,names,cursor,pos,right,left,altmode);
                cursor_pos = pos;
                break;
            case FNSYM:
                // TODO
                gc.draw_wrapped_text_at_cursor(cppp::format<u8"[function {}]"_ts>(node->getp32()),pos,right,left,1.0f,RED);
                cursor_pos = pos;
                break;
            case BOOL:
                gc.draw_wrapped_text_at_cursor(node->getp32()?u8"true"s:u8"false"s,pos,right,left,1.0f,WHITE);
                cursor_pos = pos;
                break;
            case UINT32:
                gc.draw_wrapped_text_at_cursor(cppp::format<u8"{}"_ts>(node->getp32()),pos,right,left,1.0f,WHITE);
                cursor_pos = pos;
                gc.draw_wrapped_text_at_cursor(u8"dw"sv,pos,right,left,0.5f,GRAY);
                break;
            case SINT32:
                gc.draw_wrapped_text_at_cursor(cppp::format<u8"{}"_ts>(std::bit_cast<std::int32_t>(node->getp32())),pos,right,left,1.0f,WHITE);
                cursor_pos = pos;
                gc.draw_wrapped_text_at_cursor(u8"sdw"sv,pos,right,left,0.5f,GRAY);
                break;
            case OVERCALL:
                if(auto osn=names.optget_overload_set_name(node->getp32())){
                    gc.draw_wrapped_text_at_cursor(osn->identifier(),pos,right,left,1.0f,osn->color());
                }else{
                    gc.draw_wrapped_text_at_cursor(cppp::format<u8"[overload set {}]"_ts>(node->getp32()),pos,right,left,1.0f,RED);
                }
                gc.draw_wrapped_text_at_cursor(u8"("sv,pos,right,left,1.0f,WHITE);
                for(std::size_t i=0uz;i<_children.size();++i){
                    if(i) gc.draw_wrapped_text_at_cursor(u8","sv,pos,right,left,1.0f,WHITE);
                    _children[i].draw(gc,errors,names,cursor,pos,right,left,altmode);
                }
                gc.draw_wrapped_text_at_cursor(u8")"sv,pos,right,left,1.0f,WHITE);
                cursor_pos = pos;
                break;
            case CALL:
                _children[0uz].draw(gc,errors,names,cursor,pos,right,left,altmode);
                gc.draw_wrapped_text_at_cursor(u8"("sv,pos,right,left,1.0f,WHITE);
                for(std::size_t i=1uz;i<_children.size();++i){
                    if(i > 1uz) gc.draw_wrapped_text_at_cursor(u8","sv,pos,right,left,1.0f,WHITE);
                    _children[i].draw(gc,errors,names,cursor,pos,right,left,altmode);
                }
                gc.draw_wrapped_text_at_cursor(u8")"sv,pos,right,left,1.0f,WHITE);
                cursor_pos = pos;
                break;
            case PACK:
                gc.draw_wrapped_text_at_cursor(u8"("sv,pos,right,left,1.0f,WHITE);
                for(std::uint32_t i=0;i<_children.size();++i){
                    if(i) gc.draw_wrapped_text_at_cursor(u8","sv,pos,right,left,1.0f,WHITE);
                    _children[i].draw(gc,errors,names,cursor,pos,right,left,altmode);
                }
                gc.draw_wrapped_text_at_cursor(u8")"sv,pos,right,left,1.0f,WHITE);
                cursor_pos = pos;
                break;
            case COMMA: {
                gc.draw_wrapped_text_at_cursor(u8"{"sv,pos,right,left,1.0f,WHITE);
                float inner_indentation = left + 4 * gc.charadvance();
                for(std::uint32_t i=0;i<_children.size();++i){
                    pos.x() = inner_indentation;
                    pos.y() += gc.line_height();
                    if(i == node->getp32()) gc.draw_text_at_cursor(u8"·"sv,cppp::rtl<cppp::fvec2>(auto(pos - cppp::fvec2{gc.charadvance() * 2.0f,0.0f})),1.0f,HIGHLIGHT);
                    _children[i].draw(gc,errors,names,cursor,pos,right,inner_indentation,altmode);
                    gc.draw_wrapped_text_at_cursor(u8";"sv,pos,right,left,1.0f,WHITE);
                }
                pos.x() = left;
                pos.y() += gc.line_height();
                gc.draw_wrapped_text_at_cursor(u8"}"sv,pos,right,left,1.0f,WHITE);
                cursor_pos = pos;
                break;
            }
            case PACKIND:
                _children[0uz].draw(gc,errors,names,cursor,pos,right,left,altmode);
                gc.draw_wrapped_text_at_cursor(cppp::format<u8"[{}]"_ts>(node->getp32()),pos,right,left,1.0f,WHITE);
                cursor_pos = pos;
                break;
            case FORK:
                _children[0uz].draw(gc,errors,names,cursor,pos,right,left,altmode);
                gc.draw_wrapped_text_at_cursor(u8"?"sv,pos,right,left,1.0f,RED);
                _children[1uz].draw(gc,errors,names,cursor,pos,right,left,altmode);
                gc.draw_wrapped_text_at_cursor(u8":"sv,pos,right,left,1.0f,RED);
                _children[2uz].draw(gc,errors,names,cursor,pos,right,left,altmode);
                cursor_pos = pos;
                break;
            case NTYPE:
                gc.draw_wrapped_text_at_cursor(u8"_"sv,pos,right,left,1.0f,selected?RED:WHITE);
                goto anodrawsel;
            case HAVEVAR:
                gc.draw_wrapped_text_at_cursor(cppp::format<u8"let var#{}"_ts>(node->getp32()),pos,right,left,1.0f,WHITE);
                gc.draw_wrapped_text_at_cursor(u8" = "sv,pos,right,left,1.0f,RED);
                _children[0uz].draw(gc,errors,names,cursor,pos,right,left,altmode);
                gc.draw_wrapped_text_at_cursor(u8" in "sv,pos,right,left,1.0f,WHITE);
                _children[1uz].draw(gc,errors,names,cursor,pos,right,left,altmode);
                cursor_pos = pos;
                break;
            case GETVAR:
                gc.draw_wrapped_text_at_cursor(cppp::format<u8"var#{}"_ts>(node->getp32()),pos,right,left,1.0f,WHITE);
                cursor_pos = pos;
                break;
            case SETVAR:
                gc.draw_wrapped_text_at_cursor(cppp::format<u8"let var#{}"_ts>(node->getp32()),pos,right,left,1.0f,WHITE);
                gc.draw_wrapped_text_at_cursor(u8" = "sv,pos,right,left,1.0f,RED);
                _children[0uz].draw(gc,errors,names,cursor,pos,right,left,altmode);
                cursor_pos = pos;
                break;
            case UINT32SYM:
            case UINT64:
                // TODO
                gc.draw_wrapped_text_at_cursor(cppp::format<u8"**unimplemented: {}**"_ts>(std::to_underlying(node->type())),pos,right,left,1.0f,selected?RED:WHITE);
                break;
            case EXTERN_OR_INTRIN:
                if(std::uint32_t intr=node->getp32();intr!=bbe::Function::INTR_EXTERN){
                    switch(intr){
                        case 10:
                            gc.draw_wrapped_text_at_cursor(u8"(intrin addu32)"s,pos,right,left,1.0f,WHITE);
                            break;
                        case 20:
                            gc.draw_wrapped_text_at_cursor(u8"(intrin subu32)"s,pos,right,left,1.0f,WHITE);
                            break;
                        case 30:
                            gc.draw_wrapped_text_at_cursor(u8"(intrin mulu32)"s,pos,right,left,1.0f,WHITE);
                            break;
                        case 50:
                            gc.draw_wrapped_text_at_cursor(u8"(intrin equ32)"s,pos,right,left,1.0f,WHITE);
                            break;
                        case 51:
                            gc.draw_wrapped_text_at_cursor(u8"(intrin leu32)"s,pos,right,left,1.0f,WHITE);
                            break;
                        case 60:
                            gc.draw_wrapped_text_at_cursor(u8"(intrin notb)"s,pos,right,left,1.0f,WHITE);
                            break;
                        case 100:
                            gc.draw_wrapped_text_at_cursor(u8"(intrin printu32)"s,pos,right,left,1.0f,WHITE);
                            break;
                        default:
                            gc.draw_wrapped_text_at_cursor(cppp::format<u8"(intrin unknown {})"_ts>(intr),pos,right,left,1.0f,WHITE);
                            break;
                    }
                }else{
                    gc.draw_wrapped_text_at_cursor(u8"(extern)"s,pos,right,left,1.0f,WHITE);
                }
                cursor_pos = pos;
                break;
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
        if(!errors.query(node).empty()){
            gc.line(start_pos-cppp::fvec2(0.0f,gc.descender()+gc.line_height()/2.0f),RED,pos-cppp::fvec2(0.0f,gc.descender()+gc.line_height()/2.0f),RED);
        }
    }
    
    void TypeVisualNode::draw(const GraphicsContext& gc,const bbe::ErrorDatabase&,const NameDatabase& names,const UICursor& cursor,cppp::fvec2& pos,float right,float left,bool altmode) const{
        cppp::fvec2 start_pos = pos;
        cppp::fvec2 cursor_pos;
        bool selected = cursor.selected().is(*this);
        // TODO
        static_cast<void>(altmode);
        switch(data.tag()){
            case TypeVisualNodeType::DT: {
                if(dt()){
                    const Name& dtn{names.get_defined_type_name(*dt())};
                    gc.draw_wrapped_text_at_cursor(dtn.identifier(),pos,right,left,1.0f,dtn.color());
                    cursor_pos = pos;
                }else{
                    gc.draw_wrapped_text_at_cursor(u8"_"sv,pos,right,left,1.0f,selected?RED:WHITE);
                    goto anodrawsel;
                }
                break;
            }
            case TypeVisualNodeType::CT:
                gc.draw_wrapped_text_at_cursor(u8"/* unimplemented */"s,pos,right,left,1.0f,WHITE);
                cursor_pos = pos;
                break;
            default: cppp::unreachable();
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
    }
    void FunctionVisualNode::draw(const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const NameDatabase& names,const UICursor& cursor,cppp::fvec2& pos,bool altmode) const{
        float top_y = pos.y()-gc.ascender();
        float left_x = pos.x();
        float right_x;
        {
            cppp::fvec2 line_1{pos};
            gc.draw_text_at_cursor(func->cname(),line_1,1.0f,GRAY);
            gc.draw_text_at_cursor(u8"("sv,line_1,1.0f,WHITE);
            for(std::size_t i=0uz;i<parv.size();++i){
                if(i) gc.draw_text_at_cursor(u8","sv,line_1,1.0f,WHITE);
                parv[i].draw(gc,errors,names,cursor,line_1,gc.cmap().win_size().x()-10.0f,pos.x(),altmode);
            }
            gc.draw_text_at_cursor(u8") -> "sv,line_1,1.0f,WHITE);
            retv.draw(gc,errors,names,cursor,line_1,gc.cmap().win_size().x()-10.0f,pos.x(),altmode);
            gc.draw_text_at_cursor(u8":"sv,line_1,1.0f,WHITE);
            right_x = line_1.x();
        }
        pos += cppp::fvec2(gc.charadvance()*4.0f,gc.line_height());
        _body.draw(gc,errors,names,cursor,pos,gc.cmap().win_size().x()-10.0f,pos.x(),altmode);
        
        if(cursor.selected().is(*this)){
            float bottom_y = pos.y() - gc.descender();
            float x = cursor.is_after()?std::max(right_x,pos.x()):left_x;
            gc.line({x,top_y},CURSOR_ACCENT_1,{x,bottom_y},CURSOR_ACCENT_2);
        }
    }
    void OverloadSetVisualNode::draw(const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const NameDatabase& names,const UICursor& cursor,cppp::fvec2& pos,bool altmode) const{
        float top_y = pos.y()-gc.ascender();
        float left_x = pos.x();
        float right_x;
        {
            cppp::fvec2 line_1{pos};
            const Name& osn = names.get_overload_set_name(os->index());
            gc.draw_text_at_cursor(osn.identifier(),line_1,1.0f,osn.color());
            gc.draw_text_at_cursor(u8":"sv,line_1,1.0f,WHITE);
        }
        float left = pos.x() + gc.charadvance()*4.0f;
        for(const FunctionVisualNode& c : _children){
            pos.x() = left;
            pos.y() += gc.line_height();
            c.draw(gc,errors,names,cursor,pos,altmode);
        }
        if(cursor.selected().is(*this)){
            float bottom_y = pos.y() - gc.descender();
            float x = cursor.is_after()?std::max(right_x,pos.x()):left_x;
            gc.line({x,top_y},CURSOR_ACCENT_1,{x,bottom_y},CURSOR_ACCENT_2);
        }
    }
    void ProjectVisualNode::draw(const GraphicsContext& gc,const bbe::ErrorDatabase& errors,const NameDatabase& names,const UICursor& cursor,cppp::fvec2& pos,bool altmode) const{
        float left_x = pos.x();
        for(const OverloadSetVisualNode& ch : _children){
            ch.draw(gc,errors,names,cursor,pos,altmode);
            pos.x() = left_x;
            pos.y() += gc.line_height();
        }
        if(cursor.selected().is(*this)){
            cppp::fvec2 botright = gc.cmap().win_size()-cppp::fvec2{10.0f,10.0f};
            gc.line({10.0f,10.0f},CURSOR_ACCENT_1,{botright.x(),10.0f},CURSOR_ACCENT_2);
            gc.line({botright.x(),10.0f},CURSOR_ACCENT_2,botright,CURSOR_ACCENT_1);
            gc.line({10.0f,10.0f},CURSOR_ACCENT_1,{10.0f,botright.y()},CURSOR_ACCENT_2);
            gc.line({10.0f,botright.y()},CURSOR_ACCENT_2,botright,CURSOR_ACCENT_1);
        }
    }
}
