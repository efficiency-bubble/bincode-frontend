#pragma once
#include"editor.hpp"
namespace sfe::commands{
    void open_command_palette(sfe::Window&);
    void save(sfe::Window&);
    void load(sfe::Window&);
    void reset_cursor(sfe::Window&);
    void quit(sfe::Window&);
    void debug_selection(sfe::Window&);
    void interpret(sfe::Window&);
    void compile_and_run(sfe::Window&);
}
