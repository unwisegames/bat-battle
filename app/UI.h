#ifndef INCLUDED__Bounce__UI_h
#define INCLUDED__Bounce__UI_h

#include "atlas.sprites.h"

#include <bricabrac/Game/GameController.h>
#include <bricabrac/Game/GameActor.h>

#include <memory>

class Button : public std::enable_shared_from_this<Button> {
public:
    Button(brac::SpriteDef const (&s)[2], brac::vec2 p, float scale) : sprites_{s[0], s[1]}, pos_(p), scale_(scale) { }

    void draw(brac::mat4 pmv);
    bool contains(brac::vec2 v);
    void setY(float y) { pos_.y = y; };

    brac::Signal<void()> clicked;

    brac::TouchHandler handleTouch(brac::vec2 const & p);

    static brac::TouchHandler handleTouch(brac::vec2 const & p, std::initializer_list<std::shared_ptr<Button>> buttons);

private:
    brac::SpriteDef sprites_[2]; // default/pressed
    brac::vec2 pos_;
    float scale_;
    bool pressed_ = false;
};

inline std::shared_ptr<Button> makeButton(brac::SpriteDef const (&s)[2], brac::vec2 p, float scale = 1) {
    return std::make_shared<Button>(s, p, scale);
}

#endif // INCLUDED__Bounce__UI_h
