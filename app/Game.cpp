#include "Game.h"

#include <bricabrac/Game/GameActorImpl.h>
#include <bricabrac/Game/Timer.h>
#include <bricabrac/Math/MathUtil.h>
#include <bricabrac/Math/Random.h>
#include <bricabrac/Logging/Logging.h>

using namespace brac;

enum Group : cpGroup { gr_platform = 1, gr_ring };

enum Layer : cpLayers { l_play = 1<<0, l_fixtures = 1<<1 };

enum CollisionType : cpCollisionType { ct_universe = 1, ct_sides, ct_dunk, ct_wall };

struct CharacterImpl : BodyShapes<Character> {
    CharacterImpl(int personality, vec2 const & pos)
    : BodyShapes{newBody(1, INFINITY, pos), newBoxShape(1, 1), CP_NO_GROUP, l_play | l_fixtures}
    {
        for (auto & shape : shapes()) cpShapeSetElasticity(&*shape, 1);
    }

    void drop() {
        setForce({0, 0});
    }
};

struct BirdImpl : BodyShapes<Bird> {

};

struct Game::Members : Game::State, GameImpl<CharacterImpl> {
    ShapePtr worldBox{sensor(boxShape(30, 30, {0, 0}, 0), ct_universe)};
    ShapePtr walls[3], hoop[2];
    ShapePtr dunk{sensor(segmentShape({-1, 6}, {1, 6}), ct_dunk)};
    size_t n_for_n = 0;
    bool touched_sides = false;
    int bounced_walls = 0;
    size_t score_modifier = 0;
    std::unique_ptr<Ticker> tick;
    std::unique_ptr<CancelTimer> hoop_timer;
};

Game::Game(GameMode mode) : m{new Members} {
    m->mode = mode;
}

Game::~Game() { }

Game::State const & Game::state() const { return *m; }

std::unique_ptr<TouchHandler> Game::fingerTouch(vec2 const & p, float radius) {
    return {};
}

void Game::doUpdate(float dt) { m->update(dt); }

void Game::getActors(size_t actorId, void * buf) const { m->getActorsForController(actorId, buf); }
