#ifndef game_h
#define game_h

#include "config.h" // SimpleIni tools
#include "obj.h"

enum class GameState { MainMenu, Playing, GameOver };

class Game {
public:
    Game();

    int Run();
protected:
    void GetSettings(SI_Error rc);
    void build_settingsINI(SI_Error rc);
    void find_clicked(sf::Event event);
    void find_mouse_move(sf::Event event);
    void get_event(sf::Event event);
    void key_pressed(sf::Keyboard::Scancode keycode);
    void handle_events();
    void UpdateGui();
    void DrawGui();
    void DrawText(auto& given);
    void loop();

private:
    GameState state = GameState::MainMenu;
    obj::GuiVector ui_objects;
    sf::RenderWindow window;
    CSimpleIniA config;
};

#endif
