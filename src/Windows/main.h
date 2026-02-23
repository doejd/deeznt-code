#ifndef CMD_HOST_H
#define CMD_HOST_H


#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_action.hpp>
#include <godot_cpp/classes/syntax_highlighter.hpp>
#include <windows.h>
#include <queue>

using namespace godot;

struct Segment {
    String text = "";
    int color = 0xffffff; // Color in hex format
    int bg_color = 0x000000; // Color in hex format
    bool bold = false;
    bool underlined = false;
    bool italics = false;
};

class AnsiHighlighter : public SyntaxHighlighter {
    GDCLASS(AnsiHighlighter, SyntaxHighlighter);

protected:
    static void _bind_methods();

public:
    struct Span {
        int32_t start{};
        int32_t length{};
        Color color;
        Color bg_color;
        bool bold = false;
        bool italics = false;
        bool underline = false;
    };
    Dictionary default_style;
    Vector<Span> spans;
    Vector<int64_t> line_start_index;
    void rebuild_line_indexing();
    Vector2i from_index_get_line_column(const int32_t &index) const;
    Dictionary _get_line_syntax_highlighting(int line) const override;
    void default_style_dict();
};

class WindowsHost : public TextEdit {
    GDCLASS(WindowsHost, TextEdit);

    SECURITY_ATTRIBUTES sa{};
    HANDLE child_stdin_read, parent_stdin_write;
    HANDLE parent_stdout_read, child_stdout_write;
    HPCON hPC;
    COORD size{80, 25};
    SIZE_T attrSize = 0;
    STARTUPINFOEXW si;
    PROCESS_INFORMATION pi{};
    String input;
    int32_t input_start_index = 0;
    bool running = false;
    const int16_t MAX_QUEUE_SIZE{5000};
    String history_temp;
    int32_t history_index = 0;
    PackedStringArray history;
    std::queue<String> output_queue;
    Ref<AnsiHighlighter> highlighter;
    Vector<Segment> segments;
    static void apply_style(int code, Segment &seg);
    static int ansi256_to_color(const int &code);
    static int ansi_to_color(const int &code);
    static void apply_args(Segment &seg, const String &args);
    int32_t get_caret_index() const;

protected:
    static void _bind_methods();

public:
    void _ready() override;
    void _exit_tree() override;
    void _process(double delta);
    void read_from_terminal();
    void end_pseudoconsole_session();
    void start_pseudoconsole_session();
    void write_to_pwsh(const String &text);
    void _gui_input(const Ref<InputEvent> &event) override;
    void get_color_highlighting(const String &ansi_strip);
    void clamp_caret();
};


#endif // CMD_HOST_H