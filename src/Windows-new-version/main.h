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

class Cursor{
    private:
        float elapsed_ms = 0.0f;
         
    public:
        int col = 0;
        bool visible = true;
        float blink_time_ms = 500.0f;
        void move_left() {if (col > 0) col--;};
        void move_right(int max_col) {if (col < max_col) col++;};
        void reset() {col = 0;};
        void clamp(int max_col) {
            if (max_col < 0) max_col = 0;
            if (col < 0) col = 0;
            if (col > max_col) col = max_col;
        }
        void blink(float delta_ms) {
            elapsed_ms += delta_ms;
            if (elapsed_ms >= blink_time_ms) {
                visible = !visible;
                elapsed_ms = 0.0f;
            }
        }
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
    Cursor cursor;
    float blink_time = 500.0f;
    
protected:
    static void _bind_methods();

public:
    virtual void _ready();
    virtual void _exit_tree();
    void _draw();
    void _gui_input(const Ref<InputEvent> &event);
    void _process(double delta);
    void append_ansi_text(const String &text);
    void start_pseudoconsole_session();
    void end_pseudoconsole_session();
    void main_loop();
    void write_to_cmd(const String &input);
    void set_blink_time_ms(float time);
    float get_blink_time_ms() const;
    
private:
    void parse_ansi_and_append(const String &raw_text);
    bool find_pwsh(std::wstring &path_out);
};

#endif // CMD_HOST_H
