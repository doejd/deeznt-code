

#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <sys/types.h>
#include <thread>
#include <atomic>

using namespace godot;

class CmdHost : public TextEdit {
    GDCLASS(CmdHost, TextEdit);

    int master_fd = -1;
    int slave_fd = -1;
    pid_t child_pid = -1;
    String input;
    std::thread reader_thread;
    std::atomic<bool> running = false;

    protected:
        static void _bind_methods();

    public:
        void _ready() override;
        void _exit_tree() override;
        void reader_loop();
        void end_pseudoterminal();
        void start_pseudoterminal();
        void edit_text(const String &text);
        void write_to_terminal(const String &text);
        void _gui_input(const Ref<InputEvent> &event) override;
        
};
