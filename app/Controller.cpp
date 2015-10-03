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
#include <bricabrac/Shader/ShaderUtil.h>

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
    float top = 0;
    sounds audio{0.5, 1};
    bool paused = false;
    std::unique_ptr<NormalShadeProgram> shadeProgram;

    std::vector<NormalShadeVertex> flamebuf;
    std::unique_ptr<Sprite> flame;

    // Persistent data
    Persistent<int> totalPoints{"totalPoints"};
    Persistent<int> dartsFired{"dartsFired"};
    Persistent<int> bestScore{"bestScore"};
    Persistent<int> batsKilled{"batsKilled"};
    Persistent<float> accuracy{"accuracy"};
    Persistent<int> hits{"hits"};
};

Controller::Controller() : m{new Controller::Members{}} {
    m->shadeProgram = std::make_unique<NormalShadeProgram>();

    std::vector<NormalShadeVertex> flamebox;
    m->flame = std::make_unique<Sprite>(characters.flame, ShaderUtil::AdderPTC(flamebox));
    for (int i = 0; i <= DART_TRAIL_SEGMENTS; ++i) {
        // Convert a single-segment triangle strip into multiple segments.
        // 0 2    0 2 4 ... 32
        // |/| => |/|/|    /
        // 1 3    1 3 5 ... 33
        auto top_pos = mix(flamebox[0].position, flamebox[2].position, i * (1/float(DART_TRAIL_SEGMENTS)));
        auto bot_pos = mix(flamebox[1].position, flamebox[3].position, i * (1/float(DART_TRAIL_SEGMENTS)));
        auto top_tc  = mix(flamebox[0].texcoord, flamebox[2].texcoord, i * (1/float(DART_TRAIL_SEGMENTS)));
        auto bot_tc  = mix(flamebox[1].texcoord, flamebox[3].texcoord, i * (1/float(DART_TRAIL_SEGMENTS)));
        m->flamebuf.push_back({top_pos, top_tc});
        m->flamebuf.push_back({bot_pos, bot_tc});
    }

    newGame(MODE);
}

Controller::~Controller() { }

void Controller::newGame(GameMode mode) {
    m->newGame = false;
    m->game = std::make_shared<Game>(spaceTime(), mode, m->top);

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

        /*if (state.level_passed && state.level > *m->highestCompletedLevel) {
            brag::highlevel = state.level;
        }*/

        size_t score = state.score;
        if (score > 0) {
            if (score > *m->bestScore) {
                m->bestScore = static_cast<int>(score);
            }
            m->totalPoints = *m->totalPoints + static_cast<int>(score);

            /*if (score >= 25) {
                brag::score25(100, []{});
                if (score >= 100) {
                }
            }*/
            
            size_t tp = *m->totalPoints;
            brag::totalpoints = tp;
        }

        m->dartsFired = *m->dartsFired + state.playerStats.darts;
        m->batsKilled = *m->batsKilled + state.playerStats.kills;
        m->hits = *m->hits + state.playerStats.hits;
        m->accuracy = (float(*m->hits) / float(*m->dartsFired)) * 100;
        brag::dartsfired = *m->dartsFired;
        brag::batskilled = *m->batsKilled;
        brag::accuracy = *m->accuracy;

        auto gameOver = emplaceController<GameOver>(mode, score, *m->bestScore, state.playerStats, state.characterStats);
        gameOver->back      ->clicked += newGame(m_menu);
        gameOver->restart   ->clicked += newGame(m->mode);
        gameOver->backf     ->clicked += newGame(m_menu);
        gameOver->restartf  ->clicked += newGame(m->mode);
    };

    m->game->show_menu += [=] {
        auto menu = emplaceController<Menu>();

        menu->play->clicked += newGame(m_play);

        menu->gamecenter->clicked += [=]{
            click();
            presentBragUI();
        };
        menu->twitter->clicked += [=]{
            click();
            UrlOpener::open("http://www.twitter.com/UnwiseGames");
        };
    };

    state.back_btn->clicked += [=] {
        click();
        m->newGame = true;
        m->mode = m_menu;
    };

    state.pause_btn->clicked += [=] {
        m->paused = !m->paused;
    };

    state.play_btn->clicked += [=] {
        m->paused = false;
    };
}

bool Controller::onUpdate(float dt) {
    if (!m->paused) {
        if (!m->game->update(dt)) {
            //newGame();
        }


        if (m->newGame) {
            newGame(m->mode);
            return false;
        }
        m->angle += dt;
    }
    return true;
}

bool Controller::wantTimerUpdate() const {
    // Change this to freeze all timers.
    return !m->paused;
}

void Controller::onDraw() {
    auto const & state = m->game->state();

    auto sprite_context = AutoSprite<SpriteProgram>::context();
    sprite_context->tint = {1, 1, 1, 1};
    sprite_context->alpha = 1;

    SpriteProgram::draw(background.bg, pmv() * mat4::translate({0, 9.1, 0}));

    for (auto const & b : m->game->actors<Bird>()) {
        if (b.isFlying()) {
            SpriteProgram::drawActor(b, pmv(), b.velocity().x < 0 ? mat4::scale(vec3{-1, 1, 1}) : mat4::identity());
        } else {
            SpriteProgram::drawActor(b, pmv());
        }
    }

    for (auto const & c : m->game->actors<Character>()) {
        if (c.isAiming()) {
            float dt = 0.01;
            vec2 p = c.position() - vec2::polar(LAUNCH_OFFSET, c.angle() + M_PI_2);
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
    for (auto const & b : m->game->actors<Bomb>()) {
        if (b.countdown > 0) {
            SpriteProgram::drawText(std::to_string(b.countdown), font.glyphs, 0, pmv() * mat4::translate({b.position().x, b.position().y - float(0.2), 0}) * mat4::scale(0.25));
        }
    }
    SpriteProgram::draw(m->game->actors<BombBat>                (), pmv());
    SpriteProgram::draw(m->game->actors<Blast>                  (), pmv());
    SpriteProgram::draw(m->game->actors<CharacterExplosion>     (), pmv());
    SpriteProgram::draw(m->game->actors<Grave>                  (), pmv());
    SpriteProgram::draw(m->game->actors<Character>              (), pmv());

    for (auto const & d : m->game->actors<Dart>()) {
        if (d.active) {
            SpriteProgram::drawText(std::to_string(d.score), font.glyphs, 0, pmv() * mat4::translate({d.position().x, d.position().y + float(0.6), 0}) * mat4::scale(0.3), -0.1);

            std::vector<NormalShadeVertex> buf;
            auto ctx = m->shadeProgram->context();

            ctx->tint = {1, clamp(0.6f - d.dt * 0.2f, 0, 0.6f), 0};
            ctx->texture = m->flame->activateTexture();
            ctx->pmv = pmv();

            size_t ii = 0;
            std::array<vec2, DART_TRAIL_SEGMENTS + 1> tmp = d.trail;
            for (int i = 0; i <= DART_TRAIL_SEGMENTS; ++i) {
                vec2 p = tmp[i];
                vec2 q = tmp[i + 1];
                vec2 n = cross(unit(q - p));
                buf.push_back({p + 0.13 * n, m->flamebuf[ii].texcoord});
                buf.push_back({p - 0.13 * n, m->flamebuf[ii + 1].texcoord});
                ii = ii + 2;
            }

            ctx.vs.enableArray(buf.data());
            glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(buf.size()));
        }
        SpriteProgram::drawActor(d, pmv());
    }

    if (state.mode == m_play) {
        SpriteProgram::drawText("SCORE :  " + std::to_string(state.score), font.glyphs, 1,
                                pmv() * mat4::translate({9, m->top-1, 0}) * mat4::scale(0.5), -0.1);

        state.back_btn->draw(pmv());
        state.pause_btn->draw(pmv());

        SpriteProgram::drawText(std::to_string(state.grey_bats_killed), font.glyphs, -1, pmv() * mat4::translate({-2, m->top-1, 0}) * mat4::scale(0.5));
        SpriteProgram::draw(atlas.bathead, pmv() * mat4::translate({-2.6, m->top - 0.7f, 0}) * mat4::scale(0.8));

        SpriteProgram::drawText(std::to_string(state.yellow_bats_killed), font.glyphs, -1, pmv() * mat4::translate({1.2, m->top-1, 0}) * mat4::scale(0.5));
        SpriteProgram::draw(atlas.yellowbathead, pmv() * mat4::translate({0.6, m->top - 0.7f, 0}) * mat4::scale(0.8));

        if (m->game->state().show_char_score) {
            for (auto const & c : m->game->actors<Character>()) {
                if (!c.isDead()) {
                    SpriteProgram::drawText(std::to_string(SCORE_CHAR_SURVIVED), font.glyphs, 0, pmv() * mat4::translate({c.position().x, c.position().y + float(0.8), 0}) * mat4::scale(0.3), -0.1);
                }
            }
        }

        for (auto const & a : state.alerts) {
            sprite_context->alpha = a.alpha;
            SpriteProgram::drawText(a.s, font.glyphs, 0, pmv() * mat4::translate({a.pos.x, a.pos.y, 0}) * mat4::scale(a.scale), -0.1);
            sprite_context->alpha = 1;
        }

        if (m->paused) {
            SpriteProgram::draw(atlas.fade, pmv());
            SpriteProgram::drawText("PAUSED", font.glyphs, 0, pmv() * mat4::translate({0, 9, 0}), -0.1);
            state.play_btn->draw(pmv());
        }
    }

    /*auto ctx = m->shadeProgram->context();
    ctx->tint = {1, 244/255.0f, 240/255.0f};
    ctx->texture = m->flame->activateTexture();
    ctx->pmv = pmv() * mat4::translate({6, 4, 0});

    for (int i = 0; i <= 16; ++i) {
        vec2 p{0.125f * i, 0.02f * i * (16 - i)};
        auto j = i + 1;
        vec2 q{0.125f * j, 0.02f * j * (16 - j)};
        vec2 n = cross(unit(q - p));
        m->flamebuf[2 * i].position = p + 0.1 * n;
        m->flamebuf[2 * i + 1].position = p - 0.1 * n;
    }

    ctx.vs.enableArray(m->flamebuf.data());
    glDrawArrays(GL_TRIANGLE_STRIP, 0, static_cast<GLsizei>(m->flamebuf.size()));*/
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

TouchHandler Controller::onTouch(vec2 const & worldPos, float radius) {
    return m->game->fingerTouch(worldPos, radius);
}
