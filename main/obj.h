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
        ShapePtr shapecast = std::get_if<sf::Vector2f>(&size)
        ? ShapePtr(new sf::RectangleShape(*std::get_if<sf::Vector2f>(&size)))
        : ShapePtr(new sf::CircleShape(*std::get_if<float>(&size)));

        auto* shape = dynamic_cast<sf::Shape*>(shapecast.get());
        shape->setPosition(pos.x, pos.y);
        shape->setFillColor(fillColor);
        return shapecast;
    }

    enum class GuiType { Frame, Button, TextButton, TextBox, TextInput, Image };

    struct Timer {
        sf::Clock clock;
        int duration_ms = 0;
        bool active = false;

        Timer(int duration = 0, bool isActive = false)
        : duration_ms(duration), active(isActive) {}
    };

    class TextLabel {
    public:
        TextLabel(
            const std::string& label = "I'm a label!",
            ShapeSize size = sf::Vector2f(),
            unsigned int charSize = 16,
            sf::Vector2f pos = sf::Vector2f(),
            sf::Color textColor = sf::Color::White
        ) {
            this->Font.loadFromFile("resource/fonts/Arial.ttf");
            this->Text.setFont(this->Font);
            this->SetText(label, charSize, textColor);
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

    class Gui {
    public:
        virtual ~Gui() = default;

        void Update(sf::RenderWindow& window) {
            sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
            if (this->MouseHover(mousePos)) { return; }
            dynamic_cast<sf::Shape*>(this->Hitbox.get())->setFillColor(this->default_color);
        }

        const std::string& GetName() const { return this->Name; }
        void SetName(const std::string& new_name) { this->Name = new_name; }

        GuiType GetType() { return this->Type; }

        ShapePtr& GetHitbox() { return this->Hitbox; }
        
        bool MouseHover(sf::Vector2f point) const {
            return this->Shape() && this->Shape()->getGlobalBounds().contains(point);
        }

        sf::Color GetDefaultColor() const { return this->default_color; }
        sf::Color GetColor() {
            return this->Shape() ? this->Shape()->getFillColor() : sf::Color::Transparent;
        }

        void SetColor(sf::Color goal_color = sf::Color::White) {
            this->Shape()->setFillColor(goal_color);
        }

    protected:
        Gui(
            GuiType new_type = GuiType::Frame,
            const std::string& new_name = "Gui",
            sf::Color fillColor = sf::Color::White,
            ShapeSize size = ShapeSize{0.f},
            sf::Vector2f pos = sf::Vector2f()
        ) : 
        Hitbox(std::move(NewShape(size, pos, fillColor))),
        Name(new_name),
        Type(new_type),
        default_color(fillColor) {
            this->SetColor(fillColor);
        }

        sf::Shape* Shape() const { return dynamic_cast<sf::Shape*>(this->Hitbox.get()); }
        ShapePtr Hitbox;
        std::string Name;
        GuiType Type;
        sf::Color default_color;
    };

    class Button : public Gui {
    public:
        Button(
            GuiType gui_type = GuiType::Button,
            const std::string& new_name = "Button",
            ShapeSize size = ShapeSize{0.f},
            sf::Vector2f pos = sf::Vector2f(),
            sf::Color fillColor = sf::Color::White
        ) : Gui(gui_type, new_name, fillColor, size, pos) {}

        Timer GetTimer() { return this-> press_cooldown; }
    protected:
        Timer press_cooldown;
    };

    class TextButton : public Button, public TextLabel {
    public:
        TextButton(
            const std::string& new_name = "TextButton",
            const std::string& label = "Click me!",
            ShapeSize size = sf::Vector2f(200.f, 90.f),
            unsigned int charSize = 16,
            sf::Vector2f pos = sf::Vector2f(),
            sf::Color fillColor = sf::Color::White,
            sf::Color textColor = sf::Color::Black
        ) : Button(GuiType::TextButton, new_name, size, pos, fillColor),
        TextLabel(label, size, charSize, pos, textColor) {}
    };

    class TextBox : public Gui, public TextLabel {
    public:
        TextBox(
            const std::string& new_name = "TextBox",
            const std::string& label = "Text",
            ShapeSize size = sf::Vector2f(200.f, 90.f),
            unsigned int charSize = 16,
            sf::Vector2f pos = sf::Vector2f(),
            sf::Color fillColor = sf::Color::White,
            sf::Color textColor = sf::Color::Black
        ) : Gui(GuiType::TextBox, new_name, fillColor, size, pos),
        TextLabel( label, size, charSize, pos, textColor) {}
    };

    using GuiVector = std::vector<std::unique_ptr<obj::Gui>>;

    inline void NewGui(
        GuiVector& gui_objects,
        obj::GuiType gui_type = obj::GuiType::Frame,
        const std::string& name = "Gui",
        obj::ShapeSize size = sf::Vector2f{200.f,90.f},
        sf::Vector2f pos = sf::Vector2f{400.f, 300.f},
        sf::Color fillColor = sf::Color::White,
        obj::GuiAttachment label = "Text",
        unsigned int charSize = 16,
        sf::Color textColor = sf::Color::Black
    ) {
        auto str = std::get_if<std::string>(&label);
        auto cstr = std::get_if<const char*>(&label);
        if (!str && !cstr) { return; }

        std::string _label = str ? *str : *cstr;

        std::unique_ptr<Gui> gui;
        switch(gui_type) {
            case GuiType::TextButton:
                gui = std::make_unique<TextButton>(
                    name, _label, size, charSize, pos, fillColor, textColor
                );
                break;
            case GuiType::TextBox:
                gui = std::make_unique<TextBox>(
                    name, _label, size, charSize, pos, fillColor, textColor
                );
                break;
            default:
                return;
        }

        gui_objects.push_back(std::move(gui));
    }
}

#endif
