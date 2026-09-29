#include<sfe/builtin-commands.hpp>
#include<sfe/achievements.hpp>
#include<cppp/static-functor.hpp>
#include<bbe/targets/x86.hpp>
#include<bbe/formats/elf.hpp>
#include<bbe/targets/dfg.hpp>
#include<bbe/inter/dfg.hpp>
#include<bbe/inter/magic.hpp>
#include<bbe/project_entity_pool.hpp>
#include<cppp/format.hpp>
#include<SDL3/SDL_events.h>
#include<cppp/bfile.hpp>
#include<cppp/print.hpp>
#include<cppp/int.hpp>
#include<cstdlib>
#include<stdio.h> // popen
#include<numeric>
#include<ranges>
#include<chrono>
#include<memory>
namespace sfe::commands{
    void open_command_palette(Window& ed,void*){
        ed.open_command_palette();
    }
    void rename_selection(Window& ed,void*){
        if(ed.code().cursor().selected().type() == VisualNodeType::F){
            ed.set_textbox(cppp::uvec3{0,0,10},1.0f /* TODO: actually compute layout */,TextboxTargetType::RAW_STRING,&ed.project().names().get_function_name(ed.code().cursor().selected().f().index()));
        }
    }
    void re_cname_selection(Window& ed,void*){
        if(ed.code().cursor().selected().type() == VisualNodeType::F){
            ed.set_textbox(cppp::uvec3{0,0,10},1.0f /* TODO: actually compute layout */,TextboxTargetType::RAW_STRING,&ed.code().cursor().selected().fcname());
        }
    }
    void recolor_selection(Window& ed,void*){
        if(ed.code().cursor().selected().type() == VisualNodeType::F){
            ed.remove_textbox();
            ed.color_picker().open(ed.project().names().get_function_name(ed.code().cursor().selected().f().index()).color());
        }
    }
    void save(Window& ed,void*){
        cppp::bytes save;
        bbe::SCM scm{ed.project().entities().serialize(save)};
        {
            cppp::BinaryFile bf{u8"testprog.bc"s,std::ios_base::out|std::ios_base::trunc|std::ios_base::binary};
            bf.write(save);
            save.clear();
        }
        {
            ed.project().names().serialize(save,scm);
            cppp::BinaryFile nf{u8"testprog.bc.nt"s,std::ios_base::out|std::ios_base::trunc|std::ios_base::binary};
            nf.write(save);
        }
        ed.toast().reset(cppp::format<u8"Saved {} bytes"_ts>(save.size()),1s);
    }
    void load(Window& ed,void*){
        ed.color_picker().close();
        ed.remove_textbox();
        cppp::bytes save;
        {
            cppp::read_file(save,u8"testprog.bc"s);
            cppp::frozen_byte_view scanner{save};
            ed.project().entities() = {scanner};
            save.clear();
        }
        {
            cppp::read_file(save,u8"testprog.bc.nt"s);
            cppp::frozen_byte_view scanner{save};
            ed.project().names() = {ed.project().entities().types(),scanner};
        }
        ed.code().root().prepopulate();
        ed.code().cursor().home();
    }
    void reset_cursor(Window& ed,void*){
        ed.code().cursor().home();
    }
    void inline_transform_initialize(bbe::ASTNode& dst,const bbe::ASTNode& src){
        if(src.type() == bbe::NodeType::ARG){
            dst.initialize(bbe::NodeType::GETVAR,0);
        }else{
            dst.transform_initialize(src,inline_transform_initialize);
        }
    }
    void inline_function(Window& ed,void*){
        if(VisualNode& sel = ed.code().cursor().selected();sel.type() == VisualNodeType::A && sel.a().type() == bbe::NodeType::CALL && sel.a().children()[0_u32].type() == bbe::NodeType::FNSYM){
            const bbe::Function& inlf = ed.project().entities().functions()[sel.a().children()[0_u32].getp32()];
            if(!inlf.is_intrin()){
                VisualNode::ANodeHandle anh{sel.amodify()};
                bbe::ASTNode old = std::exchange(*anh,{bbe::NodeType::HAVEVAR,0,2,bbe::uninitialize});
                anh->children()[0_u32].initialize(std::move(old.children()[1_u32]));
                inline_transform_initialize(anh->children()[1_u32],inlf.ast());
            }
        }
    }
    void quit(Window&,void*){
        SDL_Event ev{.quit={
            .type=SDL_EVENT_QUIT,
            .reserved=0,
            .timestamp=SDL_GetTicksNS()
        }};
        SDL_PushEvent(&ev);
    }
    void debug_selection(Window& ed,void*){
        if(ed.code().cursor().selected().type() == VisualNodeType::A){
            cppp::print<u8"{:p} = {}"_ts>(static_cast<const void*>(&ed.code().cursor().selected().a()),std::to_underlying(ed.code().cursor().selected().a().type()));
        }
        std::println();
    }
    namespace{
        using hrc = std::chrono::high_resolution_clock;
        using µs = std::chrono::duration<std::uint64_t,std::micro>;
        template<typename F>
        µs time_execution(const F& f){
            auto begin = hrc::now();
            f();
            auto dur = hrc::now()-begin;
            return std::chrono::duration_cast<µs>(dur);
        }
    }
    void interpret(Window& ed,void* edb){
        if(!static_cast<bbe::ErrorDatabase*>(edb)->empty()) return;
        try{
            cppp::str rbuf;
            {
                bbe::inter::dfg::CompiledFunctionPool compiled{ed.project().entities()};
                bbe::func_id entry = std::numeric_limits<bbe::func_id>::max();
                for(const bbe::Function& f : ed.project().entities().functions()){
                    if(f.cname() == u8"example"sv) entry = f.index();
                }
                if(entry == std::numeric_limits<bbe::func_id>::max()) throw std::logic_error("No entry point found"s);
                µs delta = time_execution([&]{
                    bbe::inter::stringify(compiled.call(entry,bbe::inter::uint32v{30}),rbuf);
                });
                std::span<int> a;
                if(delta.count() < 1000){
                    cppp::format_to<u8" in {} µs (dfg inter)"_ts>(rbuf,delta.count());
                }else{
                    cppp::format_to<u8" in {:.2f} ms (dfg inter)"_ts>(rbuf,static_cast<float>(delta.count())/1000.0f);
                }
            }
            // {
            //     bbe::inter::rtl::CompiledFunctionPool compiled{ed.project().entities()};
            //     µs delta = time_execution([&]{
            //         bbe::inter::stringify(compiled.call(0,{bbe::inter::uint32v{30}}),rbuf);
            //     });
            //     if(delta.count() < 1000){
            //         cppp::format_to<u8" in {} µs (rtl inter)"_ts>(rbuf,delta.count());
            //     }else{
            //         cppp::format_to<u8" in {:.2f} ms (rtl inter)"_ts>(rbuf,static_cast<float>(delta.count())/1000.0f);
            //     }
            // }
            ed.toast().reset(std::move(rbuf),3s);
        }catch(const std::exception& e){
            ed.toast().reset(cppp::tou8(std::string_view(e.what())),3s);
        }
    }
    void compile_and_run(Window& ed,void* edb){
        if(!static_cast<bbe::ErrorDatabase*>(edb)->empty()) return;
        try{
            cppp::str rbuf;
            {
                bbe::formats::elf::Elf elf;
                elf.add_text(ed.project().entities());
                cppp::BinaryFile outf{u8"testprog_c.o"s,std::ios_base::out|std::ios_base::binary|std::ios_base::trunc};
                outf.write(elf.encode());
            }
            if(int ret=std::system("g++ -O3 -m64 -s timing_helper.o testprog_c.o -o testprog_c")){
                throw std::runtime_error(std::format("GCC failed with: {}"sv,ret));
            }
            if(std::unique_ptr<std::FILE,cppp::static_functor<pclose>> dl{popen(
                reinterpret_cast<const char*>(cppp::format<u8"./testprog_c {}"_ts>(30).c_str())
                ,"r")}){
                std::array<char8_t,1024uz> buf;
                while(std::size_t nr = std::fread(buf.data(),1,buf.size(),dl.get())){
                    rbuf.append(buf.data(),nr);
                }
                if(std::ferror(dl.get())) rbuf.append(u8"<READ ERROR>"s);
                rbuf.append(u8" (x86_64)"s);
            }else{
                throw std::runtime_error("can't start testprog"s);
            }
            ed.toast().reset(std::move(rbuf),3s);
        }catch(const std::exception& e){
            ed.toast().reset(cppp::tou8(std::string_view(e.what())),3s);
        }
    }
    void adjc(Window& ed,void* dir){
        bool up = (dir != nullptr);
        for(const auto& elt : ed.code().cursor().path_elements() | std::views::reverse){
            if(elt.p->type() != VisualNodeType::A) break;
            if(elt.p->a().type() == bbe::NodeType::COMMA){
                if(up){
                    elt.p->asetp32(std::saturating_sub(elt.p->a().getp32(),1_u32));
                }else{
                    std::uint32_t newind = elt.p->a().getp32() + 1;
                    if(newind < elt.p->a().children().size()){
                        elt.p->asetp32(newind);
                    }
                }
            }
        }
    }
    void gc(Window& ed,void*
    #ifdef SFE_ENABLE_ACHIEVEMENTS
        adb
    #endif
    ){
        bbe::TypeSweeper swp{ed.project().entities().begin_gc()};
        ed.project().names().garbage_collect(swp);
        ed.project().entities().end_gc(std::move(swp));
        #ifdef SFE_ENABLE_ACHIEVEMENTS
        AchievementDB& d = *static_cast<AchievementDB*>(adb);
        d.queue.complete(d.garbage_collected);
        #endif
    }
}
