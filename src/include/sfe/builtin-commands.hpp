#pragma once
#include"editor.hpp"
namespace sfe::commands{
    void open_command_palette(Window&);
    void save(Window&);
    void load(Window&);
    void reset_cursor(Window&);
    void quit(Window&);
    void debug_selection(Window&);
    void interpret(Window&);
    void compile_and_run(Window&);
}
