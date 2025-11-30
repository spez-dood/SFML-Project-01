#ifndef obj_h
#define obj_h

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <variant>

namespace obj {
    using ShapePtr = std::unique_ptr<sf::Drawable>;
    using ShapeSize = std::variant<sf::Vector2f, float>;
    using GuiAttachment = std::variant<std::string, const char*, sf::Image>;

    inline ShapePtr NewShape(ShapeSize size, sf::Vector2f pos, sf::Color fillColor) {
        auto _shape = std::unique_ptr<sf::Shape>();
        if (std::holds_alternative<sf::Vector2f>(size)) {
            _shape = std::make_unique<sf::RectangleShape>(std::get<sf::Vector2f>(size));
        } else {
            _shape = std::make_unique<sf::CircleShape>(std::get<float>(size));
        }
        _shape->setPosition(pos.x, pos.y);
        _shape->setFillColor(fillColor);
        return _shape;
    }

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

        void Update(sf::RenderWindow& window) {
            sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
            auto shape = dynamic_cast<sf::Shape*>(this->GetHitbox().get());
            if (!shape) { return; }
            if (this->MouseHover(mousePos)) { return; }
            shape->setFillColor(this->GetDefaultColor());
        }

        const std::string& GetName() const { return this->Name; }
        void SetName(const std::string& new_name) { this->Name = new_name; }

        GuiType GetType() { return this->Type; }

        ShapePtr& GetHitbox() { return this->Hitbox; }
        
        bool MouseHover(sf::Vector2f point) const {
            return Shape() && Shape()->getGlobalBounds().contains(point);
        }

        sf::Color GetDefaultColor() const { return this->default_color; }
        sf::Color GetColor() {
            return Shape() ? Shape()->getFillColor() : sf::Color::Transparent;
        }

        void SetColor(
            sf::Color goal_color = sf::Color::White,
            int cooldown_ms = 0,
            sf::Color resetColor = sf::Color::White
        ) {
            auto shape = dynamic_cast<sf::Shape*>(this->Hitbox.get());
            if (!shape) { return; }
            shape->setFillColor(goal_color);
            // handle cooldown if provided...
        }

    protected:
        Gui(sf::RenderWindow& window) : Window(window) {}
        sf::Shape* Shape() const { return dynamic_cast<sf::Shape*>(this->Hitbox.get()); }
        sf::RenderWindow& Window;
        ShapePtr Hitbox;

        std::string Name = "Frame";
        GuiType Type = GuiType::Frame;
        sf::Color default_color = sf::Color::White;
    };

    class Button : public Gui {
    public:
        Button(
            sf::RenderWindow& window,
            const std::string& new_name = "Button",
            ShapeSize size = ShapeSize{0.f},
            sf::Vector2f pos = sf::Vector2f(),
            sf::Color fillColor = sf::Color::White
        ) : Gui(window) {
            ShapePtr new_hitbox = NewShape(size, pos, fillColor);

            this->Type = GuiType::Button;
            this->default_color = fillColor;

            auto* shape = dynamic_cast<sf::Shape*>(new_hitbox.get());
            if (shape) {
                shape->setFillColor(fillColor);
            }

            this->Hitbox = std::move(new_hitbox);
        }

        void Cooldown(int duration_ms = 2000, sf::Color resetColor = sf::Color::White) {
            if (!this->WasPressed()) { return; }
            if (this->press_cooldown.clock.getElapsedTime().asMilliseconds() < this->press_cooldown.duration_ms) { return; }
            if (auto shape = dynamic_cast<sf::Shape*>(this->Hitbox.get())) {
                shape->setFillColor(this->default_color);
            }
            this->press_cooldown.active = false;
            this->press_cooldown.duration_ms = 0;
            this->press_cooldown.clock.restart();
        }

        bool WasPressed() const { return this->press_cooldown.active; };
    protected:
        Timer press_cooldown;
    };

    class TextLabel {
    public:
        TextLabel(
            sf::RenderWindow& window,
            const std::string& label = "I'm a label!",
            ShapeSize size = sf::Vector2f(),
            sf::Vector2f pos = sf::Vector2f(),
            sf::Color textColor = sf::Color::White
        ) {
            this->Font.loadFromFile("resource/fonts/Arial.ttf");
            this->Text.setFont(this->Font);
            this->SetText(label);
            this->CenterText(std::get<sf::Vector2f>(size), pos);
        }

        void SetText(
            const std::string& str = "Default!",
            unsigned int charSize = 16,
            sf::Color textColor = sf::Color::Black
        ) {
            this->Text.setCharacterSize(charSize);
            this->Text.setFillColor(textColor);
            this->Text.setString(str);
        }

        void CenterText(sf::Vector2f boxSize, sf::Vector2f pos) {
            auto bounds = Text.getLocalBounds();
            Text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
            Text.setPosition(pos.x + boxSize.x / 2.f, pos.y + boxSize.y / 2.f);
        }

        sf::Text& GetText() { return this->Text; }
    private:
        sf::Font Font;
        sf::Text Text;
    };

    class TextButton : public Button {
    public:
        TextButton(
            sf::RenderWindow& window,
            const std::string& new_name = "TextButton",
            const std::string& label = "Click me!",
            ShapeSize size = sf::Vector2f(200.f, 90.f),
            sf::Vector2f pos = sf::Vector2f(),
            sf::Color fillColor = sf::Color::White
        ) : Button(window, new_name, size, pos, fillColor),
        textLabel(window, label, size, pos, sf::Color::Black) {}

        TextLabel& GetLabel() { return textLabel; }
    private:
        TextLabel textLabel;
    };

    using GuiVector = std::vector<std::unique_ptr<obj::Gui>>;

}

#endif
