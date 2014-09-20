#include "GameOver.h"
#include "atlas.sprites.h"
#include "background.sprites.h"
#include "font.sprites.h"

using namespace brac;

#define BRICABRAC_SHADER_NAME Sprite
#include <bricabrac/Shader/LoadShaders.h>

GameOver::GameOver(GameMode m, size_t score, size_t best) : score_(score), best_(best) {
    restart ->clicked += [=]{ pop(); };
    back    ->clicked += [=]{ pop(); };
}

bool GameOver::onUpdate(float dt) { return true; }

void GameOver::onDraw() {
    auto & sprite_context = AutoSprite<SpriteProgram>::context();
    sprite_context->tint = {1, 1, 1, 1};

    SpriteProgram::draw(atlas.fade, pmv());

    auto drawText = [&](std::string const & text, vec2 pos, float scale, float spacing) {
        SpriteProgram::drawText(text, font.glyphs, 0, pmv() * mat4::translate({pos, 0}) * mat4::scale(scale), spacing);
    };

    SpriteProgram::draw(atlas.gameover, pmv() * mat4::scale(0.9) * mat4::translate({0, 0.5, 0}));
    SpriteProgram::draw(atlas.gameovertext, pmv() * mat4::scale(1) * mat4::translate({0, 2.5, 0}));

    sprite_context->tint = {1, 0.67, 0, 1};
    drawText("SCORE", {0, 1.05}, 0.4, -0.15);
    drawText("BEST", {0, -0.45}, 0.4, -0.15);
    sprite_context->tint = {1, 1, 1, 1};

    drawText(std::to_string(score_), {0, 0.35}, 0.4, -0.1);
    drawText(std::to_string(best_), {0, -1.14}, 0.4, -0.1);

    restart->draw(pmv());
    back->draw(pmv());
}

void GameOver::onResize(brac::vec2 const & size) {
    adaptiveOrtho(-6, -6, 6, 6, 0, -INFINITY, 0, INFINITY);//, {100, INFINITY});
}

std::unique_ptr<TouchHandler> GameOver::onTouch(vec2 const & worldPos, float radius) {
    if (auto handler = Button::handleTouch(worldPos, {restart, back})) {
        return handler;
    }
    return TouchHandler::absorb();
}
