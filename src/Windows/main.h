#ifndef CMD_HOST_H
#define CMD_HOST_H


#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_action.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <windows.h>
#include <atomic>
#include <thread>

using namespace godot;

class CmdHost : public TextEdit {
    GDCLASS(CmdHost, TextEdit);

private:
    SECURITY_ATTRIBUTES sa{};
    HANDLE child_stdin_read, parent_stdin_write;
    HANDLE parent_stdout_read, child_stdout_write;
    HANDLE reader_thread_handle = nullptr;
    HPCON hPC;
    COORD size{80, 25};
    SIZE_T attrSize = 0;
    STARTUPINFOEXW si;
    PROCESS_INFORMATION pi{};
    std::thread reader_thread;
    std::atomic<bool> running = false;
    
protected:
    static void _bind_methods();

public:
    CmdHost();
    ~CmdHost();
    virtual void _ready();
    virtual void _exit_tree();
    void _gui_input(const Ref<InputEvent> &event);
    void edit_text(const String &newtext);
    void start_pseudoconsole_session();
    void end_pseudoconsole_session();
    void main_loop();
    void write_to_cmd(const String &input);
};

#endif // CMD_HOST_H
