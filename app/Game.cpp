#include "Game.h"
#include "bats.sprites.h"

#include <bricabrac/Game/GameActorImpl.h>
#include <bricabrac/Game/Timer.h>
#include <bricabrac/Math/MathUtil.h>
#include <bricabrac/Math/Random.h>
#include <bricabrac/Logging/Logging.h>

constexpr float GRAVITY = -30;

using namespace brac;

enum Group : cpGroup { gr_platform = 1, gr_ring };

enum Layer : cpLayers { l_play = 1<<0, l_fixtures = 1<<1 };

enum CollisionType : cpCollisionType { ct_universe = 1, ct_sides, ct_dunk, ct_wall };

struct CharacterImpl : BodyShapes<Character> {
    CharacterImpl(cpSpace * space, int personality, vec2 const & pos)
    : BodyShapes{space, newBody(1, INFINITY, pos), newBoxShape(1, 1), CP_NO_GROUP, l_play | l_fixtures}
    {
        for (auto & shape : shapes()) cpShapeSetElasticity(&*shape, 1);
    }

    void drop() {
        setForce({0, 0});
    }
};

enum class BirdType { grey, yellow };

struct BirdImpl : BodyShapes<Bird> {
    BirdType type_;

    BirdImpl(cpSpace * space, BirdType type, vec2 const & pos)
    : BodyShapes{space, newStaticBody(pos), sensor(type == BirdType::grey ? bats.greybat : bats.yellowbat)}
    {

    }
};

struct Game::Members : Game::State, GameImpl<CharacterImpl, BirdImpl> {
    ShapePtr worldBox{sensor(boxShape(30, 30, {0, 0}, 0), ct_universe)};
    ShapePtr walls[3], hoop[2];
    ShapePtr dunk{sensor(segmentShape({-1, 6}, {1, 6}), ct_dunk)};
    size_t n_for_n = 0;
    bool touched_sides = false;
    int bounced_walls = 0;
    size_t score_modifier = 0;
    std::unique_ptr<Ticker> tick;
    std::unique_ptr<CancelTimer> hoop_timer;

    Members(SpaceTime & st) : Impl{st} { }
};

Game::Game(SpaceTime & st, GameMode mode) : GameBase{st}, m{new Members{st}} {
    m->mode = mode;

    if (mode == m_menu)
    {
        delay(0, [=]{ show_menu(); }).cancel(destroyed);
    }
    else
    {
        m->setGravity({0, GRAVITY});

        m->emplace<BirdImpl>(BirdType::grey, vec2{-4, 10});
        m->emplace<BirdImpl>(BirdType::yellow, vec2{2, 9});
    }
}

Game::~Game() { }

Game::State const & Game::state() const { return *m; }

std::unique_ptr<TouchHandler> Game::fingerTouch(vec2 const & p, float radius) {
    return {};
}

void Game::doUpdate(float dt) { m->update(dt); }

void Game::getActors(size_t actorId, void * buf) const { m->getActorsForController(actorId, buf); }
