#include "UI.h"

using namespace brac;

#define BRICABRAC_SHADER_NAME Sprite
#include <bricabrac/Shader/LoadShaders.h>

void Button::draw(brac::mat4 pmv) {
    SpriteProgram::draw(sprites_[pressed], pmv * mat4::translate({pos_.x, pos_.y, 0}));
}

bool Button::contains(brac::vec2 v) {
    auto size = sprites_[0].size();
    return (v.x > pos_.x - size.x/2) && (v.x < pos_.x + size.x/2)
        && (v.y > pos_.y - size.y/2) && (v.y < pos_.y + size.y/2);
}

