#ifndef CMD_HOST_H
#define CMD_HOST_H

#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <thread>

using namespace godot;

class CmdHost : public TextEdit {
    GDCLASS(CmdHost, TextEdit);

private:
    int master_fd = -1;
    pid_t child_pid = -1;
    std::thread reader_thread;
    bool running = false;
    
protected:
    static void _bind_methods();

public:
    CmdHost();
    ~CmdHost();
    virtual void _ready();
    virtual void _exit_tree();
    void main_loop();
    void start_pseudoterminal_session();
    void end_pseudoterminal_session();
    void _gui_input(const Ref<InputEvent> &event);
    void edit_text(const String &newtext);
    void write_to_cmd(const String &input);
};

#endif // CMD_HOST_H
