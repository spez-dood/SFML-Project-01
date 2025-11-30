#include "game.h"

using namespace obj;

void Game::build_settingsINI(SI_Error rc) {
    printf("\nsettings.ini not found! Building from default...\n");

    CSimpleIniA def_config;
    SI_Error def_rc = def_config.LoadFile("default-settings.ini");
    if (def_rc < 0) {
        printf("\ndefault-settings.ini not found! Building...\n");
        def_rc = def_config.SetValue("Window", nullptr, nullptr);
        def_rc = def_config.SetValue("Window", "Width", "800");
        def_rc = def_config.SetValue("Window", "Height", "600");
        def_rc = def_config.SetValue("Window", "MaxFPS", "60");
        def_rc = def_config.SetValue("Window", "VSync", "True");
        def_rc = def_config.SetValue("Window", "Title", "Vector Art");

    }
    std::string data;
    def_rc = def_config.Save(data);
    def_rc = def_config.SaveFile("default-settings.ini");
    def_rc = def_config.SaveFile("settings.ini");

    rc = this->config.LoadFile("Settings.ini");
}

Game::Game() {
    this->config.SetUnicode();
    SI_Error rc = this->config.LoadFile("settings.ini");
    if (rc < 0) { build_settingsINI(rc); }
    

    int width = std::stoi(this->config.GetValue("Window", "Width", "800"));
    int height = std::stoi(this->config.GetValue("Window", "Height", "600"));
    int max_fps = std::stoi(this->config.GetValue("Window", "MaxFPS", "60"));
    bool vsync = std::string(this->config.GetValue("Window", "VSync", "True")) == "True";
    const char* title = config.GetValue("Window", "Title", "Untitled");

    this->window.create(sf::VideoMode(width, height), title);
    this->window.setFramerateLimit(max_fps);
    this->window.setVerticalSyncEnabled(vsync);

    this->state = GameState::MainMenu;
}

void Game::key_pressed(sf::Keyboard::Scancode keycode) {
    switch(keycode) {
        case sf::Keyboard::Scancode::Escape:
            this->window.close();
            break;
        case sf::Keyboard::Scancode::Space:
            break;
        default:
            break;
    }
}

bool isStartButton(const std::unique_ptr<Gui>& btn) {
    auto txtbtn = dynamic_cast<TextButton*>(btn.get());
    return txtbtn && txtbtn->GetText().getString() == "Start Game";
}

void Game::find_clicked(sf::Event event) {
    if (event.mouseButton.button != sf::Mouse::Left) { return; }
   
    sf::Vector2f mousePos(
        static_cast<float>(event.mouseButton.x),
        static_cast<float>(event.mouseButton.y)
    );

    for (auto& ui: this->ui_objects) {
        if (!ui->MouseHover(mousePos)) { continue; }

        auto rect = dynamic_cast<sf::RectangleShape*>(ui->GetHitbox().get());
        if (!rect) { continue; }

        ui->SetColor(600, sf::Color::Red, rect->getFillColor());

        auto it = std::find_if(
            ui_objects.begin(),
            ui_objects.end(),
            isStartButton
        );

        if (it != ui_objects.end()) {
            this->ui_objects.erase(it);
            this->state = GameState::Playing;
        }
    }
}

void Game::find_mouse_move(sf::Event event) {
    sf::Vector2f mousePos(
        static_cast<float>(event.mouseMove.x),
        static_cast<float>(event.mouseMove.y)
    );

    for (auto& ui : this->ui_objects) {
        auto ui_obj = static_cast<Button*>(ui.get());
        if (!ui_obj) { continue; }
        if (ui_obj->WasPressed()) { continue; }
        if (ui_obj->MouseHover(mousePos)) {
            ui_obj->SetColor(20, sf::Color::Yellow, ui_obj->GetDefaultColor());
            continue;
        }
        if ( (!ui_obj->WasPressed()) && ui_obj->GetColor() == ui_obj->GetDefaultColor()) { continue; }
        ui_obj->SetColor(40, ui_obj->GetDefaultColor());
    }
}

void Game::get_event(sf::Event event) {
    switch(event.type) {
        case sf::Event::Closed:
            this->window.close();
            break;
        case sf::Event::KeyPressed:
            this->key_pressed(event.key.scancode);
            break;
        case sf::Event::MouseButtonPressed: {
            this->find_clicked(event);
            break; }
        case sf::Event::MouseMoved: {
            this->find_mouse_move(event);
            break; }
        default:
            break;
    }
}

void Game::handle_events() {
    sf::Event event;
    while (this->window.pollEvent(event)) {
        get_event(event);
    }
}

void Game::NewTextButton(
    const char* name,
    ShapeSize size,
    sf::Vector2f pos,
    sf::Color fillColor,
    const std::string& label
) {
    this->ui_objects.push_back(
        std::make_unique<TextButton>(this->window, name, label, size, pos, fillColor)
    );
}

int Game::loop() {
    sf::Vector2f buttonSize{200.f, 90.f};
    sf::Vector2f pos {
        (this->window.getSize().x - buttonSize.x) / 2.f,
        (this->window.getSize().y - buttonSize.y) / 2.f
    };

    NewTextButton(
        "StartButton",
        buttonSize,
        pos,
        sf::Color::Cyan,
        "Start Game"
    );

    while (window.isOpen()) {
        this->handle_events();
        this->window.clear(sf::Color::Black);

        for (auto& ui : this->ui_objects) {
            ui->Update(this->window);
        }

        for (auto& ui : this->ui_objects) {
            this->window.draw(*ui->GetHitbox());
            if (auto ui_obj = dynamic_cast<TextButton*>(ui.get())) {
                this->window.draw(ui_obj->GetText());
            }
        }

        this->window.display();
    }
    return 0;
}