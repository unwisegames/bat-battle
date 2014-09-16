#include "Game.h"
#include "bats.sprites.h"
#include "characters.sprites.h"

#include <bricabrac/Game/GameActorImpl.h>
#include <bricabrac/Game/Timer.h>
#include <bricabrac/Math/MathUtil.h>
#include <bricabrac/Math/Random.h>
#include <bricabrac/Logging/Logging.h>

using namespace brac;

enum Group : cpGroup { gr_platform = 1, gr_birds };

enum Layer : cpLayers { l_play = 1<<0, l_fixtures = 1<<1 };

enum CollisionType : cpCollisionType { ct_universe = 1 };

struct CharacterImpl : BodyShapes<Character> {
    CharacterImpl(cpSpace * space, int type, vec2 const & pos)
    : BodyShapes{space, newBody(1, INFINITY, pos), characters.characters[type][0], CP_NO_GROUP, l_play | l_fixtures}
    {
        for (auto & shape : shapes()) cpShapeSetElasticity(&*shape, 1);
    }

    void drop() {
        setForce({0, 0});
    }
};

struct BirdImpl : BodyShapes<Bird> {
    BirdImpl(cpSpace * space, int type, vec2 const & pos, vec2 const & vel)
    : BodyShapes{space, newBody(1, 1, pos), bats.bats[type], gr_birds}
    {
        setVel(vel);
    }
};

struct Game::Members : Game::State, GameImpl<CharacterImpl, BirdImpl> {
    ShapePtr worldBox{sensor(boxShape(30, 30, {0, 0}, 0), ct_universe)};
    ShapePtr walls[3], hoop[2];
    size_t n_for_n = 0;
    bool touched_sides = false;
    int bounced_walls = 0;
    size_t score_modifier = 0;
    Ticker tick;

    Members(SpaceTime & st) : Impl{st} { }
};

Game::Game(SpaceTime & st, GameMode mode) : GameBase{st}, m{new Members{st}} {
    m->mode = mode;

    if (mode == m_menu) {
        delay(0, [=]{ show_menu(); }).cancel(destroyed);
    } else {
        m->tick = {3, [=]{
            m->emplace<BirdImpl>(0, vec2{-4, 10}, vec2{1, -1});
        }};

        m->emplace<CharacterImpl>(0, vec2{-4, 2});
        m->emplace<CharacterImpl>(1, vec2{3, 2});
    }

    m->onCollision([=](CharacterImpl & character, BirdImpl & bird) {
        vec2 v = bird.vel();
        v.y = -v.y;
        bird.setVel(v);
        character.setVel(v);
        for (auto & shape : character.shapes()) cpShapeSetGroup(&*shape, gr_birds);
    });
}

Game::~Game() { }

Game::State const & Game::state() const { return *m; }

std::unique_ptr<TouchHandler> Game::fingerTouch(vec2 const & p, float radius) {
    return {};
}

void Game::doUpdate(float dt) { m->update(dt); }

void Game::getActors(size_t actorId, void * buf) const { m->getActorsForController(actorId, buf); }
