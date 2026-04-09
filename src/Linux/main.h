#ifndef LINUXHOST_H
#define LINUXHOST_H

#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/classes/syntax_highlighter.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/font.hpp>
#include <sys/types.h>
#include <queue>
#include <deque>

enum class ParseState {
    Normal,
    Escape,
    CSI
};


struct Segment {
    godot::String text{""};
    int32_t starting_column{0};
    uint32_t color{0xffffff};
    uint32_t bg_color{0x000000};
    bool bold{false};
    bool operator==(const Segment &other) const;
};

class AnsiHighlighter : public godot::SyntaxHighlighter {
    GDCLASS(AnsiHighlighter, godot::SyntaxHighlighter);

    protected:
        static void _bind_methods();

    public:
     [[nodiscard]] godot::Dictionary _get_line_syntax_highlighting(int line) const override;
};

class LinuxHost : public godot::TextEdit {
    GDCLASS(LinuxHost, godot::TextEdit);


    int master_fd{-1};
    int slave_fd{-1};
    pid_t child_pid{-1};
    bool running{false};
    std::queue<godot::String> output_queue;
    godot::String input;
    godot::String history_temp;
    int32_t history_index{0};
    godot::Vector2i input_start_line_col{0, 0};
    int MAX_LINES_PER_FRAME{50};
    int TOTAL_MAX_LINES{22560};
    godot::Ref<AnsiHighlighter> highlighter;
    godot::Ref<godot::Font> font;
    godot::PackedStringArray history;
    std::deque<godot::Vector<Segment>> segments_to_line;

    void load_history(const uint32_t &max_lines);
    [[nodiscard]] int64_t get_relative_caret_idx() const;
    static bool file_exists(const char *path);
    bool clamp_caret();
    void bulk_remove(const int32_t &to_line);

    static int ansi_to_color(const int &code);
    static int ansi256_to_color(const int &code);

    static void apply_style(int code, Segment &seg);

    static void apply_args(Segment &seg, const godot::String &args);
    void get_color_highlighting(const godot::String &ansi_string, godot::String &frame_text);

protected:
    static void _bind_methods();

public:
    void _ready() override;
    void start_pseudoterminal();
    void end_pseudoterminal();
    void write_to_terminal(const godot::String &text);
    void _gui_input(const godot::Ref<godot::InputEvent> &event) override;
    void _process(double p_delta) override;
    void _draw() override;
    void read_from_terminal();
    [[nodiscard]] std::deque<godot::Vector<Segment>> get_segments_to_line() const;
};
#endif