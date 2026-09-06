#pragma once
#include"editor.hpp"
namespace sfe::commands{
    void open_command_palette(Window&,void*);
    void rename_selection(Window&,void*);
    void re_cname_selection(Window&,void*);
    void recolor_selection(Window&,void*);
    void save(Window&,void*);
    void load(Window&,void*);
    void adjc(Window&,void*);
    void gc(Window&,void*);
    void reset_cursor(Window&,void*);
    void inline_function(Window&,void*);
    void quit(Window&,void*);
    void debug_selection(Window&,void*);
    void interpret(Window&,void*);
    void compile_and_run(Window&,void*);
}
