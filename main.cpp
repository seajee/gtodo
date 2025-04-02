#include <gtkmm.h>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

class Todo : public Gtk::Box
{
public:
    Todo(const std::string& content, std::function<void(Todo*)> remove_callback);
    std::string get_content() const;
    bool is_done() const;
    void set_done(bool state);
    void remove_button_on_click(Todo& todo);

private:
    void checkbox_on_click();

private:
    bool m_Done;
    std::function<void(Todo*)> m_RemoveCallback;
    std::string m_Content;

    Gtk::CheckButton m_Checkbox;
    Gtk::Label m_Label;
    Gtk::Button m_DeleteButton;
};

class TodoList : public Gtk::Box
{
public:
    TodoList();
    void add_todo(const std::string& content, bool done = false);
    void delete_todo(Todo *todo);
    const std::vector<std::unique_ptr<Todo>>& get_todos() const;

private:
    std::vector<std::unique_ptr<Todo>> m_Todos;
};

class MainWindow : public Gtk::Window
{
public:
    MainWindow();
    ~MainWindow();

private:
    void load_config_file();
    void save_config_file();
    void button_on_click();

private:
    Gtk::Box m_Box;

    Gtk::Frame m_TodosFrame;
    Gtk::ScrolledWindow m_TodosScrolledWindow;
    TodoList m_TodoList;

    Gtk::Box m_InputBox;
    Gtk::Entry m_TextInput;
    Gtk::Button m_AddButton;
};

Todo::Todo(const std::string& content, std::function<void(Todo*)> remove_callback)
    : m_Done(false), m_RemoveCallback(remove_callback), m_Content(content)
{
    set_margin(5);
    set_spacing(5);
    set_hexpand(true);

    m_Checkbox.signal_toggled().connect(
        sigc::mem_fun(*this, &Todo::checkbox_on_click));
    append(m_Checkbox);

    m_Label.set_label(m_Content);
    m_Label.set_halign(Gtk::Align::START);
    m_Label.set_hexpand(true);
    append(m_Label);

    m_DeleteButton.set_label("Remove");
    m_DeleteButton.set_halign(Gtk::Align::END);
    m_DeleteButton.signal_clicked().connect([this](){ m_RemoveCallback(this); });
    append(m_DeleteButton);
}

std::string Todo::get_content() const
{
    return m_Content;
}

bool Todo::is_done() const
{
    return m_Done;
}

void Todo::set_done(bool state)
{
    m_Done = state;
    m_Checkbox.set_active(m_Done);
}

void Todo::checkbox_on_click()
{
    m_Done = m_Checkbox.get_active();
}

TodoList::TodoList()
{
    set_orientation(Gtk::Orientation::VERTICAL);
    set_vexpand(true);
}

void TodoList::add_todo(const std::string& content, bool done/*= false*/)
{
    auto todo = std::make_unique<Todo>(content, [this](Todo *todo) {
        delete_todo(todo);
    });
    todo->set_done(done);
    m_Todos.push_back(std::move(todo));
    append(*m_Todos.back());
}

void TodoList::delete_todo(Todo *todo)
{
    for (auto it = m_Todos.begin(); it != m_Todos.end(); ++it) {
        if ((*it).get() == todo) {
            remove(*todo);
            m_Todos.erase(it);
            break;
        }
    }
}

const std::vector<std::unique_ptr<Todo>>& TodoList::get_todos() const
{
    return m_Todos;
}

MainWindow::MainWindow()
{
    set_title("gtodo");
    set_default_size(400, 500);

    load_config_file();

    m_Box.set_orientation(Gtk::Orientation::VERTICAL);
    m_Box.set_margin(10);
    set_child(m_Box);

    m_TodosFrame.set_margin(10);
    m_TodosFrame.set_label("Todo list");
    m_Box.append(m_TodosFrame);

    m_TodosFrame.set_child(m_TodosScrolledWindow);

    // m_TodoList.add_todo("Walk the dog");
    // m_TodoList.add_todo("Buy coffe");
    // m_TodoList.add_todo("Do homework");
    // m_TodoList.add_todo("Finish the todo app");
    m_TodosScrolledWindow.set_child(m_TodoList);

    m_TextInput.set_placeholder_text("New todo");
    m_TextInput.set_margin_end(5);
    m_TextInput.set_hexpand(true);
    m_TextInput.signal_activate().connect([this](){ button_on_click(); });
    m_InputBox.append(m_TextInput);

    m_AddButton.set_label("Add");
    m_AddButton.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::button_on_click));
    m_InputBox.append(m_AddButton);

    m_Box.append(m_InputBox);
}

MainWindow::~MainWindow()
{
    save_config_file();
}

void MainWindow::load_config_file()
{
    std::ifstream config_file;
    const char *home_dir = getenv("HOME");
    if (home_dir != NULL) {
        std::string config_path(home_dir);
        config_path.append("/.gtodo");
        config_file.open(config_path);
    }

    if (!config_file.is_open()) {
        return;
    }

    std::string line;
    while (std::getline(config_file, line)) {
        std::stringstream ss(line);
        bool done;
        std::string content;

        ss >> done;
        ss >> std::ws;
        std::getline(ss, content);

        m_TodoList.add_todo(content, done);
    }

    config_file.close();
}

void MainWindow::save_config_file()
{
    std::ofstream config_file;
    const char *home_dir = getenv("HOME");
    if (home_dir != NULL) {
        std::string config_path(home_dir);
        config_path.append("/.gtodo");
        config_file.open(config_path);
    }

    if (!config_file.is_open()) {
        return;
    }

    for (const auto& todo : m_TodoList.get_todos()) {
        config_file << todo->is_done() << ' ';
        config_file << todo->get_content();
        config_file << '\n';
    }

    config_file.close();
}

void MainWindow::button_on_click()
{
    if (m_TextInput.get_text().empty()) {
        return;
    }
    m_TodoList.add_todo(m_TextInput.get_text());
    m_TextInput.set_text("");
}

int main(int argc, char **argv)
{
    auto app = Gtk::Application::create("org.seajee.gtodo");
    return app->make_window_and_run<MainWindow>(argc, argv);
}
