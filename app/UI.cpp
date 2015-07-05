#include "UI.h"

using namespace brac;

#define BRICABRAC_SHADER_NAME Sprite
#include <bricabrac/Shader/LoadShaders.h>

void Button::draw(brac::mat4 pmv) {
    SpriteProgram::draw(sprites_[pressed_], pmv * mat4::translate({pos_.x, pos_.y, 0}) * mat4::scale(scale_));
}

bool Button::contains(brac::vec2 v) {
    auto size = sprites_[0].size();
    return (v.x > pos_.x - (size.x * scale_)/2) && (v.x < pos_.x + (size.x * scale_)/2)
        && (v.y > pos_.y - (size.y * scale_)/2) && (v.y < pos_.y + (size.y * scale_)/2);
}

brac::TouchHandler Button::handleTouch(brac::vec2 const & p) {
    if (!contains(p)) {
        return {};
    }

    pressed_ = true;

    return spawn_touchHandler([weak_btn = std::weak_ptr<Button>{shared_from_this()}](auto moved, auto cancelled) {
        for (TouchMove m; moved >> m;) {
            if (auto btn = weak_btn.lock()) {
                btn->pressed_ = btn->contains(m.pos);
            }
        }
        if (!(cancelled >> poke)) {
            if (auto btn = weak_btn.lock()) {
                if (btn->pressed_) {
                    btn->pressed_ = false;
                    btn->clicked();
                }
            }
        }
    });
}

brac::TouchHandler Button::handleTouch(brac::vec2 const & p, std::initializer_list<std::shared_ptr<Button>> buttons) {
    for (auto const & button : buttons) {
        if (auto handler = button->handleTouch(p)) {
            return handler;
        }
    }
    return {};
}
