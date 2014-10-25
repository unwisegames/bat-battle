#include "GameOver.h"
#include "atlas.sprites.h"
#include "atlas2.sprites.h"
#include "background.sprites.h"
#include "font.sprites.h"
#include "character.sprites.h"
#include "characters.sprites.h"

using namespace brac;

#define BRICABRAC_SHADER_NAME Sprite
#include <bricabrac/Shader/LoadShaders.h>

GameOver::GameOver(GameMode m, int level, bool passed, size_t score, size_t best, PlayerStats ps, std::vector<CharacterStats> cs)
                    : score_(score), best_(best), passed_(passed), level_(level) {
    back    ->clicked += [=]{ pop(); };
    restart ->clicked += [=]{ pop(); };
    next    ->clicked += [=]{ pop(); };
}

bool GameOver::onUpdate(float dt) { return true; }

void GameOver::onDraw() {
    auto sprite_context = AutoSprite<SpriteProgram>::context();
    sprite_context->tint = {1, 1, 1, 1};

    SpriteProgram::draw(atlas.fade, pmv());

    auto drawHeaderText = [&](std::string const & text, vec2 pos, float scale, float spacing) {
        sprite_context->tint = {1, 0.67, 0, 1};
        SpriteProgram::drawText(text, font.glyphs, 0, pmv() * mat4::translate({pos, 0}) * mat4::scale(scale), spacing);
        sprite_context->tint = {1, 1, 1, 1};
    };
    auto drawValueText = [&](std::string const & text, vec2 pos, float scale, float spacing, float align = 0) {
        SpriteProgram::drawText(text, font.glyphs, align, pmv() * mat4::translate({pos, 0}) * mat4::scale(scale), spacing);
    };

    if (passed_) {
        SpriteProgram::draw(atlas2.passed, pmv() * mat4::scale(0.8) * mat4::translate({0, 3.3, 0}));

        SpriteProgram::draw(atlas.box, pmv() * mat4::translate({-2.3, 1.4, 0}) * mat4::scale({1.3, 0.8, 1}));
        SpriteProgram::draw(atlas.box, pmv() * mat4::translate({1.7, -0.23, 0}) * mat4::scale({1.8, 2.94, 1}));

        SpriteProgram::draw(atlas.box, pmv() * mat4::translate({-2.3, -0.93, 0}) * mat4::scale({1.3, 2, 1}));

        drawHeaderText("SCORE", {-2.35, 1.45}, 0.3, -0.15);
        drawValueText(std::to_string(score_), {-2.35, 0.95}, 0.3, -0.1);

        drawHeaderText("STATS", {-2.35, -0}, 0.3, -0.15);
        SpriteProgram::draw(character.mugshot, pmv() * mat4::scale(0.5) * mat4::translate({-6.9, -0.8, 0}));
        drawValueText("4 5 SAVED", {-3, -0.48}, 0.2, -0.15, -1);
        SpriteProgram::draw(characters.dart, pmv() * mat4::scale(0.7) * mat4::translate({-4.8, -1.15, 0}));
        drawValueText("3 DARTS FIRED", {-3, -0.91}, 0.2, -0.15, -1);
        drawValueText("4 BIRDS KILLED", {-3, -1.34}, 0.2, -0.15, -1);
        drawValueText("80 ACCURACY ", {-3, -1.77}, 0.2, -0.15, -1);
        drawValueText("3:55", {-3, -2.2}, 0.2, -0.15, -1);

        drawHeaderText("MAN OF THE MATCH", {1.7, 1.45}, 0.3, -0.15);
        SpriteProgram::draw(character.mugshot, pmv() * mat4::scale(2) * mat4::translate({0.8, 0.15, 0}));
        drawValueText("3 DARTS FIRED", {0, -1}, 0.16, -0.15, -1);
        drawValueText("3 BIRDS KILLED", {0, -1.45}, 0.16, -0.15, -1);
        drawValueText("100 ACCURACY", {0, -1.9}, 0.16, -0.15, -1);
        drawValueText("2 RESCUED", {2.2, -1}, 0.16, -0.15, -1);
        drawValueText("300 POINTS", {2.2, -1.45}, 0.16, -0.15, -1);
        //drawValueText("CAPTURED 1 TIME", {2.2, -1.9}, 0.16, -0.15, -1);



/*        drawText("BEST", {0, -0.45}, 0.4, -0.15);

        drawText(std::to_string(score_), {0, 0.35}, 0.4, -0.1);
        drawText(std::to_string(best_), {0, -1.14}, 0.4, -0.1);
*/
        back->draw(pmv());
        //restart->draw(pmv());
        next->draw(pmv());
    } else {
        SpriteProgram::draw(atlas2.failed, pmv() * mat4::scale(1) * mat4::translate({0, 2.5, 0}));

        restart->draw(pmv());
        back->draw(pmv());
    }
}

void GameOver::onResize(brac::vec2 const & size) {
    adaptiveOrtho(-6, -6, 6, 6, 0, -INFINITY, 0, INFINITY);//, {100, INFINITY});
}

std::unique_ptr<TouchHandler> GameOver::onTouch(vec2 const & worldPos, float radius) {
    if (auto handler = Button::handleTouch(worldPos, {back, restart, next})) {
        return handler;
    }
    return TouchHandler::absorb();
}
