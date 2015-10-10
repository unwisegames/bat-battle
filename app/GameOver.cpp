#include "GameOver.h"
#include "atlas.sprites.h"
#include "atlas2.sprites.h"
#include "background.sprites.h"
#include "font.sprites.h"
#include "character1.sprites.h"
#include "characters.sprites.h"

using namespace brac;

#define BRICABRAC_SHADER_NAME Sprite
#include <bricabrac/Shader/LoadShaders.h>

GameOver::GameOver(GameMode m, size_t score, size_t best, PlayerStats ps, std::vector<CharacterStats> cs)
: score_(score), best_(best), ps_(ps), cs_(cs) {
    back        ->clicked += [=]{ pop(); };
    restart     ->clicked += [=]{ pop(); };
    cont        ->clicked += [=]{ pop(); };

    //if (passed) {
        awardManOfMatch();
        formatTime();
    //}
}

void GameOver::awardManOfMatch() {
    auto i = 0;
    auto max = 0;
    for (auto & c : cs_) {
        if (c.score > max) {
            max = (int)c.score;
            mom_ = i;
        }
        ++i;
    }
}

void GameOver::formatTime() {
    int mins = int(ps_.time) / 60;
    int secs = int(ps_.time) % 60;

    char buf[6];
    snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
    formatted_time_ = buf;
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

    SpriteProgram::draw(atlas2.passed, pmv() * mat4::scale(0.8) * mat4::translate({0, 3.3, 0}));

    for (int i = 0; i < 2; i++) {
        SpriteProgram::draw(atlas.box, pmv() * mat4::translate({-2.3, 1.4, 0})      * mat4::scale({1.3, 0.8, 1}));
        SpriteProgram::draw(atlas.box, pmv() * mat4::translate({1.7, -0.23, 0})     * mat4::scale({1.8, 2.94, 1}));
        SpriteProgram::draw(atlas.box, pmv() * mat4::translate({-2.3, -0.93, 0})    * mat4::scale({1.3, 2, 1}));
    }

    drawHeaderText("SCORE", {-2.35, 1.45}, 0.3, -0.15);
    drawValueText(std::to_string(score_), {-2.35, 0.95}, 0.3, -0.1);

    auto s = [=](auto v) {
        return std::to_string(v);
    };

    drawHeaderText("STATS", {-2.35, -0}, 0.3, -0.15);
    //SpriteProgram::draw(character.mugshot,  pmv() * mat4::translate({-3.4, -0.37, 0})    * mat4::scale(0.5));
    SpriteProgram::draw(characters.dart,    pmv() * mat4::translate({-3.3, -0.4, 0})     * mat4::scale(0.7));
    SpriteProgram::draw(atlas.bathead,      pmv() * mat4::translate({-3.4, -0.96, 0})    * mat4::scale(0.35));
    SpriteProgram::draw(atlas2.target,      pmv() * mat4::translate({-3.4, -1.52, 0})    * mat4::scale(0.4));
    SpriteProgram::draw(atlas2.clock,       pmv() * mat4::translate({-3.4, -2.09, 0})    * mat4::scale(0.45));
    //drawValueText(s(ps_.remCharacters) + "/" + s(ps_.characters) + " SURVIVED",     {-3, -0.48}, 0.2, -0.15, -1);
    drawValueText(s(ps_.darts) + (ps_.darts == 1 ? " DART" : " DARTS") + " FIRED",  {-3, -0.51}, 0.2, -0.15, -1);
    drawValueText(s(ps_.kills) + (ps_.kills == 1 ? " BAT" : " BATS") + " KILLED",   {-3, -1.07}, 0.2, -0.15, -1);
    drawValueText(s(ps_.darts > 0 ? int(float(ps_.hits) / float(ps_.darts) * 100) : 0) + "% ACCURACY", {-3, -1.63}, 0.2, -0.15, -1);
    drawValueText(formatted_time_, {-3, -2.2}, 0.2, -0.15, -1);

    drawHeaderText("CONTINUE", {1.65, 1.45}, 0.3, -0.15);
    /*SpriteProgram::draw(cs_[mom_].mugshot,  pmv() * mat4::scale(2) * mat4::translate({0.85, 0.15, 0}));
    SpriteProgram::draw(characters.dart,    pmv() * mat4::translate({-0.1, -1.5, 0})    * mat4::scale(0.5));
    SpriteProgram::draw(atlas.bathead,      pmv() * mat4::translate({-0.2, -1.95, 0})   * mat4::scale(0.25));
    SpriteProgram::draw(character1.mugshot, pmv() * mat4::translate({2, -1.5, 0})       * mat4::scale(0.4));
    SpriteProgram::draw(atlas2.target,      pmv() * mat4::translate({2, -1.95, 0})      * mat4::scale(0.3));
    drawValueText(s(cs_[mom_].score) + " POINTS",                                                         {1.6, -1},     0.25, -0.15, 0);
    drawValueText(s(cs_[mom_].dartsFired) + (cs_[mom_].dartsFired == 1 ? " DART" : " DARTS") + " FIRED",    {0, -1.6},      0.16, -0.15, -1);
    drawValueText(s(cs_[mom_].birdsKilled) + (cs_[mom_].birdsKilled == 1 ? " BAT" : " BATS") + " KILLED",   {0, -2.05},     0.16, -0.15, -1);
    drawValueText(s(cs_[mom_].dartsFired > 0 ? int(float(cs_[mom_].dartsHit) / float(cs_[mom_].dartsFired) * 100) : 0) + "% ACCURACY",  {2.2, -2.05},   0.16, -0.15, -1);
    drawValueText(s(cs_[mom_].rescues) + " RESCUED",                                                        {2.2, -1.6},    0.16, -0.15, -1);*/

    back->draw(pmv());
    restart->draw(pmv());
    cont->draw(pmv());
}

void GameOver::onResize(brac::vec2 const & size) {
    adaptiveOrtho(-6, -6, 6, 6, 0, -INFINITY, 0, INFINITY);//, {100, INFINITY});
}

TouchHandler GameOver::onTouch(vec2 const & worldPos, float radius) {
    return Button::handleTouch(worldPos, {back, restart, cont});
}
