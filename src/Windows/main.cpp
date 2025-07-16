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
#include <windows.h>
#include <thread>
#include <string>
#include <regex>
#include <cctype>
using namespace godot;

CmdHost::CmdHost() {}

CmdHost::~CmdHost() {}

std::string to_lower(std::string input) {
    for (char &c : input) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return input;
}

void CmdHost::_bind_methods(){
    ClassDB::bind_method(D_METHOD("edit_text", "new_text"), &CmdHost::edit_text);
    ClassDB::bind_method(D_METHOD("start_pseudoconsole_session"), &CmdHost::start_pseudoconsole_session);
    ClassDB::bind_method(D_METHOD("end_pseudoconsole_session"), &CmdHost::end_pseudoconsole_session);
    ClassDB::bind_method(D_METHOD("write_to_cmd", "input"), &CmdHost::write_to_cmd);
}

void CmdHost::_ready(){
    start_pseudoconsole_session();
    main_loop();
    set_caret_line(get_line_count() - 1);
}

void CmdHost::_exit_tree(){
    end_pseudoconsole_session();
}

String strip_ansi_sequences(const String &input) {
    std::string utf8_input = input.utf8().get_data();
    std::regex ansi_regex("\x1B\\[[0-9;?]*[a-zA-Z]");
    std::regex osc_regex("\x1B\\].*?\x07");
    utf8_input = std::regex_replace(utf8_input, ansi_regex, "");
    utf8_input = std::regex_replace(utf8_input, osc_regex, "");
    return String::utf8(utf8_input.c_str());
}

void CmdHost::edit_text(const String &newtext){
    set_text(get_text() + newtext);
    int last_line = get_line_count() - 1;
    int last_column = get_line(last_line).length();
    call_deferred("set_caret_line", last_line);
    call_deferred("set_caret_column", last_column);
}


void CmdHost::start_pseudoconsole_session(){
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
    InitializeProcThreadAttributeList(si.lpAttributeList, 1, 0, &attrSize);
    UpdateProcThreadAttribute(si.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hPC, sizeof(hPC), NULL, NULL);
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
            return;
    }
    CloseHandle(child_stdin_read);
    CloseHandle(child_stdout_write);
}


void CmdHost::end_pseudoconsole_session(){
    ClosePseudoConsole(hPC);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    DeleteProcThreadAttributeList(si.lpAttributeList);
    HeapFree(GetProcessHeap(), 0, (LPVOID)si.lpAttributeList);
    CloseHandle(parent_stdin_write);
    CloseHandle(parent_stdout_read);
}

void CmdHost::main_loop(){
    std::thread reader_thread([this]() {
        CHAR buf[256];
        DWORD read;
        while (true) {
            BOOL success = ReadFile(parent_stdout_read, buf, sizeof(buf) - 1, &read, NULL);
            if (!success || read == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // prevent CPU spin
                continue;
            }
            buf[read] = '\0';
            String raw_out = String::utf8(buf);
            String clean_out = strip_ansi_sequences(raw_out);
            call_deferred("edit_text", clean_out);
        }
    });

    reader_thread.detach();
}


void CmdHost::_gui_input(const Ref<InputEvent> &event) {
    Ref<InputEventKey> key_event = event;
    if (!key_event.is_valid() || !key_event->is_pressed()) return;
    int keycode = key_event->get_keycode();
    if (keycode == Key::KEY_ENTER) {
        int last_line = get_line_count() - 1;
        String line_text = get_line(last_line);
        std::string utf8_line = line_text.utf8().get_data();
        std::regex prompt_regex(R"(.*[A-Z]:\\[^>]*> ?(.*)$)");
        std::smatch match;
        if (std::regex_match(utf8_line, match, prompt_regex) && match.size() >= 2) {
            std::string cmd_input = match[1];
            write_to_cmd(String::utf8(cmd_input.c_str()));
        }
        else {
            write_to_cmd(line_text);
        }
    }
}


void CmdHost::write_to_cmd(const String &input){
    if (parent_stdin_write == nullptr) return;
    String full_input = input + String("\r\n");
    std::string utf8_input = full_input.utf8().get_data();
    if (to_lower(utf8_input) == "exit\r\n"){
        utf8_input = "echo Blocked\r\n";
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
    DWORD err = GetLastError();
    }
}
