#include "obj.h"

ShapePtr obj::NewShape(ShapeSize size, sf::Vector2f pos, sf::Color fillColor) {
    auto _shape = std::unique_ptr<sf::Shape>();
    if (std::holds_alternative<sf::Vector2f>(size)) {
        _shape = std::make_unique<sf::RectangleShape>(std::get<sf::Vector2f>(size));
    } else {
        _shape = std::make_unique<sf::CircleShape>(std::get<float>(size));
    }
    _shape->setPosition(pos.x, pos.y);
    _shape->setFillColor(fillColor);
    return _shape;
};

using namespace obj;

bool Gui::MouseHover(sf::Vector2f point) const {
    if (auto shape = dynamic_cast<sf::Shape*>(Hitbox.get())) {
        return shape->getGlobalBounds().contains(point);
    }
    return false;
}

void Gui::SetColor(sf::Color goal_color) {
    auto shape = dynamic_cast<sf::Shape*>(this->GetHitbox().get());
    if (!shape) { return; }
    shape->setFillColor(goal_color);
}

void Gui::SetColor(int cooldown_ms, sf::Color goal_color, sf::Color resetColor) {
    auto shape = dynamic_cast<sf::Shape*>(this->GetHitbox().get());
    if (!shape) { return; }
    shape->setFillColor(goal_color);
}

Button::Button(
    sf::RenderWindow& window,
    GuiType new_type,
    const char* new_name,
    ShapeSize size,
    sf::Vector2f pos,
    sf::Color fillColor
) : Gui(window) {
    ShapePtr new_hitbox = NewShape(size, pos, fillColor);

    this->SetType(new_type);
    this->SetDefaultColor(fillColor);

    auto* shape = dynamic_cast<sf::Shape*>(new_hitbox.get());
    if (shape) {
        shape->setFillColor(fillColor);
    }

    this->GetHitbox() = std::move(new_hitbox);
};

void Button::Cooldown(int duration_ms, sf::Color resetColor) {
    if (!this->WasPressed()) { return; }
    if (this->press_cooldown.clock.getElapsedTime().asMilliseconds() < this->press_cooldown.duration_ms) { return; }
    if (auto shape = dynamic_cast<sf::Shape*>(this->GetHitbox().get())) {
        shape->setFillColor(this->GetDefaultColor());
    }
    this->press_cooldown.active = false;
    this->press_cooldown.duration_ms = 0;
    this->press_cooldown.clock.restart();
}

TextButton::TextButton(
    sf::RenderWindow& window,
    const char* new_name,
    const std::string& label,
    ShapeSize size,
    sf::Vector2f pos,
    sf::Color boxColor
) : Button(window, GuiType::TextButton, new_name, size, pos, boxColor) {
    this->Font.loadFromFile("resource/fonts/Arial.ttf");
    this->SetText(label);
    this->Text.setFont(this->Font);
    this->Text.setFillColor(sf::Color::Black);
    this->Text.setCharacterSize(16);

    sf::FloatRect textBounds = this->Text.getLocalBounds();
    sf::Vector2f buttonSize = std::get<sf::Vector2f>(size);

    this->Text.setOrigin(textBounds.left, textBounds.top);
    this->Text.setPosition(
        pos.x + (buttonSize.x - textBounds.width) / 2.f,
        pos.y + (buttonSize.y - textBounds.height) / 2.f
    );
}

void TextButton::SetText(const std::string& str, unsigned int charSize, sf::Color textColor) {
    this->Text.setString(str);
    this->Text.setCharacterSize(charSize);
    this->Text.setFillColor(textColor);
}