#define _CRT_SECURE_NO__WARNINGS
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
#include <fcntl.h>
#include <sys/wait.h>
#include <thread>
#include <string>
#include <regex>
#include <cctype>

using namespace godot;

CmdHost::CmdHost() {}

CmdHost::~CmdHost() {}

String strip_ansi_sequences(const String &input) {
    std::string utf8_input = input.utf8().get_data();
    std::regex ansi_regex("\x1B\\[[0-9;?]*[a-zA-Z]");
    std::regex osc_regex("\x1B\\].*?\x07");
    utf8_input = std::regex_replace(utf8_input, ansi_regex, "");
    utf8_input = std::regex_replace(utf8_input, osc_regex, "");
    return String::utf8(utf8_input.c_str());
}

std::string to_lower(std::string input) {
    for (char &c : input) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
    return input;
}

void CmdHost::_bind_methods(){
    ClassDB::bind_method(D_METHOD("edit_text", "new_text"), &CmdHost::edit_text);
    ClassDB::bind_method(D_METHOD("start_pseudotermainal_session"), &CmdHost::start_pseudoterminal_session);
    ClassDB::bind_method(D_METHOD("end_pseudoterminal_session"), &CmdHost::end_pseudoterminal_session);
    ClassDB::bind_method(D_METHOD("write_to_terminal", "input"), &CmdHost::write_to_terminal);
}

void CmdHost::_ready() {
    start_pseudoterminal_session();
    main_loop();
    set_caret_line(get_line_count() - 1);
}

void CmdHost::_exit_tree() {
    end_pseudoterminal_session();
}

void CmdHost::_gui_input(const Ref<InputEvent> &event) {
    Ref<InputEventKey> key_event = event;
    if (key_event.is_valid() && key_event->is_pressed() && key_event->get_keycode() == Key::KEY_ENTER){
        int last_line = get_line_count() - 1;
        String line_text = get_line(last_line);
        std::string utf8_line = line_text.utf8().get_data();
        std::regex prompt_regex(R"(.*[A-Z]:\\[^>]*> ?(.*)$)");
        std::smatch match;
        if (std::regex_match(utf8_line, match, prompt_regex) && match.size() >= 2){
            std::string cmd_input = match[1];
            write_to_cmd(String::utf8(cmd_input.c_str()));
        }
        else {
            write_to_cmd(line_text);
        }
    }
}

void CmdHost::edit_text(const String &newtext) {
    set_text(get_text() + newtext);
    int last_line = get_line_count() - 1;
    int last_column = get_line(last_line).length();
    call_deferred("set_caret_line", last_line);
    call_deferred("set_caret_column", last_column);
}

void CmdHost::main_loop() {
    const int buffer_size = 1024;
    char buffer[buffer_size];
    while (running) {
        int status;
        if (child_pid > 0 && waitpid(child_pid, &status, WNOHANG) > 0) {
            UtilityFunctions::print("Shell process exited unexpectedly");
            running = false;
            break;
        }
        ssize_t n = read(master_fd, buffer, buffer_size - 1);
        if (n > 0) {
            buffer[n] = '\0';
            String output = String::utf8(buffer);
            output = strip_ansi_sequences(output);
            call_deferred("edit_text", output);
        } else if (n == -1 && running)
            UtilityFunctions::print("read failed");
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void CmdHost::start_pseudoterminal_session() {
    int slave_fd;
    struct termios termios_p;
    struct winsize winsz;
    memset(&termios_p, 0, sizeof(termios_p));
    memset(&winsz, 0, sizeof(winsz));
    winsz.ws_row = 24;
    winsz.ws_col = 80;
    if (openpty(&master_fd, &slave_fd, nullptr, &termios_p, &winsz) == -1) {
        UtilityFunctions::print("openpty failed");
        return;
    }
    child_pid = fork();
    if (child_pid == -1) {
        UtilityFunctions::print("fork failed");
        close(master_fd);
        close(slave_fd);
        return;
    }
    if (child_pid == 0) {
        // CHILD PROCESS
        close(master_fd);
        setsid();
        ioctl(slave_fd, TIOCSCTTY, 0);

        dup2(slave_fd, STDIN_FILENO);
        dup2(slave_fd, STDOUT_FILENO);
        dup2(slave_fd, STDERR_FILENO);
        if (slave_fd > STDERR_FILENO)
            close(slave_fd);

        execlp("bash", "bash", nullptr); // or zsh/fish
        execlp("sh", "sh", nullptr);
        UtilityFunctions::print("execlp failed");
        _exit(1);
    }

    // PARENT PROCESS
    close(slave_fd);
    fcntl(master_fd, F_SETFL, O_NONBLOCK);

    running = true;
    reader_thread = std::thread([this]() { this->main_loop(); });
}

void CmdHost::end_pseudoterminal_session() {
    if (running) {
        running = false;
        if (reader_thread.joinable())
            reader_thread.join();
    }

    if (child_pid > 0) {
        kill(child_pid, SIGTERM);
        waitpid(child_pid, nullptr, 0);
        child_pid = -1;
    }

    if (master_fd != -1) {
        close(master_fd);
        master_fd = -1;
    }
}


void CmdHost::write_to_terminal(const String &input) {
    if (master_fd == -1) return;
    std::string input_str = input.utf8().get_data();
    input_str += "\n"; // Add newline to simulate Enter
    ssize_t result = write(master_fd, input_str.c_str(), input_str.size());
    if (result == -1){
        UtilityFunctions::print("write failed");
    }

}

#endif
