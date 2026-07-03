#include<sfe/builtin-commands.hpp>
#include<cppp/static-functor.hpp>
#include<bbe/targets/x86.hpp>
#include<bbe/formats/elf.hpp>
#include<bbe/targets/rtl.hpp>
#include<bbe/targets/dfg.hpp>
#include<bbe/inter/rtl.hpp>
#include<bbe/inter/dfg.hpp>
#include<bbe/inter/magic.hpp>
#include<bbe/project_entity_pool.hpp>
#include<cppp/format.hpp>
#include<SDL3/SDL_events.h>
#include<cppp/bfile.hpp>
#include<cppp/print.hpp>
#include<cstdlib>
#include<stdio.h> // popen
#include<chrono>
#include<memory>
namespace sfe::commands{
    using namespace cppp::literals;
    void open_command_palette(Window& ed,void*){
        ed.open_command_palette();
    }
    void rename_selection(Window& ed,void*){
        if(ed.code().cursor().selected().type() == VisualNodeType::F){
            ed.set_textbox(cppp::uvec3{0,0,10},1.0f /* TODO: actually compute layout */,TextboxTargetType::FUNCTION_NAME,&ed.project().names().get_function_name(ed.code().cursor().selected().f().index()));
        }
    }
    void recolor_selection(Window& ed,void*){
        if(ed.code().cursor().selected().type() == VisualNodeType::F){
            ed.color_picker().open(ed.project().names().get_function_name(ed.code().cursor().selected().f().index()).color());
        }
    }
    void save(Window& ed,void*){
        cppp::bytes save;
        bbe::SCM scm{ed.project().entities().serialize(save)};
        ed.project().names().serialize(save,scm);
        cppp::BinaryFile bf{u8"testprog"s,std::ios_base::out|std::ios_base::trunc|std::ios_base::binary};
        bf.write(save);
        ed.toast().reset(cppp::format<u8"Saved {} bytes"_ts>(save.size()),1s);
    }
    void load(Window& ed,void*){
        ed.color_picker().close();
        ed.remove_textbox();
        cppp::BinaryFile bf{u8"testprog"s,std::ios_base::in|std::ios_base::binary};
        cppp::bytes save;
        std::array<std::byte,1024uz> buf;
        std::size_t nread;
        do{
            nread = bf.read(buf);
            save.append(std::span{buf.data(),nread});
        }while(nread);
        cppp::frozen_byte_view scanner{save};
        ed.project().entities() = {scanner};
        ed.project().names() = {scanner};
        ed.code().root().prepopulate();
        ed.code().cursor().home();
    }
    void reset_cursor(Window& ed,void*){
        ed.code().cursor().home();
        ed.code().root().prerender();
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
            cppp::print<u8"{:p} = {}"_ts>(static_cast<void*>(&ed.code().cursor().selected().a()),std::to_underlying(ed.code().cursor().selected().a().type()));
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
                µs delta = time_execution([&]{
                    bbe::inter::stringify(compiled.call(0,bbe::inter::uint32v{30}),rbuf);
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
                bbe::targets::x86::Program prog;
                {
                    std::size_t cumsize = 0uz;
                    for(const auto& fn : ed.code().root().p().functions()){
                        bbe::targets::x86::Function compiled{fn,ed.project().entities().types()};
                        cumsize = compiled.instructions().size();
                        prog.export_function(cppp::format<u8"fn_{}"_ts>(fn.index()),fn.index(),std::move(compiled));
                    }
                    cppp::format_to<u8"{} bytes; "_ts>(rbuf,cumsize);
                }
                elf.add_text(prog);
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
}
