#include "Game.h"
#include "bats.sprites.h"
#include "characters.sprites.h"
#include "reload.sprites.h"

#include <bricabrac/Game/GameActorImpl.h>
#include <bricabrac/Game/Timer.h>
#include <bricabrac/Math/MathUtil.h>
#include <bricabrac/Math/Random.h>
#include <bricabrac/Logging/Logging.h>

using namespace brac;

enum Group : cpGroup { gr_bird = 1 };

enum Layer : cpLayers { l_all = 1<<0, l_character = 1<<1 };

enum CollisionType : cpCollisionType { ct_universe = 1 };

struct CharacterImpl : BodyShapes<Character> {
    vec2 launchVel_ = {0, 0};

    CharacterImpl(cpSpace * space, int type, vec2 const & pos)
    : BodyShapes{space, newBody(1, INFINITY, pos), characters.characters[type][0], CP_NO_GROUP, l_all | l_character}
    {
        for (auto & shape : shapes()) cpShapeSetElasticity(&*shape, 1);
    }

    virtual bool isAiming() const override {
        return !!launchVel_;
    }

    virtual vec2 const & launchVel() const override {
        return launchVel_;
    }

    void aim(vec2 const & v) {
        launchVel_ = v;
        setAngle(brac::angle(v));
    }

    void dontAim() {
        launchVel_ = {0, 0};
        setAngle(0);
    }
};

struct BirdImpl : BodyShapes<Bird> {
    BirdImpl(cpSpace * space, int type, vec2 const & pos, vec2 const & vel)
    : BodyShapes{space, newBody(1, 1, pos), bats.bats[type], gr_bird, l_all}
    {
        setVel(vel);
    }
};

struct DartImpl : BodyShapes<Dart> {
    DartImpl(cpSpace * space, vec2 const & pos, vec2 const & vel)
    : BodyShapes{space, newBody(1, 1, pos), characters.dart, CP_NO_GROUP, l_all}
    {
        setVel(vel);
        setForce({0, WORLD_GRAVITY});
    }

    virtual void doUpdate(float) override {
        setAngle(::brac::angle(vel()));
    }
};

struct ReloadImpl : BodyShapes<Reload> {
    ReloadImpl(cpSpace * space, vec2 const & pos)
    : BodyShapes{space, newStaticBody(pos), sensor(reload.reload)}
    { }
};

struct Game::Members : Game::State, GameImpl<CharacterImpl, BirdImpl, DartImpl, ReloadImpl> {
    ShapePtr worldBox{sensor(boxShape(30, 30, {0, 0}, 0), ct_universe)};
    ShapePtr walls[3], hoop[2];
    size_t n_for_n = 0;
    bool touched_sides = false;
    int bounced_walls = 0;
    size_t score_modifier = 0;
    Ticker tick;

    Members(SpaceTime & st) : Impl{st} { }
};

Game::Game(SpaceTime & st, GameMode mode, float top) : GameBase{st}, m{new Members{st}} {
    m->mode = mode;

    if (mode == m_menu) {
        delay(0, [=]{ show_menu(); }).cancel(destroyed);
    } else {
        m->back->setY(top - 0.8);
        m->restart->setY(top - 0.8);

        m->tick = {3, [=]{
            m->emplace<BirdImpl>(0, vec2{-4, 10}, vec2{1, -1});
        }};

        m->emplace<CharacterImpl>(0, vec2{-4, 2});
        m->emplace<CharacterImpl>(1, vec2{3, 2});

        m->emplace<ReloadImpl>(vec2{0, 2.6});
    }

    m->onCollision([=](CharacterImpl & character, BirdImpl & bird) {
        vec2 v = bird.vel();
        v.y = -v.y;
        bird.setVel(v);
        character.setVel(v);
        for (auto & shape : character.shapes()) cpShapeSetGroup(&*shape, gr_bird);
    });

    m->onCollision([=](DartImpl &, BirdImpl &, cpArbiter * arb) {
        if(cpArbiterIsFirstContact(arb)) {
            m->score += 10;
        }
    });
}

Game::~Game() { }

Game::State const & Game::state() const { return *m; }

std::unique_ptr<TouchHandler> Game::fingerTouch(vec2 const & p, float radius) {
    if (auto backHandler = m->back->handleTouch(p)) {
        return backHandler;
    }
    if (auto restartHandler = m->restart->handleTouch(p)) {
        return restartHandler;
    }

    CharacterImpl * character = nullptr;
    float dist_sq = INFINITY;
    AabbQuery(&m->spaceTime, cpBBNewForCircle(to_cpVect(p), radius), l_character, CP_NO_GROUP,
              [&](cpShape *shape) {
                  if (auto c = static_cast<CharacterImpl *>(cpShapeGetUserData(shape))) {
                      float dsq = length_sq(p - c->pos());
                      if (dist_sq > dsq) {
                          dist_sq = dsq;
                          character = c;
                      }
                  }
              });

    if (character) {
        struct CharacterAimAndFireHandler : TouchHandler {
            std::weak_ptr<Game> weak_self;
            CharacterImpl * character;
            vec2 first_p, vel;

            CharacterAimAndFireHandler(Game & self, vec2 const & p, CharacterImpl * character)
            : weak_self{self.shared_from_this()}
            , character{character}
            , first_p{p}
            { }

            ~CharacterAimAndFireHandler() {
                if (auto self = weak_self.lock()) {
                    // TODO: Return smoothly to upright posture.
                    if (vec2 const & vel = character->launchVel()) {
                        self->m->emplace<DartImpl>(character->pos() + LAUNCH_OFFSET * unit(vel), vel);
                        character->dontAim();
                    }
                }
            }

            virtual void moved(vec2 const & p, bool) {
                if (auto self = weak_self.lock()) {
                    auto v = first_p - p;
                    if (float s = length(v)) {
                        character->aim((14 + 2 * s) / s * v);
                    }
                }
            }
        };
        
        return std::unique_ptr<TouchHandler>{new CharacterAimAndFireHandler{*this, p, character}};
    }

    return {};
}

void Game::doUpdate(float dt) { m->update(dt); }

void Game::getActors(size_t actorId, void * buf) const { m->getActorsForController(actorId, buf); }
