#define _CRT_SECURE_NO_WARNINGS
#define __LINUX__LEAN_AND_MEAN
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

void CmdHost::_bind_methods(){
    ClassDB::bind_method(D_METHOD("edit_text", "text"), &CmdHost::edit_text);
    ClassDB::bind_method(D_METHOD("end_pseudoterminal"), &CmdHost::end_pseudoterminal);
    ClassDB::bind_method(D_METHOD("start_pseudoterminal"), &CmdHost::start_pseudoterminal);
}

void CmdHost::_ready(){
    set_focus_mode(FOCUS_ALL);
    start_pseudoterminal();
}

void CmdHost::_exit_tree(){
    end_pseudoterminal();
}


void CmdHost::_gui_input(const Ref<InputEvent> &event){
    Ref<InputEventKey> key_event = event;
    if (!key_event.is_valid() || !key_event->is_pressed()) return;
    int keycode = key_event->get_keycode();
    if (keycode == KEY_ENTER){
        write_to_terminal(input + "\n");
        input = "";
        accept_event();
        return;
    }
    if (keycode == KEY_BACKSPACE) {
        if (!input.is_empty()) {
            input = input.substr(0, input.length() - 1);
        }
        return;
    }
    char32_t unicode = key_event->get_unicode();
    if (unicode != 0) input += String::chr(unicode);
}

void CmdHost::reader_loop(){
    char buffer[256];

    while (running){
        ssize_t n = read(master_fd, buffer, sizeof(buffer));
        if (n > 0){
            buffer[n] = '\0';
            String out(buffer);
            
            call_deferred("edit_text", out);
        }
        else if (n == 0) break;
        else {
            if (errno == EAGAIN || errno == EINTR) continue;
            break;
        }
    }
    running = false;
}

void CmdHost::edit_text(const String &text){
    this->insert_text_at_caret(text);
}

void CmdHost::end_pseudoterminal(){
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

void CmdHost::start_pseudoterminal(){
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
    reader_thread = std::thread(&CmdHost::reader_loop, this);

    UtilityFunctions::print("PTY searched, PID: ", child_pid);
    write_to_terminal("export TERM=xterm-256color\n");
}

void CmdHost::write_to_terminal(const String &text){
    std::string native_str_text = text.utf8().get_data();
    ssize_t result;

    if (native_str_text == "clear\n") clear();

    if (master_fd != -1) result = write(master_fd, native_str_text.c_str(), native_str_text.size());
    if (result == -1) perror("write");
}

#else

void CmdHost::_bind_methods() {}
void CmdHost::start_pseudoterminal() {}
void CmdHost::end_pseudoterminal() {}
void CmdHost::reader_loop() {}
void CmdHost::write_to_terminal(const String &text) {}
void CmdHost::edit_text(const String &text) {}

#endif