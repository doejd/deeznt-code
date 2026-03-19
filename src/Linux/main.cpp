#define UNICODE
#ifdef __linux__

#include "main.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_action.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <unistd.h>
#include <pty.h>
#include <utmp.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <string>

using namespace godot;

bool Span::operator==(const Span &other) const {
    return this->color == other.color &&
           this->bg_color == other.bg_color &&
           this->bold == other.bold;
}

bool Span::operator!=(const Span &other) const {return !operator==(other);}

void AnsiHighlighter::_bind_methods() {}

void AnsiHighlighter::rebuild_line_indexing() {
    int32_t cur_index = 0;
    line_start_index.clear();
    line_start_index.push_back(cur_index);

    for (int32_t i = 0; i < get_text_edit()->get_line_count(); i++) {
        cur_index += static_cast<int32_t>(get_text_edit()->get_line(i).length()) + 1;
        line_start_index.push_back(cur_index);
    }
    spans_per_line.resize(line_start_index.size() - 1);
}

Vector2i AnsiHighlighter::from_index_get_line_column(const int32_t &index) const {
    int32_t L = 0, R = static_cast<int32_t>(line_start_index.size()) - 1;
    while (L <= R) {
        if (const int32_t M = (L + R) / 2; line_start_index[M] <= index) L = M + 1;
        else R = M - 1;
    }

    return {R, index - line_start_index[R]};
}

Dictionary AnsiHighlighter::_get_line_syntax_highlighting(const int line) const {
    Dictionary res;
    const auto host = cast_to<TextEdit>(get_text_edit());
    if (!host) return res;
    if (line < 0 || line >= spans_per_line.size()) return res;

    for (const Span &span : spans_per_line[line]) {
        const Vector2i p0 = from_index_get_line_column(span.start);
        const Vector2i p1 = from_index_get_line_column(span.start + span.length);

        if (line < p0.x || line > p1.x) continue;

        const int start_col = (line == p0.x) ? p0.y : 0;
        const int end_col   = (line == p1.x) ? p1.y : static_cast<int>(host->get_line(line).length());

        Dictionary style;
        style["color"]= Color::hex(span.color << 8 | 0xFF);
        for (int col = start_col; col < end_col; ++col) res[static_cast<Variant>(col)] = style;
    }
    return res;
}


void AnsiHighlighter::default_style_dict() {default_style["color"] = Color(1, 1, 1);}

void AnsiHighlighter::append_span(const Span &new_span) {
    if (line_start_index.is_empty()) rebuild_line_indexing();

    constexpr int last_line_idx = 0;
    int line_idx = last_line_idx;

    while (line_idx + 1 < line_start_index.size() && new_span.start >= line_start_index[line_idx + 1]) ++line_idx;

    if (spans_per_line.size() <= line_idx) spans_per_line.resize(line_idx + 1);

    const_cast<Vector<Span> &>(spans_per_line[line_idx]).push_back(new_span);
}

uint32_t LinuxHost::ansi256_to_color(const uint32_t &code){
    if (code >= 16 && code <= 231){
        const uint32_t idx = code - 16;

        const uint32_t r = idx / 36;
        const uint32_t g = (idx % 36) / 6;
        const uint32_t b = idx % 6;

        const int steps[6] = {0, 95, 135, 175, 215, 255};

        return steps[r] << 16 | steps[g] << 8 | steps[b];

    }
    const uint32_t gray = 8 + (code - 232) * 10;
    return gray << 16 | gray << 8 | gray;
}

uint32_t LinuxHost::ansi_to_color(const uint32_t &code) {
    switch (code) {
        case 0: return 0x000000;     // black
        case 1: return 0xff0000;     // red
        case 2: return 0x00ff00;     // green
        case 3: return 0xffff00;     // yellow
        case 4: return 0x0000ff;     // blue
        case 5: return 0xff00ff;     // magenta
        case 6: return 0x00ffff;     // cyan
        case 7: return 0xffffff;     // white
        case 8: return 0x888888;     // bright black / dark gray
        case 9: return 0xff8888;     // bright red
        case 10: return 0x88ff88;    // bright green
        case 11: return 0xffff88;    // bright yellow
        case 12: return 0x8888ff;    // bright blue
        case 13: return 0xff88ff;    // bright magenta
        case 14: return 0x88ffff;    // bright cyan
        default: return 0xffffff;    // case 15 included
    }
}


void LinuxHost::apply_args(Span &span, const String &args){
    auto params = args.split(";");
    for (ssize_t i = 0; i < params.size();){
        const int code = static_cast<int>(params[i].to_int());

        if (code == 38 || code == 48){
            const bool is_fg = (code == 38);
            const int mode = static_cast<int>(params[i+1].to_int());
            if (mode == 5){
                const int idx = static_cast<int>(params[i+2].to_int());
                const uint32_t rgb = ansi256_to_color(idx);

                if (is_fg) span.color = rgb;
                else span.bg_color = rgb;

                i += 3;
                continue;
            }
            if (mode == 2){
                const int r = static_cast<int>(params[i+2].to_int());
                const int g = static_cast<int>(params[i+3].to_int());
                const int b = static_cast<int>(params[i+4].to_int());

                if (is_fg) span.color = r << 16 | g << 8 | b;
                else span.bg_color = r << 16 | g << 8 | b;

                i += 5;
                continue;
            }
            i++;
            continue;
        }
        apply_style(code, span);
        i++;
    }
}

void LinuxHost::apply_style(const uint32_t code, Span &span){
    switch(code){
        case 0:
            span = Span();
            break;

        case 1: span.bold = true; break;
        case 22: span.bold = false; break;

        case 30 ... 37: span.color = ansi_to_color(code - 30); break;
        case 40 ... 47: span.bg_color = ansi_to_color(code - 40); break;
        case 90 ... 97: span.color = ansi_to_color(code - 90 + 8); break;
        case 100 ... 107: span.bg_color = ansi_to_color(code - 100 + 8); break;
        default: ;
    }
}

bool LinuxHost::file_exists(const char *path) {
    struct stat st{};
    return path && stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

uint32_t LinuxHost::get_caret_index() const {
    const int cl = get_caret_line();
    const int cc = get_caret_column();
    int32_t caret_index = static_cast<int>(get_line(cl).substr(0, cc).length());
    for (int i = 0; i < cl; i++) caret_index += static_cast<int>(get_line(i).length() + 1);
    return caret_index;
}


void LinuxHost::load_history(const uint32_t max_lines) {
    const char* hist_path = getenv("HISTFILE");
    String path;
    if (!hist_path) {
        const char* home_path = getenv("HOME");
        if (!home_path) {history=PackedStringArray(); return;}
        path = String(home_path) + "/.bash_history";
    }
    else path = hist_path;
    if (!file_exists(path.utf8().get_data())) {history=PackedStringArray(); return;}

    const Ref<FileAccess> file = FileAccess::open(String(path), FileAccess::READ);
    if (file.is_null()) {history=PackedStringArray(); return;}

    const auto file_size = static_cast<int64_t>(file->get_length());

    int64_t pos{file_size};
    int newline_count{0};
    String buffer;

    while (pos > 0 && newline_count <= max_lines) {
        constexpr int64_t chunk_size{4096};
        const int64_t read_size = Math::min(chunk_size, pos);
        pos -= read_size;

        file->seek(pos);
        PackedByteArray bytes = file->get_buffer(read_size);
        String chunk = bytes.get_string_from_utf8();

        buffer = chunk + buffer;
        newline_count += static_cast<int>(chunk.count("\n"));
    }

    file->close();

    PackedStringArray lines = buffer.split("\n", false);
    if (lines.size() > max_lines) lines = lines.slice(lines.size() - max_lines, lines.size());

    history = lines;
    history_index = static_cast<int>(history.size());
    history_temp = "";
    UtilityFunctions::print("Successfully loaded history");
}

void LinuxHost::merge_same_spans(Vector<Span> &spans) {
    if (spans.is_empty()) return;
    Vector<Span> merged;
    Span cur_span = spans[0];

    for (int i = 1; i < spans.size(); i++) {
        if (spans[i] == cur_span) cur_span.length += spans[i].length;
        else {
            merged.push_back(cur_span);
            cur_span = spans[i];
        }
    }
    merged.push_back(cur_span);
    spans = merged;
}

void LinuxHost::_bind_methods(){
    ClassDB::bind_method(D_METHOD("end_pseudoterminal"), &LinuxHost::end_pseudoterminal);
    ClassDB::bind_method(D_METHOD("start_pseudoterminal"), &LinuxHost::start_pseudoterminal);
    ClassDB::bind_method(D_METHOD("clamp_caret"), &LinuxHost::clamp_caret);
}

void LinuxHost::_ready(){
    set_focus_mode(FOCUS_ALL);
    set_process(true);

    const Ref hl = memnew(AnsiHighlighter);
    this->set_syntax_highlighter(hl);
    highlighter = hl;
    hl->default_style_dict();
    font = get_theme_font("font", "TextEdit");

    start_pseudoterminal();
}

void LinuxHost::_exit_tree(){
    end_pseudoterminal();
}

void LinuxHost::_process(double p_delta) {
    read_from_terminal();

    int processed = 0;
    String frame_text{""};
    Vector<Span> frame_spans;
    const int64_t base_offset = get_text().length();

    while (!output_queue.empty() && processed < MAX_LINES_PER_FRAME) {
        const auto parsed = get_color_highlighting(output_queue.front(), frame_text, base_offset);
        output_queue.pop();
        frame_spans.append_array(parsed);
        processed++;
    }
    if (frame_text.is_empty()) return;
    set_caret_line(get_line_count() - 1);
    set_caret_column(static_cast<int32_t>(get_line(get_line_count() - 1).length()));
    insert_text_at_caret(frame_text);
    highlighter->rebuild_line_indexing();
    input_start_index = static_cast<int32_t>(get_text().length());
    for (Span &span : frame_spans) highlighter->append_span(span);
    queue_redraw();
}

void LinuxHost::_draw() {
    if (highlighter->spans_per_line.is_empty()) return;

    const int first_visible = get_first_visible_line();
    const int last_visible = first_visible + get_visible_line_count();

    const int font_size = get_theme_font_size("font_size");
    const float ascent = font->get_ascent(font_size);
    const float char_width = font->get_char_size('M', font_size).x;

    for (int line = first_visible; line < last_visible; line++) {
        if (line >= highlighter->spans_per_line.size()) continue;

        for (const auto &span : highlighter->spans_per_line[line]) {
            if (span.bg_color == 0x000000) continue;
            const int char_column = Math::max(0 , static_cast<int>(span.start - highlighter->line_start_index[line]));
            const Vector2i pos = get_pos_at_line_column(line, char_column);

            Rect2 rect{static_cast<real_t>(pos.x), static_cast<real_t>(pos.y) - ascent, char_width * static_cast<float>(span.length), ascent};
            draw_rect(rect, Color::hex(span.bg_color << 8 | 0xFF));
        }
    }
}

void LinuxHost::read_from_terminal() {
    if (!running || master_fd == -1) return;

    pollfd pfd{};
    pfd.fd = master_fd;
    pfd.events = POLLIN;

    if (const int ret = poll(&pfd, 1, 0); ret <= 0) return;

    if (pfd.revents & POLLIN) {
        char buffer[4097];

        if (const ssize_t n = read(master_fd, buffer, sizeof(buffer) - 1); n > 0) {
            buffer[n] = '\0';
            if (constexpr size_t MAX_QUEUE_SIZE{5000}; output_queue.size() < MAX_QUEUE_SIZE) output_queue.emplace(buffer);
        }
        else if (n == 0) running = false;
    }
}

void LinuxHost::end_pseudoterminal(){
    if (!running) return;

    running = false;

    if (master_fd != -1){
        close(master_fd);
        master_fd = -1;
    }

    if (child_pid > 0){
        kill(child_pid, SIGHUP);
        waitpid(child_pid, nullptr, 0);
        child_pid = -1;
    }
}

void LinuxHost::start_pseudoterminal(){
    if (running) return;
    if (openpty(&master_fd, &slave_fd, nullptr, nullptr, nullptr) == -1){
        UtilityFunctions::print("Openpty Failed");
        return;
    }
    const int flags = fcntl(master_fd, F_GETFL, 0);
    fcntl(master_fd, F_SETFL, flags | O_NONBLOCK);

    child_pid = fork();
    if (child_pid == -1){
        UtilityFunctions::print("Fork Failed");
        return;
    }

    if (child_pid == 0){
        close(master_fd);
        if (setsid() == -1) {
            perror("setsid");
            _exit(1);
        }

        login_tty(slave_fd);

        execlp("bash", "bash", nullptr);

        perror("execlp");
        _exit(1);
    }

    close(slave_fd);
    running = true;

    UtilityFunctions::print("PTY searched, PID: ", child_pid);

    write_to_terminal("export TERM=xterm-256color\n");
    load_history(500);
}

void LinuxHost::write_to_terminal(const String &text) {
    const std::string native = text.utf8().get_data();

    if (native == "clear\n") {
        clear();
        highlighter->spans_per_line.clear();
    }

    if (master_fd != -1) {
        const ssize_t result = write(master_fd, native.c_str(), native.size());
        if (result == -1) perror("write");
    }
}

void LinuxHost::_gui_input(const Ref<InputEvent> &event) {
    const Ref<InputEventKey> key_event = event;
    if (event->is_class("InputEventMouseButton") || event->is_class("InputEventMouseMotion")) call_deferred("clamp_caret");
    if (!key_event.is_valid() || !key_event->is_pressed()) return;
    if (!has_focus()) return;
    const int keycode = key_event->get_keycode();
    if (keycode == KEY_LEFT || keycode == KEY_PAGEUP || keycode == KEY_HOME) {
        call_deferred("clamp_caret");
        return;
    }
    if (keycode == KEY_C && key_event->is_ctrl_pressed()) {
        if (master_fd == -1) return;
        const pid_t fg_pgid = tcgetpgrp(master_fd);
        if (fg_pgid > 0) kill(-fg_pgid, SIGINT);

        input = "";
        accept_event();
        return;
    }
    if (keycode == KEY_ENTER) {
        if (!input.strip_edges().is_empty()) history.push_back(input); history_index = static_cast<int32_t>(history.size());
        const Vector2i line_col = highlighter->from_index_get_line_column(input_start_index);
        remove_text(line_col.x, line_col.y, get_line_count() - 1, static_cast<int32_t>(get_line(get_line_count() - 1).length()));
        write_to_terminal(input + "\n");
        input = "";
        accept_event();
        return;
    }
    if (keycode == KEY_BACKSPACE) {
        if (const int64_t caret_index = get_caret_index(); caret_index > input_start_index) {
            const int64_t rel = caret_index - input_start_index;
            input = input.substr(0, rel-1) + input.substr(rel + 1);
            backspace();
        }
        else clamp_caret();
        accept_event();
        return;
    }
    if (keycode == KEY_UP) {
        if (history.is_empty()) {accept_event(); return;}
        if (history_index == history.size()) history_temp = input;
        history_index = std::max(static_cast<uint32_t>(0), history_index - 1);
        input = history[history_index];
        const Vector2i line_col = highlighter->from_index_get_line_column(input_start_index);
        remove_text(line_col.x, line_col.y, get_line_count() - 1, static_cast<int32_t>(get_line(get_line_count() - 1).length()));
        insert_text(input,line_col.x, line_col.y);
        accept_event();
        return;
    }
    if (keycode == KEY_DOWN) {
        history_index = std::min(static_cast<uint32_t>(history.size()) , history_index + 1);
        if (history_index == history.size()) input = history_temp;
        else if (history_index < history.size()) input = history[history_index];
        const Vector2i line_col = highlighter->from_index_get_line_column(input_start_index);
        remove_text(line_col.x, line_col.y, get_line_count() - 1, static_cast<int32_t>(get_line(get_line_count() - 1).length()));
        insert_text(input,line_col.x, line_col.y);
        accept_event();
        return;
    }
    if (!key_event->is_ctrl_pressed() && !key_event->is_alt_pressed()) {
        if (const char32_t unicode = key_event->get_unicode(); unicode != 0) {
            const int64_t rel = get_caret_index() - input_start_index;
            input = input.substr(0, rel) + String::chr(unicode) + input.substr(rel);
        }
    }
}

Vector<Span> LinuxHost::get_color_highlighting(const String &ansi_strip, String &frame_text, const uint32_t &base_offset) const {
    String normalised = ansi_strip.replace("\r\n", "\n").replace("\r", "");
    Vector<Span> local_spans;
    Span current;
    String cur_args = "", cur_text = "";
    uint32_t local_offset = 0;
    for (ssize_t i = 0; i < normalised.length();){
        if (normalised[i] == '\e' && i+1 < normalised.length() && normalised[i+1] == '['){
            if (!cur_text.is_empty()){
                current.start = local_offset + base_offset;
                current.length = static_cast<uint32_t>(cur_text.length());
                local_spans.push_back(current);
                frame_text += cur_text;
                local_offset += current.length;
                cur_text = "";
            }
            cur_args = "";
            i += 2;

            while (i < normalised.length() && !(normalised[i] >= '@' && normalised[i] <= '~')) // command characters
            {
                cur_args += normalised[i];
                i++;
            }

            if (i < normalised.length()) {
                const char32_t command = normalised[i];
                i++;
                if (command == 'm') apply_args(current, cur_args);
                if (command == 'J') {highlighter->spans_per_line.clear(); current = Span();}
                if (command == 'K') current = Span();
            }
        }
        else {
            cur_text += normalised[i];
            i++;
        }
    }
    if (!cur_text.is_empty()) {
        current.start = local_offset + base_offset;
        current.length = static_cast<int32_t>(cur_text.length());
        local_spans.push_back(current);
        frame_text += cur_text;
    }
    merge_same_spans(local_spans);
    return local_spans;
}

void LinuxHost::clamp_caret() {
    const Vector2i p = highlighter->from_index_get_line_column(input_start_index);

    if (const int64_t caret_index = get_caret_index(); caret_index < input_start_index) {
        set_caret_line(p.x);
        set_caret_column(p.y);
    }
}

#else

void LinuxHost::_bind_methods() {}
void LinuxHost::start_pseudoterminal() {}
void LinuxHost::end_pseudoterminal() {}
void LinuxHost::read_from_terminal() {}
void LinuxHost::write_to_terminal(const String &text) {}
void LinuxHost::edit_text(const String &text) {}

#endif