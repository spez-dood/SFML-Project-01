#ifndef obj_h
#define obj_h

#include <SFML/Graphics.hpp>
#include <assert.h>
#include <memory>
#include <vector>
#include <variant>
#include <cstring>


using ShapePtr = std::unique_ptr<sf::Drawable>;
using ShapeSize = std::variant<sf::Vector2f, float>;

namespace obj {
    ShapePtr NewShape(ShapeSize size, sf::Vector2f pos, sf::Color fillColor);

    enum class GuiType { Frame, Button, TextButton, TextLabel, TextBox, ScrollBar, Image };

    struct Timer {
        sf::Clock clock;
        int duration_ms = 0;
        bool active = false;

        Timer(int duration = 0, bool isActive = false)
        : duration_ms(duration), active(isActive) {}
    };

    class Gui {
    public:
        virtual ~Gui() = default;
        virtual bool MouseHover(sf::Vector2f point) const;

        virtual void Update(sf::RenderWindow& window) {
            sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
            auto shape = dynamic_cast<sf::Shape*>(this->GetHitbox().get());
            if (!shape) { return; }
            if (this->MouseHover(mousePos)) { return; }
            shape->setFillColor(this->GetDefaultColor());
        };

        virtual char* GetName() { return this->Name; }; 
        virtual void SetName(const char* new_name) {
            std::strncpy(this->Name, new_name, sizeof(this->Name));
            this->Name[sizeof(this->Name)-1] = '\0';
        }

        virtual GuiType GetType() { return this->Type; };
        virtual void SetType(GuiType new_type) { this->Type = new_type; }

        virtual ShapePtr& GetHitbox() { return this->Hitbox; };
        virtual void SetHitbox(ShapePtr hitbox) { this->Hitbox = std::move(hitbox); };

        virtual sf::Color GetDefaultColor() const { return this->default_color; };
        virtual void SetDefaultColor(sf::Color color) { this->default_color = color; };

        virtual sf::Color GetColor() {
            return this->Hitbox ? dynamic_cast<sf::Shape*>(this->GetHitbox().get())->getFillColor()
                : sf::Color::Transparent;
        };
        virtual void SetColor(sf::Color goal_color = sf::Color::White);
        virtual void SetColor(
            int cooldown_ms,
            sf::Color goal_color = sf::Color::White,
            sf::Color resetColor = sf::Color::White
        );

        virtual sf::RenderWindow& GetWindow() { return this->Window; }
    protected:
         Gui(sf::RenderWindow& window) : Window(window) {}
        sf::RenderWindow& Window;
        char Name[32];
        GuiType Type;
        ShapePtr Hitbox;
        sf::Color default_color;
    };

    class Button : public Gui {
    public:
        Button(
            sf::RenderWindow& window,
            GuiType new_type = GuiType::Button,
            const char* new_name = "Button",
            ShapeSize size = ShapeSize{0.f},
            sf::Vector2f pos = sf::Vector2f(),
            sf::Color fillColor = sf::Color::White
        );
        virtual void Cooldown(int duration_ms = 2000, sf::Color resetColor = sf::Color::White);
        virtual bool WasPressed() const { return this->press_cooldown.active; };
    protected:
        Timer press_cooldown;
    };

    class TextButton : public Button {
    public:
        TextButton(
            sf::RenderWindow& window,
            const char* new_name = "TextButton",
            const std::string& label = "Click me!",
            ShapeSize size = sf::Vector2f(200.f, 90.f),
            sf::Vector2f pos = sf::Vector2f(),
            sf::Color boxColor = sf::Color::White
        );
    
        void SetText(
            const std::string& str,
            unsigned int charSize = 16,
            sf::Color textColor = sf::Color::Black
        );
        const sf::Text& GetText() const { return this->Text; };
    private:
        sf::Font Font;
        sf::Text Text;
    };

}

using GuiVector = std::vector<std::unique_ptr<obj::Gui>>;

#endif