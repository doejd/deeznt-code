#ifndef CMDHOST_H
#define CMDHOST_H

#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/classes/syntax_highlighter.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <sys/types.h>
#include <thread>
#include <atomic>

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
        Vector<int32_t> line_start_index;
        void rebuild_line_indexing();
        Vector2i from_index_get_line_column(const int32_t &index) const;
        Dictionary _get_line_syntax_highlighting(int line) const override;
        void default_style_dict();
};

class LinuxHost : public TextEdit {
    GDCLASS(LinuxHost, TextEdit);

    int master_fd = -1;
    int slave_fd = -1;
    pid_t child_pid = -1;
    std::thread reader_thread;
    std::atomic<bool> running = false;
    String input;
    String history_temp;
    int32_t history_index = 0;
    int32_t input_start_index = 0;
    Ref<AnsiHighlighter> highlighter;
    Vector<Segment> segments;
    Vector<String> history;
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
    void reader_loop();
    void end_pseudoterminal();
    void start_pseudoterminal();
    void write_to_terminal(const String &text);
    void _gui_input(const Ref<InputEvent> &event) override;
    void get_color_highlighting(const String &ansi_strip);
    void clamp_caret();
};

#endif
