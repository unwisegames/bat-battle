#include "Controller.h"
#include "background.sprites.h"
#include "font.sprites.h"
#include "characters.sprites.h"
#include "Menu.h"
#include "GameOver.h"

#include "brag.h"

#include <bricabrac/Data/Persistent.h>

#include <iostream>

using namespace brac;

#define BRICABRAC_SHADER_NAME Sprite
#include <bricabrac/Shader/LoadShaders.h>

struct Controller::Members {
    std::shared_ptr<Game> game;
    float angle = 0;
    bool newGame = false;
    GameMode mode = m_menu;
    float top = 0;

    // Persistent data
    Persistent<int> totalPoints{"totalPoints"};
    Persistent<int> bestScore{"bestScore"};
    Persistent<int> arcGamesPlayed{"arcGamesPlayed"};
    Persistent<int> buzGamesPlayed{"buzGamesPlayed"};
};

Controller::Controller() : m{new Controller::Members{}} {
    newGame(MODE);
}

Controller::~Controller() { }

void Controller::newGame(GameMode mode) {
    m->newGame = false;
    m->game = std::make_shared<Game>(spaceTime(), mode, m->top);

    auto const & state = m->game->state();

    auto newGame = [=](GameMode mode) {
        return [=]{
            m->mode = mode;
            m->newGame = true;
        };
    };

    // TODO: Announce achievements.

    m->game->bounced += [=](Character const & character, vec2 const & impulse) {
        //if(brac::length_sq(impulse) > 400) {
        //}
    };
    
    m->game->door_open += [=] {
    };
    
    m->game->release_ball += [=] {
    };
    
    m->game->clock_beep += [=] {
    };
    
    m->game->bounced_wall += [=] {
    };
    
    m->game->touched_sides += [=] {
    };
    
    m->game->foul += [=] {
    };
    
    m->game->scored += [=]() {
        /*size_t score = m->game->state().score;
        if (score <= 25) {
            brag::score25(4 * score, []{});
        }
        if (score <= 100) {
            brag::score100(score, []{});
        }*/
    };

    m->game->n_for_n += [=](size_t n){
        switch (n) {
        }
    };

    m->game->sharpshot += [=]{
    };
    
    m->game->ended += [=] {
        auto const & state = m->game->state();

        size_t score = state.score;
        if (score > 0) {
            if (score > *m->bestScore) {
                m->bestScore = static_cast<int>(score);
            }
            m->totalPoints = *m->totalPoints + static_cast<int>(score);

            if (score >= 25) {
                brag::score25(100, []{});
                if (score >= 100) {
                }
            }
            
            size_t tp = *m->totalPoints + *m->totalPoints;
            brag::total = tp;
        }

        auto gameOver = emplaceController<GameOver>(mode, score, *m->bestScore);
        gameOver->restart   ->clicked += newGame(m->mode);
        gameOver->back      ->clicked += newGame(m_menu);
    };

    m->game->show_menu += [=] {
        auto menu = emplaceController<Menu>();

        menu->play->clicked += newGame(m_play);
    };

    state.back->clicked += [=] {
        m->newGame = true;
        m->mode = m_menu;
    };

    state.restart->clicked += [=] {
        m->game->end();
//        m->newGame = true;
//        m->mode = m_play;
    };

}

bool Controller::onUpdate(float dt) {
    if (!m->game->update(dt)) {
        //newGame();
    }
    if (m->newGame) {
        newGame(m->mode);
        return false;
    }
    m->angle += dt;
    return true;
}

void Controller::onDraw() {
    auto const & state = m->game->state();

    auto & sprite_context = AutoSprite<SpriteProgram>::context();
    sprite_context->tint = {1, 1, 1, 1};

    SpriteProgram::draw(background.bg, pmv() * mat4::translate({0, 9.1, 0}));

    SpriteProgram::draw(m->game->actors<Bird>       (), pmv());

    for (auto const & c : m->game->actors<Character>()) {
        if (c.isAiming()) {
            float dt = 0.01;
            vec2 p = c.pos() - vec2::polar(LAUNCH_OFFSET, c.angle());
            vec2 v = c.launchVel();
            vec2 g_dt = vec2{0, WORLD_GRAVITY} * dt;
            for (int i = 0; p.y > 0; ++i) {
                if (i % 4 == 0) {
                    SpriteProgram::draw(characters.dot, pmv() * mat4::translate({p, 0}));
                }
                p += v * dt;
                v += g_dt;
            }
        }
    }

    SpriteProgram::draw(m->game->actors<Character>  (), pmv());
    SpriteProgram::draw(m->game->actors<Dart>       (), pmv());

    if (state.mode == m_play) {
        SpriteProgram::drawText("SCORE :  " + std::to_string(state.score), font.glyphs, 1,
                                pmv() * mat4::translate({9, m->top-1, 0}) * mat4::scale(0.5), -0.1);

        state.back->draw(pmv());
        state.restart->draw(pmv());
    }
}

void Controller::onResize(brac::vec2 const & size) {
    //float halfH = 0.5 * background.bg.size().y;
    /*GameController::adaptiveOrtho(float l_inner, float l_outer,
                                  float r_inner, float r_outer,
                                  float b_inner, float b_outer,
                                  float t_inner, float t_outer,
                                  vec2 maxSizeMm)*/
    adaptiveOrtho(-10, -10, 10, 10, 0, 0, 6, INFINITY);
    auto top = inv_pmv() * vec3{0, 1, 0};
    m->top = top.y;
}

std::unique_ptr<TouchHandler> Controller::onTouch(vec2 const & worldPos, float radius) {
    return m->game->fingerTouch(worldPos, radius);
}
