#include "game.h"

using namespace obj;

void Game::build_settingsINI(SI_Error rc) {
    printf("\nsettings.ini not found! Building from default...\n");

    CSimpleIniA def_config;
    def_config.SetUnicode();
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
    if (rc < 0) { this->build_settingsINI(rc); }
}

void Game::GetSettings(SI_Error rc) {
    if (rc < 0) { this->build_settingsINI(rc); }
    int width = std::stoi(this->config.GetValue("Window", "Width", "800"));
    int height = std::stoi(this->config.GetValue("Window", "Height", "600"));
    int max_fps = std::stoi(this->config.GetValue("Window", "MaxFPS", "60"));
    bool vsync = std::string(this->config.GetValue("Window", "VSync", "True")) == "True";
    const char* title = config.GetValue("Window", "Title", "Untitled");

    this->window.create(sf::VideoMode(width, height), title);
    this->window.setFramerateLimit(max_fps);
    this->window.setVerticalSyncEnabled(vsync);
}

Game::Game() {
    this->config.SetUnicode();
    SI_Error rc = this->config.LoadFile("settings.ini");
    this->GetSettings(rc);
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

void Game::find_clicked(sf::Event event) {
    if (event.mouseButton.button != sf::Mouse::Left) { return; }
   
    sf::Vector2f mousePos(
        static_cast<float>(event.mouseButton.x),
        static_cast<float>(event.mouseButton.y)
    );

    for (auto& ui: this->ui_objects) {
        if (!ui->MouseHover(mousePos)) { continue; }

        auto btn = dynamic_cast<TextButton*>(ui.get());
        if ( (!btn) || (btn && btn->GetText().getString() != "Start Game") ) { continue; }

        this->ui_objects.clear();
        this->state = GameState::Playing;
    }
}

void Game::find_mouse_move(sf::Event event) {
    sf::Vector2f mousePos(
        static_cast<float>(event.mouseMove.x),
        static_cast<float>(event.mouseMove.y)
    );

    for (auto& ui : this->ui_objects) {
        auto ui_obj = static_cast<Button*>(ui.get());
        if ( !ui_obj || (ui_obj && ui_obj->GetTimer().active) ) { continue; }
        if (ui_obj->MouseHover(mousePos)) {
            ui_obj->SetColor(sf::Color::Yellow);
            continue;
        }
        if ( (!ui_obj->GetTimer().active) && ui_obj->GetColor() == ui_obj->GetDefaultColor()) { continue; }
        ui_obj->SetColor(ui_obj->GetDefaultColor());
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

void Game::UpdateGui() {
    for (auto& ui : this->ui_objects) {
        ui->Update(this->window);
    }
}

void Game::DrawText(auto& given) {
    if (auto draw_obj = dynamic_cast<TextButton*>(given.get())) {
        this->window.draw(draw_obj->GetText());
    } else if (auto draw_obj = dynamic_cast<TextBox*>(given.get())) {
        this->window.draw(draw_obj->GetText());
    }
}

void Game::DrawGui() {
    for (auto& ui : this->ui_objects) {
        this->window.draw(*ui->GetHitbox());

        this->DrawText(ui);
    }
}

void Game::loop() {
    while (this->window.isOpen()) {
        this->handle_events();
        this->window.clear(sf::Color::Black);

        this->UpdateGui();
        this->DrawGui();

        this->window.display();
    }
}

int Game::Run() {
    sf::Vector2f titleSize{260.f, 120.f};
    sf::Vector2f titlePos{
        (this->window.getSize().x - titleSize.x) / 2.f,
        (this->window.getSize().y - titleSize.y) / 5.f
    };

    sf::Vector2f buttonSize{200.f, 90.f};
    sf::Vector2f buttonPos {
        (this->window.getSize().x - buttonSize.x) / 2.f,
        (this->window.getSize().y - buttonSize.y) / 2.f
    };

    NewGui(this->ui_objects, GuiType::TextBox, "TitleBar", titleSize, titlePos, sf::Color::Transparent, "WELCOME", 32, sf::Color::Green );

    NewGui(this->ui_objects, GuiType::TextButton, "StartButton", buttonSize, buttonPos, sf::Color::Cyan, "Start Game", 16, sf::Color::Black );

    this->loop();

    return 0;
}
