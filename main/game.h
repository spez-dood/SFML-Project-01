#ifndef game_h
#define game_h

#include "config.h" // SimpleIni tools
#include "gui.h"
// #include "entity.h"

using namespace gui;

enum class GameState { Boot, StartScreen, MainMenu, Playing, GameOver };

class Game {
public:
    Game();

    int Run();
protected:
    void GetSettings(SI_Error rc);
    void build_settingsINI(SI_Error rc, CSimpleIniA& def_config, SI_Error def_rc);
    void find_clicked(sf::Event::MouseButtonEvent mouseButton);
    void find_mouse_move(sf::Event::MouseMoveEvent mouseMove);
    void get_event(sf::Event event);
    void key_pressed(sf::Keyboard::Scancode keycode);

    void Setup_StartScreen();
    void Setup_MainMenu();
    void Setup_Gameplay();

    void handle_state_change();
    void handle_events();
    void UpdateGui();
    void DrawGui();
    void loop();

    gui::GuiConfig Prep(
        gui::GuiConfig cfg, 
        PrepGui posCalc = {},
        sf::Vector2f offset = {0.f, 0.f}
    ) const {
        if (posCalc) {
            cfg.pos = posCalc(cfg);
            return cfg;
        }

        auto s = cfg.SizeVec();
        cfg.pos = {
            (static_cast<float>(this->window.getSize().x) - s.x) / offset.x,
            (static_cast<float>(this->window.getSize().y) - s.y) / offset.y
        };
        return cfg;
    };

    gui::GuiConfig Prep(
        gui::GuiConfig cfg,
        sf::Vector2f (Game::*posCalc)(const gui::GuiConfig&) const
    ) const {
        if (posCalc) {
            cfg.pos = (this->*posCalc)(cfg);
            return cfg;
        }
        return Prep(std::move(cfg), PrepGui());
    };

    sf::Vector2f CenterGui(const gui::GuiConfig& cfg) const {
        return sf::Vector2f{
            (this->window.getSize().x - cfg.SizeVec().x) / 2.f,
            (this->window.getSize().y - cfg.SizeVec().y) / 2.f
        };
    }

    sf::Vector2f TopCenterGui(const gui::GuiConfig& cfg) const {
        return sf::Vector2f{
            (this->window.getSize().x - cfg.SizeVec().x) / 2.f,
            (this->window.getSize().y - cfg.SizeVec().y) / 5.f
        };
    }

private:
    GameState state = GameState::Boot;
    bool state_changed = false;
    gui::GuiVector ui_objects;
    sf::RenderWindow window;
    CSimpleIniA config;
};

#endif
