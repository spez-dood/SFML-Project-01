#ifndef gui_h
#define gui_h

#include <SFML/Graphics.hpp>
#include <optional>
#include <memory>
#include <variant>
#include <vector>
#include <functional>
#include <string>

namespace gui {
    using DrawPtr = std::unique_ptr<sf::Drawable>;
    using ShapeSize = std::variant<sf::Vector2f, float>;
    using GuiPtr = std::unique_ptr<class Gui>;
    using GuiAttachment = std::variant<std::string, const char*, sf::Image>;

    enum class GuiType { Frame, Button, TextButton, TextBox, TextInput, Image };

    inline sf::Shape* get_shape(const DrawPtr& shape) {
        return dynamic_cast<sf::Shape*>(shape.get());
    }

    inline DrawPtr NewShape(ShapeSize size, sf::Vector2f pos, sf::Color fillColor) {
        DrawPtr shape = std::get_if<sf::Vector2f>(&size)
        ? DrawPtr(new sf::RectangleShape(*std::get_if<sf::Vector2f>(&size)))
        : DrawPtr(new sf::CircleShape(*std::get_if<float>(&size)));

        get_shape(shape)->setPosition(pos.x, pos.y);
        get_shape(shape)->setFillColor(fillColor);
        return shape;
    }

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
        DrawPtr hitbox = nullptr;
        ShapeSize size = sf::Vector2f{200.f, 90.f};
        sf::Vector2f pos = sf::Vector2f{400.f, 300.f};
        sf::Color fillColor = sf::Color::White;
        sf::Color defaultColor = sf::Color::White;

        std::optional<GuiAttachment> attachment = std::nullopt;
        std::optional<unsigned int> charSize = std::nullopt;
        std::optional<sf::Color> textColor = std::nullopt;

        std::optional<std::string> GetLabelString() const {
            if (!attachment) return std::nullopt;
            if (auto ps = std::get_if<std::string>(&*attachment)) { return *ps; }
            if (auto pc = std::get_if<const char*>(&*attachment)) { return std::string(*pc); }
            return std::nullopt;
        }

        sf::Vector2f SizeVec() const {
            if (auto pv = std::get_if<sf::Vector2f>(&size)) return *pv;
            return sf::Vector2f{std::get<float>(size), std::get<float>(size)};
        }
    };

    class TextLabel {
    public:
        TextLabel(const GuiConfig& cfg) {
            this->Font.loadFromFile("resource/fonts/Arial.ttf");
            this->Text.setFont(this->Font);
            this->SetTextSize(cfg.charSize.value_or(16));
            this->SetTextColor(cfg.textColor.value_or(sf::Color::Black));
            this->SetText(cfg.GetLabelString().value_or(""));
            this->CenterText(cfg.SizeVec(), cfg.pos);
        }

        void SetTextSize(unsigned int charSize = 16) {
            this->Text.setCharacterSize(charSize);
        }

        void SetTextColor(sf::Color textColor = sf::Color::Black) {
            this->Text.setFillColor(textColor);
        }

        void SetText(const std::string& str = "Default!") {
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

        virtual void MouseEnter() { if (auto cb = onMouseEnter) cb(); }
        virtual void MouseExit() { if (auto cb = onMouseExit) cb(); }
        virtual void Hover() { if (auto cb = onHover) cb(); }

        GuiConfig& GetInfo() { return this->Info; }

        void SetName(const std::string& new_name) { this->Info.name = new_name; }
        
        bool MouseHover(sf::Vector2f point) const {
            return get_shape(this->Info.hitbox)->getGlobalBounds().contains(point);
        }

        sf::Color GetColor() {
            auto shape = get_shape(this->Info.hitbox);
            return shape ? shape->getFillColor() : sf::Color::Transparent;
        }

        void SetColor(sf::Color goal_color = sf::Color::White) {
            get_shape(this->Info.hitbox)->setFillColor(goal_color);
        }

        void Update(sf::RenderWindow& window) {
            sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
            bool hoveringNow = this->MouseHover(mousePos);

            if (!this->hover_exit_timer.active) {
                if (hoveringNow) { return; }

                this->SetColor(this->Info.fillColor);
                return;
            }

            if (hoveringNow) {
                this->hover_exit_timer.active = false;
                return;
            }
            auto elapsed = this->hover_exit_timer.clock.getElapsedTime().asMilliseconds();
            if (elapsed <= this->hover_exit_timer.duration_ms) { return; }

            this->hover_exit_timer.active = false;
            if (!this->is_hovered) { return; }
            this->is_hovered = false;
            this->MouseExit();
        }

        std::function<void()> onClick;
        std::function<void()> onRelease;
        std::function<void()> onClickHeld;
        std::function<void()> onMouseEnter;
        std::function<void()> onMouseExit;
        std::function<void()> onHover;

        void HandleMouseMove(sf::Vector2f point) {
            if (this->MouseHover(point)) {
                if (this->hover_exit_timer.active) { this->hover_exit_timer.active = false; }

                if (this->is_hovered) {
                    this->Hover();
                    return;
                }

                this->is_hovered = true;
                this->MouseEnter();
                return;
            }

            if (!this->is_hovered) { return; }

            if (!this->hover_exit_timer.active) {
                this->hover_exit_timer.active = true;
                this->hover_exit_timer.clock.restart();
                this->hover_exit_timer.duration_ms = this->hover_exit_debounce_ms;
                return;
            }

            auto elapsed = this->hover_exit_timer.clock.getElapsedTime().asMilliseconds();
            if (elapsed >= this->hover_exit_timer.duration_ms) {
                this->hover_exit_timer.active = false;
                this->is_hovered = false;
                this->MouseExit();
            }
        }

        void SetHoverExitDebounce(int duration_ms) {
            if (duration_ms < 0) duration_ms = 0;
            this->hover_exit_debounce_ms = duration_ms;
        }
    protected:
        Gui(GuiConfig cfg = GuiConfig()) : Info(std::move(cfg)){
            this->Info.defaultColor = this->Info.fillColor;
            this->SetColor(Info.fillColor);

            this->hover_exit_debounce_ms = 30;
            this->hover_exit_timer = Timer(0, false);
        }

        GuiConfig Info;
        bool is_hovered = false;

        Timer hover_exit_timer;
        int hover_exit_debounce_ms = 30;
    };

    class Button : public Gui {
    public:
        Button(GuiConfig cfg = GuiConfig()) : Gui(std::move(cfg)) {}

        void Clicked() { if (auto cb = this->onClick) cb(); }
        void Released() { if (auto cb = this->onRelease) cb(); }
        void Held() { if (auto cb = this->onClickHeld) cb(); }

        Timer GetTimer() { return this-> press_cooldown; }
    protected:
        Timer press_cooldown;
        bool is_clicked = false;
    };

    class TextButton : public TextLabel, public Button {
    public:
        TextButton(GuiConfig cfg = GuiConfig()) : TextLabel(cfg), Button(std::move(cfg)) {}
    };

    class TextBox : public TextLabel, public Gui {
    public:
        TextBox(GuiConfig cfg = GuiConfig()) : TextLabel(cfg), Gui(std::move(cfg)) {}
    };

    using GuiVector = std::vector<std::unique_ptr<Gui>>;

    inline Gui* NewGui(GuiVector& gui_objects, GuiConfig cfg = GuiConfig()) {
        cfg.defaultColor = cfg.fillColor;
        cfg.hitbox = NewShape(cfg.size, cfg.pos, cfg.fillColor);

        Gui* gui;

        switch(cfg.type) {
            case GuiType::TextButton:
                gui = new TextButton(std::move(cfg));
                break;
            case GuiType::TextBox:
                gui = new TextBox(std::move(cfg));
                break;
            default:
                return nullptr;
        }

        gui_objects.push_back(GuiPtr(gui));
        return gui_objects.back().get();
    }

    using PrepGui = std::function<sf::Vector2f(const GuiConfig&)>;
}

#endif
