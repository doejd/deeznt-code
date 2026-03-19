#ifndef LINUXHOST_H
#define LINUXHOST_H

#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/classes/syntax_highlighter.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/font.hpp>
#include <sys/types.h>
#include <queue>

using namespace godot;

struct Span {
    uint32_t start{};
    uint32_t length{};
    uint32_t color{0xffffff};
    uint32_t bg_color{0x000000};
    bool bold = false;
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
        Vector<int32_t> line_start_index;
        void rebuild_line_indexing();
        Vector2i from_index_get_line_column(const int32_t &index) const;
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
    uint32_t history_index = 0;
    uint32_t input_start_index = 0;
    int MAX_LINES_PER_FRAME{50};
    int TOTAL_MAX_LINES{22560};
    Ref<AnsiHighlighter> highlighter;
    Ref<Font> font;
    PackedStringArray history;
    static uint32_t ansi256_to_color(const uint32_t &code);
    static uint32_t ansi_to_color(const uint32_t &code);
    static void apply_args(Span &span, const String &args);
    static void apply_style(uint32_t code, Span &span);
    static bool file_exists(const char* path);
    [[nodiscard]] uint32_t get_caret_index() const;
    void load_history(uint32_t max_lines);
    static void merge_same_spans(Vector<Span> &spans);

protected:
    static void _bind_methods();

public:
    void _ready() override;
    void _exit_tree() override;
    void _process(double p_delta) override;
    void _draw() override;
    void read_from_terminal();
    void end_pseudoterminal();
    void start_pseudoterminal();
    void write_to_terminal(const String &text);
    void _gui_input(const Ref<InputEvent> &event) override;
    Vector<Span> get_color_highlighting(const String &ansi_strip, String &frame_text, const uint32_t &base_offset) const;
    void clamp_caret();
};
#endif