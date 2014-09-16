#include "Controller.h"
#include "background.sprites.h"
#include "Menu.h"

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

    // Persistent data
    Persistent<int> careerArcPoints{"careerArcPoints"};
    Persistent<int> careerBuzPoints{"careerBuzPoints"};
    Persistent<int> bestArcScore{"bestArcScore"};
    Persistent<int> bestBuzScore{"bestBuzScore"};
    Persistent<int> arcGamesPlayed{"arcGamesPlayed"};
    Persistent<int> buzGamesPlayed{"buzGamesPlayed"};
};

Controller::Controller() : m{new Controller::Members{}} {
    newGame(MODE);
}

Controller::~Controller() { }

void Controller::newGame(GameMode mode) {
    m->newGame = false;
    m->game = std::make_shared<Game>(spaceTime(), mode);

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
        auto mode = state.mode;

        if (mode == m_arcade) {
            m->arcGamesPlayed = ++*m->arcGamesPlayed;
        } else if (mode == m_buzzer) {
            m->buzGamesPlayed = ++*m->buzGamesPlayed;
        }
        
        size_t score = state.score;
        if (score > 0) {
            switch (state.mode) {
                case m_arcade:
                    if (score > *m->bestArcScore) {
                        m->bestArcScore = static_cast<int>(score);
                    }
                    m->careerArcPoints = *m->careerArcPoints + static_cast<int>(score);
                    break;
                case m_buzzer:
                    if (score > *m->bestBuzScore) {
                        m->bestBuzScore = static_cast<int>(score);
                    }
                    m->careerBuzPoints = *m->careerBuzPoints + static_cast<int>(score);
                    break;
                case m_menu: break;
                case m_play: break;
            }
            
            if (score >= 25) {
                brag::score25(100, []{});
                if (score >= 100) {
                }
            }
            
            size_t cp = *m->careerArcPoints + *m->careerBuzPoints;
            brag::career = cp;
        }
    };

    m->game->show_menu += [=] {
        auto menu = emplaceController<Menu>();

        menu->play->clicked += newGame(m_play);
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
    auto & sprite_context = AutoSprite<SpriteProgram>::context();
    sprite_context->tint = {1, 1, 1, 1};

    SpriteProgram::draw(background.bg, pmv() * mat4::translate({0, 8.5, 0}));

    
    SpriteProgram::draw(m->game->actors<Bird>       (), pmv());

}

void Controller::onResize(brac::vec2 const & size) {
    //float halfH = 0.5 * background.bg.size().y;
    adaptiveOrtho(-10, -10, 10, 10, 0, 0, 6, INFINITY);
}

std::unique_ptr<TouchHandler> Controller::onTouch(vec2 const & worldPos, float radius) {
    return m->game->fingerTouch(worldPos, radius);
}
