#ifndef CMDHOST_H
#define CMDHOST_H

#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/classes/syntax_highlighter.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <sys/types.h>
#include <atomic>
#include <queue>

using namespace godot;

struct Span {
    int64_t start{};
    int32_t length{};
    int color{0xffffff};
    int bg_color{0x000000};
    bool bold = false;
    bool italics = false;
    bool underline = false;
    bool operator==(const Span &other) const;
    bool operator!=(const Span &other) const;
};

class AnsiHighlighter : public SyntaxHighlighter {
    GDCLASS(AnsiHighlighter, SyntaxHighlighter);

    protected:
        static void _bind_methods();

    public:
        Dictionary default_style;
        Vector<Vector<Span>> spans_per_line;
        Vector<int64_t> line_start_index;
        void rebuild_line_indexing();
        Vector2i from_index_get_line_column(const int64_t &index) const;
        Dictionary _get_line_syntax_highlighting(int line) const override;
        void default_style_dict();
        void append_span(const Span &new_span);
};

class LinuxHost : public TextEdit {
    GDCLASS(LinuxHost, TextEdit);

    int master_fd = -1;
    int slave_fd = -1;
    pid_t child_pid = -1;
    bool running = false;
    std::queue<String> output_queue;
    String input;
    String history_temp;
    int32_t history_index = 0;
    int32_t input_start_index = 0;
    Ref<AnsiHighlighter> highlighter;
    PackedStringArray history;
    static int ansi256_to_color(const int &code);
    static int ansi_to_color(const int &code);
    static void apply_args(Span &span, const String &args);
    static void apply_style(int code, Span &span);
    static bool file_exists(const char* path);
    [[nodiscard]] int64_t get_caret_index() const;
    void load_history(int max_lines);
    static void merge_same_spans(Vector<Span> &spans);

protected:
    static void _bind_methods();

public:
    void _ready() override;
    void _exit_tree() override;
    void _process(double p_delta) override;
    void read_from_terminal();
    void end_pseudoterminal();
    void start_pseudoterminal();
    void write_to_terminal(const String &text);
    void _gui_input(const Ref<InputEvent> &event) override;
    Vector<Span> get_color_highlighting(const String &ansi_strip, String &frame_text) const;
    void clamp_caret();
};
#endif