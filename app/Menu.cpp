#include "Menu.h"
#include "atlas.sprites.h"
#include <bricabrac/Game/Bragging.h>
#include <bricabrac/Utility/UrlOpener.h>

using namespace brac;

#define BRICABRAC_SHADER_NAME Sprite
#include <bricabrac/Shader/LoadShaders.h>

Menu::Menu() {
    play->clicked += [=]{ pop(); };
}

bool Menu::onUpdate(float dt) { return true; }

void Menu::onDraw() {
    SpriteProgram::draw(atlas.title, pmv() * mat4::translate({0, 1.7, 0}) * mat4::scale(1.8));

    play      ->draw(pmv());
}

void Menu::onResize(brac::vec2 const & size) {
    adaptiveOrtho(-6, -6, 6, 6, 0, -INFINITY, 0, INFINITY);// , {100, INFINITY});
}

std::unique_ptr<TouchHandler> Menu::onTouch(vec2 const & worldPos, float) {
    if (auto handler = Button::handleTouch(worldPos, {play})) {
        return handler;
    }
    return TouchHandler::absorb();
}
