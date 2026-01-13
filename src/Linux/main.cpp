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
#include <unistd.h>
#include <pty.h>
#include <utmp.h>
#include <sys/wait.h>
#include <string>

using namespace godot;

static uint32_t rgb_to_rgba(const uint32_t rgb) {
    const uint8_t r = (rgb >> 16) & 0xFF;
    const uint8_t g = (rgb >> 8) & 0xFF;
    const uint8_t b = rgb & 0xFF;
    return 0x000000FF | (r << 24) | (g << 16) | b << 8;
}

void AnsiHighlighter::default_style_dict() {
    default_style["color"] = Color(1, 1, 1);
    default_style["bg_color"] = Color(0, 0, 0, 0);
    default_style["bold"] = false;
    default_style["italic"] = false;
    default_style["underlie"] = false;
}


void AnsiHighlighter::rebuild_line_indexing() {
    int64_t cur_index = 0;
    line_start_index.clear();
    line_start_index.push_back(static_cast<int32_t>(cur_index));

    for (int32_t i = 0; i < get_text_edit()->get_line_count(); i++) {
        cur_index += get_text_edit()->get_line(i).length() + 1;
        line_start_index.push_back(static_cast<int32_t>(cur_index));
    }
}


Vector2i AnsiHighlighter::from_index_get_line_column(const int32_t &index) const {
    int32_t L = 0, R = static_cast<int32_t>(line_start_index.size()) - 1;
    while (L <= R) {
        if (const int32_t M = (L + R) / 2; line_start_index[M] <= index) L = M + 1;
        else R = M - 1;
    }

    return {R, index - line_start_index[R]};
}


Dictionary AnsiHighlighter::_get_line_syntax_highlighting(const int line) const{
    Dictionary res = {};

    const auto host = cast_to<TextEdit>(get_text_edit());
    if (!host) return res;

    for (const Span &span : spans) {
        const Vector2i p0 = from_index_get_line_column(span.start);
        const Vector2i p1 = from_index_get_line_column(span.start + span.length);

        if (line < p0.x || line > p1.x) continue;

        const int start_col = (line == p0.x) ? p0.y : 0;
        const int end_col = (line == p1.x) ? p1.y : static_cast<int>(host->get_line(line).length());

        Dictionary style;
        style["color"] = span.color;
        style["bg_color"] = span.bg_color;
        style["bold"] = span.bold;
        style["italic"] = span.italics;
        style["underline"] = span.underline;

        res[static_cast<Variant>(start_col)] = style;
        res[static_cast<Variant>(end_col)] = default_style;

    }
    return res;
}

void AnsiHighlighter::_bind_methods() {}


int LinuxHost::ansi_to_color(const int &code) {
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
        case 15: return 0xffffff;    // bright white
    }
    return 0xffffff; // default white
}

int LinuxHost::ansi256_to_color(const int &code){
    if (code >= 16 && code <= 231){
        const int idx = code - 16;

        const int r = idx / 36;
        const int g = (idx % 36) / 6;
        const int b = idx % 6;

        const int steps[6] = {0, 95, 135, 175, 215, 255};

        return steps[r] << 16 | steps[g] << 8 | steps[b];

    }
    const int gray = 8 + (code - 232) * 10;
    return gray << 16 | gray << 8 | gray;
}


void LinuxHost::apply_style(const int code, Segment &seg){
    switch(code){
        case 0:
            seg = Segment();
            break;

        case 1: seg.bold = true; break;
        case 3: seg.italics = true; break;
        case 4: seg.underlined = true; break;

        case 22: seg.bold = false; break;
        case 23: seg.italics = false; break;
        case 24: seg.underlined = false; break;

        case 30 ... 37: seg.color = ansi_to_color(code - 30); break;
        case 40 ... 47: seg.bg_color = ansi_to_color(code - 40); break;
        case 90 ... 97: seg.color = ansi_to_color(code - 90 + 8); break;
        case 100 ... 107: seg.bg_color = ansi_to_color(code - 100 + 8); break;
    }
}

void LinuxHost::apply_args(Segment &seg, const String &args){
    auto params = args.split(";");
    for (ssize_t i = 0; i < params.size();){
        const int code = static_cast<int>(params[i].to_int());

        if (code == 38 || code == 48){
            const bool is_fg = (code == 38);
            const int mode = static_cast<int>(params[i+1].to_int());
            if (mode == 5){
                int idx = static_cast<int>(params[i+2].to_int());
                const int rgb = ansi256_to_color(idx);

                if (is_fg) seg.color = rgb;
                else seg.bg_color = rgb;

                i += 3;
                continue;
            }
            if (mode == 2){
                const int r = static_cast<int>(params[i+2].to_int());
                const int g = static_cast<int>(params[i+3].to_int());
                const int b = static_cast<int>(params[i+4].to_int());

                if (is_fg) seg.color = r << 16 | g << 8 | b;
                else seg.bg_color = r << 16 | g << 8 | b;

                i += 5;
                continue;
            }
            i++;
            continue;
        }
        apply_style(code, seg);
        i++;
    }
}

void LinuxHost::get_bbcode(const String &ansi_strip){
    Segment current;
    String cur_args = "";
    String res = "";
    for (ssize_t i = 0; i < ansi_strip.length();){
        if (ansi_strip[i] == '\e' && i+1 < ansi_strip.length() && ansi_strip[i+1] == '['){
            if (!current.text.is_empty()){
                segments.push_back(current);
                current.text = "";
            }
            cur_args = "";
            i += 2;

            while (i < ansi_strip.length() && !( (ansi_strip[i] >= '@' && ansi_strip[i] <= '~') )) // command characters
            {
                cur_args += ansi_strip[i];
                i++;
            }

            if (i < ansi_strip.length()) {
                const char32_t command = ansi_strip[i];
                i++;
                if (command == 'm') apply_args(current, cur_args);
            }
        }
        else {
            current.text += ansi_strip[i];
            i++;
        }
    }
    if (!current.text.is_empty()) segments.push_back(current);
    for (const Segment &seg : segments) {
        const int32_t start = static_cast<int32_t>(this->get_text().length());
        this->insert_text_at_caret(seg.text);
        const int32_t end = static_cast<int32_t>(this->get_text().length());

        highlighter->rebuild_line_indexing();
        highlighter->spans.push_back({
            start,
            end-start,
            Color::hex(rgb_to_rgba(seg.color)),
            Color::hex(rgb_to_rgba(seg.bg_color)),
            seg.bold,
            seg.underlined,
            seg.italics
        });
        input_start_index = static_cast<int>(get_text().length());
    }
}

int32_t LinuxHost::get_caret_index() const {
    const int cl = get_caret_line();
    const int cc = get_caret_column();
    int32_t caret_index = static_cast<int>(get_line(cl).substr(0, cc).length());
    for (int i = 0; i < cl; i++) caret_index += static_cast<int>(get_line(i).length() + 1);
    return caret_index;
}

void LinuxHost::clamp_caret() {
    const Vector2i p = highlighter->from_index_get_line_column(input_start_index);

    if (const int caret_index = get_caret_index(); caret_index < input_start_index) {
        set_caret_line(p.x);
        set_caret_column(p.y);
    }
}

void LinuxHost::_bind_methods(){
    ClassDB::bind_method(D_METHOD("get_bbcode", "ansi_strip"), &LinuxHost::get_bbcode);
    ClassDB::bind_method(D_METHOD("end_pseudoterminal"), &LinuxHost::end_pseudoterminal);
    ClassDB::bind_method(D_METHOD("start_pseudoterminal"), &LinuxHost::start_pseudoterminal);
    ClassDB::bind_method(D_METHOD("clamp_caret"), &LinuxHost::clamp_caret);
}

void LinuxHost::_ready(){
    set_focus_mode(FOCUS_ALL);

    const Ref hl = memnew(AnsiHighlighter);
    this->set_syntax_highlighter(hl);
    highlighter = hl;
    hl->default_style_dict();

    start_pseudoterminal();

}

void LinuxHost::_exit_tree(){
    end_pseudoterminal();
}

void LinuxHost::_gui_input(const Ref<InputEvent> &event) {
    const Ref<InputEventKey> key_event = event;
    if (event->is_class("InputEventMouseButton") || event->is_class("InputEventMouseMotion")) call_deferred("clamp_caret");
    if (!key_event.is_valid() || !key_event->is_pressed()) return;
    const int keycode = key_event->get_keycode();
    if (keycode == KEY_LEFT || keycode == KEY_UP || keycode == KEY_PAGEUP || keycode == KEY_HOME) {
        call_deferred("clamp_caret");
        return;
    }
    if (keycode == KEY_ENTER) {
        write_to_terminal(input + "\n");
        input = "";
        accept_event();
        return;
    }
    if (keycode == KEY_BACKSPACE) {
        if (const int caret_index = get_caret_index(); caret_index > input_start_index) {
            const int rel = caret_index - input_start_index;
            input = input.substr(0, rel-1) + input.substr(rel + 2);
            backspace();
        }
        else clamp_caret();
        accept_event();
        return;
    }
     if (const char32_t unicode = key_event->get_unicode(); unicode != 0) {
         const int rel = get_caret_index() - input_start_index;
         input = input.substr(0, rel) + String::chr(unicode) + input.substr(rel + 1);
     }
}

void LinuxHost::reader_loop(){
    char buffer[256];

    while (running){
        if (const ssize_t n = read(master_fd, buffer, sizeof(buffer)); n > 0){
            buffer[n] = '\0';
            String out(buffer);

            call_deferred("get_bbcode", out);
        }
        else if (n == 0) break;
        else {
            if (errno == EAGAIN || errno == EINTR) continue;
            break;
        }
    }
    running = false;
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

    if (reader_thread.joinable()) reader_thread.join();
}

void LinuxHost::start_pseudoterminal(){
    if (running) return;
    if (openpty(&master_fd, &slave_fd, nullptr, nullptr, nullptr) == -1){
        UtilityFunctions::print("Openpty Failed");
        return;
    }

    child_pid = fork();
    if (child_pid == -1){
        UtilityFunctions::print("Fork Failed");
        return;
    }

    if (child_pid == 0){
        close(master_fd);
        login_tty(slave_fd);

        execlp("bash", "bash", nullptr);

        perror("execlp");
        _exit(1);
    }

    close(slave_fd);
    running = true;
    reader_thread = std::thread(&LinuxHost::reader_loop, this);

    UtilityFunctions::print("PTY searched, PID: ", child_pid);
    write_to_terminal("export TERM=xterm-256color\n");
}

void LinuxHost::write_to_terminal(const String &text){
    const std::string native_str_text = text.utf8().get_data();
    ssize_t result;

    if (native_str_text == "clear\n") {
        clear();
        highlighter->spans.clear();
    }

    if (master_fd != -1) result = write(master_fd, native_str_text.c_str(), native_str_text.size());
    if (result == -1) perror("write");
}

#else

void LinuxHost::_bind_methods() {}
void LinuxHost::start_pseudoterminal() {}
void LinuxHost::end_pseudoterminal() {}
void LinuxHost::reader_loop() {}
void LinuxHost::write_to_terminal(const String &text) {}
void LinuxHost::edit_text(const String &text) {}

#endif