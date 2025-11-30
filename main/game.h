#ifndef game_h
#define game_h

#include "config.h" // SimpleIni tools
#include "obj.h"

enum class GameState { MainMenu, Playing, GameOver };

class Game {
public:
    Game();

    int loop();
protected:
    void build_settingsINI(SI_Error rc);
    void find_clicked(sf::Event event);
    void find_mouse_move(sf::Event event);
    void get_event(sf::Event event);
    void key_pressed(sf::Keyboard::Scancode keycode);
    void handle_events();

    void NewButton(
    const std::string& name = "TextButton",
    obj::ShapeSize size = sf::Vector2f{200.f,90.f},
    sf::Vector2f pos = sf::Vector2f{400.f, 300.f},
    sf::Color fillColor = sf::Color::White,
    obj::GuiAttachment = "Click me!"
    );
private:
    GameState state = GameState::MainMenu;
    obj::GuiVector ui_objects;
    sf::RenderWindow window;
    CSimpleIniA config;
};

#endif
