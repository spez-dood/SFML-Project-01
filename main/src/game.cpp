#include "game.h"

using namespace gui;
// using namespace entity;

void build_default_settingsINI(CSimpleIniA& def_config, SI_Error & def_rc) {
    printf("\ndefault-settings.ini not found! Building...\n");

    def_rc = def_config.SetValue("Window", nullptr, nullptr);
    def_rc = def_config.SetValue("Window", "Width", "800");
    def_rc = def_config.SetValue("Window", "Height", "600");
    def_rc = def_config.SetValue("Window", "MaxFPS", "60");
    def_rc = def_config.SetValue("Window", "VSync", "True");
    def_rc = def_config.SetValue("Window", "Title", "Vector Art");

    def_rc = def_config.SaveFile("default-settings.ini");
}

void Game::build_settingsINI(SI_Error rc, CSimpleIniA& def_config, SI_Error def_rc) {
    printf("\nsettings.ini not found! Building from default...\n");

    if (def_rc < 0) { build_default_settingsINI(def_config, def_rc); }

    CSimpleIniA::TNamesDepend sections;
    def_config.GetAllSections(sections);

    for (const auto& sec : sections) {
        const char* sectionName = sec.pItem;

        CSimpleIniA::TNamesDepend keys;
        def_config.GetAllKeys(sectionName, keys);

        for (const auto& key : keys) {
            const char* keyName = key.pItem;
            const char* value = def_config.GetValue(sectionName, keyName, "");

            this->config.SetValue(sectionName, keyName, value);
        }
    }

    SI_Error save_rc = this->config.SaveFile("settings.ini");
    if (save_rc < 0) {
        printf("Failed to save settings.ini!\n");
    }

    rc = this->config.LoadFile("settings.ini");
    if (rc < 0) { this->build_settingsINI(rc, def_config, def_rc); }
}

void Game::GetSettings(SI_Error rc) {
    CSimpleIniA def_config;
    def_config.SetUnicode();
    SI_Error def_rc = def_config.LoadFile("default-settings.ini");
    if (rc < 0) { this->build_settingsINI(rc, def_config, def_rc); }

    const char* def_width = def_config.GetValue("Window", "Width");
    const char* def_height = def_config.GetValue("Window", "Height");
    const char* def_maxfps = def_config.GetValue("Window", "MaxFPS");
    const char* def_vsync = def_config.GetValue("Window", "VSync");
    const char* def_title = def_config.GetValue("Window", "Title");

    int width = std::stoi(this->config.GetValue("Window", "Width", def_width));
    int height = std::stoi(this->config.GetValue("Window", "Height", def_height));
    int max_fps = std::stoi(this->config.GetValue("Window", "MaxFPS", def_maxfps));
    bool vsync = std::string(this->config.GetValue("Window", "VSync", def_vsync)) == "True";
    const char* title = this->config.GetValue("Window", "Title", def_title);

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

void Game::find_clicked(sf::Event::MouseButtonEvent mouseButton) {
    if (mouseButton.button != sf::Mouse::Left) { return; }
   
    sf::Vector2f mousePos(
        static_cast<float>(mouseButton.x),
        static_cast<float>(mouseButton.y)
    );

    for (auto& ui: this->ui_objects) {
        if ( !ui->MouseHover(mousePos) || !ui->onClick ) { continue; }
        ui->onClick();
        return;
    }
}

void Game::find_mouse_move(sf::Event::MouseMoveEvent mouseMove) {
    sf::Vector2f mousePos(
        static_cast<float>(mouseMove.x),
        static_cast<float>(mouseMove.y)
    );

    for (auto& ui : this->ui_objects) {
        if (auto btn = dynamic_cast<Button*>(ui.get())) {
            if (btn->GetTimer().active) continue;
        }

        ui->HandleMouseMove(mousePos);
    }
}

void Game::get_event(sf::Event event) {
    switch(event.type) {
        case sf::Event::Closed:
            this->window.close();
            break;
        case sf::Event::KeyPressed: {
            this->key_pressed(event.key.scancode);
            break; }
        case sf::Event::MouseButtonPressed: {
            this->find_clicked(event.mouseButton);
            break; }
        case sf::Event::MouseMoved: {
            this->find_mouse_move(event.mouseMove);
            break; }
        default:
            break;
    }
}

void Game::Setup_StartScreen() {
    NewGui(this->ui_objects, this->Prep(GuiConfig{
        .type = GuiType::TextBox,
        .name = "TitleBar",
        .size = sf::Vector2f{260.f, 120.f},
        .fillColor = sf::Color::Transparent,
        .attachment = "WELCOME",
        .charSize = 36,
        .textColor = sf::Color::Green
    }, &Game::TopCenterGui));
    
    Gui* startButton = NewGui(this->ui_objects, this->Prep(GuiConfig{
        .type = GuiType::TextButton,
        .name = "StartButton",
        .size = sf::Vector2f{200.f, 90.f},
        .fillColor = sf::Color(240, 180, 20),
        .attachment = "Start Game",
        .charSize = 18,
        .textColor = sf::Color::Black
    }, &Game::CenterGui));

    startButton->onClick = [this]() {
        this->state = GameState::MainMenu;
        this->state_changed = true;
    };
    startButton->onMouseEnter = [startButton]() {
        startButton->SetColor(sf::Color(200, 150, 10));
    };
    startButton->onMouseExit = [startButton]() {
        startButton->SetColor(startButton->GetInfo().defaultColor);
    };
}

void Game::Setup_MainMenu() {
    this->ui_objects.clear();

    GuiConfig titleBarConfig = GuiConfig{
        .type = GuiType::TextBox,
        .name = "MenuTitle",
        .size = sf::Vector2f{220.f, 100.f},
        .fillColor = sf::Color::Transparent,
        .attachment = "MAIN MENU",
        .charSize = 32,
        .textColor = sf::Color::Yellow
    };

    NewGui(this->ui_objects, this->Prep(GuiConfig{
        .type = GuiType::TextBox,
        .name = "MenuTitle",
        .size = sf::Vector2f{220.f, 100.f},
        .fillColor = sf::Color::Transparent,
        .attachment = "MAIN MENU",
        .charSize = 32,
        .textColor = sf::Color::Yellow
    }, &Game::TopCenterGui));

    Gui* playButton = NewGui(this->ui_objects, this->Prep(GuiConfig{
        .type = GuiType::TextButton,
        .name = "PlayButton",
        .size = sf::Vector2f{200.f, 90.f},
        .fillColor = sf::Color(20, 20, 80),
        .attachment = "Play Game",
        .charSize = 18,
        .textColor = sf::Color::Cyan
    }, &Game::CenterGui));

    playButton->onClick = [this]() {
        this->state = GameState::Playing;
        this->state_changed = true;
    };
    playButton->onMouseEnter = [playButton]() {
        playButton->SetColor(sf::Color(10, 150, 200));
    };
    playButton->onMouseExit = [playButton]() {
        playButton->SetColor(playButton->GetInfo().defaultColor);
    };
}

void Game::Setup_Gameplay() {
    this->ui_objects.clear();

    NewGui(this->ui_objects, this->Prep(GuiConfig{
        .type = GuiType::TextBox,
        .name = "GameplayLabel",
        .size = sf::Vector2f{300.f, 100.f},
        .fillColor = sf::Color::Transparent,
        .attachment = "GAMEPLAY STATE - Press ESC to Quit",
        .charSize = 24,
        .textColor = sf::Color::White
    }, &Game::TopCenterGui));
}

void Game::handle_state_change() {
    if (!this->state_changed) { return; }

    this->state_changed = false;

    switch (this->state) {
        case GameState::StartScreen:
            this->Setup_StartScreen();
            break;
        case GameState::MainMenu:
            this->Setup_MainMenu();
            break;
        case GameState::Playing:
            this->Setup_Gameplay();
            break;
        case GameState::GameOver:
            this->ui_objects.clear();
            break;
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

void Game::DrawGui() {
    for (auto& ui : this->ui_objects) {
        this->window.draw(*ui->GetInfo().hitbox);
        if (auto label = dynamic_cast<TextLabel*>(ui.get())) {
            this->window.draw(label->GetText());
        }
    }
}

void Game::loop() {
    while (this->window.isOpen()) {
        this->handle_state_change();
        this->handle_events();
        this->window.clear(sf::Color::Black);

        this->UpdateGui();
        this->DrawGui();

        this->window.display();
    }
}

int Game::Run() {
    this->state = GameState::StartScreen;
    this->state_changed = true;

    this->loop();

    return 0;
}
