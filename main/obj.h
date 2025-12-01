#ifndef obj_h
#define obj_h

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <variant>
#include <optional>

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

    struct GuiConfig {
        GuiType type = GuiType::Frame;
        std::string name = "Gui";
        ShapeSize size = sf::Vector2f{200.f, 90.f};
        sf::Vector2f pos = sf::Vector2f{400.f, 300.f};
        sf::Color fillColor = sf::Color::White;

        std::optional<GuiAttachment> attachment = std::nullopt;

        std::optional<unsigned int> charSize = std::nullopt;
        std::optional<sf::Color> textColor = std::nullopt;

        std::optional<std::string> GetLabelString() const {
            if (!attachment) return std::nullopt;
            if (auto ps = std::get_if<std::string>(&*attachment)) return *ps;
            if (auto pc = std::get_if<const char*>(&*attachment)) return std::string(*pc);
            return std::nullopt;
        }

        void SetSize(const sf::Vector2f& v) { size = v; }
        void SetSize(float radius) { size = radius; }

        sf::Vector2f SizeVec() const {
            if (auto pv = std::get_if<sf::Vector2f>(&size)) return *pv;
            return sf::Vector2f{std::get<float>(size), std::get<float>(size)};
        }
    };

    class TextLabel {
    public:
        TextLabel(GuiConfig cfg = GuiConfig{.attachment = "I'm a label!"}) {
            this->Font.loadFromFile("resource/fonts/Arial.ttf");
            this->Text.setFont(this->Font);
            this->SetText(cfg.GetLabelString().value_or(""), cfg.charSize.value_or(16), cfg.textColor.value_or(sf::Color::Black));
            this->CenterText(std::get<sf::Vector2f>(cfg.size), cfg.pos);
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
            if (this->MouseHover(static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)))) { return; }
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
        Gui(GuiConfig cfg = GuiConfig()) : 
        Hitbox(std::move(NewShape(cfg.size, cfg.pos, cfg.fillColor))),
        Name(cfg.name),
        Type(cfg.type),
        default_color(cfg.fillColor) {
            this->SetColor(cfg.fillColor);
        }

        sf::Shape* Shape() const { return dynamic_cast<sf::Shape*>(this->Hitbox.get()); }
        ShapePtr Hitbox;
        std::string Name;
        GuiType Type;
        sf::Color default_color;
    };

    class Button : public Gui {
    public:
        Button(GuiConfig cfg = GuiConfig()) : Gui(cfg) {}

        Timer GetTimer() { return this-> press_cooldown; }
    protected:
        Timer press_cooldown;
    };

    class TextButton : public Button, public TextLabel {
    public:
        TextButton(GuiConfig cfg = GuiConfig{.attachment = "TextButton"}) : Button(cfg), TextLabel(cfg) {}
    };

    class TextBox : public Gui, public TextLabel {
    public:
        TextBox(GuiConfig cfg = GuiConfig{.attachment = "TextBox"}) : Gui(cfg), TextLabel(cfg) {}
    };

    using GuiVector = std::vector<std::unique_ptr<obj::Gui>>;

    inline void NewGui(GuiVector& gui_objects, GuiConfig cfg = GuiConfig()) {
        std::unique_ptr<Gui> gui;
        
        switch(cfg.type) {
            case GuiType::TextButton:
                gui = std::make_unique<TextButton>(cfg);
                break;
            case GuiType::TextBox:
                gui = std::make_unique<TextBox>(cfg);
                break;
            default:
                return;
        }

        gui_objects.push_back(std::move(gui));
    }
}

#endif
