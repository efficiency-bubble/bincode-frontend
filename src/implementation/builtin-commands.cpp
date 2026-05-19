#include<sfe/builtin-commands.hpp>
#include<cppp/static-functor.hpp>
#include<bbe/targets/x86.hpp>
#include<bbe/formats/elf.hpp>
#include<bbe/targets/rtl.hpp>
#include<bbe/targets/dfg.hpp>
#include<bbe/inter/rtl.hpp>
#include<bbe/inter/dfg.hpp>
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
    void open_command_palette(sfe::Window& w){
        w.open_command_palette();
    }
    void save(sfe::Window& ed){
        cppp::bytes save;
        static_cast<const sfe::VisualFunctionNode&>(ed.code().root()).func().ast().serialize(save);
        cppp::BinaryFile bf{u8"testprog"s,std::ios_base::out|std::ios_base::trunc|std::ios_base::binary};
        bf.write(save);
        ed.toast().reset(cppp::format<u8"Saved {} bytes"_ts>(save.size()),1s);
    }
    void load(sfe::Window& ed){
        cppp::BinaryFile bf{u8"testprog"s,std::ios_base::in|std::ios_base::binary};
        cppp::bytes save;
        std::array<std::byte,1024uz> buf;
        std::size_t nread;
        do{
            nread = bf.read(buf);
            save.append(std::span{buf.data(),nread});
        }while(nread);
        static_cast<sfe::VisualFunctionNode&>(ed.code().root()).func().ast() = bbe::ASTNode(cppp::rtl<cppp::frozen_byte_view>(save));
        ed.code().root().rerender_children();
        ed.code().home();
    }
    void reset_cursor(sfe::Window& ed){
        ed.code().home();
        ed.code().root().rerender_children();
    }
    void quit(sfe::Window&){
        SDL_Event ev{.quit={
            .type=SDL_EVENT_QUIT,
            .reserved=0,
            .timestamp=SDL_GetTicksNS()
        }};
        SDL_PushEvent(&ev);
    }
    void debug_selection(sfe::Window& ed){
        if(auto* vn=dynamic_cast<sfe::VisualASTNode*>(&ed.code().selected())){
            cppp::print<u8"{:p} = {}"_ts>(static_cast<void*>(vn),std::to_underlying(vn->type()));
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
    void interpret(sfe::Window& ed){
        try{
            cppp::str rbuf;
            {
                bbe::inter::dfg::CompiledFunctionPool compiled{ed.project().entities()};
                µs delta = time_execution([&]{
                    bbe::inter::stringify(compiled.call(0,{bbe::inter::uint32v{30}}),rbuf);
                });
                std::span<int> a;
                if(delta.count() < 1000){
                    cppp::format_to<u8" in {} µs (dfg inter); "_ts>(rbuf,delta.count());
                }else{
                    cppp::format_to<u8" in {:.2f} ms (dfg inter); "_ts>(rbuf,static_cast<float>(delta.count())/1000.0f);
                }
            }
            {
                bbe::inter::rtl::CompiledFunctionPool compiled{ed.project().entities()};
                µs delta = time_execution([&]{
                    bbe::inter::stringify(compiled.call(0,{bbe::inter::uint32v{30}}),rbuf);
                });
                if(delta.count() < 1000){
                    cppp::format_to<u8" in {} µs (rtl inter)"_ts>(rbuf,delta.count());
                }else{
                    cppp::format_to<u8" in {:.2f} ms (rtl inter)"_ts>(rbuf,static_cast<float>(delta.count())/1000.0f);
                }
            }
            ed.toast().reset(std::move(rbuf),3s);
        }catch(const std::exception& e){
            ed.toast().reset(cppp::tou8(std::string_view(e.what())),3s);
        }
    }
    void compile_and_run(sfe::Window& ed){
        try{
            cppp::str rbuf;
            {
                bbe::formats::elf::Elf elf;
                bbe::targets::x86::Program prog;
                {
                    bbe::targets::x86::Function fn{ed.code().root().func(),ed.project().entities().types()};
                    cppp::format_to<u8"{} bytes; "_ts>(rbuf,fn.instructions().size());
                    prog.export_function(u8"example"s,std::move(fn));
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
