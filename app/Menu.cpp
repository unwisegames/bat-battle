#include "Menu.h"
#include "atlas.sprites.h"
#include "atlas2.sprites.h"
#include "bats.sprites.h"
#include "blast.sprites.h"
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
    SpriteProgram::draw(blast.blackbat, pmv() * mat4::translate({0, 1.7, 0}) * mat4::scale(1));
    SpriteProgram::draw(atlas2.title, pmv() * mat4::translate({0, 1.2, 0}) * mat4::scale(0.9));
    //SpriteProgram::draw(atlas2.title, pmv() * mat4::translate({0, 1.7, 0}) * mat4::scale(1));
    //SpriteProgram::draw(bats.wings, pmv() * mat4::translate({0, 0, 0}) * mat4::scale(1));

    play        ->draw(pmv());
    gamecenter  ->draw(pmv());
    twitter     ->draw(pmv());
}

void Menu::onResize(brac::vec2 const & size) {
    adaptiveOrtho(-6, -6, 6, 6, 0, -INFINITY, 0, INFINITY);// , {100, INFINITY});
}

TouchHandler Menu::onTouch(vec2 const & worldPos, float) {
    if (auto handler = Button::handleTouch(worldPos, {play, gamecenter, twitter})) {
        return handler;
    }
    return {};
}
