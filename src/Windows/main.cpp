#define _CRT_SECURE_NO__WARNINGS
#define WIN32_LEAN_AND_MEAN
#define UNICODE

#include "main.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_action.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <windows.h>
#include <thread>
#include <string>
#include <regex>
#include <cctype>
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


int WindowsHost::ansi_to_color(const int &code) {
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

int WindowsHost::ansi256_to_color(const int &code){
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


void WindowsHost::apply_style(const int code, Segment &seg){
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

void WindowsHost::apply_args(Segment &seg, const String &args){
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

void WindowsHost::get_color_highlighting(const String &ansi_strip){
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

int32_t WindowsHost::get_caret_index() const {
    const int cl = get_caret_line();
    const int cc = get_caret_column();
    int32_t caret_index = static_cast<int>(get_line(cl).substr(0, cc).length());
    for (int i = 0; i < cl; i++) caret_index += static_cast<int>(get_line(i).length() + 1);
    return caret_index;
}

void WindowsHost::clamp_caret() {
    const Vector2i p = highlighter->from_index_get_line_column(input_start_index);

    if (const int caret_index = get_caret_index(); caret_index < input_start_index) {
        set_caret_line(p.x);
        set_caret_column(p.y);
    }
}

void WindowsHost::_bind_methods(){
    ClassDB::bind_method(D_METHOD("edit_text", "new_text"), &WindowsHost::edit_text);
    ClassDB::bind_method(D_METHOD("start_pseudoconsole_session"), &WindowsHost::start_pseudoconsole_session);
    ClassDB::bind_method(D_METHOD("end_pseudoconsole_session"), &WindowsHost::end_pseudoconsole_session);
    ClassDB::bind_method(D_METHOD("write_to_pwsh", "input"), &WindowsHost::write_to_pwsh);
}

void WindowsHost::_ready(){
    set_focus_mode(FOCUS_ALL);

    const Ref hl = memnew(AnsiHighlighter);
    this->set_syntax_highlighter(hl);
    highlighter = hl;
    hl->default_style_dict();

    start_pseudoconsole_session();
    reader_loop();
}

void WindowsHost::_exit_tree(){
    end_pseudoconsole_session();
}

void WindowsHost::start_pseudoconsole_session(){
    if(!CreatePipe(&child_stdin_read, &parent_stdin_write, &sa, 0) ||
    !CreatePipe(&parent_stdout_read, &child_stdout_write, &sa, 0)){
        return;
    }
    HRESULT hr = CreatePseudoConsole(size, child_stdin_read, child_stdout_write, 0, &hPC);
    if (FAILED(hr)) {
        return;
    }
    ZeroMemory(&si, sizeof(si));
    si.StartupInfo.cb = sizeof(si);
    InitializeProcThreadAttributeList(NULL, 1, 0, &attrSize);
    si.lpAttributeList = (LPPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, attrSize);
    if (!si.lpAttributeList){
        UtilityFunctions::print("Failed to allocate memory, HeapAlloc() -> Failed");
        return;
    }
    if (!InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attrSize)){
        UtilityFunctions::print("InitializeProcThreadAttributeList() -> Failed");
        return;
    }
    if(!UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hPC, sizeof(hPC), NULL, NULL)){
        UtilityFunctions::print("UpdateProcThreadAttribute() -> Failed");
    }
    if(!CreateProcessW(
        L"c:\\Windows\\System32\\cmd.exe",
        NULL,
        NULL, NULL,
        TRUE,
        EXTENDED_STARTUPINFO_PRESENT,
        NULL,
        NULL,
        &si.StartupInfo,
        &pi)){
            UtilityFunctions::print("CreateProcessW() -> Failed");
            return;
    }
    CloseHandle(child_stdin_read);
    CloseHandle(child_stdout_write);
}


void WindowsHost::end_pseudoconsole_session(){
    running = false;
    if (reader_thread_handle) {
    CancelSynchronousIo(reader_thread_handle);
    CloseHandle(reader_thread_handle); // release handle
    reader_thread_handle = nullptr;
    }
    if (reader_thread.joinable()) reader_thread.join();
    ClosePseudoConsole(hPC);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    DeleteProcThreadAttributeList(si.lpAttributeList);
    HeapFree(GetProcessHeap(), 0, (LPVOID)si.lpAttributeList);
    CloseHandle(parent_stdin_write);
    CloseHandle(parent_stdout_read);
}

void WindowsHost::reader_loop(){
    running = true;
    reader_thread = std::thread([this]() {
        CHAR buf[256];
        DWORD read;
        while (running) {
            BOOL success = ReadFile(parent_stdout_read, buf, sizeof(buf) - 1, &read, NULL);
            if (!success || read == 0) {
                if (!running){
                    UtilityFunctions::print("ReadFile returned, shutting down reader thread...");
                    break;
                    }
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // prevent CPU spin
                continue;
            }
            buf[read] = '\0';
            String raw_out = String::utf8(buf);
            String clean_out = strip_ansi_sequences(raw_out);
            call_deferred("edit_text", clean_out);
        }
    });
    reader_thread_handle = OpenThread(THREAD_ALL_ACCESS, FALSE, GetThreadId(static_cast<HANDLE>(reader_thread.native_handle())));
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


void WindowsHost::write_to_pwsh(const String &input){
    if (parent_stdin_write == nullptr) return;
    String full_input = input + String("\r\n");
    std::string utf8_input = full_input.utf8().get_data();
    if (to_lower(utf8_input) == "exit\r\n"){
        utf8_input = "\r\n";
    }
    if (to_lower(utf8_input) == "cls\r\n"){
        utf8_input = "\r\n";
        set_text("");
    }
    DWORD written = 0;
    BOOL success = WriteFile(
        parent_stdin_write,
        utf8_input.c_str(),
        (DWORD)utf8_input.size(),
        &written,
        NULL
    );
    if (!success) {
        UtilityFunctions::print("WriteFile() -> Failed");
        return;
    }
}