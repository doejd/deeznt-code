#ifndef CMD_HOST_H
#define CMD_HOST_H

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <windows.h>
#include <atomic>
#include <thread>

using namespace godot;

struct Segment {
    String text;
    Color color;
    bool bold = false;
    bool underlined = false;
};

class PwshHost : public Control {
    GDCLASS(PwshHost, Control);

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
    Vector<Vector<Segment>> lines;
    Ref<Font> font;
    Vector<String> history;
    int hist_ind = -1;
    String current_input;
    size_t cursor_x = 0;
    size_t cursor_y = 0;
    
protected:
    static void _bind_methods();

public:
    virtual void _ready();
    virtual void _exit_tree();
    void _draw();
    void _gui_input(const Ref<InputEvent> &event);
    void append_ansi_text(const String &text);
    void start_pseudoconsole_session();
    void end_pseudoconsole_session();
    void main_loop();
    void write_to_cmd(const String &input);
    
private:
    void parse_ansi_and_append(const String &raw_text);
    bool find_pwsh(std::wstring &path_out);
};

#endif // CMD_HOST_H
