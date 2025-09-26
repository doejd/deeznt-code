#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define UNICODE

#include "main.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <windows.h>
#include <thread>
#include <regex>
#include <string>
#include <cctype>
#include <sstream>
#include <algorithm>
#include <chrono>
using namespace godot;

std::string to_lower(std::string input) {
    for (char &c : input) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return input;
}

Color ansi_to_color(int code) {
    switch (code) {
        // Normal colors
        case 30: return Color(0,0,0);     // black
        case 31: return Color(1,0,0);     // red
        case 32: return Color(0,1,0);     // green
        case 33: return Color(1,1,0);     // yellow
        case 34: return Color(0,0,1);     // blue
        case 35: return Color(1,0,1);     // magenta
        case 36: return Color(0,1,1);     // cyan
        case 37: return Color(1,1,1);     // white

        // Bright colors
        case 90: return Color(0.5,0.5,0.5);   // bright black / dark gray
        case 91: return Color(1,0.5,0.5);     // bright red
        case 92: return Color(0.5,1,0.5);     // bright green
        case 93: return Color(1,1,0.5);       // bright yellow
        case 94: return Color(0.5,0.5,1);     // bright blue
        case 95: return Color(1,0.5,1);       // bright magenta
        case 96: return Color(0.5,1,1);       // bright cyan
        case 97: return Color(1,1,1);         // bright white
    }
    return Color(1,1,1);
}



Color ansi_256_to_color(int n) {
    if (n < 16) {
        return ansi_to_color((n < 8 ? 30 : 90) + (n % 8));
    } else if (n >= 16 && n <= 231) {
        int idx = n - 16;
        int r = (idx / 36) % 6;
        int g = (idx / 6) % 6;
        int b = idx % 6;
        return Color(r / 5.0f, g / 5.0f, b / 5.0f);
    } else if (n >= 232 && n <= 255) {
        float gray = (n - 232) / 23.0f;
        return Color(gray, gray, gray);
    }
    return Color(1,1,1);
}


void PwshHost::_bind_methods(){
    ClassDB::bind_method(D_METHOD("append_ansi_text", "text"), &PwshHost::append_ansi_text);
    ClassDB::bind_method(D_METHOD("set_blink_time_ms", "time"), &PwshHost::set_blink_time_ms);
    ClassDB::bind_method(D_METHOD("get_blink_time_ms"), &PwshHost::get_blink_time_ms);
    
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "blink_time_ms", PROPERTY_HINT_RANGE, "50,2000,10"), "set_blink_time_ms", "get_blink_time_ms");
}

void PwshHost::set_blink_time_ms(float time){blink_time = time;}
float PwshHost::get_blink_time_ms() const {return blink_time;}


void PwshHost::parse_ansi_and_append(const String &raw_text){
    std::string s = raw_text.utf8().get_data();
    
    std::regex ansi_regex(R"(\x1b\[([0-9;?]*)([@-~]))");
    std::sregex_iterator iter(s.begin(), s.end(), ansi_regex);
    std::sregex_iterator end;
    
    Color cur_color = Color(1, 1, 1);
    bool bold = false;
    bool underline = false;
    
    Vector<Segment> line;
    size_t last_pos = 0;
    
    while(iter != end){
        std::smatch match = *iter;
        size_t match_pos = match.position();
        
        if(match_pos > last_pos){
            std::string text = s.substr(last_pos, match_pos - last_pos);
            if(!text.empty()){
                Segment seg{String(text.c_str()), cur_color, bold, underline};
                line.push_back(seg);
            }
        }
        
        std::string code_str = match[1];
        char command = match[2].str()[0];
        
        if (command == 'm'){
            std::string token;
            std::stringstream ss(code_str);
            while(std::getline(ss, token, ';')){
                int code = std::stoi(token);
                if(code == 0){
                    cur_color = Color(1, 1, 1);
                    bold = false;
                    underline = false;
                }
                else if (code == 1){bold = true;}
                else if (code == 4){underline = true;}
                else if (code >= 30 && code <= 37) {cur_color = ansi_to_color(code);}
                else if (code >= 90 && code <= 97) {cur_color = ansi_to_color(code);}
                else if (code == 38) {
                    std::string next;
                    if (std::getline(ss, next, ';')) {
                        int mode = std::stoi(next);
                        if (mode == 5) {
                            if (std::getline(ss, next, ';')) {
                                int idx = std::stoi(next);
                                cur_color = ansi_256_to_color(idx);
                            }
                        } else if (mode == 2) {
                            int r, g, b;
                            if (std::getline(ss, next, ';')) r = std::stoi(next);
                            if (std::getline(ss, next, ';')) g = std::stoi(next);
                            if (std::getline(ss, next, ';')) b = std::stoi(next);
                            cur_color = Color(r/255.0f, g/255.0f, b/255.0f);
                        }
                    }
                }
            }
        }
        else if (command == 'J') lines.clear();
        else if (command == 'K') {
            if (!lines.is_empty()) lines.write[lines.size() - 1].clear();
        }
        last_pos = match_pos + match.length();
        ++iter;
    }
    if (last_pos < s.length()){
        std::string text = s.substr(last_pos);
        Segment seg{String(text.c_str()), cur_color, bold, underline}; 
        line.push_back(seg);
}
    
    lines.push_back(line);
}


bool PwshHost::find_pwsh(std::wstring &path_out) {
    wchar_t buffer[MAX_PATH];
    DWORD len = SearchPathW(
        NULL,
        L"pwsh.exe",
        NULL,
        MAX_PATH,
        buffer,
        NULL
    );
    if (len > 0 && len < MAX_PATH) {
        path_out = buffer;
        return true;
    }
    return false;
}



void PwshHost::_ready(){
    font = get_theme_default_font();
    set_focus_mode(FOCUS_ALL);
    start_pseudoconsole_session();
    main_loop();
}

void PwshHost::_exit_tree(){
    end_pseudoconsole_session();
}

void PwshHost::_process(double delta){
    cursor.blink_time_ms = blink_time;
    cursor.blink(delta * 1000.0);
    queue_redraw();
}


void PwshHost::_draw(){
    if (!font.is_valid()) return;
    float y = 0.0f;
    Vector2 win_size = Vector2(DisplayServer::get_singleton()->window_get_size());
    float font_size = std::min(win_size.x, win_size.y) * 0.015f;
    for(auto &line : lines){
        float x = 0.0f;
        for(auto &seg : line){
            font->draw_string(get_canvas_item(), Vector2(x, y + font->get_ascent(font_size)), seg.text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, seg.color);
            x += font->get_string_size(seg.text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size).x;
        }
        y += font->get_height(font_size);
    }
    Vector2 input_pos(0, y + font->get_ascent(font_size));
    font->draw_string(get_canvas_item(), input_pos, current_input, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, Color(1, 1, 1));

    cursor.clamp(current_input.length());
    String before_cursor = current_input.left(cursor.col);
    float cursor_x = font->get_string_size(before_cursor, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size).x;
    float top_y = input_pos.y - font->get_ascent(font_size);
    float bottom_y = top_y + font->get_height(font_size);

    if (has_focus() && cursor.visible) draw_line(Vector2(cursor_x, top_y), Vector2(cursor_x, bottom_y), Color(1,1,1), 2.0f);
}

void PwshHost::_gui_input(const Ref<InputEvent> &event) {
    Ref<InputEventKey> key_event = event;
    if (!(key_event.is_valid() && key_event->is_pressed())) return;

    cursor.clamp(current_input.length());

    int keycode = key_event->get_keycode();
    if (keycode == Key::KEY_ENTER) {
        history.push_back(current_input);
        hist_ind = history.size();
        write_to_cmd(current_input);
        current_input = "";
        cursor.reset();
        queue_redraw();
    }
    else if (keycode == Key::KEY_BACKSPACE) {
        if (!(cursor.col > 0)) return;
        current_input = current_input.left(cursor.col - 1) + current_input.substr(cursor.col);
        cursor.move_left();
        queue_redraw();
    }
    else if (keycode == Key::KEY_DELETE) {
        if (!(cursor.col < current_input.length())) return;
        current_input = current_input.left(cursor.col) + current_input.substr(cursor.col + 1);
        queue_redraw();
    }
    else if (keycode == Key::KEY_LEFT) {
        cursor.move_left();
        queue_redraw();
    }
    else if (keycode == Key::KEY_RIGHT) {
        cursor.move_right(current_input.length());
        queue_redraw();
    }
    else if (keycode == Key::KEY_UP){
        if (!(hist_ind > 0)) return;
        hist_ind--;
        current_input = history[hist_ind];
        queue_redraw();
    }
    else if (keycode == Key::KEY_DOWN){
        if ((hist_ind < history.size() - 1)){
            hist_ind++;
            current_input = history[hist_ind];
            queue_redraw();
        }
        else {
            hist_ind = history.size();
            current_input = "";
            queue_redraw();
        }
    }
    else {
      char32_t unicode = key_event->get_unicode();
      if (!(unicode >= ' ' && unicode <= '~')) return;
      current_input = current_input.left(cursor.col) +  String::chr(unicode) + current_input.substr(cursor.col);
      cursor.move_right(current_input.length());
      queue_redraw();
    }
    cursor.clamp(current_input.length());
    get_viewport()->set_input_as_handled();
}


void PwshHost::append_ansi_text(const String &text){
    parse_ansi_and_append(text);
    queue_redraw();
}

void PwshHost::start_pseudoconsole_session(){
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
    std::wstring shell_path;
    if (!find_pwsh(shell_path)) {
        // fallback: Windows PowerShell 5.1
        UtilityFunctions::print("RIP, No pwsh.exe found");
        shell_path = L"C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\powershell.exe";
    }
    
    if (!CreateProcessW(
        shell_path.c_str(),
        NULL,
        NULL, NULL,
        TRUE,
        EXTENDED_STARTUPINFO_PRESENT,
        NULL,
        NULL,
        &si.StartupInfo,
        &pi))
    {
        DeleteProcThreadAttributeList(si.lpAttributeList);
        HeapFree(GetProcessHeap(), 0, (LPVOID)si.lpAttributeList);
        UtilityFunctions::print("CreateProcessW() -> Failed");
        return;
    }

    CloseHandle(child_stdin_read);
    CloseHandle(child_stdout_write);
}


void PwshHost::end_pseudoconsole_session(){
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

void PwshHost::main_loop(){
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
            call_deferred("append_ansi_text", raw_out);
        }
    });
    reader_thread_handle = OpenThread(THREAD_ALL_ACCESS, FALSE, GetThreadId(static_cast<HANDLE>(reader_thread.native_handle())));
}


void PwshHost::write_to_cmd(const String &input){
    if (parent_stdin_write == nullptr) return;
    String full_input = input + String("\r\n");
    std::string utf8_input = full_input.utf8().get_data();
    if (to_lower(utf8_input) == "exit\r\n"){
        utf8_input = "\r\n";
    }
    if (to_lower(utf8_input) == "cls\r\n" || to_lower(utf8_input) == "clear\r\n"){
        lines.clear();
        queue_redraw();
        utf8_input = "\r\n";
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
