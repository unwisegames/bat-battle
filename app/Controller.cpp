#include "Controller.h"
#include "background.sprites.h"
#include "font.sprites.h"
#include "characters.sprites.h"
#include "atlas.sprites.h"
#include "Menu.h"
#include "GameOver.h"
#include "sounds.h"

#include "brag.h"

#include <bricabrac/Data/Persistent.h>
#include <bricabrac/Utility/UrlOpener.h>
#include <bricabrac/Math/Random.h>

#include <iostream>

using namespace brac;

#define BRICABRAC_SHADER_NAME Sprite
#include <bricabrac/Shader/LoadShaders.h>
#define BRICABRAC_SHADER_NAME NormalShade
#include <bricabrac/Shader/LoadShaders.h>

struct Controller::Members {
    std::shared_ptr<Game> game;
    float angle = 0;
    bool newGame = false;
    GameMode mode = m_menu;
    int level = 0;
    float top = 0;
    sounds audio{0.5, 1};
    std::unique_ptr<NormalShadeProgram> shadeProgram;
    std::vector<NormalShadeVertex> shadeBuffer;

    // Persistent data
    Persistent<int> highestCompletedLevel{"highestCompletedLevel"};
    Persistent<int> totalPoints{"totalPoints"};
    Persistent<int> bestScore{"bestScore"};
    Persistent<int> arcGamesPlayed{"arcGamesPlayed"};
    Persistent<int> buzGamesPlayed{"buzGamesPlayed"};
};

Controller::Controller() : m{new Controller::Members{}} {
    m->shadeProgram.reset(new NormalShadeProgram);

    newGame(MODE, 1);
}

Controller::~Controller() { }

void Controller::newGame(GameMode mode, int level) {
    m->newGame = false;
    m->game = std::make_shared<Game>(spaceTime(), mode, level, m->top);

    auto const & state = m->game->state();

    m->audio.ambience->setLoopCount(-1);
    mode == m_play ? m->audio.ambience->play() : m->audio.ambience->stop();
    m->audio.tension->stop();
    m->audio.fail->stop();

    auto & a = m->audio;
    brac::AudioSystem::SoundPool * yays[] = {&a.yay1, &a.yay2, &a.yay3, &a.yay4, &a.yay5, &a.yay6, &a.yay7, &a.yay8, &a.yay9};

    auto stopYays = [=]() {
        // stop celebration sounds
        for (auto y : yays) {
            y->stop();
        }
    };
    stopYays();

    auto click = [=] { m->audio.click.play(); };

    auto newGame = [=](GameMode mode, int level = -1) {
        return [=]{
            click();
            m->mode = mode;
            m->level = level;
            m->newGame = true;
        };
    };

    // TODO: Announce achievements.

    m->game->aim        += [=] { m->audio.aim       .play(); };
    m->game->shoot      += [=] { m->audio.shoot     .play(); };
    m->game->shot       += [=] { m->audio.shot      .play(); };
    m->game->die        += [=] { m->audio.ooh       .play(); };
    m->game->aah        += [=] { m->audio.aah       .play(); };
    m->game->char_score += [=] { m->audio.score     .play(); };
    m->game->pop        += [=] { m->audio.pop       .play(); };
    m->game->alert      += [=] { m->audio.alert     .play(); };
    m->game->fall       += [=] { m->audio.fall      .play(); };
    m->game->lose       += [=] { m->audio.fail2     .play(); };
    m->game->dundundun  += [=] { m->audio.dundundun .play(); };
    m->game->beep       += [=] { m->audio.beep      .play(); };
    m->game->tick       += [=] { m->audio.tick      .play(); };
    m->game->charblast  += [=] { m->audio.charblast .play(); };
    m->game->boom       += [=] { m->audio.boom      .play(); };
    m->game->reloading  += [=] { m->audio.reloading .play(); };

    m->game->bombwhistle_start += [=] { m->audio.bombwhistle.play(); };
    m->game->bombwhistle_stop  += [=] { m->audio.bombwhistle.stop(); };

    m->game->yay += [=] {
        auto p = randomChoice(yays);
        p->play();
    };

    m->game->pumped += [=] {
        auto p = randomChoice({&m->audio.comeon, &m->audio.yay7, &m->audio.aha});
        p->play();
    };

    m->game->tension_start += [=] {
        m->audio.tension->setLoopCount(-1);
        m->audio.tension->play();
    };

    m->game->tension_stop += [=] {
        m->audio.tension->stop();
    };

    m->game->failed += [=] {
        m->audio.fail->setLoopCount(0);
        m->audio.fail->play();
    };

    m->game->help += [=] {
        m->audio.help.play();
    };

    m->game->n_for_n += [=](size_t n){
        switch (n) {
        }
    };

    m->game->sharpshot += [=]{
    };
    
    m->game->ended += [=] {
        auto const & state = m->game->state();

        m->audio.ambience->stop();

        stopYays();

        if (state.level_passed && state.level > *m->highestCompletedLevel) {
            m->highestCompletedLevel = state.level;
        }

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

        auto gameOver = emplaceController<GameOver>(mode, m->level, state.level_passed, score, *m->bestScore, state.playerStats, state.characterStats);
        gameOver->back      ->clicked += newGame(m_menu);
        gameOver->restart   ->clicked += newGame(m->mode, m->level);
        gameOver->next      ->clicked += newGame(m->mode, m->level + 1);
        gameOver->backf     ->clicked += newGame(m_menu);
        gameOver->restartf  ->clicked += newGame(m->mode, m->level);
    };

    m->game->show_menu += [=] {
        auto menu = emplaceController<Menu>();

        menu->play->clicked += newGame(m_play, *m->highestCompletedLevel + 1);

        menu->gamecenter->clicked += [=]{
            click();
            presentBragUI();
        };
        menu->twitter->clicked += [=]{
            click();
            UrlOpener::open("http://www.twitter.com/UnwiseGames");
        };
    };

    state.back->clicked += [=] {
        click();
        m->newGame = true;
        m->mode = m_menu;
    };

    state.restart->clicked += [=] {
        click();
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
        newGame(m->mode, m->level);
        return false;
    }
    m->angle += dt;
    return true;
}

void Controller::onDraw() {
    //auto const & state = m->game->state();

    auto sprite_context = AutoSprite<SpriteProgram>::context();
    sprite_context->tint = {1, 1, 1, 1};

    SpriteProgram::draw(background.bg, pmv() * mat4::translate({0, 9.1, 0}));

    /*for (auto const & b : m->game->actors<Bird>()) {
        if (b.isFlying()) {
            SpriteProgram::drawActor(b, pmv(), b.vel().x < 0 ? mat4::scale(vec3{-1, 1, 1}) : mat4::identity());
        } else {
            SpriteProgram::drawActor(b, pmv());
        }
    }*/

    for (auto const & c : m->game->actors<Character>()) {
        if (c.isAiming()) {
            float dt = 0.01;
            vec2 p = c.pos() - vec2::polar(LAUNCH_OFFSET, c.angle() + M_PI_2);
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

    SpriteProgram::draw(m->game->actors<Bomb>       (), pmv());
    /*for (auto const & b : m->game->actors<Bomb>()) {
        if (b.countdown > 0) {
            SpriteProgram::drawText(std::to_string(b.countdown), font.glyphs, 0, pmv() * mat4::translate({b.pos().x, b.pos().y - float(0.2), 0}) * mat4::scale(0.25));
        }
    }
    SpriteProgram::draw(m->game->actors<BombBat>                (), pmv());
    SpriteProgram::draw(m->game->actors<Blast>                  (), pmv());
    SpriteProgram::draw(m->game->actors<CharacterExplosion>     (), pmv());
    SpriteProgram::draw(m->game->actors<Grave>                  (), pmv());
    SpriteProgram::draw(m->game->actors<Character>              (), pmv());
    SpriteProgram::draw(m->game->actors<Dart>                   (), pmv());

    for (auto const & d : m->game->actors<Dart>()) {
        if (d.active) {
            SpriteProgram::drawText(std::to_string(d.score), font.glyphs, 0, pmv() * mat4::translate({d.pos().x, d.pos().y + float(0.6), 0}) * mat4::scale(0.3), -0.1);
        }
    }

    if (state.mode == m_play) {
        SpriteProgram::drawText("SCORE :  " + std::to_string(state.score), font.glyphs, 1,
                                pmv() * mat4::translate({9, m->top-1, 0}) * mat4::scale(0.5), -0.1);

        state.back->draw(pmv());
        state.restart->draw(pmv());

        if (state.params.yellow_bats == 0) {
            SpriteProgram::drawText(std::to_string(state.rem_grey_bats), font.glyphs, -1, pmv() * mat4::translate({0    , m->top-1, 0}) * mat4::scale(0.5));
            SpriteProgram::draw(atlas.bathead, pmv() * mat4::translate({-0.6, m->top - float(0.7), 0}) * mat4::scale(0.8));
        } else {
            SpriteProgram::drawText(std::to_string(state.rem_grey_bats), font.glyphs, -1, pmv() * mat4::translate({-2, m->top-1, 0}) * mat4::scale(0.5));
            SpriteProgram::draw(atlas.bathead, pmv() * mat4::translate({-2.6, m->top - float(0.7), 0}) * mat4::scale(0.8));

            SpriteProgram::drawText(std::to_string(state.rem_yellow_bats), font.glyphs, -1, pmv() * mat4::translate({1.2, m->top-1, 0}) * mat4::scale(0.5));
            SpriteProgram::draw(atlas.yellowbathead, pmv() * mat4::translate({0.6, m->top - float(0.7), 0}) * mat4::scale(0.8));
        }

        if (!m->game->state().started) {
            SpriteProgram::drawText("LEVEL " + std::to_string(m->level), font.glyphs, 0, pmv() * mat4::translate({0, m->top - 5, 0}), -0.1);
        }

        if (m->game->state().show_char_score) {
            for (auto const & c : m->game->actors<Character>()) {
                if (!c.isDead()) {
                    SpriteProgram::drawText(std::to_string(SCORE_CHAR_SURVIVED), font.glyphs, 0, pmv() * mat4::translate({c.pos().x, c.pos().y + float(0.8), 0}) * mat4::scale(0.3), -0.1);
                }
            }
        }

        SpriteProgram::drawText(state.text_alert.s, font.glyphs, 0, pmv() * mat4::translate({state.text_alert.pos.x, state.text_alert.pos.y, 0}) * mat4::scale(0.3));
    }*/

    vec4 color = {0, 0, 1, 1};
    m->shadeBuffer.clear();
    m->shadeBuffer.emplace_back(vec2{0, 3}, vec2{1, 0});
    m->shadeBuffer.emplace_back(vec2{1, 3}, vec2{1, 0});
    m->shadeBuffer.emplace_back(vec2{1, 1}, vec2{1, 0});
    m->shadeBuffer.emplace_back(vec2{0, 1}, vec2{1, 0});
    m->shadeBuffer.emplace_back(vec2{0, 3}, vec2{1, 0});

    auto ctx = (*m->shadeProgram)();
    ctx->tint = {1, 244/255.0f, 240/255.0f};
    //ctx->texture = vec4{1, 1, 1, 0.5};
    ctx->pmv = pmv();// projection() * modelview();

    std::cerr << m->shadeBuffer.size() << "\n";

    ctx.vs.enableArray(m->shadeBuffer.data());
    glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(m->shadeBuffer.size()));
    glLineWidth(4);
    

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
